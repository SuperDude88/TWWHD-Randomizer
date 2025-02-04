#pragma once

#include <QLabel>
#include <QPushButton>
#include <logic/Location.hpp>
#include <logic/Entrance.hpp>

enum struct TrackerLabelType
{
    NONE = 0,
    Location,
    EntranceSource,
    EntranceDestination,
    Chart
};

class MainWindow;

class TrackerLabel : public QLabel
{
    Q_OBJECT
public:
    TrackerLabel() = default;
    TrackerLabel(TrackerLabelType type_, int pointSize, MainWindow* mainWindow = nullptr, Location* location = nullptr, Entrance* entrance = nullptr, GameItem chart_ = GameItem::INVALID);

    void set_location(Location* loc);
    Location* get_location() const;
    void set_entrance(Entrance* entrance_);
    Entrance* get_entrance() const;
    void update_entrance_text();
    void set_disconnect_button(QPushButton* button);
    QPushButton* get_disconnect_button() const;
    void set_chart(GameItem chart_);
    GameItem get_chart() const;
    void update_colors();
    void mark_location();
    void updateShowLogic(int show, bool started);
    void showLogicTooltip();
    QString getTooltipText();
    QString formatRequirement(const Requirement& req, const bool& isTopLevel = false);
    QString formatEntrancePath(const EntrancePath& path, const QString& headerText = "Entrance Path");
    QString getUsefulInformationText();
    void showAll();
    void hideAll();

signals:
    void location_label_clicked();
    void mouse_over_location_label(Location* location);
    void mouse_left_location_label();
    void entrance_source_label_clicked(Entrance* entrance);
    void entrance_destination_label_clicked(Entrance* target);
    void entrance_source_label_disconnect(Entrance* entrance);
    void mouse_over_entrance_label(Entrance* entrance);
    void mouse_left_entrance_label();
    void chart_label_clicked(TrackerLabel* label, GameItem chart);
    void mouse_over_chart_label(const std::string& currentItem);
    void mouse_left_chart_label();

protected:
    void mouseReleaseEvent(QMouseEvent* e) override;
    void mouseMoveEvent(QMouseEvent* e) override;
    void enterEvent(QEnterEvent* e) override;
    void leaveEvent(QEvent* e) override;

private:
    TrackerLabelType type = TrackerLabelType::NONE;
    MainWindow* mainWindow = nullptr;
    Location* location = nullptr;
    Entrance* entrance = nullptr;
    GameItem chart = GameItem::INVALID;
    bool showLogic = true;
    QPoint mouseEnterPosition = QPoint();
    QPushButton* disconnectButton = nullptr;
};
