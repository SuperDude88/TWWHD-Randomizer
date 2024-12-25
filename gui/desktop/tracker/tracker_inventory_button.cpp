#include "tracker_inventory_button.hpp"

#include <gui/desktop/tracker/tracker_data.hpp>
#include <gui/desktop/mainwindow.hpp>

#include <QToolTip>

TrackerInventoryButton::TrackerInventoryButton() {}

TrackerInventoryButton::TrackerInventoryButton(const std::vector<TrackerInventoryItem>& itemStates_, QWidget* parent, MainWindow* mainWindow_) :
    QLabel("", parent),
    itemStates(itemStates_),
    mainWindow(mainWindow_)
{
    state = 0;
    setCursor(Qt::PointingHandCursor);
    updateIcon();

    // Set appropriate tracker label state names
    for (size_t i = itemStates.size(); i > 0; i--)
    {
        auto& itemState = itemStates[i - 1];
        if (itemState.trackerLabelStr == "")
        {
            if (itemState.gameItem == GameItem::NOTHING)
            {
                itemState.trackerLabelStr = itemStates[i].trackerLabelStr;
            }
            else
            {
                itemState.trackerLabelStr = gameItemToName(itemState.gameItem);
            }
        }
    }
}

void TrackerInventoryButton::updateIcon()
{
    setStyleSheet("background-image: url(" + getTrackerAssetPath(itemStates[state].filename) + ");"
                + "background-repeat: none;"
                + "background-position: center;");
}

void TrackerInventoryButton::removeCurrentItem()
{
    if (itemStates[state].gameItem != GameItem::NOTHING && !forbiddenStates.contains(state - 1))
    {
        removeElementFromPool(*trackerInventory, Item(itemStates[state].gameItem, trackerWorld));
    }
}

void TrackerInventoryButton::addCurrentItem()
{
    if (itemStates[state].gameItem != GameItem::NOTHING && !forbiddenStates.contains(state - 1))
    {
        addElementToPool(*trackerInventory, Item(itemStates[state].gameItem, trackerWorld));
    }
}

void TrackerInventoryButton::removeAllItems()
{
    for (size_t i = 0; i < itemStates.size(); i++)
    {
        if (forbiddenStates.contains(i-1))
        {
            continue;
        }

        auto& [gameItem, filename, trackerLabelStr] = itemStates[i];
        removeElementFromPool(*trackerInventory, Item(gameItem, trackerWorld));
    }
}

void TrackerInventoryButton::addAllItems()
{
    for (size_t i = 0; i < itemStates.size(); i++)
    {
        if (forbiddenStates.contains(i-1))
        {
            continue;
        }

        auto& [gameItem, filename, trackerLabelStr] = itemStates[i];
        addElementToPool(*trackerInventory, Item(gameItem, trackerWorld));
    }
}

void TrackerInventoryButton::clearForbiddenStates() {
    forbiddenStates.clear();
}

void TrackerInventoryButton::addForbiddenState(int state)
{
    forbiddenStates.insert(state);
}

int TrackerInventoryButton::getState() {
    return state;
}

void TrackerInventoryButton::setState(int state_) {
    this->state = state_;
}

void TrackerInventoryButton::mouseReleaseEvent(QMouseEvent* e)
{
    if (e->button() == Qt::LeftButton)
    {
        do
        {
            state++;
            if (state >= itemStates.size())
            {
                state = 0;
                removeAllItems();
            }
            addCurrentItem();
        }
        while (forbiddenStates.contains(state));
    }
    else if (e->button() == Qt::RightButton)
    {
        do
        {
            removeCurrentItem();
            state--;
            if (state < 0)
            {
                state = itemStates.size() - 1;
                addAllItems();
            }
        }
        while (forbiddenStates.contains(state));
    }
    updateIcon();
    emit inventory_button_pressed();
    emit mouse_over_item(itemStates[state].trackerLabelStr);
}

void TrackerInventoryButton::mouseMoveEvent(QMouseEvent* e)
{}

void TrackerInventoryButton::enterEvent(QEnterEvent* e)
{
    mouseEnterPosition = e->position().toPoint();
    emit mouse_over_item(itemStates[state].trackerLabelStr);
}

void TrackerInventoryButton::leaveEvent(QEvent* e)
{
    emit mouse_over_item("");
}

TrackerChartButton::TrackerChartButton(const uint8_t& island, MainWindow* mainWindow_, QWidget* parent) :
    QLabel("", parent),
    islandNum(island),
    mainWindow(mainWindow_)
{
    setCursor(Qt::PointingHandCursor);
    updateIcon();
}

void TrackerChartButton::updateIcon()
{
    std::string chartState = "treasure_chart_closed.png";
    if(mainWindow->trackerSettings.randomize_charts) {
        if(const GameItem chart = mainWindow->chartForIsland(islandNum); chart != GameItem::INVALID) {
            if(chart == GameItem::TriforceChart1 || chart == GameItem::TriforceChart2 || chart == GameItem::TriforceChart3) {
                chartState = "triforce_chart_open.png";
            }
            else {
                chartState = "treasure_chart_open.png";
            }
        }
    }
    else if(const GameItem chart = roomNumToDefaultChart(islandNum); elementInPool(Item(chart, &mainWindow->trackerWorlds[0]), mainWindow->trackerInventory)) {
        if(chart == GameItem::TriforceChart1 || chart == GameItem::TriforceChart2 || chart == GameItem::TriforceChart3) {
            chartState = "triforce_chart_open.png";
        }
        else {
            chartState = "treasure_chart_open.png";
        }
    }
    else {
        if(chart == GameItem::TriforceChart1 || chart == GameItem::TriforceChart2 || chart == GameItem::TriforceChart3) {
            chartState = "triforce_chart_closed.png";
        }
    }

    setStyleSheet("background-image: url(" + getTrackerAssetPath(chartState) + ");"
                + "background-repeat: none;"
                + "background-position: center;");
}

void TrackerChartButton::mouseReleaseEvent(QMouseEvent* e)
{
    if (e->button() == Qt::LeftButton)
    {
        emit chart_map_button_pressed(islandNum);
    }
}

void TrackerChartButton::mouseMoveEvent(QMouseEvent* e)
{}

void TrackerChartButton::enterEvent(QEnterEvent* e)
{
    mouseEnterPosition = e->position().toPoint();
    if(mainWindow->trackerSettings.randomize_charts) {
        if(!mainWindow->isIslandMappedToChart(islandNum)) {
            emit mouse_over_item("Chart for " + roomNumToIslandName(islandNum));
        }
        else {
            emit mouse_over_item(gameItemToName(mainWindow->chartForIsland(islandNum)) + " -> " + roomNumToIslandName(islandNum));
        }
    }
    else {
        emit mouse_over_item(gameItemToName(mainWindow->chartForIsland(islandNum)));
    }
}

void TrackerChartButton::leaveEvent(QEvent* e)
{
    emit mouse_over_item("");
}
