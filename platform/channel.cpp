#include "channel.hpp"

#include <string>
#include <thread>
#include <chrono>
#include <filesystem>

#include <coreinit/ios.h>
#include <coreinit/filesystem_fsa.h>

#include <mocha/mocha.h>

#include <libs/tinyxml2.hpp>

#include <command/Log.hpp>
#include <utility/platform.hpp>
#include <utility/string.hpp>
#include <utility/file.hpp>
#include <nuspack/packer.hpp>


#define MCP_COMMAND_INSTALL_ASYNC   0x81


extern "C" MCPError MCP_GetLastRawError(void);

static const fspath SD_ROOT_PATH = "fs:/vol/external01";
static const fspath SD_ROOT_PATH_MCP = "/vol/app_sd";
static const fspath CHANNEL_DATA_PATH = "temp/data";
static const fspath CHANNEL_OUTPUT_PATH = "temp/install";


MCPError getTitlePath(const uint64_t& titleID, fspath& outPath) {
    const int32_t handle = MCP_Open();

    if(handle < 0) {
        ErrorLog::getInstance().log("MCP_Open encountered error " + Utility::Str::intToHex(handle, 8));
        return handle;
    }

    alignas(0x40) MCPTitleListType info;
    if(const MCPError err = MCP_GetTitleInfo(handle, titleID, &info); err < 0) {
        ErrorLog::getInstance().log("MCP_GetTitleInfo encountered error " + Utility::Str::intToHex(err, 8) + " with title ID " + Utility::Str::intToHex(titleID, 16));
        return err;
    }

    if(const MCPError err = MCP_Close(handle); err < 0) {
        ErrorLog::getInstance().log("MCP_Close encountered error " + Utility::Str::intToHex(err, 8));
        return err;
    }

    outPath = info.path;

    return 0;
}

bool checkEnoughFreeSpace(const MCPInstallTarget& device, const uint64_t& minSpace) {
    static const std::string deviceToPath[2] = {"/vol/storage_mlc01", "/vol/storage_usb01"};
    if(device < 0 || device > 1) {
        ErrorLog::getInstance().log("Invalid device for checkEnoughFreeSpace()!");
        return false;
    }
    const std::string& path = deviceToPath[device]; // 0 is TARGET_MLC, 1 is TARGET_USB

    const FSAClientHandle handle = FSAAddClient(NULL);
    if(handle < 0) {
        ErrorLog::getInstance().log("Failed to add FSA client, error " + Utility::Str::intToHex(handle, 8));
        return false;
    }

    uint64_t freeSize = 0;
    if(const FSError err = FSAGetFreeSpaceSize(handle, path.c_str(), &freeSize); err == FS_ERROR_STORAGE_FULL) {
        ErrorLog::getInstance().log("Storage device " + path + " is full!");
        return false;
    }
    else if(err != FS_ERROR_OK) {
        ErrorLog::getInstance().log("Failed to get free space size, error " + Utility::Str::intToHex(static_cast<int32_t>(err), 8));
        return false;
    }

    if(const FSError err = FSADelClient(handle); err != FS_ERROR_OK) {
        ErrorLog::getInstance().log("Failed to delete FSA client, error " + Utility::Str::intToHex(static_cast<int32_t>(err), 8));
        return false;
    }

    if(freeSize < minSpace) {
        Utility::platformLog(path + " does not have enough free space!");
        Utility::platformLog("Has " + std::to_string(freeSize) + " free, needs " + std::to_string(minSpace));
        ErrorLog::getInstance().log(path + " does not have enough free space! It needs " + std::to_string(minSpace) + " bytes but has " + std::to_string(freeSize) + "available!");
        return false;
    }

    return true;
}

static bool installCompleted = false;
static std::underlying_type_t<IOSError> installStatus = IOS_ERROR_OK;

static void IosInstallCB(IOSError status, void* context)
{
    installStatus = status;
    installCompleted = true;
}

