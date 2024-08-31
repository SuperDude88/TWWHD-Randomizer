#include <cstdlib>

#ifdef QT_GUI
    #include <QApplication>
    #include <QResource>
    #include <QDirIterator>

    #include <gui/desktop/palette.hpp>
    #include <gui/desktop/mainwindow.hpp>
#elif defined(DEVKITPRO)
    #include <gui/wiiu/SettingsMenu.hpp>
    #include <gui/wiiu/ExitMenu.hpp>

    #include <utility/platform.hpp>
    #include <command/Log.hpp>
    #include <randomizer.hpp>

    static int endApplication(const ExitMode& mode) {
        if(Utility::platformIsRunning()) {
            waitForExitConfirm(mode);
        }

        Utility::platformShutdown();

        return 0;
    }
#else
    #include <thread>

    #include <utility/platform.hpp>
    #include <command/Log.hpp>
    #include <randomizer.hpp>
#endif

int main(int argc, char *argv[]) {
    // Initialze RNG for choosing random colors and seed strings (fill algorithm does not use this)
    srand(time(NULL));

#ifdef QT_GUI
    // Init embedded resources if we're using them
    #if defined(EMBED_DATA)
        Q_INIT_RESOURCE(data);
    #endif

    QApplication::setStyle("Fusion");

    QApplication a(argc, argv);
    QApplication::setPalette(getColorPalette());
    MainWindow w;
    w.show();

    return a.exec();
#elif defined(DEVKITPRO)
    if(!Utility::platformInit()) {
        return endApplication(ExitMode::PLATFORM_ERROR);
    }

    using Result = SettingsMenu::Result;
    switch(SettingsMenu::run()) {
        case Result::CONTINUE:
            break;
        case Result::EXIT:
            return endApplication(ExitMode::IMMEDIATE);
        case Result::CONFIG_ERROR:
            ErrorLog::getInstance().log("Error saving/loading config file.");
            [[fallthrough]];
        default: // this shouldn't happen so treat it as an error
            return endApplication(ExitMode::GUI_ERROR);
    }

    if(const int retVal = mainRandomize(); retVal != 0) {
        return endApplication(ExitMode::RANDOMIZATION_ERROR);
    }

    return endApplication(ExitMode::RANDOMIZATION_COMPLETE);
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

    return 0;
#endif
}
