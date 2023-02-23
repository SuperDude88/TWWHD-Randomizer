#include "tracker_area_widget.h"

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
}

std::string TrackerAreaWidget::getPrefix() const
{
    return areaPrefix;
}

void TrackerAreaWidget::setBossLocation(Location* bossLoc)
{
    boss = bossLoc;
}

void TrackerAreaWidget::setLocations(LocationPool& locations_)
{
    locations = locations_;
}

void TrackerAreaWidget::setChart(TrackerInventoryButton* chart)
{
    chart->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    chart->setFixedSize(14, 14);
    stackedLayout.insertWidget(0, chart);
}

void TrackerAreaWidget::enterEvent(QEnterEvent* e)
{
    // Update label
}

void TrackerAreaWidget::updateArea()
{
    size_t totalLocations = std::count_if(locations.begin(), locations.end(), [](Location* loc){return !loc->marked;});
    size_t accessibleLocations = std::count_if(locations.begin(), locations.end(), [](Location* loc){return loc->hasBeenFound && !loc->marked;});

    if (totalLocations == 0 && accessibleLocations == 0)
    {
        locationsRemaining.setStyleSheet("color: black;");
    }
    else if (accessibleLocations == 0)
    {
        locationsRemaining.setStyleSheet("color: red;");
    }
    else
    {
        locationsRemaining.setStyleSheet("color: blue;");
    }

    std::string locationsRemainingText = std::to_string(accessibleLocations) + "/" + std::to_string(totalLocations);
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
                              "background-image: url(" DATA_PATH "assets/tracker/" + filename + ".png);"
                              "background-repeat: none;"
                              "background-position: center;").c_str());
}
