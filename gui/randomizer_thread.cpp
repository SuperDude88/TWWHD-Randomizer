#include "randomizer_thread.hpp"

#include <iostream>
#include <algorithm>
#include <QDirIterator>

#include <randomizer.hpp>
#include <command/Log.hpp>
#include <gui/update_dialog_header.hpp>

RandomizerThread::RandomizerThread()
{

}

RandomizerThread::~RandomizerThread()
{

}

void RandomizerThread::run()
{
    #if defined(EMBED_DATA) && defined(QT_GUI)
        // Copy embedded files into temporary directory
        UPDATE_DIALOG_LABEL("Loading Resources...");
        QDirIterator it(":/", QDir::Files, QDirIterator::Subdirectories);
        while(it.hasNext())
        {
            auto filepath = it.next().toStdString();
            auto splitFilepath = Utility::Str::split(filepath, '/');
            // ignore these directories
            if (splitFilepath[1] == "qpdf" || splitFilepath[1] == "qt-project.org")
            {
                continue;
            }
            // Replace ':' with data directory (without '.' or '/')
            auto& dataPath = splitFilepath[0];
            dataPath = DATA_PATH;
            dataPath.erase(std::remove(dataPath.begin(), dataPath.end(), '.'));
            dataPath.erase(std::remove(dataPath.begin(), dataPath.end(), '/'));

            auto realFilepath = Utility::Str::merge(splitFilepath, '/');

            // Remove '/' at the end
            realFilepath.pop_back();

            // Remove filename from path
            splitFilepath.pop_back();
            auto dirPath = Utility::Str::merge(splitFilepath, '/');
            std::filesystem::create_directories(dirPath);
            QFile file(QString::fromStdString(filepath));
            if(!file.copy(QString::fromStdString(realFilepath)))
            {
                ErrorLog::getInstance().log("Failed to copy - " + filepath + " to " + realFilepath);
                ErrorLog::getInstance().log(file.errorString().toStdString());
            }
        }
    #endif

    int retVal = mainRandomize();

    #if defined(EMBED_DATA) && defined(QT_GUI)
        // Remove temporary directory
        QDir dir(DATA_PATH);
        dir.setFilter(QDir::NoDotAndDotDot);
        dir.removeRecursively();
    #endif

    if (retVal != 0)
    {
        emit errorUpdate(ErrorLog::getInstance().getLastErrors());
    }
}
