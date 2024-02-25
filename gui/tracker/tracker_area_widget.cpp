#include "tracker_area_widget.hpp"

TrackerAreaWidget::TrackerAreaWidget()
{

}

TrackerAreaWidget::TrackerAreaWidget(const std::string& areaPrefix_, TrackerInventoryButton* chart) : areaPrefix(areaPrefix_)
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

void TrackerAreaWidget::setChart(TrackerInventoryButton* chart)
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
    totalAccessibleLocations = std::count_if(areaLocations->at(areaPrefix).begin(), areaLocations->at(areaPrefix).end(), [](Location* loc){return loc->hasBeenFound && !loc->marked;});

    if (totalRemainingLocations == 0 && totalAccessibleLocations == 0)
    {
        locationsRemaining.setStyleSheet("color: black;");
    }
    else if (totalAccessibleLocations == 0)
    {
        locationsRemaining.setStyleSheet("color: red;");
    }
    else
    {
        locationsRemaining.setStyleSheet("color: blue;");
    }

    std::string locationsRemainingText = std::to_string(totalAccessibleLocations) + "/" + std::to_string(totalRemainingLocations);
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

    bossImageWidget.setStyleSheet(std::string(
                              "background-image: url(" DATA_PATH "tracker/" + filename + ".png);"
                              "background-repeat: none;"
                              "background-position: center;").c_str());
}

void TrackerAreaWidget::enterEvent(QEnterEvent* e)
{
    emit mouse_over_area(this);
}

void TrackerAreaWidget::leaveEvent(QEvent* e)
{
    emit mouse_left_area();
}

