#include "tracker_inventory_button.h"

#include <iostream>

TrackerInventoryButton::TrackerInventoryButton() {}

TrackerInventoryButton::TrackerInventoryButton(const std::vector<TrackerInventoryItem>& itemStates_, QWidget* parent) :
    QLabel("", parent),
    itemStates(itemStates_)
{
    state = 0;
    updateIcon();
}

void TrackerInventoryButton::updateIcon()
{
    setStyleSheet((std::string("background-image: url(" DATA_PATH "assets/tracker/" + itemStates[state].filename + ");"
                               "background-repeat: none;"
                               "background-position: center;").c_str()));
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

        auto& [gameItem, filename] = itemStates[i];
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

        auto& [gameItem, filename] = itemStates[i];
        addElementToPool(*trackerInventory, Item(gameItem, trackerWorld));
    }
}

void TrackerInventoryButton::addForbiddenState(int state)
{
    forbiddenStates.insert(state);
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
}

