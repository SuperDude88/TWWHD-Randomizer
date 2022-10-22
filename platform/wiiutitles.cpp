#include "wiiutitles.hpp"

#include <filesystem>
#include <sys/types.h>
#include <sys/stat.h>
#include <map>
#include <functional>
#include <thread>
#include <chrono>
#include <algorithm>
#include <cstdio>
#include <cstring>
#include <fstream>

#include <utility/platform.hpp>
#include <utility/file.hpp>

bool fileExist(const char* path) {
    struct stat existStat;
    if (stat(path, &existStat) == 0 && S_ISREG(existStat.st_mode)) return true;
    return false;
}

bool dirExist(const char* path) {
    struct stat existStat;
    if (strlen(path) >= 2 && path[strlen(path) - 1] == ':') return true;
    if (stat(path, &existStat) == 0 && S_ISDIR(existStat.st_mode)) return true;
    return false;
}

std::string convertToPosixPath(const char* volPath) {
    std::string posixPath;

    // volPath has to start with /vol/
    if (std::strncmp("/vol/", volPath, 5) != 0) return "";

    // Get and append the mount path
    const char* drivePathEnd = strchr(volPath + 5, '/');
    if (drivePathEnd == nullptr) {
        // Return just the mount path
        posixPath.append(volPath + 5);
        posixPath.append(":");
    }
    else {
        // Return mount path + the path after it
        posixPath.append(volPath, 5, drivePathEnd - (volPath + 5));
        posixPath.append(":/");
        posixPath.append(drivePathEnd + 1);
    }
    return posixPath;
}

//LARGELY COPIED WHOLESALE FROM https://github.com/emiyl/dumpling/blob/master/source/app/titles.cpp
namespace Utility {
    titleLocation deviceToLocation(char* device) {
        if (memcmp(device, "odd", 3) == 0) return titleLocation::Disc;
        if (memcmp(device, "usb", 3) == 0) return titleLocation::USB;
        if (memcmp(device, "mlc", 3) == 0) return titleLocation::Nand;
        return titleLocation::Unknown;
    }

    bool readInfoFromXML(titleEntry& title, titlePart& part) {
        // Check if /meta/meta.xml file exists
        std::string metaPath(part.path);
        metaPath.append("/meta/meta.xml");
        if (!fileExist(metaPath.c_str())) {
            Utility::platformLog(
                "The meta.xml file doesn't seem to exist at the given path: %s\n", metaPath.c_str()
            );
            return false;
        }

        // Read meta.xml
        std::ifstream xmlFile(metaPath);
        std::string line;

        // Parse data from it using string matching
        bool foundShortTitle = !title.shortTitle.empty();
        bool foundProductCode = !title.productCode.empty();
        while (getline(xmlFile, line)) {
            if (!foundShortTitle && line.find("shortname_en") != std::string::npos) {
                title.shortTitle = line.substr(line.find(">") + 1, (line.find_last_of("<")) - (line.find_first_of(">") + 1));
                title.normalizedTitle = normalizeTitle(title.shortTitle);

                // Create generic names for games that have non-standard characters (like kanji)
                if (title.normalizedTitle.empty() && !title.productCode.empty()) {
                    title.shortTitle = std::string("Undisplayable Game Name [") + title.productCode + "]";
                    title.normalizedTitle = title.productCode;
                }
                else foundShortTitle = true;
            }
            else if (!foundProductCode && line.find("product_code") != std::string::npos) {
                title.productCode = line.substr(line.find(">") + 1, (line.find_last_of("<")) - (line.find_first_of(">") + 1));
                foundProductCode = true;
            }
            if (foundShortTitle && foundProductCode) {
                // Finish up information
                part.outputPath += title.normalizedTitle;
                return true;
            }
        }

        //Utility::platformLog("shortname_en = %s\n", title.shortTitle.c_str());
        return false;
    }

