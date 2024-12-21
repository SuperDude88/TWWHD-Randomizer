#include "tracker_label.hpp"

#include <gui/desktop/tracker/set_font.hpp>
#include <gui/desktop/mainwindow.hpp>

#include <logic/Area.hpp>

#include <QMouseEvent>
#include <QToolTip>

#ifdef __APPLE__
#define TOOLTIP_UNMET "orange"
#else
#define TOOLTIP_UNMET "red"
#endif
#define TOOLTIP_MET "dodgerblue"

TrackerLabel::TrackerLabel(TrackerLabelType type_, int pointSize, MainWindow* mainWindow_, Location* location_, Entrance* entrance_, GameItem chart_) : type(type_), mainWindow(mainWindow_)
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

    case TrackerLabelType::Chart:
        setMinimumHeight(15);
        setMaximumWidth(345);
        set_chart(chart_);
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
    update_entrance_text();
}

Entrance* TrackerLabel::get_entrance() const
{
    return entrance;
}

void TrackerLabel::update_entrance_text()
{
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

void TrackerLabel::set_disconnect_button(QPushButton* button)
{
    disconnectButton = button;
}

QPushButton* TrackerLabel::get_disconnect_button() const
{
    return disconnectButton;
}

void TrackerLabel::set_chart(GameItem chart_) {
    chart = chart_;
    setText(gameItemToName(chart_).c_str());
    update_colors();
}

GameItem TrackerLabel::get_chart() const {
    return chart;
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
    case TrackerLabelType::Chart:
        emit chart_label_clicked(this, chart);
    default:
        break;
    }
}

void TrackerLabel::mouseMoveEvent(QMouseEvent* e)
{
    switch(type)
    {
    case TrackerLabelType::Location:
    case TrackerLabelType::EntranceSource:
        break;
    default:
        break;
    }
}

void TrackerLabel::enterEvent(QEnterEvent* e)
{
    mouseEnterPosition = e->position().toPoint();
    switch(type) {
        case TrackerLabelType::Location:
            emit mouse_over_location_label(location);
            showLogicTooltip();
            break;
        case TrackerLabelType::EntranceSource:
            showLogicTooltip();
        case TrackerLabelType::EntranceDestination:
            emit mouse_over_entrance_label(entrance);
            break;
        case TrackerLabelType::Chart:
            emit mouse_over_chart_label(gameItemToName(chart));
            showLogicTooltip();
            break;
        case TrackerLabelType::NONE:
        default:
            break;
    }
}

