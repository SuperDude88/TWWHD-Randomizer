#include "tracker_area_label.hpp"

#include <gui/tracker/set_font.hpp>

#include <QMouseEvent>

TrackerAreaLabel::TrackerAreaLabel()
{
    set_font(this, "fira_sans", 10);
    setText("0/0");
}

void TrackerAreaLabel::mouseReleaseEvent(QMouseEvent* e)
{
    if (e->button() == Qt::LeftButton)
    {
        emit area_label_clicked(areaPrefix);
    }
    else if (e->button() == Qt::RightButton)
    {
        emit area_label_right_clicked(areaPrefix);
    }

}
