#include "mainwindow.h"

#include <QApplication>
#include <QResource>

#include <fstream>

int main(int argc, char *argv[])
{
    // Check for config file
    std::ifstream conf(APP_SAVE_PATH "config.yaml");
    if (!conf.is_open())
    {
        // No config file, create default
        delete_and_create_default_config();
    }
    conf.close();

    // Init embedded resources if we're using them
    #if defined(EMBED_DATA) && defined(QT_GUI)
        Q_INIT_RESOURCE(data);
    #endif

    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
