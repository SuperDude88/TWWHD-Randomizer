#include "tracker_label.hpp"

#include <gui/tracker/set_font.hpp>
#include <gui/mainwindow.hpp>

#include <logic/Area.hpp>

#include <QMouseEvent>
#include <QToolTip>

#ifdef __APPLE__
#define TOOLTIP_UNMET "orange"
#else
#define TOOLTIP_UNMET "red"
#endif
#define TOOLTIP_MET "dodgerblue"

TrackerLabel::TrackerLabel(TrackerLabelType type_, int pointSize, MainWindow* mainWindow_, Location* location_, Entrance* entrance_) : type(type_), mainWindow(mainWindow_)
{
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);
    set_font(this, "fira_sans", pointSize);
    setWordWrap(true);
    setCursor(Qt::PointingHandCursor);
    setMouseTracking(true);

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

void TrackerLabel::mouseMoveEvent(QMouseEvent* e)
{
    switch(type)
    {
    case TrackerLabelType::Location:
        showLogicTooltip();
        break;
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
        else if (!location->progression)
        {
            setStyleSheet("color: olive;");
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

void TrackerLabel::updateShowLogic(int show, bool started)
{
    showLogic = show ? true : false;
    if(started) {
        update_colors();
    }
}

void TrackerLabel::showLogicTooltip()
{
    // Don't show tooltips if logic isn't being shown
    if (!showLogic)
    {
        return;
    }
    auto coords = mapToGlobal(QPoint(width() / 2 - 45, height() - 15));
    QToolTip::showText(coords, getTooltipText());
}

QString TrackerLabel::getTooltipText()
{
    auto& req = location->computedRequirement;
    std::vector<QString> text = {};
    switch (req.type)
    {
    case RequirementType::AND:
        for (auto& arg : req.args)
        {
            auto& argReq = std::get<Requirement>(arg);
            text.push_back(formatRequirement(argReq, true));
        }
        break;
    default:
        text.push_back(formatRequirement(req, true));
    }

    QString returnStr = "Item Requirements:<ul style=\"margin-top: 0px; margin-bottom: 0px; margin-left: 8px; margin-right: 0px; -qt-list-indent:0;\"><li>";
    for (auto i = 0; i < text.size(); i++)
    {
        auto str = text[i];
        // Check for special shorthand cases
        // If we're listing the whole triforce, combine it into 1 item
        if (str.contains("Triforce Shard 1") && text.size() > i + 7 && text[i + 7].contains("Triforce Shard 8"))
        {
            QString color = TOOLTIP_MET;
            // Color the text red if we don't have even 1 shard
            for (auto j = i; j <= i + 7; j++)
            {
                auto& shard = text[j];
                if (shard.contains(TOOLTIP_UNMET))
                {
                    color = TOOLTIP_UNMET;
                    break;
                }
            }
            str = "<span style=\"color:" + color + "\">Triforce of Courage</span>";
            // Skip over the rest of the shards
            i += 7;
        }
        if (str.size() == 0)
        {
            continue;
        }
        returnStr += str;
        if (i < text.size() - 1)
        {
            returnStr += "</li><li>";
        }
    }
    returnStr += "</li></ul>";
    return returnStr;
}

QString TrackerLabel::formatRequirement(const Requirement& req, const bool& isTopLevel /*= false*/)
{
    QString returnStr = "";
    QString color = "";
    uint32_t expectedCount = 0;
    Item item;
    Requirement nestedReq;
    int processedTerms = 0;
    switch(req.type)
    {
    case RequirementType::NOTHING:
        return "<span style=\"color:" TOOLTIP_MET "\">Nothing</span>";
    case RequirementType::IMPOSSIBLE:
        return "<span style=\"color:" TOOLTIP_UNMET "\">Impossible (please discover an entrance first)</span>";
    case RequirementType::OR:
        for (const auto& arg : req.args)
        {
            nestedReq = std::get<Requirement>(arg);
            returnStr += formatRequirement(nestedReq);
            if (returnStr.size() > 0)
            {
                returnStr += " or ";
                processedTerms++;
            }
        }
        // chop off the last " or "
        while (returnStr.endsWith(" or "))
        {
            returnStr.chop(4);
            processedTerms--;
        }
        if (!isTopLevel && processedTerms >= 1)
        {
            returnStr.prepend("(");
            returnStr += ")";
        }
        return returnStr;
    case RequirementType::AND:
        for (auto i = 0; i < req.args.size(); i++)
        {
            auto& arg = req.args[i];
            nestedReq = std::get<Requirement>(arg);
            // Shorten the triforce if it appears (kind of stupid we have to do this in 2 places, but oh well)
            if (nestedReq.type == RequirementType::HAS_ITEM && std::get<Item>(nestedReq.args[0]).getGameItemId() == GameItem::TriforceShard1)
            {
                if (req.args.size() > i + 7)
                {
                    auto& lastShard = std::get<Requirement>(req.args[i + 7]);
                    if (lastShard.type == RequirementType::HAS_ITEM && std::get<Item>(lastShard.args[0]).getGameItemId() == GameItem::TriforceShard8)
                    {
                        QString color = TOOLTIP_MET;
                        for (auto j = i; j <= i + 7; j++)
                        {
                            auto& shardArg = std::get<Requirement>(req.args[j]);
                            if (formatRequirement(shardArg).contains(TOOLTIP_UNMET))
                            {
                                color = TOOLTIP_UNMET;
                                break;
                            }
                        }
                        returnStr += "<span style=\"color:" + color + "\">Triforce of Courage</span>";
                        i += 7;
                    }
                }
            }
            else
            {
                returnStr += formatRequirement(nestedReq);
            }
            if (returnStr.size() > 0)
            {
                returnStr += " and ";
                processedTerms++;
            }
        }
        // chop off the last " and "
        while (returnStr.endsWith(" and "))
        {
            returnStr.chop(5);
            processedTerms--;
        }
        if (!isTopLevel && processedTerms >= 1)
        {
            returnStr.prepend("(");
            returnStr += ")";
        }
        return returnStr;
    case RequirementType::HAS_ITEM:
        // Determine if the user has marked this item
        item = std::get<Item>(req.args[0]);
        color = elementInPool(item, mainWindow->trackerInventory) || elementInPool(item, mainWindow->trackerWorlds[0].getStartingItems()) ? TOOLTIP_MET : TOOLTIP_UNMET;
        return "<span style=\"color:" + color + "\">" + prettyTrackerName(item, 1, mainWindow) + "</span>";
    case RequirementType::COUNT:
        expectedCount = std::get<int>(req.args[0]);
        item = std::get<Item>(req.args[1]);
        color = elementCountInPool(item, mainWindow->trackerInventory) + elementCountInPool(item, mainWindow->trackerWorlds[0].getStartingItems()) >= expectedCount ? TOOLTIP_MET : TOOLTIP_UNMET;
        return "<span style=\"color:" + color + "\">" + prettyTrackerName(item, expectedCount, mainWindow) + "</span>";
    default:
        return returnStr;
    }
}
