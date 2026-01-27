#include "tracker_required_boss_checkbox.hpp"

#include <gui/desktop/tracker/set_font.hpp>

RequiredBossCheckBox::RequiredBossCheckBox(const QString& bossLocationName_, const QString& bossName_, QWidget* parent) :
    QCheckBox{"Required", parent},
    bossLocationName{bossLocationName_},
    bossName{bossName_}
{
    setContentsMargins(4, 0, 0, 0);
    set_font(this, "Fira Sans", 11);
    connect(this, &QCheckBox::checkStateChanged, this, &RequiredBossCheckBox::emit_required_boss_signal);
}

void RequiredBossCheckBox::emit_required_boss_signal()
{
    emit setRequiredBoss(bossName, checkState());
}
