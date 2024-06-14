#include "tracker_area_label.hpp"

#include <gui/tracker/set_font.hpp>

TrackerAreaLabel::TrackerAreaLabel()
{
    set_font(this, "fira_sans", 10);
    setText("0/0");
}

void TrackerAreaLabel::mouseReleaseEvent(QMouseEvent* e)
{
    emit area_label_clicked(areaPrefix);
}
