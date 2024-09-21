#include "tracker_inventory_button.hpp"

#include <gui/tracker/tracker_data.hpp>
#include <gui/mainwindow.hpp>

#include <QToolTip>

#include <iostream>

TrackerInventoryButton::TrackerInventoryButton() {}

TrackerInventoryButton::TrackerInventoryButton(const std::vector<TrackerInventoryItem>& itemStates_, QWidget* parent, bool onlyText_, MainWindow* mainWindow_) :
    QLabel("", parent),
    itemStates(itemStates_),
    onlyText(onlyText_),
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

    // Set text if this is a text only button as well as mouse tracking
    if (onlyText)
    {
        setMouseTracking(true);
        setText(itemStates[state].trackerLabelStr.c_str());
    }
}

void TrackerInventoryButton::updateIcon()
{
    // If this is a text button (currently only on the chart list)
    // then style it's colors properly
    if (onlyText)
    {
        if (itemStates[state].gameItem == GameItem::NOTHING)
        {
            setStyleSheet("color: blue;");
        }
        else
        {
            setStyleSheet("color: black; text-decoration: line-through;");
        }
    }
    else
    {
        setStyleSheet("background-image: url(" + getTrackerAssetPath(itemStates[state].filename) + ");"
                    + "background-repeat: none;"
                    + "background-position: center;");
    }
}

void TrackerInventoryButton::removeCurrentItem()
{
    if (itemStates[state].gameItem != GameItem::NOTHING && !forbiddenStates.contains(state - 1) && !onChartListWhenRandomCharts())
    {
        removeElementFromPool(*trackerInventory, Item(itemStates[state].gameItem, trackerWorld));
    }
}

void TrackerInventoryButton::addCurrentItem()
{
    if (itemStates[state].gameItem != GameItem::NOTHING && !forbiddenStates.contains(state - 1) && !onChartListWhenRandomCharts())
    {
        addElementToPool(*trackerInventory, Item(itemStates[state].gameItem, trackerWorld));
    }
}

void TrackerInventoryButton::removeAllItems()
{
    for (size_t i = 0; i < itemStates.size(); i++)
    {
        if (forbiddenStates.contains(i-1) || onChartListWhenRandomCharts())
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
        if (forbiddenStates.contains(i-1) || onChartListWhenRandomCharts())
        {
            continue;
        }

        auto& [gameItem, filename, trackerLabelStr] = itemStates[i];
        addElementToPool(*trackerInventory, Item(gameItem, trackerWorld));
    }
}

void TrackerInventoryButton::clearForbiddenStates() {
    forbiddenStates.clear();

    updateDuplicates();
}

void TrackerInventoryButton::addForbiddenState(int state)
{
    forbiddenStates.insert(state);

    updateDuplicates();
}

int TrackerInventoryButton::getState() {
    return state;
}

void TrackerInventoryButton::setState(int state_) {
    this->state = state_;

    updateDuplicates();
}

bool TrackerInventoryButton::onChartListWhenRandomCharts()
{
    return onlyText && mainWindow && mainWindow->trackerSettings.randomize_charts && Item(itemStates[1].gameItem, nullptr).isChartForSunkenTreasure();
}

void TrackerInventoryButton::showChartTooltip()
{
    auto coords = mapToGlobal(QPoint(mouseEnterPosition.x() + 30, height() - 15));
    auto text = std::string("Chart leads to<br><b>") + roomIndexToIslandName(chartToRoomIndex(itemStates[1].gameItem)) + "</b>";
    QToolTip::showText(coords, text.c_str());
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
    if (!onChartListWhenRandomCharts())
    {
        emit mouse_over_item(itemStates[state].trackerLabelStr);
    }

    updateDuplicates();
}

void TrackerInventoryButton::mouseMoveEvent(QMouseEvent* e)
{
    if (onlyText && Item(itemStates[1].gameItem, nullptr).isChartForSunkenTreasure() && mainWindow && !mainWindow->trackerSettings.randomize_charts)
    {
        showChartTooltip();
    }
}

void TrackerInventoryButton::enterEvent(QEnterEvent* e)
{
    mouseEnterPosition = e->position().toPoint();
    if (!onChartListWhenRandomCharts())
    {
        emit mouse_over_item(itemStates[state].trackerLabelStr);
    }
}

void TrackerInventoryButton::leaveEvent(QEvent* e)
{
    emit mouse_over_item("");
}

void TrackerInventoryButton::updateDuplicates() {
    // Update duplicate buttons to the proper state. This is
    // mainly just for charts on the chart list and their
    // corresponding chart buttons on the map
    for (auto duplicate : duplicates)
    {
        if (!duplicate->onChartListWhenRandomCharts() && !onChartListWhenRandomCharts())
        {
            duplicate->state = state;
            duplicate->updateIcon();
        }
    }
}
