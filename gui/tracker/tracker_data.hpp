#pragma once

#include <utility/path.hpp>

inline QString getTrackerAssetPath(const std::string& file) {
    return Utility::toQString(Utility::get_data_path() / "tracker" / file);
}
