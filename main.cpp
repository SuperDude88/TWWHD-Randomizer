#include <cstdlib>
#include <iostream>
#include <logic/flatten/bits.hpp>

#ifdef QT_GUI
    #include <QApplication>
    #include <QResource>
    #include <QDirIterator>
    
    #include <gui/mainwindow.hpp>
#elif defined(DEVKITPRO)
    #include <utility/platform.hpp>
    #include <platform/gui/ExitMenu.hpp>
    #include <randomizer.hpp>
#else
    #include <thread>

    #include <utility/platform.hpp>
    #include <command/Log.hpp>
    #include <randomizer.hpp>
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
#elif defined(DEVKITPRO)
    ExitMode mode;

    if(!Utility::platformInit()) {
        mode = ExitMode::PLATFORM_ERROR;
    }
    else if(const int retVal = mainRandomize(); retVal == 0) {
        mode = ExitMode::RANDOMIZATION_COMPLETE;
    }
    else {
        mode = ExitMode::RANDOMIZATION_ERROR;
    }

    if(Utility::platformIsRunning()) {
        waitForExitConfirm(mode);
    }

    Utility::platformShutdown();
    
    return 0;
#else
    using namespace std::literals::chrono_literals;

    if(Utility::platformInit()) {
        int retVal = mainRandomize();

        if (retVal == 1) {
            Utility::platformLog("An error has occured! See the error log for details.\n" + ErrorLog::getInstance().getLastErrors());

            std::this_thread::sleep_for(5s);
        }
    }
    else {
        Utility::platformLog("Failed to initialize platform!");
        std::this_thread::sleep_for(3s);
    }

    Utility::platformShutdown();
    
    BitVector bits = BitVector();
    std::cout << bits.isEmpty() << std::endl;

    return 0;
#endif
}
