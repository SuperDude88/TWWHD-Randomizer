#include "tracker_location_label.h"

#include <set_font.h>

TrackerLocationLabel::TrackerLocationLabel()
{

}

TrackerLocationLabel::TrackerLocationLabel(int pointSize)
{
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);

    SET_FONT(this, "fira_sans", 14);
    // Have to reset pointSize since we can't pass into a macro
    auto f = font();
    f.setPointSize(pointSize);
    setFont(f);

    setWordWrap(true);
    setCursor(Qt::PointingHandCursor);
}

void TrackerLocationLabel::set_location(Location* loc)
{
    location = loc;
    auto noPrefixPos = loc->getName().find("- ");
    setText(loc->getName().substr(noPrefixPos + 2).c_str());
    update_colors();
}

void TrackerLocationLabel::mouseReleaseEvent(QMouseEvent* e)
{
    location->marked = !location->marked;
    update_colors();
    emit location_label_clicked();
}

void TrackerLocationLabel::update_colors()
{
    if (location->marked)
    {
        setStyleSheet("color: black; text-decoration: line-through;");
    }
    else if (!location->hasBeenFound)
    {
        setStyleSheet("color: red;");
    }
    else
    {
        setStyleSheet("color: blue;");
    }
}
