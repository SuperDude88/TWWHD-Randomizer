#pragma once

#include <cstdint>
#include <array>
#include <filesystem>

//from https://github.com/devkitPro/wut/blob/78300f2693405b86b3482520b95b4a3f826d4e72/include/coreinit/mcp.h#L24
enum class AppType : uint32_t {
    GAME_UPDATE            = 0x0800001B,
    GAME_DLC               = 0x0800000E,
    BOOT1                  = 0x10000009,
    SYSTEM_LIBRARIES       = 0x1000000A,
    BLUETOOTH_FIRMWARE     = 0x10000012,
    DRH_FIRMWARE           = 0x10000013,
    DRC_FIRMWARE           = 0x10000014,
    SYSTEM_VERSION         = 0x10000015,
    DRC_LANGUAGE           = 0x1000001A,
    EXCEPTIONS_DATA        = 0x18000010,
    SHARED_DATA            = 0x1800001C,
    CERT_STORE             = 0x1800001E,
    PATCH_MAP_DATA         = 0x18000023,
    WAGONU_MIGRATION_LIST  = 0x18000029,
    CAFFEINE_TITLE_LIST    = 0x18000030,
    MCP_TITLE_LIST         = 0x18000031,
    GAME                   = 0x80000000,
    GAME_WII               = 0x8000002E,
    SYSTEM_MENU            = 0x90000001,
    SYSTEM_UPDATER         = 0x9000000B,
    SYSTEM_APPS            = 0x90000020,
    ACCOUNT_APPS           = 0x90000021,
    SYSTEM_SETTINGS        = 0x90000022,
    ECO_PROCESS            = 0x9000002F,
    EMANUAL                = 0xD0000003,
    HOME_MENU              = 0xD0000004,
    ERROR_DISPLAY          = 0xD0000005,
    BROWSER                = 0xD0000006,
    MIIVERSE_POST          = 0xD000000D,
    MIIVERSE               = 0xD0000016,
    ESHOP                  = 0xD0000017,
    FRIEND_LIST            = 0xD0000018,
    DOWNLOAD_MANAGEMENT    = 0xD0000019,
    AOC_OVERLAY            = 0xD000002C,
    AMIIBO_SETTINGS        = 0xD0000033
};

class AppInfo {
public:
    uint32_t version = 0;
    
    uint64_t OSVersion = 0x000500101000400A;
    uint64_t titleID = 0;
    uint16_t titleVer = 0;
    uint32_t sdkVer = 0;
    AppType appType = AppType::GAME;
    uint16_t groupID = 0;
    std::array<uint8_t, 32> osMask{0};
    uint64_t common_id = 0;

    bool parseFromXML(const std::filesystem::path& xmlPath);
};
