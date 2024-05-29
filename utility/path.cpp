#include <utility/path.hpp>
#include <version.hpp>

#if defined(__APPLE__) && defined(QT_GUI) && defined(MAC_APP_DIRS)
#include <QStandardPaths>
#include <filesystem>
#endif

namespace Utility {
std::string get_app_save_path()
{
    #if defined(__APPLE__) && defined(QT_GUI) && defined(MAC_APP_DIRS)
        std::string path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation).toStdString() + "/" + RANDOMIZER_VERSION + std::string(APP_SAVE_PATH).substr(1);
        if (!std::filesystem::exists(path))
        {
            std::filesystem::create_directories(path);
        }
        return path;
    #else
        return APP_SAVE_PATH;
    #endif
}

std::string get_logs_path()
{
    #if defined(__APPLE__) && defined(QT_GUI) && defined(MAC_APP_PATHS)
        std::string path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation).toStdString() + "/" + RANDOMIZER_VERSION + std::string(LOGS_PATH).substr(1);
        if (!std::filesystem::exists(path))
        {
            std::filesystem::create_directories(path);
        }
        return path;
    #else
        return LOGS_PATH;
    #endif
}

std::string get_temp_dir()
{
    #if defined(__APPLE__) && defined(QT_GUI) && defined(MAC_APP_PATHS)
        return QStandardPaths::writableLocation(QStandardPaths::TempLocation).toStdString();
    #else
        return TEMP_DIR;
    #endif
}
}
