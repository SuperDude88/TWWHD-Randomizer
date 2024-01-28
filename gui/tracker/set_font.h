#ifndef SET_FONT_H
#define SET_FONT_H

#include <QFontDatabase>

template<typename W>
void set_font(W* widget, const std::string& font_filename, int point_size)
{
    int fontId = QFontDatabase::addApplicationFont(std::string(DATA_PATH "assets/tracker/" + font_filename + ".ttf").c_str());
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
#endif // SET_FONT_H
