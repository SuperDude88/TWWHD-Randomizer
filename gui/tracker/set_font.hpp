#pragma once

#include <QFontDatabase>

#include <gui/tracker/tracker_data.hpp>

template<typename W>
void set_font(W* widget, const std::string& font_filename, int point_size)
{
#ifdef __APPLE__
    point_size += 3;
#endif
    int fontId = QFontDatabase::addApplicationFont(getTrackerAssetPath(font_filename + ".ttf"));
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
