#include "tracker_preferences_dialog.hpp"

#include <QColorDialog>
#include <QFileDialog>

#include <../ui_mainwindow.h>
#include <gui/desktop/mainwindow.hpp>

TrackerPreferencesDialog::TrackerPreferencesDialog(MainWindow* main_) : main(main_) {
    ui.setupUi(this);
    ui.show_location_logic->setChecked(main->trackerPreferences.showLocationLogic);
    ui.show_nonprogress_locations->setChecked(main->trackerPreferences.showNonProgressLocations);
    ui.right_click_to_clear_all->setChecked(main->trackerPreferences.rightClickClearAll);
    ui.clear_all_includes_dungeon_mail->setChecked(main->trackerPreferences.clearAllIncludesDungeonMail);
    ui.override_items_background_color->setChecked(main->trackerPreferences.overrideItemsColor);
    ui.override_locations_background_color->setChecked(main->trackerPreferences.overrideLocationsColor);
    ui.override_statistics_background_color->setChecked(main->trackerPreferences.overrideStatsColor);
}

void TrackerPreferencesDialog::on_show_location_logic_stateChanged(int arg1)
{
    auto ui = main->getUI();
    // Only make the locations in logic visible if we're showing logic
    ui->locations_accessible_label->setVisible(arg1);
    ui->locations_accessible_number->setVisible(arg1);

    // Update showing logic for all tracker labels
    for (auto child : ui->tracker_tab->findChildren<TrackerAreaWidget*>())
    {
        child->updateShowLogic(arg1, main->trackerStarted);
    }
    for (auto child : ui->tracker_tab->findChildren<TrackerLabel*>())
    {
        child->updateShowLogic(arg1, main->trackerStarted);
    }

    main->trackerPreferences.showLocationLogic = arg1;
    main->autosave_current_tracker_preferences();
}


void TrackerPreferencesDialog::on_show_nonprogress_locations_stateChanged(int arg1)
{
    // Update showing nonprogress locations for all areas
    for (auto child : main->getUI()->tracker_tab->findChildren<TrackerAreaWidget*>())
    {
        child->updateShowNonprogress(arg1, main->trackerStarted);
    }
    main->trackerPreferences.showNonProgressLocations = arg1;
    main->set_areas_locations();
    main->update_tracker();
}


void TrackerPreferencesDialog::on_right_click_to_clear_all_stateChanged(int arg1)
{
    main->trackerPreferences.rightClickClearAll = arg1;
}


void TrackerPreferencesDialog::on_clear_all_includes_dungeon_mail_stateChanged(int arg1)
{
    main->trackerPreferences.clearAllIncludesDungeonMail = arg1;
}


void TrackerPreferencesDialog::on_override_items_background_color_stateChanged(int arg1)
{
    main->trackerPreferences.overrideItemsColor = arg1;
    main->update_items_color();
}


void TrackerPreferencesDialog::on_override_locations_background_color_stateChanged(int arg1)
{
    main->trackerPreferences.overrideLocationsColor = arg1;
    main->update_locations_color();
}


void TrackerPreferencesDialog::on_override_statistics_background_color_stateChanged(int arg1)
{
    main->trackerPreferences.overrideStatsColor = arg1;
    main->update_stats_color();
}


void TrackerPreferencesDialog::on_pick_items_background_color_clicked()
{
    QColor& itemsColor = main->trackerPreferences.itemsColor;
    QColor color = QColorDialog::getColor(itemsColor, this, "Select color");
    if (!color.isValid()) {
        return;
    }

    itemsColor = color;
    main->update_items_color();
}


void TrackerPreferencesDialog::on_pick_locations_background_color_clicked()
{
    QColor& locationsColor = main->trackerPreferences.locationsColor;
    QColor color = QColorDialog::getColor(locationsColor, this, "Select color");
    if (!color.isValid()) {
        return;
    }

    locationsColor = color;
    main->update_locations_color();
}


void TrackerPreferencesDialog::on_pick_statistics_background_color_clicked()
{
    QColor& statsColor = main->trackerPreferences.statsColor;
    QColor color = QColorDialog::getColor(statsColor, this, "Select color");
    if (!color.isValid()) {
        return;
    }

    statsColor = color;
    main->update_stats_color();
}

void TrackerPreferencesDialog::on_tracker_save_path_browse_button_clicked() {
    QString dir = QFileDialog::getSaveFileName(this, tr("Choose Tracker Save"), QDir::current().absolutePath(), "Tracker Saves (*.yaml)", nullptr, QFileDialog::DontResolveSymlinks);
    if (!dir.isEmpty() && !dir.isNull())
    {
        main->trackerPreferences.autosaveFilePath = Utility::fromQString(dir);
        main->autosave_current_tracker();
    }
}

void TrackerPreferencesDialog::on_tracker_load_path_browse_button_clicked() {
    QString dir = QFileDialog::getOpenFileName(this, tr("Choose Tracker Save"), QDir::current().absolutePath(), "Tracker Saves (*.yaml)", nullptr, QFileDialog::DontResolveSymlinks);
    if (!dir.isEmpty() && !dir.isNull())
    {
        main->trackerPreferences.autosaveFilePath = Utility::fromQString(dir);
        main->autosave_current_tracker_preferences();
        main->load_tracker_autosave();
    }
}
