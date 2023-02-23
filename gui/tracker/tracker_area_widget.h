#ifndef TRACKERAREAWIDGET_H
#define TRACKERAREAWIDGET_H

#include <QWidget>
#include <QStackedLayout>
#include <QVBoxLayout>
#include <QLabel>

#include <tracker/tracker_inventory_button.h>
#include <tracker/tracker_area_label.h>

class TrackerAreaWidget : public QWidget
{
    Q_OBJECT
public:
    TrackerAreaWidget();

    // Island Widgets constructor
    TrackerAreaWidget(const std::string& areaPrefix_, TrackerInventoryButton* chart);

    // Other Areas constructor
    TrackerAreaWidget(const std::string& areaPrefix_,
                      const std::string& iconFileName,
                      TrackerInventoryButton* smallKeys,
                      TrackerInventoryButton* bigKey,
                      TrackerInventoryButton* map,
                      TrackerInventoryButton* compass);

    QStackedLayout stackedLayout = QStackedLayout(this);
    TrackerAreaLabel locationsRemaining = TrackerAreaLabel();

    // Stuff for other area widgets
    QWidget dungeonItemWidget = QWidget();
    QWidget bossImageWidget = QWidget();
    QHBoxLayout dungeonItemLayout = QHBoxLayout(&dungeonItemWidget);
    std::string iconFileName = "";
    Location* boss = nullptr;

    std::string getPrefix() const;
    void setBossLocation(Location* bossLoc);
    void setLocations(LocationPool& locations);
    void setChart(TrackerInventoryButton* chart);
    void updateArea();
    void updateBossImageWidget();

    std::string areaPrefix = "";
    LocationPool locations = {};

protected:
    void enterEvent(QEnterEvent* e) override;

};

#endif // TRACKERAREAWIDGET_H
