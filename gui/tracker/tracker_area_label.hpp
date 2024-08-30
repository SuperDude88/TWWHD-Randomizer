#pragma once

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
    void area_label_clicked(const std::string& areaPrefix_);
    void area_label_right_clicked(const std::string& areaPrefix_);

protected:
    void mouseReleaseEvent(QMouseEvent* e) override;
};
