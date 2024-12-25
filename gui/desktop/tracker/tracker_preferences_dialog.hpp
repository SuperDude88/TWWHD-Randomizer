#pragma once

#include <ui_tracker_preferences_dialog.h>

#include <QDialog>

class MainWindow;
class TrackerPreferencesDialog : public QDialog
{
    Q_OBJECT

public:
    TrackerPreferencesDialog(MainWindow* main_);

private slots:
    void on_show_location_logic_stateChanged(int arg1);
    void on_show_nonprogress_locations_stateChanged(int arg1);
    void on_right_click_to_clear_all_stateChanged(int arg1);
    void on_clear_all_includes_dungeon_mail_stateChanged(int arg1);
    void on_override_items_background_color_stateChanged(int arg1);
    void on_override_locations_background_color_stateChanged(int arg1);
    void on_override_statistics_background_color_stateChanged(int arg1);
    void on_pick_items_background_color_clicked();
    void on_pick_locations_background_color_clicked();
    void on_pick_statistics_background_color_clicked();
    void on_tracker_save_path_browse_button_clicked();
    void on_tracker_load_path_browse_button_clicked();

private:
    Ui::TrackerPreferencesDialog ui;
    MainWindow* main = nullptr;

protected:
    void closeEvent();
};
