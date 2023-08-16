#include "channel.hpp"

#include <string>
#include <thread>
#include <chrono>

#include <coreinit/memory.h>
#include <coreinit/ios.h>

#include <mocha/mocha.h>

#include <libs/tinyxml2.h>

#include <command/Log.hpp>
#include <utility/platform.hpp>
#include <utility/string.hpp>
#include <utility/file.hpp>
#include <nuspack/packer.hpp>


#define MCP_COMMAND_INSTALL_ASYNC   0x81
#define MAX_INSTALL_PATH_LENGTH     0x27F


extern "C" MCPError MCP_GetLastRawError(void);

static const std::filesystem::path SD_ROOT_PATH = "fs:/vol/external01";
static const std::filesystem::path SD_ROOT_PATH_MCP = "/vol/app_sd";
static const std::filesystem::path CHANNEL_DATA_PATH = "temp/data";
static const std::filesystem::path CHANNEL_OUTPUT_PATH = "temp/install";


MCPError getTitlePath(const uint64_t& titleID, std::filesystem::path& outPath) {
    const int32_t handle = MCP_Open();

    MCPTitleListType info;
    if(const MCPError err = MCP_GetTitleInfo(handle, titleID, &info); err < 0) {
        Utility::platformLog("MCP_GetTitleInfo encountered error %08X\n", err);
        Utility::platformLog("With title ID 0x%016X\n", titleID);
        return err;
    }
    
    outPath = info.path;
    return 0;
}

static int installCompleted = 0;
static uint32_t installError = 0;

static int IosInstallCallback(unsigned int errorCode, unsigned int * priv_data)
{
    installError = errorCode;
    installCompleted = 1;
    return 0;
}

