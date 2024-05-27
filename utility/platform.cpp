#include "platform.hpp"
#include <thread>
#include <mutex>

#ifdef PLATFORM_DKP
    #include <platform/proc.hpp>
    #include <platform/home.hpp>
    #include <platform/energy_saver.hpp>
    #include <platform/gui/screen.hpp>
    #include <platform/gui/LogConsole.hpp>
    #include <coreinit/filesystem_fsa.h>

    #include <mocha/mocha.h>

    #define PRINTF_BUFFER_LENGTH 2048

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

    if(FSAFlushVolume(handle, vol.c_str()) != FS_ERROR_OK) {
        return false;
    }

    if(FSADelClient(handle) != FS_ERROR_OK) {
        return false;
    }

    return true;
}

bool initMocha()
{
    Utility::platformLog("Starting libmocha...");
    
    if(MochaUtilsStatus status = Mocha_InitLibrary(); status != MOCHA_RESULT_SUCCESS) {
        Utility::platformLog("Mocha_InitLibrary() failed");
        return false;
    }

    Utility::platformLog("Mocha initialized, storage mounted");
    return true;
}

void closeMocha() {
    if(MLCMounted) {
        if(!flushVolume("/vol/storage_mlc01")) { //maybe check if we wrote to MLC
            Utility::platformLog("Could not flush MLC");
        }
        if(MochaUtilsStatus status = Mocha_UnmountFS("storage_mlc01"); status != MOCHA_RESULT_SUCCESS) {
            Utility::platformLog(std::string("Error unmounting MLC: ") + Mocha_GetStatusStr(status));
        }
        MLCMounted = false;
    }

    if(USBMounted) {
        if(!flushVolume("/vol/storage_usb01")) { //maybe check if we wrote to USB
            Utility::platformLog("Could not flush USB");
        }
        if(MochaUtilsStatus status = Mocha_UnmountFS("storage_usb01"); status != MOCHA_RESULT_SUCCESS) {
            Utility::platformLog(std::string("Error unmounting USB: ") + Mocha_GetStatusStr(status));
        }
        USBMounted = false;
    }

    if(DiscMounted) {
        if(MochaUtilsStatus status = Mocha_UnmountFS("storage_odd_content"); status != MOCHA_RESULT_SUCCESS) {
            Utility::platformLog(std::string("Error unmounting disc: ") + Mocha_GetStatusStr(status));
        }
        DiscMounted = false;
    }

    if(MochaUtilsStatus status = Mocha_DeInitLibrary(); status != MOCHA_RESULT_SUCCESS) {
        Utility::platformLog("Mocha_DeinitLibrary() failed");
    }

    return;
}

namespace Utility {
    bool mountDeviceAndConvertPath(std::filesystem::path& path) {
        if(path.string().starts_with("/vol/storage_mlc01")) {
            if(!MLCMounted) {
                Utility::platformLog("Attempting to mount MLC");
                if(MochaUtilsStatus status = Mocha_MountFS("storage_mlc01", nullptr, "/vol/storage_mlc01"); status != MOCHA_RESULT_SUCCESS)
                {
                    Utility::platformLog(std::string("Failed to mount MLC: ") + Mocha_GetStatusStr(status));
                    return false;
                }

                MLCMounted = true;
            }
        }
        else if(path.string().starts_with("/vol/storage_usb01")) {
            if(!USBMounted) {
                Utility::platformLog("Attempting to mount USB");
                if(MochaUtilsStatus status = Mocha_MountFS("storage_usb01", nullptr, "/vol/storage_usb01"); status != MOCHA_RESULT_SUCCESS)
                {
                    Utility::platformLog(std::string("Failed to mount USB: ") + Mocha_GetStatusStr(status));
                    return false;
                }

                USBMounted = true;
            }
        }
        else if(path.string().starts_with("/vol/storage_odd")) {
            if(!DiscMounted) {
                Utility::platformLog("Attempting to mount disc");
                if(MochaUtilsStatus status = Mocha_MountFS("storage_odd_content", "/dev/odd03", "/vol/storage_odd_content"); status != MOCHA_RESULT_SUCCESS)
                {
                    Utility::platformLog(std::string("Failed to mount disc: ") + Mocha_GetStatusStr(status));
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
    void platformLog(const std::string& str)
    {
        std::unique_lock<std::mutex> lock(printMut);
        #ifdef PLATFORM_DKP
            LogConsoleWrite(str.c_str());

            if(ProcIsForeground()) {
                LogConsoleDraw();
            }
        #else
            printf("%s\n", str.c_str());
            fflush(stdout); //vscode debug console works better with this
        #endif
        lock.unlock();
    }

    bool platformInit()
    {
#ifdef PLATFORM_DKP
        ProcInit();
        ConsoleScreenInit();

        initHomeMenu();
        initEnergySaver();

        setHomeMenuEnable(false);
        setDim(false);
        setAPD(false);

        if(!initMocha())
        {
            Utility::platformLog("Failed to init libmocha");
            return false;
        }
        mochaOpen = true;
#endif
        return true;
    }

    

    bool platformIsRunning()
    {
#ifdef PLATFORM_DKP
        return ProcIsRunning();
#else
        return true; //not sure if it's worth doing anything for this
#endif
    }

    void waitForPlatformStop()
    {
#ifdef PLATFORM_DKP //only need to wait on console
        while(platformIsRunning()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(33)); //Check ~30 times a second
        }
#endif
    }

    void platformShutdown()
    {
#ifdef PLATFORM_DKP
        if (mochaOpen)
        {
            closeMocha();
            mochaOpen = false;
        }

        resetHomeMenu();
        resetEnergySaver();

        if(platformIsRunning()) {
            ProcExit();
        }
        waitForPlatformStop();
#endif
    }
}
