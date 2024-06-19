#include "tracker_label.hpp"

#include <gui/tracker/set_font.hpp>

#include <logic/Area.hpp>

#include <QMouseEvent>

TrackerLabel::TrackerLabel()
{

}

TrackerLabel::TrackerLabel(TrackerLabelType type_, int pointSize, Location* location_, Entrance* entrance_) : type(type_)
{
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);
    set_font(this, "fira_sans", pointSize);
    setWordWrap(true);
    setCursor(Qt::PointingHandCursor);

    switch (type)
    {
    case TrackerLabelType::Location:
        set_location(location_);
        break;

    case TrackerLabelType::EntranceSource:
        setMinimumHeight(15);
        setMaximumWidth(345);
        set_entrance(entrance_);
        break;

    case TrackerLabelType::EntranceDestination:
        setMinimumHeight(15);
        setMaximumWidth(345);
        set_entrance(entrance_);
        break;

    default:
        break;
    }
}

void TrackerLabel::set_location(Location* loc)
{
    location = loc;
    auto noPrefixPos = loc->getName().find("- ");
    setText(loc->getName().substr(noPrefixPos + 2).c_str());
    update_colors();
}

Location* TrackerLabel::get_location() const
{
    return location;
}

void TrackerLabel::set_entrance(Entrance* entrance_)
{
    entrance = entrance_;

    std::string destination;
    std::string source;
    switch (type)
    {
    case TrackerLabelType::EntranceSource:
        destination = entrance->getConnectedArea() == nullptr ? "?" : entrance->getConnectedArea()->name;
        if (entrance->getOriginalName(true).find("Battle Arena Exit") != std::string::npos)
        {
            source = entrance->getOriginalName(true).substr(0, entrance->getOriginalName(true).find(" -> ")) + " Exit";
        }
        else
        {
            source = entrance->getOriginalConnectedArea()->name;
        }
        setText(std::string(source + " -> " + destination).c_str());
        break;
    case TrackerLabelType::EntranceDestination:
        setText(entrance->getConnectedArea()->name.c_str());
        break;
    default:
        break;
    }

    update_colors();
}

Entrance* TrackerLabel::get_entrance() const
{
    return entrance;
}

void TrackerLabel::mark_location()
{
    location->marked = !location->marked;
    update_colors();
    emit location_label_clicked();
}

void TrackerLabel::mouseReleaseEvent(QMouseEvent* e)
{
    switch (type)
    {
    case TrackerLabelType::Location:
        mark_location();
        break;
    case TrackerLabelType::EntranceSource:
        if (e->button() == Qt::LeftButton)
        {
            emit entrance_source_label_clicked(entrance);
        }
        else if (e->button() == Qt::RightButton)
        {
            emit entrance_source_label_disconnect(entrance);
        }

        break;
    case TrackerLabelType::EntranceDestination:
        emit entrance_destination_label_clicked(entrance);
    default:
        break;
    }
}

void TrackerLabel::enterEvent(QEnterEvent* e)
{
    switch(type) {
        case TrackerLabelType::Location:
            emit mouse_over_location_label(location);
            break;
        case TrackerLabelType::EntranceSource:
        case TrackerLabelType::EntranceDestination:
            emit mouse_over_entrance_label(entrance);
            break;
        case TrackerLabelType::NONE:
        default:
            break;
    }
}

void TrackerLabel::leaveEvent(QEvent* e)
{
    switch(type) {
        case TrackerLabelType::Location:
            emit mouse_left_location_label();
            break;
        case TrackerLabelType::EntranceSource:
        case TrackerLabelType::EntranceDestination:
            emit mouse_left_entrance_label();
            break;
        case TrackerLabelType::NONE:
        default:
            break;
    }
}

void TrackerLabel::update_colors()
{
    switch (type)
    {
    case TrackerLabelType::Location:
        if (location->marked)
        {
            setStyleSheet("color: black; text-decoration: line-through;");
        }
        else if (!location->hasBeenFound && showLogic)
        {
            setStyleSheet("color: red;");
        }
        else
        {
            setStyleSheet("color: blue;");
        }
        break;

    case TrackerLabelType::EntranceSource:
        if (entrance->getConnectedArea())
        {
            setStyleSheet("color: black;");
        }
    //    else if (!entrance->hasBeenFound)
    //    {
    //        setStyleSheet("color: red;");
    //    }
        else
        {
            setStyleSheet("color: blue;");
        }
        break;
    case TrackerLabelType::EntranceDestination:
        setStyleSheet("color: black;");
    default:
        break;
    }


}

void TrackerLabel::updateShowLogic(int show)
{
    showLogic = show ? true : false;
    update_colors();
}
