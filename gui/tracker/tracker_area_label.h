#ifndef TRACKER_AREA_LABEL_H
#define TRACKER_AREA_LABEL_H

#include <QLabel>

class MainWindow;

class TrackerAreaLabel : public QLabel
{
    Q_OBJECT
public:
    TrackerAreaLabel();
    TrackerAreaLabel(const std::string& areaPrefix_);

    std::string areaPrefix = "";

signals:
    void area_label_clicked(std::string areaPrefix_);

protected:
    void mouseReleaseEvent(QMouseEvent* e) override;
};

#endif // TRACKER_AREA_LABEL_H