// adapted from https://github.com/Fangal-Airbag/wup-installer-gx2/blob/aecb01d573d904e37dc067fbe450180938e41b0f/src/menu/InstallWindow.cpp#L169
static bool installFreeChannel(const fspath& relPath, const MCPInstallTarget& loc) {
    using namespace std::literals::chrono_literals;

    if(!Utility::dirExists(SD_ROOT_PATH / relPath)) {
        ErrorLog::getInstance().log("Channel data path is not a directory!");
        return false;
    }

    Utility::platformLog("Installing channel...");

    const fspath& path = SD_ROOT_PATH_MCP / relPath;

    installCompleted = false;
    installStatus = IOS_ERROR_OK;

    const int32_t mcpHandle = MCP_Open();
    if(mcpHandle < 0) {
        ErrorLog::getInstance().log("Failed to open MCP for install, error " + Utility::Str::intToHex(mcpHandle, 8));
        return false;
    }

    if(path.string().length() + 1 > FS_MAX_PATH) {
        ErrorLog::getInstance().log("Channel path \"" + relPath.string() + "\" is too long");
        MCP_Close(mcpHandle);

        return false;
    }

    alignas(0x40) char installPath[FS_MAX_PATH]{};
    const size_t count = path.string().copy(installPath, sizeof(installPath));
    if(count != path.string().length()) {
        ErrorLog::getInstance().log("Could not copy full channel path \"" + relPath.string() + "\" to buffer");
        MCP_Close(mcpHandle);

        return false;
    }
    installPath[count] = '\0';

    alignas(0x40) MCPInstallInfo installInfo{};
    if(const MCPError err = MCP_InstallGetInfo(mcpHandle, installPath, &installInfo); err != 0) {
        ErrorLog::getInstance().log("Could not find complete WUP files in the folder, error " + Utility::Str::intToHex(err, 8));
        MCP_Close(mcpHandle);

        return false;
    }

    const uint32_t titleType = reinterpret_cast<uint32_t*>(&installInfo)[0] & 0xF; // titleIdHigh & 0xF

    if (titleType != 0x0 && titleType != 0x2 && titleType != 0xC && titleType != 0xE) { // Game || Demo || DLC || Update
        ErrorLog::getInstance().log("Not a game, update, DLC, or demo title");
        MCP_Close(mcpHandle);

        return false;
    }

    if(const MCPError err = MCP_InstallSetTargetDevice(mcpHandle, loc); err != 0) {
        ErrorLog::getInstance().log("MCP_InstallSetTargetDevice " + Utility::Str::intToHex(MCP_GetLastRawError(), 8));
        MCP_Close(mcpHandle);

        return false;
    }

    if(const MCPError err = MCP_InstallSetTargetUsb(mcpHandle, loc); err != 0) {
        ErrorLog::getInstance().log("MCP_InstallSetTargetUsb " + Utility::Str::intToHex(MCP_GetLastRawError(), 8));
        MCP_Close(mcpHandle);

        return false;
    }

    alignas(0x40) IOSVec pathInfoVector[1]{};

    pathInfoVector[0].vaddr = installPath;
    pathInfoVector[0].len = FS_MAX_PATH;

    reinterpret_cast<uint32_t*>(&installInfo)[2] = MCP_COMMAND_INSTALL_ASYNC;
    reinterpret_cast<uint32_t*>(&installInfo)[3] = reinterpret_cast<uint32_t>(&pathInfoVector[0]);
    reinterpret_cast<uint32_t*>(&installInfo)[4] = 1;
    reinterpret_cast<uint32_t*>(&installInfo)[5] = 0;

    if(const MCPError err = IOS_IoctlvAsync(mcpHandle, MCP_COMMAND_INSTALL_ASYNC, 1, 0, pathInfoVector, IosInstallCB, &installInfo); err != 0) {
        ErrorLog::getInstance().log("MCP_InstallTitleAsync " + Utility::Str::intToHex(MCP_GetLastRawError(), 8));
        MCP_Close(mcpHandle);

        return false;
    }

    alignas(0x40) MCPInstallProgress progress{};

    while(!installCompleted) {
        MCP_InstallGetProgress(mcpHandle, &progress);

        if(progress.inProgress != 0 && progress.sizeTotal != 0)
        {
            const uint32_t percent = (progress.sizeProgress * 100.0f) / progress.sizeTotal;
            Utility::platformLog("Channel " + std::to_string(percent) + "% installed");
        }

        std::this_thread::sleep_for(500ms);
    }

    if(installStatus != IOS_ERROR_OK) {
        const uint32_t category = (~installStatus >> 16) & 0x3FF;
        const int16_t code = static_cast<int16_t>(installStatus);

        ErrorLog::getInstance().log("Install error: category " + Utility::Str::intToHex(category, 4) + ", code " + Utility::Str::intToHex(code, 4) + " (" + Utility::Str::intToHex(installStatus, 8) + ")");

        switch(category) {
            case 0x0: // Kernel
                switch(code) {
                    case -0x7DB:
                        ErrorLog::getInstance().log("Possible bad SD card. Reformat (32k blocks) or replace");
                        break;
                    default:
                        // Unknown codes
                        break;
                }

                break;
            case 0x3: // FSA
                switch(code) {
                    case -0x17:
                        if(loc == MCPInstallTarget::MCP_INSTALL_TARGET_USB) {
                            ErrorLog::getInstance().log("Storage access failed (no USB storage attached?)");
                        }

                        break;
                    case -0x1C:
                        ErrorLog::getInstance().log("Possible not enough memory on target device");
                        break;
                    default:
                        // Unknown codes
                        break;
                }

                break;
            case 0x4: // MCP
                switch(code) {
                    case -0xBBA:
                    case -0xBC1:
                        ErrorLog::getInstance().log("Possible missing or bad title.tik file");
                        break;
                    case -0xBBF:
                        ErrorLog::getInstance().log("Possible incorrect console for DLC title.tik file");
                        break;
                    default:
                        ErrorLog::getInstance().log("Signature patches may be missing or install files may be corrupted.");
                        break;
                }

                break;
            default:
                // Unknown codes
                break;
        }

        MCP_Close(mcpHandle);

        return false;
    }

    if(const MCPError err = MCP_Close(mcpHandle); err < 0) {
        ErrorLog::getInstance().log("MCP_Close encountered error " + Utility::Str::intToHex(err, 8));
        return false;
    }

    return true;
}

