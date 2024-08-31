#include "palette.hpp"

#include <QApplication>
#include <QStyleHints>

QPalette getColorPalette() {
    QPalette result = QApplication::palette();

    if(QApplication::styleHints()->colorScheme() != Qt::ColorScheme::Light) {
        result.setColor(QPalette::ColorRole::Window, QColor("#353535"));
        result.setColor(QPalette::ColorRole::WindowText, QColor("#D9D9D9"));
        result.setColor(QPalette::ColorRole::Base, QColor("#2A2A2A"));
        result.setColor(QPalette::ColorRole::AlternateBase, QColor("#424242"));
        result.setColor(QPalette::ColorRole::ToolTipBase, QColor("#353535"));
        result.setColor(QPalette::ColorRole::ToolTipText, QColor("#D9D9D9"));
        result.setColor(QPalette::ColorRole::Text, QColor("#D9D9D9"));
        result.setColor(QPalette::ColorRole::Dark, QColor("#232323"));
        result.setColor(QPalette::ColorRole::Shadow, QColor("#141414"));
        result.setColor(QPalette::ColorRole::Button, QColor("#353535"));
        result.setColor(QPalette::ColorRole::ButtonText, QColor("#D9D9D9"));
        result.setColor(QPalette::ColorRole::BrightText, QColor("#D90000"));
        result.setColor(QPalette::ColorRole::Link, QColor("#2A82DA"));
        result.setColor(QPalette::ColorRole::Highlight, QColor("#2A82DA"));
        result.setColor(QPalette::ColorRole::HighlightedText, QColor("#D9D9D9"));

        result.setColor(QPalette::ColorGroup::Disabled, QPalette::ColorRole::WindowText, QColor("#7F7F7F"));
        result.setColor(QPalette::ColorGroup::Disabled, QPalette::ColorRole::Text, QColor("#7F7F7F"));
        result.setColor(QPalette::ColorGroup::Disabled, QPalette::ColorRole::ButtonText, QColor("#7F7F7F"));
        result.setColor(QPalette::ColorGroup::Disabled, QPalette::ColorRole::Highlight, QColor("#505050"));
        result.setColor(QPalette::ColorGroup::Disabled, QPalette::ColorRole::HighlightedText, QColor("#7F7F7F"));
    }

    return result;
}
