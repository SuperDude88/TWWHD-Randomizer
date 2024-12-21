#pragma once

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

class MainWindow;

class TrackerInventoryButton : public QLabel
{
    Q_OBJECT
public:
    TrackerInventoryButton();
    TrackerInventoryButton(const std::vector<TrackerInventoryItem>& itemStates_, QWidget* parent = nullptr, MainWindow* mainWindow_ = nullptr );

    std::vector<TrackerInventoryItem> itemStates = {};
    World* trackerWorld = nullptr;
    ItemPool* trackerInventory = nullptr;
    MainWindow* mainWindow = nullptr;
    QPoint mouseEnterPosition = QPoint();

    void updateIcon();
    void removeCurrentItem();
    void addCurrentItem();
    void removeAllItems();
    void addAllItems();

    void clearForbiddenStates();
    void addForbiddenState(int state);

    int getState();
    void setState(int state_);

signals:
    void inventory_button_pressed();
    void mouse_over_item(const std::string& currentItem);
    void mouse_left_item();

protected:
    void mouseReleaseEvent(QMouseEvent* e) override;
    void mouseMoveEvent(QMouseEvent* e) override;
    void enterEvent(QEnterEvent* e) override;
    void leaveEvent(QEvent* e) override;

private:
    int state = 0;
    std::unordered_set<int> forbiddenStates = {};
};


class TrackerChartButton : public QLabel
{
    Q_OBJECT
public:
    TrackerChartButton(const uint8_t& island, MainWindow* mainWindow_, QWidget* parent = nullptr);

    MainWindow* mainWindow = nullptr;
    QPoint mouseEnterPosition = QPoint();

    void updateIcon();

signals:
    void chart_map_button_pressed(uint8_t islandNum);
    void mouse_over_item(const std::string& currentItem);
    void mouse_left_item();

protected:
    void mouseReleaseEvent(QMouseEvent* e) override;
    void mouseMoveEvent(QMouseEvent* e) override;
    void enterEvent(QEnterEvent* e) override;
    void leaveEvent(QEvent* e) override;

private:
    const uint8_t islandNum;
};
