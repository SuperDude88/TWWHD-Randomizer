#pragma once

#include <QFontDatabase>

template<typename W>
void set_font(W* widget, const std::string& font_filename, int point_size)
{
#ifdef __APPLE__
    point_size += 3;
#endif
    int fontId = QFontDatabase::addApplicationFont(std::string(DATA_PATH "tracker/" + font_filename + ".ttf").c_str());
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
