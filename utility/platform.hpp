#pragma once

#include <string>

#ifdef DEVKITPRO
    #define PLATFORM_DKP

    #include <filesystem>
#elif defined(_MSC_VER)
    #define PLATFORM_MSVC
#elif defined(__GNUC__) || defined(__GNUG__)
    #define PLATFORM_GCC
#elif defined(__clang__)
    #define PLATFORM_CLANG
#else 
    #error UNKNOWN PLATFORM 
#endif

namespace Utility
{
    void platformLog(const std::string& str);

    bool platformInit();

    bool platformIsRunning();

    void waitForPlatformStop();

    void platformShutdown();

#ifdef DEVKITPRO
    bool mountDeviceAndConvertPath(std::filesystem::path& path);
#endif
}
