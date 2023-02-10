#ifndef TRACKERINVENTORYBUTTON_H
#define TRACKERINVENTORYBUTTON_H

#include <QPushButton>
#include <QMouseEvent>
#include <logic/World.hpp>

#define INVENTORY_BUTTON_SIZE 50

struct TrackerInventoryItem {
    GameItem gameItem;
    std::string filename;
};

class TrackerInventoryButton : public QPushButton {
public:
    TrackerInventoryButton();
    TrackerInventoryButton(const std::vector<TrackerInventoryItem>& itemStates_, QWidget* parent = nullptr );

    std::vector<TrackerInventoryItem> itemStates;
    int state = 0;
    World* trackerWorld;
    ItemPool* trackerInventory;

    void updateIcon();
    void removeCurrentItem();
    void addCurrentItem();
    void removeAllItems();
    void addAllItems();
protected:
    void mouseReleaseEvent(QMouseEvent* e) override;
};


#endif // TRACKERINVENTORYBUTTON_H
