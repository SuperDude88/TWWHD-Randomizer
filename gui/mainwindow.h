#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListView>
#include <QPointer>
#include <QProgressDialog>
#include <QString>
#include <QStringListModel>
#include <QStringList>
#include <QStandardItemModel>

#include <filesystem>

#include <seedgen/config.hpp>
#include <logic/Location.hpp>

void delete_and_create_default_config();

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void show_warning_dialog(const std::string& s, const std::string& title = "");
    void show_info_dialog(const std::string& s, const std::string& title = "");

protected:
    bool eventFilter(QObject *object, QEvent *event) override;

private:
    Ui::MainWindow *ui;
    Config config;
    QStringListModel* randomizedGearModel;
    QStringListModel* startingGearModel;

    // Variables for setting the mixed pools
    // combobox as we want
    QList<QStandardItem*> poolCheckBoxes;
    QStringList poolNames;
    QStandardItemModel poolModel;
    QListView eventsByName;
    QStandardItem mix_dungeons;
    QStandardItem mix_caves;
    QStandardItem mix_doors;
    QStandardItem mix_misc;

    std::string defaultWindowTitle;
    QString currentPermalink;
    bool encounteredError;
    std::list<std::set<LocationCategory>> locationCategories = {};

    void closeEvent(QCloseEvent *event) override;
    void load_config_into_ui();
    void setup_gear_menus();
    void setup_mixed_pools_combobox();
    void apply_config_settings();
    void update_progress_locations_text();
    void swap_selected_gear(QListView* gearFrom, QStringListModel* gearTo);
    void update_starting_gear();
    void update_plandomizer_widget_visbility();
    void update_starting_health_text();
    void update_option_description_text(const std::string& descrption = "");
    void update_permalink();
    void update_encryption_files();
    void load_locations();

private slots:
    void show_error_dialog(const std::string& s, const std::string& title = "An error has occured!");
    void update_mixed_pools_combobox_text(const QString& text);
    void update_mixed_pools_combobox_option(const QString& text = "");
    void update_mixed_pools_on_text_click(const QModelIndex&);
    void on_base_game_path_browse_button_clicked();
    void on_output_folder_browse_button_clicked();
    void on_generate_seed_button_clicked();

    // Progression Locations
    void on_progression_battlesquid_stateChanged(int arg1);
    void on_progression_big_octos_gunboats_stateChanged(int arg1);
    void on_progression_combat_secret_caves_stateChanged(int arg1);
    void on_progression_dungeons_stateChanged(int arg1);
    void on_progression_expensive_purchases_stateChanged(int arg1);
    void on_progression_eye_reef_chests_stateChanged(int arg1);
    void on_progression_free_gifts_stateChanged(int arg1);
    void on_progression_great_fairies_stateChanged(int arg1);
    void on_progression_island_puzzles_stateChanged(int arg1);
    void on_progression_long_sidequests_stateChanged(int arg1);
    void on_progression_mail_stateChanged(int arg1);
    void on_progression_minigames_stateChanged(int arg1);
    void on_progression_misc_stateChanged(int arg1);
    void on_progression_obscure_stateChanged(int arg1);
    void on_progression_platforms_rafts_stateChanged(int arg1);
    void on_progression_puzzle_secret_caves_stateChanged(int arg1);
    void on_progression_savage_labyrinth_stateChanged(int arg1);
    void on_progression_short_sidequests_stateChanged(int arg1);
    void on_progression_spoils_trading_stateChanged(int arg1);
    void on_progression_submarines_stateChanged(int arg1);
    void on_progression_tingle_chests_stateChanged(int arg1);
    void on_progression_treasure_charts_stateChanged(int arg1);
    void on_progression_triforce_charts_stateChanged(int arg1);

    // Additional Randomization Options
    void on_sword_mode_currentIndexChanged(int index);
    void on_randomize_charts_stateChanged(int arg1);
    void on_chest_type_matches_contents_stateChanged(int arg1);
    void on_damage_multiplier_valueChanged(int multiplier);
    void on_keylunacy_stateChanged(int arg1);
    void on_race_mode_stateChanged(int arg1);
    void on_num_race_mode_dungeons_currentIndexChanged(int index);
    void on_num_starting_triforce_shards_currentIndexChanged(int index);

    // Convenience Tweaks
    void on_invert_sea_compass_x_axis_stateChanged(int arg1);
    void on_instant_text_boxes_stateChanged(int arg1);
    void on_reveal_full_sea_chart_stateChanged(int arg1);
    void on_skip_rematch_bosses_stateChanged(int arg1);
    void on_add_shortcut_warps_between_dungeons_stateChanged(int arg1);
    void on_remove_music_stateChanged(int arg1);

    // Starting Gear
    void on_add_gear_clicked();
    void on_remove_gear_clicked();
    void on_starting_hcs_valueChanged(int arg1);
    void on_starting_pohs_valueChanged(int arg1);

    // Player Customization
    void on_player_in_casual_clothes_stateChanged(int arg1);

    // Advanced Options
    void on_do_not_generate_spoiler_log_stateChanged(int arg1);
    void on_plandomizer_stateChanged(int arg1);
    void on_plandomizer_path_browse_button_clicked();

    // Hints
    void on_ho_ho_hints_stateChanged(int arg1);
    void on_korl_hints_stateChanged(int arg1);
    void on_use_always_hints_stateChanged(int arg1);
    void on_path_hints_valueChanged(int path_hints);
    void on_barren_hints_valueChanged(int barren_hints);
    void on_location_hints_valueChanged(int location_hints);
    void on_item_hints_valueChanged(int item_hints);

    // Entrance Randomizer
    void on_randomize_dungeon_entrances_stateChanged(int arg1);
    void on_randomize_cave_entrances_stateChanged(int arg1);
    void on_randomize_door_entrances_stateChanged(int arg1);
    void on_randomize_misc_entrances_stateChanged(int arg1);
    void on_decouple_entrances_stateChanged(int arg1);
    void on_randomize_starting_island_stateChanged(int arg1);

    // General
    void on_permalink_textEdited(const QString &arg1);
    void on_reset_settings_to_default_clicked();
    void on_randomize_button_clicked();
    void on_clearer_hints_stateChanged(int arg1);
    void on_repack_for_console_stateChanged(int arg1);
    void on_console_output_browse_button_clicked();
};
#endif // MAINWINDOW_H
