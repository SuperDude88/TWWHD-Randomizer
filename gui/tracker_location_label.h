#ifndef TRACKERLOCATIONLABEL_H
#define TRACKERLOCATIONLABEL_H

#include <QLabel>

#include <logic/Location.hpp>

class TrackerLocationLabel : public QLabel
{
public:
    TrackerLocationLabel();
    TrackerLocationLabel(Location* location_);

    Location* location;

    void update_colors();

protected:
    void mouseReleaseEvent(QMouseEvent* e) override;
};

#endif // TRACKERLOCATIONLABEL_H
