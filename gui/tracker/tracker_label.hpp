#pragma once

#include <QLabel>
#include <logic/Location.hpp>
#include <logic/Entrance.hpp>

enum struct TrackerLabelType
{
    NONE = 0,
    Location,
    EntranceSource,
    EntranceDestination
};

class TrackerLabel : public QLabel
{
    Q_OBJECT
public:
    TrackerLabel();
    TrackerLabel(TrackerLabelType type_, int pointSize, Location* location = nullptr, Entrance* entrance = nullptr);

    void set_location(Location* loc);
    Location* get_location() const;
    void set_entrance(Entrance* entrance_);
    Entrance* get_entrance() const;
    void update_colors();
    void mark_location();

signals:
    void location_label_clicked();
    void entrance_source_label_clicked(Entrance* entrance);
    void entrance_destination_label_clicked(Entrance* target);
    void entrance_source_label_disconnect(Entrance* entrance);
    void mouse_over_entrance_label(Entrance* entrance);
    void mouse_left_entrance_label();

protected:
    void mouseReleaseEvent(QMouseEvent* e) override;
    void enterEvent(QEnterEvent* e) override;
    void leaveEvent(QEvent* e) override;

private:
    TrackerLabelType type = TrackerLabelType::NONE;
    Location* location = nullptr;
    Entrance* entrance = nullptr;
};
