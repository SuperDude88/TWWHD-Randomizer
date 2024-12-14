#include "tracker_area_widget.hpp"

#include <gui/desktop/tracker/tracker_data.hpp>

TrackerAreaWidget::TrackerAreaWidget()
{

}

TrackerAreaWidget::TrackerAreaWidget(const std::string& areaPrefix_, TrackerChartButton* chart) : areaPrefix(areaPrefix_)
{
    QWidget();
    locationsRemaining.areaPrefix = areaPrefix_;
    locationsRemaining.setAlignment(Qt::AlignBottom | Qt::AlignHCenter);
    locationsRemaining.setStyleSheet("color: black");

    setChart(chart);

    stackedLayout.setStackingMode(QStackedLayout::StackAll);
    stackedLayout.addWidget(&locationsRemaining);
}

TrackerAreaWidget::TrackerAreaWidget(const std::string& areaPrefix_,
                                     const std::string& iconFileName_,
                                     TrackerInventoryButton* smallKeys,
                                     TrackerInventoryButton* bigKey,
                                     TrackerInventoryButton* map,
                                     TrackerInventoryButton* compass):
    areaPrefix(areaPrefix_),
    iconFileName(iconFileName_)
{
    QWidget();

    setFixedSize(115, 135);

    locationsRemaining.areaPrefix = areaPrefix_;
    locationsRemaining.setAlignment(Qt::AlignBottom | Qt::AlignHCenter);
    locationsRemaining.setStyleSheet("color: black");

    // Set point size to 15
    auto font = locationsRemaining.font();
    font.setPointSize(15);
    locationsRemaining.setFont(font);


    dungeonItemWidget.setFixedHeight(24);
    dungeonItemLayout.setContentsMargins(0, 0, 0, 0);
    dungeonItemLayout.setAlignment(Qt::AlignTop | Qt::AlignHCenter);
    dungeonItemLayout.setSpacing(0);
    for (auto dungeonItem : {smallKeys, bigKey, map, compass})
    {
        if (dungeonItem)
        {
            dungeonItem->setFixedSize(24, 24);
            dungeonItemLayout.addWidget(dungeonItem);
        }
    }

    stackedLayout.setStackingMode(QStackedLayout::StackAll);
    stackedLayout.addWidget(&dungeonItemWidget);
    stackedLayout.addWidget(&locationsRemaining);
    stackedLayout.addWidget(&bossImageWidget);
    setCursor(Qt::PointingHandCursor);

    updateBossImageWidget();
}

std::string TrackerAreaWidget::getPrefix() const
{
    return areaPrefix;
}

void TrackerAreaWidget::setBossLocation(Location* bossLoc)
{
    boss = bossLoc;
}

void TrackerAreaWidget::setLocations(std::unordered_map<std::string, LocationSet>* areaLocations_)
{
    areaLocations = areaLocations_;
}

void TrackerAreaWidget::setEntrances(std::unordered_map<std::string, std::list<Entrance*>>* areaEntrances_)
{
    areaEntrances = areaEntrances_;
}

void TrackerAreaWidget::setChart(TrackerChartButton* chart)
{
    chart->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    chart->setFixedSize(14, 14);
    stackedLayout.insertWidget(0, chart);
}

void TrackerAreaWidget::updateArea()
{
    // Insert an empty set if this area doesn't exist
    if (!areaLocations->contains(areaPrefix))
    {
        areaLocations->insert({areaPrefix, {}});
    }

    totalRemainingLocations = std::count_if(areaLocations->at(areaPrefix).begin(), areaLocations->at(areaPrefix).end(), [](Location* loc){return !loc->marked;});
    auto remainingProgressLocations = std::count_if(areaLocations->at(areaPrefix).begin(), areaLocations->at(areaPrefix).end(), [](Location* loc){return !loc->marked && loc->progression;});
    totalAccessibleLocations = std::count_if(areaLocations->at(areaPrefix).begin(), areaLocations->at(areaPrefix).end(), [](Location* loc){return loc->hasBeenFound && !loc->marked;});
    auto accessibleNonProgressLocations = std::count_if(areaLocations->at(areaPrefix).begin(), areaLocations->at(areaPrefix).end(), [](Location* loc){return loc->hasBeenFound && !loc->marked && !loc->progression;});
    auto accessibleProgressLocations = totalAccessibleLocations - accessibleNonProgressLocations;

    if (totalRemainingLocations == 0 && totalAccessibleLocations == 0)
    {
        locationsRemaining.setStyleSheet("color: black;");
    }
    else if (totalAccessibleLocations == 0 && showLogic)
    {
        locationsRemaining.setStyleSheet("color: red;");
    }
    else if (showNonprogress && ((accessibleProgressLocations == 0 && showLogic) || (remainingProgressLocations == 0 && !showLogic)))
    {
        locationsRemaining.setStyleSheet("color: olive");
    }
    else
    {
        locationsRemaining.setStyleSheet("color: blue;");
    }

    // Change formatting slightly depending on if the island has
    // undiscovered entrances or not
    int undiscoveredEntrances = !areaEntrances->contains(areaPrefix) ? 0 :
                                    std::count_if(areaEntrances->at(areaPrefix).begin(), areaEntrances->at(areaPrefix).end(), [](Entrance* e){return e->getConnectedArea() == nullptr;});

    std::string locationsRemainingText = "";
    if (showLogic && !undiscoveredEntrances)
    {
        locationsRemainingText = std::to_string(totalAccessibleLocations)+ "/" + std::to_string(totalRemainingLocations);
    }
    else if (showLogic && undiscoveredEntrances)
    {
        locationsRemainingText = std::to_string(totalAccessibleLocations) + "/?";
    }
    else if (!showLogic && undiscoveredEntrances)
    {
        locationsRemainingText = std::to_string(totalRemainingLocations) + "?";
    }
    else
    {
        locationsRemainingText = std::to_string(totalRemainingLocations);
    }
    locationsRemaining.setText(locationsRemainingText.c_str());

    updateBossImageWidget();
}

void TrackerAreaWidget::updateBossImageWidget()
{
    if (iconFileName.empty())
    {
        return;
    }
    std::string filename = (boss != nullptr && boss->marked) ? iconFileName + "_dead" : iconFileName;
    filename += ".png";

    bossImageWidget.setStyleSheet("background-image: url(" + getTrackerAssetPath(filename) + ");"
                                + "background-repeat: none;"
                                + "background-position: center;");
}

void TrackerAreaWidget::updateShowLogic(int show, bool started)
{
    showLogic = show ? true : false;
    if(started)
    {
        updateArea();
    }
}

void TrackerAreaWidget::updateShowNonprogress(int show, bool started)
{
    showNonprogress = show ? true : false;
    if (started)
    {
        updateArea();
    }
}

void TrackerAreaWidget::enterEvent(QEnterEvent* e)
{
    emit mouse_over_area(this);
}

void TrackerAreaWidget::leaveEvent(QEvent* e)
{
    emit mouse_left_area();
}