static const fspath DataPath = SD_ROOT_PATH / CHANNEL_DATA_PATH;

static bool packFreeChannel(const fspath& baseDir) {
    Utility::platformLog("Packing channel...");

    // create necessary folders
    Utility::platformLog("Creating data folder at " + DataPath.string());
    if(!Utility::create_directories(DataPath)) {
        ErrorLog::getInstance().log("Failed to create data folder " + DataPath.string());
        return false;
    }

    // copy over channel data
    Utility::platformLog("Copying channel data");
    if(!Utility::copy(baseDir / "code", DataPath / "code")) {
        ErrorLog::getInstance().log("Failed to copy code folder " + (baseDir / "code").string());
        return false;
    }
    std::filesystem::remove(DataPath / "code/title.fst"); // would cause nullptr issues when packing
    std::filesystem::remove(DataPath / "code/title.tmd"); // would cause nullptr issues when packing

    if(!Utility::create_directories(DataPath / "content")) { // don't encrypt all the data, we can copy it over much faster
        ErrorLog::getInstance().log("Failed to create content folder " + (DataPath / "content").string());
        return false;
    }
    std::ofstream data(DataPath / "content" / "filler.txt", std::ios::binary);
    if(!data.is_open()) {
        ErrorLog::getInstance().log("Failed to open filler content " + (DataPath / "content" / "filler.txt").string());
    }
    const std::string str("This is filler data so things don't break!");
    data.write(&str[0], str.size());
    data.close();

    if(!Utility::copy(baseDir / "meta", DataPath / "meta")) {
        ErrorLog::getInstance().log("Failed to copy meta folder " + (baseDir / "meta").string());
        return false;
    }

    // change the title ID so it gets its own channel
    Utility::platformLog("Modifying XMLs");
    tinyxml2::XMLPrinter printer;

    tinyxml2::XMLDocument meta;
    if(const tinyxml2::XMLError err = LoadXML(meta, DataPath / "meta" / "meta.xml"); err != tinyxml2::XMLError::XML_SUCCESS) {
        ErrorLog::getInstance().log("Could not parse " + (DataPath / "meta" / "meta.xml").string() + ", " + meta.ErrorStr());
        return false;
    }

    tinyxml2::XMLElement* metaRoot = meta.RootElement();
    metaRoot->FirstChildElement("title_id")->SetText("0005000010143599");
    
    meta.Print(&printer);
    std::ofstream metaOut(DataPath / "meta" / "meta.xml", std::ios::binary);
    std::string metaStr(printer.CStr());
    metaOut.write(&metaStr[0], metaStr.size());
    metaOut.close();
    printer.ClearBuffer();

    tinyxml2::XMLDocument app;
    if(const tinyxml2::XMLError err = LoadXML(app, DataPath / "code" / "app.xml"); err != tinyxml2::XMLError::XML_SUCCESS) {
        ErrorLog::getInstance().log("Could not parse " + (DataPath / "code" / "app.xml").string() + ", " + app.ErrorStr());
        return false;
    }

    tinyxml2::XMLElement* appRoot = app.RootElement();
    appRoot->FirstChildElement("title_id")->SetText("0005000010143599");
    
    app.Print(&printer);
    std::ofstream appOut(DataPath / "code" / "app.xml", std::ios::binary);
    std::string appStr(printer.CStr());
    appOut.write(&appStr[0], appStr.size());
    appOut.close();
    
    // get common key
    Utility::platformLog("Getting keys");
    WiiUConsoleOTP otp;
    if(MochaUtilsStatus status = Mocha_ReadOTP(&otp); status != MOCHA_RESULT_SUCCESS) {
        ErrorLog::getInstance().log(std::string("Mocha_ReadOTP() returned error ") + Mocha_GetStatusStr(status));
        return false;
    }

    Key commonKey;
    std::copy(otp.wiiUBank.wiiUCommonKey, otp.wiiUBank.wiiUCommonKey + 0x10, commonKey.begin());

    // pack the channel
    Utility::platformLog("Creating package");
    if(const PackError err = createPackage(DataPath, SD_ROOT_PATH / CHANNEL_OUTPUT_PATH, defaultEncryptionKey, commonKey); err != PackError::NONE)
    {
        ErrorLog::getInstance().log("Failed to create console package, error " + packErrorGetName(err));
        return false;
    }

    return true;
}

bool createOutputChannel(const fspath& baseDir, const MCPInstallTarget& loc) {
    Utility::platformLog("Creating output channel...");
    
    // Channel data needs a little under 2GB
    if(!checkEnoughFreeSpace(loc, 1024ULL * 1024 * 1024 * 2)) return false; // Unsigned literal to avoid overflow warning
    
    if(!packFreeChannel(baseDir)) return false;
    
    if(!installFreeChannel(CHANNEL_OUTPUT_PATH, loc)) return false;

    fspath outPath;
    if(const auto& err = getTitlePath(0x0005000010143599, outPath); err < 0) return false;
    if(!Utility::mountDeviceAndConvertPath(outPath)) {
        ErrorLog::getInstance().log("Failed to mount output device");
        return false;
    }

    // Game data gets copied through RandoSession as first time setup
    std::filesystem::remove(outPath / "content/filler.txt");

    return true;
}
