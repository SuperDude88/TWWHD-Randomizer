#pragma once

#include <QCheckBox>

class Location;

class RequiredBossCheckBox : public QCheckBox
{
    Q_OBJECT
public:
    RequiredBossCheckBox(const QString& bossLocationName_, const QString& bossName_, QWidget* parent);

    QString getBossLocationName () const { return bossLocationName; }
    QString getBossName() const { return bossName; }
signals:
    void setRequiredBoss(const QString& bossName_, Qt::CheckState checked);

private:
    QString bossLocationName = "";
    QString bossName = "";

private slots:
    void emit_required_boss_signal();


};

