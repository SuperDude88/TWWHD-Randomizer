#include <thread>
#include <filesystem>
#include <randomizer.hpp>
#include <command/Log.hpp>
#include <utility/platform.hpp>

static void clearOldLogs() {
    if(std::filesystem::is_regular_file(DebugLog::LOG_PATH)) {
        std::filesystem::remove(DebugLog::LOG_PATH);
    }

    if(std::filesystem::is_regular_file(ErrorLog::LOG_PATH)) {
        std::filesystem::remove(ErrorLog::LOG_PATH);
    }

    return;
}

int main() {
    using namespace std::literals::chrono_literals;

    clearOldLogs(); // clear these when a console/CLI instance is opened (GUI handles this differently)

    if(Utility::platformInit()) {
        int retVal = mainRandomize();

        if (retVal == 1) {
            Utility::platformLog("An error has occured!\n" + ErrorLog::getInstance().getLastErrors());

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