    bool getRawTitles(std::vector<MCPTitleListType>& rawTitlesOut) {
        //Utility::platformLog("Loading titles...\n");

        rawTitlesOut.clear();

        //Utility::platformLog("getting MCPHandle\n");
        // acquire MCP tunnel
        int32_t mcpHandle = MCP_Open();
        if (mcpHandle < 0) {
            Utility::platformLog("Failed to open MCP to read all the games and updates\n");
            return false;
        }

        //Utility::platformLog("got MCPHandle: %d\n", mcpHandle);
        // Get titles from MCP
        //Utility::platformLog("getting title count\n");
        int32_t titleCount = MCP_TitleCount(mcpHandle);
        if(titleCount < 0)
        {
            Utility::platformLog("Failed to retrieve title count\n");
            return false;
        }
        //Utility::platformLog("title count: %d\n", titleCount);
        uint32_t titleByteSize = titleCount * sizeof(struct MCPTitleListType);
        rawTitlesOut.resize(titleCount);

        uint32_t titlesListed = 0;
        //Utility::platformLog("getting title list, titleByteSize=%u\n", titleByteSize);
        MCP_TitleList(mcpHandle, &titlesListed, rawTitlesOut.data(), titleByteSize);

        //Utility::platformLog("title list acquired, count: %u\n", titlesListed);
        if(titlesListed != titleCount)
        {
            //Utility::platformLog("Warning: Got %d titles when %d were expected\n", titlesListed, titleCount);
        }

        MCP_Close(mcpHandle);
        rawTitlesOut.resize(titlesListed);

        return true;
    }

    bool loadDetailedTitles(std::vector<MCPTitleListType>& rawTitles, std::vector<titleEntry>& titlesOut) {
        //Utility::platformLog("Searching for games...\n");

        // Queue and group parts of each title
        std::map<uint32_t, std::vector<std::reference_wrapper<MCPTitleListType>>> sortedQueue;
        for (auto& title : rawTitles) {
            // always skip discs for our purposes
            if (deviceToLocation(title.indexedDevice) == titleLocation::Disc) continue;

            // Check if it's a supported app type - restrict this to basic later?
            if (isBase(title.appType) || isUpdate(title.appType) || isDLC(title.appType) || isSystemApp(title.appType)) {
                std::string posixPath = convertToPosixPath(title.path);
                if (!posixPath.empty() && dirExist(posixPath.c_str())) {
                    uint32_t gameId = (uint32_t)title.titleId & 0x00000000FFFFFFFF;
                    sortedQueue[gameId].emplace_back(std::ref(title));
                }
                else {
                    Utility::platformLog("Couldn't convert the path or find this folder: %s\n", posixPath.c_str());
                    std::this_thread::sleep_for(std::chrono::seconds(1));
                }
            }
        }

        // Parse meta files to get name, title etc.
        for (auto& sortedTitle : sortedQueue) {
            titlesOut.emplace_back(titleEntry{});
            titleEntry& title = titlesOut.back();
            title.titleLowID = sortedTitle.first;
            title.base = titlePart{};
            title.update = titlePart{};
            title.dlc = titlePart{};

            // Loop over each part of a title
            for (MCPTitleListType& part : sortedTitle.second) {
                if (isBase(part.appType) || isSystemApp(part.appType)) {
                    title.base.path = convertToPosixPath(part.path);
                    title.base.version = part.titleVersion;
                    title.base.type = part.appType;
                    title.base.partHighID = (uint32_t)((part.titleId & 0xFFFFFFFF00000000) >> 32);
                    title.base.location = deviceToLocation(part.indexedDevice);

                    // Change the output path
                    if (isBase(part.appType)) title.base.outputPath = "/Games/";
                    if (isSystemApp(part.appType)) title.base.outputPath = "/System Applications/";

                    if (readInfoFromXML(title, title.base)) title.hasBase = true;
                    else {
                        //Utility::platformLog("Failed to read meta from game!\n");
                        std::this_thread::sleep_for(std::chrono::seconds(1));
                    }
                }
                else if (isUpdate(part.appType)) { // TODO: Log cases where maybe two updates are found (one on disc and one later via the title system)
                    title.update.path = convertToPosixPath(part.path);
                    title.update.version = part.titleVersion;
                    title.update.type = part.appType;
                    title.update.partHighID = (uint32_t)((part.titleId & 0xFFFFFFFF00000000) >> 32);
                    title.update.location = deviceToLocation(part.indexedDevice);
                    title.update.outputPath = "/Updates/";
                    if (readInfoFromXML(title, title.update)) title.hasUpdate = true;
                    else {
                        Utility::platformLog("Failed to read meta from update!\n");
                        std::this_thread::sleep_for(std::chrono::seconds(1));
                    }
                }
                else if (isDLC(part.appType)) {
                    title.dlc.path = convertToPosixPath(part.path);
                    title.dlc.version = part.titleVersion;
                    title.dlc.type = part.appType;
                    title.dlc.partHighID = (uint32_t)((part.titleId & 0xFFFFFFFF00000000) >> 32);
                    title.dlc.location = deviceToLocation(part.indexedDevice);
                    title.dlc.outputPath = "/DLCs/";
                    if (readInfoFromXML(title, title.dlc)) title.hasDLC = true;
                    else {
                        Utility::platformLog("Failed to read meta from dlc!\n");
                        std::this_thread::sleep_for(std::chrono::seconds(1));
                    }
                }
            }
        }

        return true;
    }

