#include "tracker_location_label.h"

#include <tracker/set_font.h>

TrackerLocationLabel::TrackerLocationLabel()
{

}

TrackerLocationLabel::TrackerLocationLabel(int pointSize)
{
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);
    set_font(this, "fira_sans", pointSize);
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

Location* TrackerLocationLabel::get_location() const
{
    return location;
}

void TrackerLocationLabel::mark_location()
{
    location->marked = !location->marked;
    update_colors();
    emit location_label_clicked();
}

void TrackerLocationLabel::mouseReleaseEvent(QMouseEvent* e)
{
    mark_location();
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
