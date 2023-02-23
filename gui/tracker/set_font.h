#ifndef SET_FONT_H
#define SET_FONT_H

#include <QFontDatabase>

// Putting this in a macro because having to do it everytime is annoying
#define SET_FONT(widget, font_filename, point_size)                                                   \
    int fontId = QFontDatabase::addApplicationFont(DATA_PATH "assets/tracker/" font_filename ".ttf"); \
    QString family = QFontDatabase::applicationFontFamilies(fontId).at(0);                            \
    QFont new_font(family);                                                                           \
    new_font.setPointSize(point_size);                                                                \
    widget->setFont(new_font);                                                                        \

#endif // SET_FONT_H
