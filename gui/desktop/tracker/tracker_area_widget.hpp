#pragma once

#include <QWidget>
#include <QStackedLayout>
#include <QVBoxLayout>
#include <QLabel>

#include <gui/desktop/tracker/tracker_inventory_button.hpp>
#include <gui/desktop/tracker/tracker_area_label.hpp>

class TrackerAreaWidget : public QWidget
{
    Q_OBJECT
public:
    TrackerAreaWidget();

    // Island Widgets constructor
    TrackerAreaWidget(const std::string& areaPrefix_, TrackerChartButton* chart);

    // Other Areas constructor
    TrackerAreaWidget(const std::string& areaPrefix_,
                      const std::string& iconFileName,
                      TrackerInventoryButton* smallKeys = nullptr,
                      TrackerInventoryButton* bigKey = nullptr,
                      TrackerInventoryButton* map = nullptr,
                      TrackerInventoryButton* compass = nullptr);

    QStackedLayout stackedLayout = QStackedLayout(this);
    TrackerAreaLabel locationsRemaining = TrackerAreaLabel();

    std::string areaPrefix = "";
    std::unordered_map<std::string, LocationSet>* areaLocations;
    std::unordered_map<std::string, std::list<Entrance*>>* areaEntrances;
    int totalRemainingLocations = 0;
    int totalAccessibleLocations = 0;
    bool showLogic = true;
    bool showNonprogress = false;

    // Stuff for other area widgets
    QWidget dungeonItemWidget = QWidget();
    QWidget bossImageWidget = QWidget();
    QHBoxLayout dungeonItemLayout = QHBoxLayout(&dungeonItemWidget);
    std::string iconFileName = "";
    Location* boss = nullptr;

    std::string getPrefix() const;
    void setBossLocation(Location* bossLoc);
    void setLocations(std::unordered_map<std::string, LocationSet>* locations_);
    void setEntrances(std::unordered_map<std::string, std::list<Entrance*>>* entrances_);
    void setChart(TrackerChartButton* chart);
    void updateArea();
    void updateBossImageWidget();
    void updateShowLogic(int show, bool started);
    void updateShowNonprogress(int show, bool started);

signals:
    void mouse_over_area(TrackerAreaWidget* areaPrefix);
    void mouse_left_area();

protected:
    void enterEvent(QEnterEvent* e) override;
    void leaveEvent(QEvent* e) override;

};