void TrackerLabel::leaveEvent(QEvent* e)
{
    QToolTip::hideText();
    switch(type) {
        case TrackerLabelType::Location:
            emit mouse_left_location_label();
            break;
        case TrackerLabelType::EntranceSource:
        case TrackerLabelType::EntranceDestination:
            emit mouse_left_entrance_label();
            break;
        case TrackerLabelType::Chart:
            emit mouse_left_chart_label();
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
        else if (!entrance->hasBeenFound() && showLogic)
        {
            setStyleSheet("color: red;");
        }
        else
        {
            setStyleSheet("color: blue;");
        }
        break;
    case TrackerLabelType::EntranceDestination:
        setStyleSheet("color: black;");
        break;
    case TrackerLabelType::Chart:
        if (mainWindow->isMappingChart())
        {
            if(mainWindow->isChartMapped(chart)) {
                setStyleSheet("color: black; text-decoration: line-through;");
            }
            else if(elementInPool(Item(chart, &mainWindow->trackerWorlds[0]), mainWindow->trackerInventory)) {
                setStyleSheet("color: blue;");
            }
            else {
                setStyleSheet("color: red;");
            }
        }
        else if(elementInPool(Item(chart, &mainWindow->trackerWorlds[0]), mainWindow->trackerInventory)) {
            setStyleSheet("color: black; text-decoration: line-through;");
        }
        else {
            setStyleSheet("color: blue;");
        }

        break;
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
    // Don't show tooltips if logic isn't being shown (unless this is a chart mapped to an island)
    if ((type != TrackerLabelType::Chart && !showLogic) || (type == TrackerLabelType::Chart && !mainWindow->isChartMapped(chart)))
    {
        return;
    }
    // Use the position where the mouse entered the label to know
    // where to display the tooltip.
    auto coords = mapToGlobal(QPoint(mouseEnterPosition.x() + 30, height() - 15));
    QToolTip::showText(coords, getTooltipText(), nullptr, {}, 120000); // Set timer for 2 minutes
}

QString TrackerLabel::getTooltipText()
{
    // Calling QToolTip::showText with the same text that was
    // previously used won't update the position of the tooltip.
    // This can be an issue for consecutive labels that have
    // the same requirement text, so everytime we generate
    // new requirement text, we'll alternate the bottom margin
    // between 0 and 1 so that the tooltip text is always different
    static QString marginBottom = "0";
    marginBottom = (marginBottom == "0") ? "1" : "0";

    QString returnStr = "";

    // First, list the entrance path for this location if there is one
    EntrancePath entrancePath = {{}, EntrancePath::Logicality::None};
    auto& currentArea = mainWindow->currentTrackerArea;
    if(type == TrackerLabelType::Chart)
    {
        returnStr = "Chart leads to:<ul style=\"margin-top: 0px; margin-bottom: " + marginBottom + "px; margin-left: 8px; margin-right: 0px; -qt-list-indent:0;\"><li>";
        returnStr += roomNumToIslandName(mainWindow->islandForChart(chart));
        return returnStr;
    }
    else if (type == TrackerLabelType::Location)
    {
        if (mainWindow->entrancePathsByLocation.contains(location) && currentArea != "")
        {
            // By default, show whatever the shortest logical path to
            // the location is
            entrancePath = mainWindow->entrancePathsByLocation[location];
            auto& entrancePaths = mainWindow->entrancePaths[currentArea];
            for (auto locAcc : location->accessPoints)
            {
                // If there's a better path that we can show, use that one
                // instead
                if (entrancePaths.contains(locAcc->area))
                {
                    auto& newPath = entrancePaths[locAcc->area];
                    if (newPath.isBetterThan(entrancePath, currentArea))
                    {
                        entrancePath = newPath;
                    }
                }
            }
        }
    }
    else if (type == TrackerLabelType::EntranceSource)
    {
        // If we have a source entrance, get the entrance path
        // to the area this entrance is in
        auto area = entrance->getParentArea();
        // Iteratively compare all the paths so that we end
        // up with the shortest, most logical one
        for (auto& [region, paths] : mainWindow->entrancePaths)
        {
            // If this entrance path is better than the current one, set it
            if (paths.contains(area) && paths[area].isBetterThan(entrancePath, currentArea))
            {
                entrancePath = paths[area];
            }
        }
    }

    // Get the formatted entrance path
    returnStr += formatEntrancePath(entrancePath);

    // Show special text for some locations to remind users of
    // potentially useful information regarding the location
    returnStr += getUsefulInformationText();

    // Get the requirement we're generating text for
    Requirement req;
    switch (type)
    {
    case TrackerLabelType::Location:
        req = location->computedRequirement;
        break;
    case TrackerLabelType::EntranceSource:
        req = entrance->getComputedRequirement();
        break;
    default:
        break;
    }

    // Generate the text for the requirement.
    // If the outermost requirement is an AND,
    // then split up the arguments of the AND.
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

    // Add in item requirements text
    returnStr += "Item Requirements:<ul style=\"margin-top: 0px; margin-bottom: " + marginBottom + "px; margin-left: 8px; margin-right: 0px; -qt-list-indent:0;\"><li>";
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
        else if(str == "<span>RANDOMIZED CHART PLACEHOLDER</span>") {
            const std::string island = mainWindow->trackerWorlds[0].getArea(mainWindow->currentTrackerArea)->island;
            const uint8_t islandNum = islandNameToRoomNum(island);
            if(!mainWindow->isIslandMappedToChart(islandNum)) {
                str = std::string("<span style=\"color:" TOOLTIP_UNMET "\">Chart for " + island + "</span>").c_str();
            }
            else {
                str = std::string("<span style=\"color:" TOOLTIP_MET "\">" + gameItemToName(mainWindow->chartForIsland(islandNum)) + " -> " + island + "</span>").c_str();
            }
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

// Format an item requirement into bullet points
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

        // Special case for randomized charts: this requires some knowledge of the island this requirement is for
        // so put a placeholder we can handle higher in the tooltip code (in TrackerLabel::getTooltipText)
        // There's probably better ways to do this, but this was easy enough for now
        if(mainWindow->trackerSettings.randomize_charts && item.isChartForSunkenTreasure()) {
            return "<span>RANDOMIZED CHART PLACEHOLDER</span>";
        }

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

// Format an entrance path into bullet points
QString TrackerLabel::formatEntrancePath(const EntrancePath& path, const QString& headerText /*= "Entrance Path"*/)
{
    QString returnStr = "";
    if (!path.list.empty())
    {
        returnStr = headerText + ":<ul style=\"margin-top: 0px; margin-bottom: 0px; margin-left: 8px; margin-right: 0px; -qt-list-indent:0;\"><li>";
        auto counter = 0;
        for (auto e : path.list)
        {
            counter++;
            returnStr += e->getOriginalName(true).c_str();
            if (counter < path.list.size())
            {
                returnStr += "</li><li>";
            }
        }
        returnStr += "</li></ul><hr>";
    }
    return returnStr;
}

QString TrackerLabel::getUsefulInformationText()
{
    auto& currentArea = mainWindow->currentTrackerArea;
    // Anonymous helper function
    auto formatData = [&](const std::string& noteMsg,
                          std::set<std::string>& areaNames)
    {
        QString formatStr = QString::fromStdString(noteMsg);
        for (const auto& areaName : areaNames)
        {
            // Loop through and find the best path to this area
            EntrancePath bestPath = {{}, EntrancePath::Logicality::None};
            auto pathArea = mainWindow->trackerWorlds[0].getArea(areaName);
            for (auto& [area, paths] : mainWindow->entrancePaths)
            {
                if (paths.contains(pathArea) && paths[pathArea].isBetterThan(bestPath, currentArea))
                {
                    bestPath = paths[pathArea];
                }
            }

            // Don't display an entrance path if we didn't find any
            if (bestPath.list.size() > 0 || bestPath.logicality > EntrancePath::Logicality::None)
            {
                formatStr += formatEntrancePath(bestPath, std::string("Entrance Path to " + areaName).c_str());
            }
        }

        return formatStr;
    };

    QString returnStr = "";
    if (type == TrackerLabelType::Location && location && location->trackerNote != "")
    {
        returnStr += formatData(location->trackerNote, location->trackerNoteAreas);
    }
    return returnStr;
}

// Will show this widget as well as the disconnect button if it
// has one
void TrackerLabel::showAll()
{
    this->setVisible(true);
    // Only show the disconnect button if we have an entrance to disconnect
    if (disconnectButton && entrance && entrance->getReplaces())
    {
        disconnectButton->setVisible(true);
    }
}

// Will hide this widget as well as the disconnect button if it
// has one
void TrackerLabel::hideAll()
{
    this->setVisible(false);
    if (disconnectButton)
    {
        disconnectButton->setVisible(false);
    }
}
