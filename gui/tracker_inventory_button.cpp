#include "tracker_inventory_button.h"

#include <iostream>

TrackerInventoryButton::TrackerInventoryButton() {}

TrackerInventoryButton::TrackerInventoryButton(const std::vector<TrackerInventoryItem>& itemStates_, QWidget* parent) :
    QPushButton("", parent),
    itemStates(itemStates_)
{
    state = 0;
    setIconSize(QSize(INVENTORY_BUTTON_SIZE, INVENTORY_BUTTON_SIZE));
    updateIcon();
}

void TrackerInventoryButton::updateIcon()
{
    setIcon(QIcon(std::string(DATA_PATH "assets/tracker/" + itemStates[state].filename).c_str()));
}

void TrackerInventoryButton::removeCurrentItem()
{
    removeElementFromPool(*trackerInventory, Item(itemStates[state].gameItem, trackerWorld));
}

void TrackerInventoryButton::addCurrentItem()
{
    addElementToPool(*trackerInventory, Item(itemStates[state].gameItem, trackerWorld));
}

void TrackerInventoryButton::removeAllItems()
{
    for (auto& [gameItem, filename] : itemStates)
    {
        removeElementFromPool(*trackerInventory, Item(gameItem, trackerWorld));
    }
}

void TrackerInventoryButton::addAllItems()
{
    for (auto& [gameItem, filename] : itemStates)
    {
        addElementToPool(*trackerInventory, Item(gameItem, trackerWorld));
    }
}

void TrackerInventoryButton::mouseReleaseEvent(QMouseEvent* e)
{
    if (e->button() == Qt::LeftButton)
    {
        state++;
        if (state >= itemStates.size())
        {
            state = 0;
            removeAllItems();
        }
        addCurrentItem();
    }
    else if (e->button() == Qt::RightButton)
    {
        removeCurrentItem();
        state--;
        if (state < 0)
        {
            state = itemStates.size() - 1;
            addAllItems();
        }
    }
    updateIcon();
}