//based on https://github.com/Fangal-Airbag/wup-installer-gx2/blob/Wuhb/src/menu/InstallWindow.cpp#L169
static bool installFreeChannel(const std::filesystem::path& relPath, const MCPInstallTarget& loc) {
    using namespace std::literals::chrono_literals;
    
    if(!Utility::dirExists(SD_ROOT_PATH / relPath)) {
        ErrorLog::getInstance().log("Channel data path is not a directory!");
        return false;
    }

    Utility::platformLog("Installing channel...\n");

    const std::filesystem::path path = SD_ROOT_PATH_MCP / relPath;
    
    int result = 0;
    installCompleted = 0;
    installError = 0;

    const int32_t mcpHandle = MCP_Open();
    if(mcpHandle == 0)
    {
        ErrorLog::getInstance().log("Failed to open MCP for install");
        
        result = -1;
    }
    else
    {
        char installPath[256];
        unsigned int* mcpInstallInfo = (unsigned int *)OSAllocFromSystem(0x24, 0x40);
        char* mcpInstallPath = (char *)OSAllocFromSystem(MAX_INSTALL_PATH_LENGTH, 0x40);
        IOSVec* mcpPathInfoVector = (IOSVec *)OSAllocFromSystem(0x0C, 0x40);
        
        do
        {
            if(!mcpInstallInfo || !mcpInstallPath || !mcpPathInfoVector)
            {
                ErrorLog::getInstance().log("Could not allocate memory for install");
                
                result = -2;
                break;
            }
            
            snprintf(installPath, sizeof(installPath), "%s", path.string().c_str());
            
            int res = MCP_InstallGetInfo(mcpHandle, installPath, (MCPInstallInfo*)mcpInstallInfo);
            if(res != 0)
            {
                ErrorLog::getInstance().log("Could not find complete WUP files in the folder.");
                
                result = -3;
                break;
            }

            const uint32_t titleIdHigh = mcpInstallInfo[0];

            if (  (titleIdHigh == 0x0005000E)     // game update
               || (titleIdHigh == 0x00050000)     // game
               || (titleIdHigh == 0x0005000C)     // DLC
               || (titleIdHigh == 0x00050002))    // Demo
            {
                res = MCP_InstallSetTargetDevice(mcpHandle, loc);
                if(res != 0)
                {
                    ErrorLog::getInstance().log("MCP_InstallSetTargetDevice " + Utility::Str::intToHex(MCP_GetLastRawError(), 8, true));
                    
                    result = -5;
                    break;
                }
                res = MCP_InstallSetTargetUsb(mcpHandle, loc);
                if(res != 0)
                {
                    ErrorLog::getInstance().log("MCP_InstallSetTargetUsb " + Utility::Str::intToHex(MCP_GetLastRawError(), 8, true));
                    
                    result = -6;
                    break;
                }
                
                mcpInstallInfo[2] = (unsigned int)MCP_COMMAND_INSTALL_ASYNC;
                mcpInstallInfo[3] = (unsigned int)mcpPathInfoVector;
                mcpInstallInfo[4] = (unsigned int)1;
                mcpInstallInfo[5] = (unsigned int)0;
                
                memset(mcpInstallPath, 0, MAX_INSTALL_PATH_LENGTH);
                snprintf(mcpInstallPath, MAX_INSTALL_PATH_LENGTH, path.string().c_str());
                memset(mcpPathInfoVector, 0, 0x0C);
                
                mcpPathInfoVector->vaddr = mcpInstallPath;
                mcpPathInfoVector->len = (unsigned int)MAX_INSTALL_PATH_LENGTH;
                
                res = IOS_IoctlvAsync(mcpHandle, MCP_COMMAND_INSTALL_ASYNC, 1, 0, mcpPathInfoVector, (IOSAsyncCallbackFn)IosInstallCallback, mcpInstallInfo);
                if(res != 0)
                {
                    ErrorLog::getInstance().log("MCP_InstallTitleAsync " + Utility::Str::intToHex(MCP_GetLastRawError(), 8, true));
                    result = -7;
                    break;
                }
                
                while(!installCompleted)
                {
                    memset(mcpInstallInfo, 0, 0x24);
                    
                    MCP_InstallGetProgress(mcpHandle, (MCPInstallProgress*)mcpInstallInfo);
                    
                    if(mcpInstallInfo[0] == 1)
                    {
                        uint64_t totalSize = ((uint64_t)mcpInstallInfo[3] << 32ULL) | mcpInstallInfo[4];
                        uint64_t installedSize = ((uint64_t)mcpInstallInfo[5] << 32ULL) | mcpInstallInfo[6];
                        int percent = (totalSize != 0) ? ((installedSize * 100.0f) / totalSize) : 0;
                        
                        //std::string message = fmt("%0.1f / %0.1f MB (%i", installedSize / (1024.0f * 1024.0f), totalSize / (1024.0f * 1024.0f), percent);
                        //message += "%)";
                        //
                        //messageBox->setProgress(percent);
                        //messageBox->setProgressBarInfo(message);
                        Utility::platformLog("Channel " + std::to_string(percent) + "%% installed");
                    }
                    
                    std::this_thread::sleep_for(500ms);
                }
                
                if(installError != 0)
                {
                    if((installError == 0xFFFCFFE9) && (loc == MCPInstallTarget::MCP_INSTALL_TARGET_USB)) {
                        ErrorLog::getInstance().log(Utility::Str::intToHex(installError, 8, true) + " access failed (no USB storage attached?)");
                    }
                    if (installError == 0xFFFBF446 || installError == 0xFFFBF43F) {
                        ErrorLog::getInstance().log("Possible missing or bad title.tik file");
                    }
                    else if (installError == 0xFFFBF441) {
                        ErrorLog::getInstance().log("Possible incorrect console for DLC title.tik file");
                    }
                    else if (installError == 0xFFFCFFE4) {
                        ErrorLog::getInstance().log("Possible not enough memory on target device");
                    }
                    else if (installError == 0xFFFFF825) {
                        ErrorLog::getInstance().log("Possible bad SD card.  Reformat (32k blocks) or replace");
                    }
                    else if ((installError & 0xFFFF0000) == 0xFFFB0000) {
                        ErrorLog::getInstance().log("Verify WUP files are correct & complete. DLC/E-shop require Sig Patch");
                    }

                    result = -9;
                }
            }
            else
            {
                ErrorLog::getInstance().log("Not a game, game update, DLC, demo or version title");
                result = -4;
            }
        } while(0);
        
        MCP_Close(mcpHandle);
        if(mcpPathInfoVector) OSFreeToSystem(mcpPathInfoVector);
        if(mcpInstallPath) OSFreeToSystem(mcpInstallPath);
        if(mcpInstallInfo) OSFreeToSystem(mcpInstallInfo);
    }
    
    if(result >= 0) {
        return true;
    }
    else {
        return false;
    }
}

