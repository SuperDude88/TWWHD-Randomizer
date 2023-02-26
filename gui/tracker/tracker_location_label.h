#ifndef TRACKERLOCATIONLABEL_H
#define TRACKERLOCATIONLABEL_H

#include <QLabel>
#include <logic/Location.hpp>

class TrackerLocationLabel : public QLabel
{
    Q_OBJECT
public:
    TrackerLocationLabel();
    TrackerLocationLabel(int pointSize);

    void set_location(Location* loc);
    Location* get_location() const;
    void update_colors();
    void mark_location();

signals:
    void location_label_clicked();

protected:
    void mouseReleaseEvent(QMouseEvent* e) override;

private:
    Location* location = nullptr;
};

#endif // TRACKERLOCATIONLABEL_H
