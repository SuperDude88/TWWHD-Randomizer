#include "tracker_area_label.h"

#include <QFontDatabase>

TrackerAreaLabel::TrackerAreaLabel()
{
    int firaSansFontId = QFontDatabase::addApplicationFont(DATA_PATH "assets/tracker/fira_sans.ttf");
    QString family = QFontDatabase::applicationFontFamilies(firaSansFontId).at(0);
    QFont firaSans(family);
    firaSans.setStyleStrategy(QFont::PreferAntialias);
    firaSans.setPointSize(10);
    setFont(firaSans);
    setText("0/0");
}

TrackerAreaLabel::TrackerAreaLabel(const std::string& areaPrefix_) : areaPrefix(areaPrefix_)
{

}

void TrackerAreaLabel::mouseReleaseEvent(QMouseEvent* e)
{
    emit area_label_clicked(areaPrefix);
}
