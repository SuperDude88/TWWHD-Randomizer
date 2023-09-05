#include <thread>
#include <randomizer.hpp>
#include <command/Log.hpp>
#include <utility/platform.hpp>

int main() {
    using namespace std::literals::chrono_literals;

    if(Utility::platformInit()) {
        int retVal = mainRandomize();

        if (retVal == 1) {
            auto message = "An error has occured!\n" + ErrorLog::getInstance().getLastErrors();
            Utility::platformLog(message.c_str());

            std::this_thread::sleep_for(5s);
        }
    }
    else {
        Utility::platformLog("Failed to initialize platform!\n");
        std::this_thread::sleep_for(3s);
    }

    Utility::platformShutdown();
    
    return 0;
}
