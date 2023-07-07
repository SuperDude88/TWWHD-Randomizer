#include <randomizer.hpp>
#include <command/Log.hpp>
#include <utility/platform.hpp>
#ifdef DEVKITPRO
    #include <thread>
    #include <sysapp/launch.h>
#endif

int main()
{
    Utility::platformInit();
    int retVal = mainRandomize();

    if (retVal == 1) {
        auto message = "An error has occured!\n" + ErrorLog::getInstance().getLastErrors();
        Utility::platformLog(message.c_str());
    }

    // Close logs
    ErrorLog::getInstance().close();
    DebugLog::getInstance().close();
    BasicLog::getInstance().close();
    
    //launch Wii U menu, wait for procUI to do its thing
    #ifdef DEVKITPRO
        SYSLaunchMenu();
        while(Utility::platformIsRunning()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(33)); //Check ~30 times a second
        }
    #endif
    
    Utility::platformShutdown();
    
    return 0;
}
