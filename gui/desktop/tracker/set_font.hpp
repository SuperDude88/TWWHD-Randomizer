#pragma once

#include <QFontDatabase>

#include <gui/desktop/tracker/tracker_data.hpp>

template<typename W>
void set_font(W* widget, const std::string& font_filename, int point_size)
{
    static std::unordered_map<std::string, int> font_name_to_id;

    if(!font_name_to_id.contains(font_filename)) {
        font_name_to_id[font_filename] = QFontDatabase::addApplicationFont(getTrackerAssetPath(font_filename + ".ttf"));
    }

#ifdef __APPLE__
    point_size += 3;
#endif

    const int fontId = font_name_to_id.at(font_filename);
    if (fontId != -1) {
        QString family = QFontDatabase::applicationFontFamilies(fontId).at(0);
        QFont new_font(family);
        new_font.setPointSize(point_size);
        widget->setFont(new_font);
    } else {
        auto font = widget->font();
        font.setPointSize(point_size);
        widget->setFont(font);
    }

}