static const std::filesystem::path DataPath = SD_ROOT_PATH / CHANNEL_DATA_PATH;

static bool packFreeChannel(const std::filesystem::path& baseDir) {
    Utility::platformLog("Packing channel...\n");

    //create necessary folders
    Utility::platformLog("Creating output folder at " + DataPath.string() + '\n');
    Utility::create_directories(DataPath);

    //copy over channel data
    Utility::platformLog("Copying channel data\n");
    Utility::copy(baseDir / "code", DataPath / "code");
    std::filesystem::remove(DataPath / "code/title.fst"); //would cause nullptr issues when packing
    std::filesystem::remove(DataPath / "code/title.tmd"); //would cause nullptr issues when packing

    Utility::create_directories(DataPath / "content"); //don't encrypt all the data, we can copy it over much faster
    std::ofstream data(DataPath / "content" / "filler.txt", std::ios::binary);
    const std::string str("This is filler data so things don't break!");
    data.write(&str[0], str.size());
    data.close();

    Utility::copy(baseDir / "meta", DataPath / "meta");

    //change the title ID so it gets its own channel
    Utility::platformLog("Modifying XMLs\n");
    tinyxml2::XMLPrinter printer;

    tinyxml2::XMLDocument meta;
    meta.LoadFile((DataPath / "meta" / "meta.xml").string().c_str());

    tinyxml2::XMLElement* metaRoot = meta.RootElement();
    metaRoot->FirstChildElement("title_id")->SetText("0005000010143599");
    
    meta.Print(&printer);
    std::ofstream metaOut(DataPath / "meta" / "meta.xml", std::ios::binary);
    std::string metaStr(printer.CStr());
    metaOut.write(&metaStr[0], metaStr.size());
    metaOut.close();
    printer.ClearBuffer();

    tinyxml2::XMLDocument app;
    app.LoadFile((DataPath / "code" / "app.xml").string().c_str());

    tinyxml2::XMLElement* appRoot = app.RootElement();
    appRoot->FirstChildElement("title_id")->SetText("0005000010143599");
    
    app.Print(&printer);
    std::ofstream appOut(DataPath / "code" / "app.xml", std::ios::binary);
    std::string appStr(printer.CStr());
    appOut.write(&appStr[0], appStr.size());
    appOut.close();
    
    //get common key
    Utility::platformLog("Getting keys\n");
    WiiUConsoleOTP otp;
    Mocha_ReadOTP(&otp);

    Key commonKey;
    std::copy(otp.wiiUBank.wiiUCommonKey, otp.wiiUBank.wiiUCommonKey + 0x10, commonKey.begin());

    //pack the channel
    Utility::platformLog("Creating package\n");
    if(createPackage(DataPath, SD_ROOT_PATH / CHANNEL_OUTPUT_PATH, defaultEncryptionKey, commonKey) != PackError::NONE)
    {
        ErrorLog::getInstance().log("Failed to create console package");
        return false;
    }

    return true;
}

bool createOutputChannel(const std::filesystem::path& baseDir, const MCPInstallTarget& loc) {
    Utility::platformLog("Creating output channel...\n");
    
    if(!packFreeChannel(baseDir)) return false;
    
    if(!installFreeChannel(CHANNEL_OUTPUT_PATH, loc)) return false;

    std::filesystem::path outPath;
    if(const auto& err = getTitlePath(0x0005000010143599, outPath); err < 0) return false;
    if(!Utility::mountDeviceAndConvertPath(outPath)) {
        ErrorLog::getInstance().log("Failed to mount output device");
        return false;
    }

    Utility::platformLog("Installed channel, copying game data...\n");
    std::filesystem::remove(outPath / "content/filler.txt");
    if(!Utility::copy(baseDir / "content", outPath / "content")) return false; //might try routing this through RandoSession later

    return true;
}
