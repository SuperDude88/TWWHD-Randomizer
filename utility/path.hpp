#pragma once

#include <filesystem>

#ifdef QT_GUI
    #include <QString>
#endif

using fspath = std::filesystem::path;

namespace Utility {
    fspath get_data_path();
    fspath get_app_save_path();
    fspath get_logs_path();
    fspath get_temp_dir();

    #ifdef QT_GUI
        // Wrapper for path -> QString
        // Use a wide string type to cover Windows where paths are UTF-16 encoded (and hopefully still be fine on other platforms)
        // Also use the "generic" version with '/' separators because the Windows '\' breaks some paths
        inline QString toQString(const fspath& path) { return QString::fromStdU32String(path.generic_u32string()); }
    #endif
}
