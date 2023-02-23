#include "tracker_area_label.h"

#include <tracker/set_font.h>

TrackerAreaLabel::TrackerAreaLabel()
{
    SET_FONT(this, "fira_sans", 10);
    setText("0/0");
}

void TrackerAreaLabel::mouseReleaseEvent(QMouseEvent* e)
{
    emit area_label_clicked(areaPrefix);
}