    static inline void ltrim(std::string& s) {
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
            return !std::isspace(ch);
            }));
    }

    static inline void rtrim(std::string& s) {
        s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
            return !std::isspace(ch);
            }).base(), s.end());
    }

    std::string normalizeTitle(std::string& unsafeTitle) {
        std::string retTitle;
        uint32_t actualCharacters = 0;
        for (char& chr : unsafeTitle) {
            if (isalnum(chr)) {
                actualCharacters++;
                retTitle += chr;
            }
            else if (chr == ' ') {
                retTitle += chr;
            }
        }
        if (actualCharacters <= 2) return "";

        rtrim(retTitle);
        ltrim(retTitle);

        return retTitle;
    }

    bool isBase(MCPAppType type) {
        return type == MCP_APP_TYPE_GAME;
    }

    bool isUpdate(MCPAppType type) {
        return type == MCP_APP_TYPE_GAME_UPDATE;
    }

    bool isDLC(MCPAppType type) {
        return type == MCP_APP_TYPE_GAME_DLC;
    }

    bool isSystemApp(MCPAppType type) {
        return type == MCP_APP_TYPE_BROWSER || type == MCP_APP_TYPE_ESHOP || type == MCP_APP_TYPE_FRIEND_LIST || type == MCP_APP_TYPE_AMIIBO_SETTINGS || type == MCP_APP_TYPE_DOWNLOAD_MANAGEMENT || type == MCP_APP_TYPE_HOME_MENU || type == MCP_APP_TYPE_EMANUAL || type == MCP_APP_TYPE_SYSTEM_APPS || type == MCP_APP_TYPE_SYSTEM_SETTINGS;
    }

    int test(int arg)
    {
        return arg + 1;
    }

    inline dumpTypeFlags operator &(dumpTypeFlags a, dumpTypeFlags b) {
        return static_cast<dumpTypeFlags>(static_cast<std::underlying_type_t<dumpTypeFlags>>(a) | static_cast<std::underlying_type_t<dumpTypeFlags>>(b));
    }

    bool dumpGame(const titleEntry& entry, const dumpingConfig& config, const std::string& outPath) {
        Utility::platformLog("Dumping game, this may take a while...\n");
        if ((config.dumpTypes & dumpTypeFlags::Game) == dumpTypeFlags::Game && entry.hasBase) {
            if(!Utility::create_directories(outPath)) {
                Utility::platformLog("Failed to create output dir %s\n", outPath.c_str());
                return false;
            }
            if(!Utility::copy(entry.base.path, outPath)) {
                Utility::platformLog("Failed to copy game files %s\n", outPath.c_str());
                return false;
            }
        }

        return true;
    }
}
