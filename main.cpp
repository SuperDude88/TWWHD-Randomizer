#include <cstdlib>

#ifdef QT_GUI
    #include <QApplication>
    #include <QResource>
    #include <QDirIterator>
    
    #include <gui/mainwindow.h>
#else
    #include <thread>
    #include <filesystem>

    #include <utility/platform.hpp>
    #include <command/Log.hpp>
    #include <randomizer.hpp>

    static void clearOldLogs() {
        if(std::filesystem::is_regular_file(DebugLog::LOG_PATH)) {
            std::filesystem::remove(DebugLog::LOG_PATH);
        }

        if(std::filesystem::is_regular_file(ErrorLog::LOG_PATH)) {
            std::filesystem::remove(ErrorLog::LOG_PATH);
        }

        return;
    }
#endif

int main(int argc, char *argv[]) {
    // Initialze RNG for choosing random colors (fill algorithm does not use this)
    srand(time(NULL));

#ifdef QT_GUI
    // Init embedded resources if we're using them
    #if defined(EMBED_DATA)
        Q_INIT_RESOURCE(data);
    #endif

    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
#else
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
#endif
}
