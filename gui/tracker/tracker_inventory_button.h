#ifndef TRACKERINVENTORYBUTTON_H
#define TRACKERINVENTORYBUTTON_H

#include <QPushButton>
#include <QLabel>
#include <QMouseEvent>
#include <logic/World.hpp>

#define INVENTORY_BUTTON_SIZE 50

struct TrackerInventoryItem {
    GameItem gameItem = GameItem::NOTHING;
    std::string filename = "";
    std::string trackerLabelStr = "";
};

class TrackerInventoryButton : public QLabel
{
    Q_OBJECT
public:
    TrackerInventoryButton();
    TrackerInventoryButton(const std::vector<TrackerInventoryItem>& itemStates_, QWidget* parent = nullptr );

    std::vector<TrackerInventoryItem> itemStates = {};
    int state = 0;
    std::unordered_set<int> forbiddenStates = {};
    World* trackerWorld = nullptr;
    ItemPool* trackerInventory = nullptr;

    void updateIcon();
    void removeCurrentItem();
    void addCurrentItem();
    void removeAllItems();
    void addAllItems();

    void addForbiddenState(int state);

signals:
    void inventory_button_pressed();
    void mouse_over_item(const std::string& currentItem);
    void mouse_left_item();

protected:
    void mouseReleaseEvent(QMouseEvent* e) override;
    void enterEvent(QEnterEvent* e) override;
    void leaveEvent(QEvent* e) override;
};


#endif // TRACKERINVENTORYBUTTON_H
