#include "mainwindow.h"

#include <QApplication>

#include <fstream>

int main(int argc, char *argv[])
{
    // Check for config file
    std::ifstream conf("./config.yaml");
    if (!conf.is_open())
    {
        // No config file, create default
        delete_and_create_default_config();
    }
    conf.close();

    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
