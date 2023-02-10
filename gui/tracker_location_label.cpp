#include "tracker_location_label.h"

TrackerLocationLabel::TrackerLocationLabel()
{

}

TrackerLocationLabel::TrackerLocationLabel(Location* location_) : location(location_)
{
    setText(location_->getName().c_str());
    update_colors();
}

void TrackerLocationLabel::mouseReleaseEvent(QMouseEvent* e)
{
    location->marked = !location->marked;
    update_colors();
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
