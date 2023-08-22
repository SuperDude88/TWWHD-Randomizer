#include "platform.hpp"
#include <thread>
#include <mutex>

#ifdef PLATFORM_DKP
    #include <whb/proc.h>
    #include <whb/log.h>
    #include <whb/log_console.h>
    #include <coreinit/filesystem_fsa.h>

    #include <mocha/mocha.h>

    #define PRINTF_BUFFER_LENGTH 2048

    static bool whbInit = false;
    static bool mochaOpen = false;
    static bool MLCMounted = false;
    static bool USBMounted = false;
    static bool DiscMounted = false;
#endif 

static std::mutex printMut;

#ifdef PLATFORM_DKP
static bool flushVolume(const std::string& vol) {
    const FSAClientHandle handle = FSAAddClient(NULL);
    if(handle < 0) {
        return false;
    }
    FSError ret = FSAFlushVolume(handle, vol.c_str());
    if(ret != FS_ERROR_OK) {
        return false;
    }
    ret = FSADelClient(handle);
    if(ret != FS_ERROR_OK) {
        return false;
    }
    return true;
}

bool initMocha()
{
    Utility::platformLog("Starting libmocha...\n");
    
    if(MochaUtilsStatus status = Mocha_InitLibrary(); status != MOCHA_RESULT_SUCCESS) {
        Utility::platformLog("Mocha_InitLibrary() failed\n");
        return false;
    }

    Utility::platformLog("Mocha initialized, storage mounted\n");
    return true;
}

void closeMocha() {
    if(MLCMounted) {
        if(!flushVolume("/vol/storage_mlc01")) { //maybe check if we wrote to MLC
            Utility::platformLog("Could not flush MLC\n");
        }
        if(MochaUtilsStatus status = Mocha_UnmountFS("storage_mlc01"); status != MOCHA_RESULT_SUCCESS) {
            Utility::platformLog("Error unmounting MLC: %s\n", Mocha_GetStatusStr(status));
        }
        MLCMounted = false;
    }

    if(USBMounted) {
        if(!flushVolume("/vol/storage_usb01")) { //maybe check if we wrote to USB
            Utility::platformLog("Could not flush USB\n");
        }
        if(MochaUtilsStatus status = Mocha_UnmountFS("storage_usb01"); status != MOCHA_RESULT_SUCCESS) {
            Utility::platformLog("Error unmounting USB: %s\n", Mocha_GetStatusStr(status));
        }
        USBMounted = false;
    }

    if(DiscMounted) {
        if(MochaUtilsStatus status = Mocha_UnmountFS("storage_odd03"); status != MOCHA_RESULT_SUCCESS) {
            Utility::platformLog("Error unmounting disc: %s\n", Mocha_GetStatusStr(status));
        }
        DiscMounted = false;
    }

    if(MochaUtilsStatus status = Mocha_DeInitLibrary(); status != MOCHA_RESULT_SUCCESS) {
        Utility::platformLog("Mocha_DeinitLibrary() failed\n");
    }

    return;
}

namespace Utility {
    bool mountDeviceAndConvertPath(std::filesystem::path& path) {
        if(path.string().starts_with("/vol/storage_mlc01")) {
            if(!MLCMounted) {
                Utility::platformLog("Attempting to mount MLC\n");
                if(MochaUtilsStatus status = Mocha_MountFS("storage_mlc01", nullptr, "/vol/storage_mlc01"); status != MOCHA_RESULT_SUCCESS)
                {
                    Utility::platformLog("Failed to mount MLC: %s\n", Mocha_GetStatusStr(status));
                    return false;
                }

                MLCMounted = true;
            }
        }
        else if(path.string().starts_with("/vol/storage_usb01")) {
            if(!USBMounted) {
                Utility::platformLog("Attempting to mount USB\n");
                if(MochaUtilsStatus status = Mocha_MountFS("storage_usb01", nullptr, "/vol/storage_usb01"); status != MOCHA_RESULT_SUCCESS)
                {
                    Utility::platformLog("Failed to mount USB: %s\n", Mocha_GetStatusStr(status));
                    return false;
                }

                USBMounted = true;
            }
        }
        else if(path.string().starts_with("/vol/storage_odd")) {
            if(!DiscMounted) {
                Utility::platformLog("Attempting to mount disc\n");
                if(MochaUtilsStatus status = Mocha_MountFS("storage_odd03", "/dev/odd03", "/vol/storage_odd_content"); status != MOCHA_RESULT_SUCCESS)
                {
                    Utility::platformLog("Failed to mount disc: %s\n", Mocha_GetStatusStr(status));
                    return false;
                }

                DiscMounted = true;
            }
        }
        else {
            return false;
        }

        //https://github.com/emiyl/dumpling/blob/9290dad8f8d91cc3ef4c4b9602898d244a2a1454/source/app/filesystem.cpp#L130
        std::string working_path = path.string().substr(5);
        if (const auto& driveEnd = working_path.find_first_of('/'); driveEnd != std::string::npos) {
            // Return mount path + the path after it
            working_path.replace(driveEnd, 1, ":/", 2);
        } else {
            // Return just the mount path
            working_path.append(":");
        }
        path = working_path;

        return true;
    }
}
#endif

namespace Utility
{
    void platformLog(const char* f, ...)
    {
#ifdef PLATFORM_DKP
        char buf[PRINTF_BUFFER_LENGTH];
#endif
        va_list args;
        va_start(args, f);
        std::unique_lock<std::mutex> lock(printMut);
#ifdef PLATFORM_DKP
        vsnprintf(buf, PRINTF_BUFFER_LENGTH - 1, f, args);
        
        WHBLogWrite(buf);
        WHBLogConsoleDraw();
#else
        vprintf(f, args);
        fflush(stdout); //vscode debug console works better with this
#endif
        lock.unlock();
        va_end(args);
    }

    void platformLog(const std::string& str)
    {
        platformLog(str.c_str());
    }

    bool platformInit()
    {
#ifdef PLATFORM_DKP
        WHBProcInit();
        WHBLogConsoleInit();
        whbInit = true;

        if(!initMocha())
        {
            Utility::platformLog("Failed to init libmocha\n");
            return false;
        }
        mochaOpen = true;
#endif
        return true;
    }

    

    bool platformIsRunning()
    {
#ifdef PLATFORM_DKP
        return WHBProcIsRunning();
#else
        return true; //not sure if it's worth doing anything for this
#endif
    }

    void waitForPlatformStop()
    {
        while(platformIsRunning()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(33)); //Check ~30 times a second
        }
    }

    void platformShutdown()
    {
#ifdef PLATFORM_DKP
        if (mochaOpen)
        {
            closeMocha();
            mochaOpen = false;
        }

        if(whbInit) {
            WHBLogConsoleFree();
            WHBProcShutdown();
        }
#endif
    }
}
