#pragma once

#include <QMainWindow>
#include <QListView>
#include <QPointer>
#include <QProgressDialog>
#include <QString>
#include <QStringListModel>
#include <QStringList>
#include <QStandardItemModel>
#include <QLabel>

#include <seedgen/config.hpp>
#include <logic/Location.hpp>
#include <logic/World.hpp>
#include <logic/EntranceShuffle.hpp>
#include <logic/PoolFunctions.hpp>

#include <gui/desktop/tracker/tracker_preferences.hpp>
#include <gui/desktop/tracker/tracker_preferences_dialog.hpp>
#include <gui/desktop/tracker/tracker_inventory_button.hpp>
#include <gui/desktop/tracker/tracker_label.hpp>
#include <gui/desktop/tracker/tracker_area_widget.hpp>

void delete_and_create_default_config();
QString prettyTrackerName(Item& item, const int& count, MainWindow* mainWindow);

#define LOCATION_TRACKER_OVERWORLD 0
#define LOCATION_TRACKER_SPECIFIC_AREA 1
#define LOCATION_TRACKER_SPECIFIC_AREA_ENTRANCES 2
#define LOCATION_TRACKER_ENTRANCE_DESTINATIONS 3
#define LOCATION_TRACKER_CHART_LIST 4

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
    Ui::MainWindow* getUI();

protected:
    bool eventFilter(QObject *object, QEvent *event) override;

private:
    Ui::MainWindow *ui;
    Config config;
    QStringListModel* randomizedGearModel = nullptr;
    QStringListModel* startingGearModel = nullptr;
    QStringListModel* progressionLocationsModel = nullptr;
    QStringListModel* excludedLocationsModel = nullptr;

    // Variables for setting the mixed pools
    // combobox as we want
    QList<QStandardItem*> poolCheckBoxes;
    QStringList poolNames;
    QStandardItemModel poolModel;
    QListView eventsByName;
    QStandardItem mix_dungeons;
    QStandardItem mix_bosses;
    QStandardItem mix_minibosses;
    QStandardItem mix_caves;
    QStandardItem mix_doors;
    QStandardItem mix_misc;

    std::string defaultWindowTitle;
    QString currentPermalink;
    bool encounteredError;
    std::unordered_map<std::string, std::set<LocationCategory>> locationCategories = {};
    std::unordered_map<std::string, QPushButton*> customColorSelectorButtons = {};
    std::unordered_map<std::string, QLineEdit*> customColorHexCodeInputs = {};
    std::unordered_map<std::string, QPushButton*> customColorResetButtons = {};
    // Mask Pixels keyed by hero or casual, and option name
    std::unordered_map<std::string, std::unordered_map<std::string, std::list<std::tuple<int, int>>>> maskPixels = {};

    void closeEvent(QCloseEvent *event) override;
    std::tuple<std::string, std::string> get_option_name_and_color_name_from_sender_object_name();
    void clear_layout(QLayout* layout);
    void load_config_into_ui();
    void setup_gear_menus();
    void setup_location_menus();
    void setup_color_options();
    void setup_mixed_pools_combobox();
    void apply_config_settings();
    int  calculate_total_progress_locations();
    void update_progress_locations_text();
    void swap_selected_gear(QListView* gearFrom, QStringListModel* gearTo);
    void swap_selected_locations(QListView* locsFrom, QStringListModel* locsTo);
    void update_starting_gear();
    void update_excluded_locations();
    void update_plandomizer_widget_visbility();
    void update_starting_health_text();
    void update_option_description_text(const std::string& descrption = "");
    void update_permalink_and_seed_hash();
    void load_locations();

    void initialize_tracker();
    void initialize_tracker_world(Settings& settings, const GameItemPool& markedItems = {}, const std::vector<std::string>& markedLocations = {}, const std::unordered_map<std::string, std::string>& connectedEntrances = {}, const std::map<GameItem, uint8_t>& chartMappings = {});
    void calculate_own_dungeon_key_locations();
    void set_location_list_widget_background(const std::string& area);
    void clear_tracker_labels(QLayout* layout);

public:
    void set_areas_locations();
    void set_areas_entrances();
    void calculate_entrance_paths();
    void switch_to_overworld_tracker();
    void switch_to_area_tracker();
    void switch_to_entrances_tracker();
    void switch_to_entrance_destinations_tracker();
    void switch_to_chart_list_tracker();
    void set_current_tracker_area(const std::string& areaPrefix);
    void update_tracker();
    void setup_tracker_entrances();
    void load_tracker_autosave();
    void autosave_current_tracker();
    bool autosave_current_tracker_config();
    bool autosave_current_tracker_preferences();
    void filter_entrance_list(const QString& filter, const bool arenaExitNameChange);
    bool isMappingChart();
    bool isChartMapped(GameItem chart);
    void mapChart(GameItem chart, uint8_t islandNum);
    bool isIslandMappedToChart(uint8_t island);
    uint8_t islandForChart(GameItem chart);
    GameItem chartForIsland(uint8_t islandNum);

private slots:
    void show_error_dialog(const std::string& s, const std::string& title = "An error has occured!");
    void update_mixed_pools_combobox_text(const QString& text);
    void update_mixed_pools_combobox_option(const QString& text = "");
    void update_mixed_pools_on_text_click(const QModelIndex&);
    void on_base_game_path_browse_button_clicked();
    void on_base_game_path_textChanged(const QString &arg1);
    void on_output_folder_browse_button_clicked();
    void on_output_folder_textChanged(const QString &arg1);
    void on_generate_seed_button_clicked();
    void on_open_logs_folder_button_clicked();


    // Progression Locations
    void on_progression_battlesquid_stateChanged(int arg1);
    void on_progression_big_octos_gunboats_stateChanged(int arg1);
    void on_progression_combat_secret_caves_stateChanged(int arg1);
    void on_progression_dungeons_currentTextChanged(const QString &arg1);
    void on_progression_dungeon_secrets_stateChanged(int arg1);
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
    void on_dungeon_small_keys_currentTextChanged(const QString &arg1);
    void on_dungeon_big_keys_currentTextChanged(const QString &arg1);
    void on_dungeon_maps_compasses_currentTextChanged(const QString &arg1);
    void on_remove_swords_stateChanged(int arg1);
    void on_randomize_charts_stateChanged(int arg1);
    void on_chest_type_matches_contents_stateChanged(int arg1);
    void on_damage_multiplier_valueChanged(int multiplier);
    void on_num_required_dungeons_currentIndexChanged(int index);

    // Convenience Tweaks
    void on_invert_sea_compass_x_axis_stateChanged(int arg1);
    void on_instant_text_boxes_stateChanged(int arg1);
    void on_quiet_swift_sail_stateChanged(int arg1);
    void on_reveal_full_sea_chart_stateChanged(int arg1);
    void on_skip_rematch_bosses_stateChanged(int arg1);
    void on_add_shortcut_warps_between_dungeons_stateChanged(int arg1);
    void on_remove_music_stateChanged(int arg1);

    // Starting Gear
    void on_add_gear_clicked();
    void on_remove_gear_clicked();
    void on_starting_hcs_valueChanged(int arg1);
    void on_starting_pohs_valueChanged(int arg1);
    void on_starting_blue_chu_jellys_valueChanged(int arg1);
    void on_starting_green_chu_jellys_valueChanged(int arg1);
    void on_starting_red_chu_jellys_valueChanged(int arg1);
    void on_starting_knights_crests_valueChanged(int arg1);
    void on_starting_golden_feathers_valueChanged(int arg1);
    void on_starting_boko_baba_seeds_valueChanged(int arg1);
    void on_starting_skull_necklaces_valueChanged(int arg1);
    void on_starting_joy_pendants_valueChanged(int arg1);

    // Excluded Locations
    void on_remove_locations_clicked();
    void on_add_locations_clicked();

    // Player Customization
    void on_player_in_casual_clothes_stateChanged(int arg1);
    void initialize_color_presets_list();
    void initialize_mask_pixels();
    void set_color(const std::string& optionName, std::string color, const bool& updatePreview = true, const bool& saveColorAsCustom = true);
    void open_custom_color_chooser();
    bool custom_color_hex_code_changed();
    void custom_color_hex_code_finished_editing();
    void randomize_one_custom_color();
    void reset_one_custom_color();
    void update_preview();
    void on_custom_color_preset_currentIndexChanged(const int &arg1);
    void on_randomize_all_custom_colors_together_clicked();
    void on_randomize_all_custom_colors_separately_clicked();

    // Advanced Options
    void on_do_not_generate_spoiler_log_stateChanged(int arg1);
    void on_start_with_random_item_stateChanged(int arg1);
    void on_random_item_slide_item_stateChanged(int arg1);
    void on_classic_mode_stateChanged(int arg1);
    void on_performance_stateChanged(int arg1);
    void on_plandomizer_stateChanged(int arg1);
    void validate_plandomizer_path();
    void on_plandomizer_path_browse_button_clicked();
    void on_plandomizer_path_editingFinished();
    void on_fix_rng_stateChanged(int arg1);

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
    void on_randomize_boss_entrances_stateChanged(int arg1);
    void on_randomize_miniboss_entrances_stateChanged(int arg1);
    void on_randomize_cave_entrances_currentTextChanged(const QString &arg1);
    void on_randomize_door_entrances_stateChanged(int arg1);
    void on_randomize_misc_entrances_stateChanged(int arg1);
    void on_decouple_entrances_stateChanged(int arg1);
    void on_randomize_starting_island_stateChanged(int arg1);

    // General
    void on_permalink_textEdited(const QString &arg1);
    void on_copy_permalink_clicked();
    void on_paste_permalink_clicked();
    void on_reset_settings_to_default_clicked();
    void on_randomize_button_clicked();
    void on_clearer_hints_stateChanged(int arg1);
    void on_hint_importance_stateChanged(int arg1);
    void on_seed_textChanged(const QString &arg1);
    void on_target_type_currentTextChanged(const QString &arg1);
    void on_camera_currentTextChanged(const QString &arg1);
    void on_first_person_camera_currentTextChanged(const QString &arg1);
    void on_gyroscope_currentTextChanged(const QString &arg1);
    void on_ui_display_currentTextChanged(const QString &arg1);
    void on_about_button_clicked();

    // Tracker
    void on_start_tracker_button_clicked();
    void on_location_list_close_button_released();
    void on_clear_all_button_released();
    void check_special_accessibility_conditions();
    void update_tracker_areas_and_autosave();
    void tracker_show_specific_area(const std::string& areaPrefix);
    void tracker_area_right_clicked(const std::string& areaPrefix);
    void tracker_clear_specific_area(const std::string& areaPrefix);
    void tracker_display_current_item_text(const std::string& currentItem);
    void tracker_clear_current_item_text();
    void tracker_display_current_area_text(TrackerAreaWidget* currentArea);
    void tracker_clear_current_area_text();
    void tracker_display_current_location(Location* location);
    void tracker_display_current_entrance(Entrance* entrance);
    void tracker_show_available_target_entrances(Entrance* entrance);
    void tracker_change_entrance_connections(Entrance* target);
    void tracker_disconnect_entrance(Entrance* entrance);
    void on_location_list_entrances_button_released();
    void on_entrance_list_close_button_released();
    void on_entrance_list_locations_button_released();
    void on_source_entrance_filter_lineedit_textChanged(const QString &arg1);
    void on_entrance_destination_back_button_released();
    void on_target_entrance_filter_lineedit_textChanged(const QString &arg1);
    void on_chart_list_back_button_released();
    void on_open_chart_list_button_clicked();
    void on_open_tracker_settings_button_clicked();
    void tracker_preferences_closed(int result);
    void on_view_all_entrances_button_clicked();
    void open_chart_mapping_list(uint8_t islandNum);
    void tracker_give_and_map_chart(TrackerLabel* label, GameItem chart);


public:
    void update_items_color();
    void update_locations_color();
    void update_stats_color();
    bool trackerStarted = false;
    TrackerPreferences trackerPreferences;
    Settings trackerSettings = Settings();
    WorldPool trackerWorlds = {};
    ItemPool trackerInventory = {};
    std::string currentTrackerArea = "";

    // Maps each area to the shortest entrance path it has to each sub region
    std::unordered_map<std::string, std::unordered_map<Area*, EntrancePath>> entrancePaths = {};
    // Maps each location to its shortest path
    std::unordered_map<Location*, EntrancePath> entrancePathsByLocation = {};
private:
    LocationPool trackerLocations = {};


    // Maps area name to the locations in the area. This can dynamically
    // change when entrance randomizer is on
    std::unordered_map<std::string, LocationSet> areaLocations = {};

    // Maps dungeon keys to the pool of locations they could possibly appear in
    // with the Own Dungeon setting
    std::unordered_map<Item, std::vector<LocationPool>> ownDungeonKeyLocations;

    // Maps island name to randomized entrances on the island
    std::unordered_map<std::string, std::list<Entrance*>> areaEntrances = {};

    // Maps connected targets to their connected source entrances
    std::unordered_map<Entrance*, Entrance*> connectedTargets = {};

    EntrancePools targetEntrancePools = {};
    Entrance* selectedEntrance = nullptr;

    // Maps room numbers to their (randomized) chart
    //TODO: make this entire system less sketchy
    std::map<GameItem, uint8_t> mappedCharts = {};
    uint8_t selectedChartIsland = 0;

    TrackerPreferencesDialog* prefDialog = nullptr;

    using TIB = TrackerInventoryButton;

    // Tracker Buttons
    TIB trackerTelescope             = TIB({{GameItem::NOTHING,               "telescope_gray.png"},
                                            {GameItem::Telescope,             "telescope_color.png"}});
    TIB trackerProgressiveSail       = TIB({{GameItem::NOTHING,               "sail_gray.png"},
                                            {GameItem::ProgressiveSail,       "sail_color.png",                      "Normal Sail"},
                                            {GameItem::ProgressiveSail,       "sail_swift_color.png",                "Swift Sail"}});
    TIB trackerWindWaker             = TIB({{GameItem::NOTHING,               "wind_waker_gray.png"},
                                            {GameItem::WindWaker,             "wind_waker_color.png"}});
    TIB trackerGrapplingHook         = TIB({{GameItem::NOTHING,               "grappling_hook_gray.png"},
                                            {GameItem::GrapplingHook,         "grappling_hook_color.png"}});
    TIB trackerSpoilsBag             = TIB({{GameItem::NOTHING,               "spoils_bag_gray.png"},
                                            {GameItem::SpoilsBag,             "spoils_bag_color.png"}});
    TIB trackerBoomerang             = TIB({{GameItem::NOTHING,               "boomerang_gray.png"},
                                            {GameItem::Boomerang,             "boomerang_color.png"}});
    TIB trackerDekuLeaf              = TIB({{GameItem::NOTHING,               "deku_leaf_gray.png"},
                                            {GameItem::DekuLeaf,              "deku_leaf_color.png"}});
    TIB trackerProgressiveSword      = TIB({{GameItem::NOTHING,               "sword_hero_gray.png"},
                                            {GameItem::ProgressiveSword,      "sword_hero_color.png",                "Hero's Sword"},
                                            {GameItem::ProgressiveSword,      "sword_master_unpowered_color.png",    "Master Sword"},
                                            {GameItem::ProgressiveSword,      "sword_master_half_powered_color.png", "Master Sword\n(Half Power)"},
                                            {GameItem::ProgressiveSword,      "sword_master_full_powered_color.png", "Master Sword\n(Full Power)"}});
    TIB trackerTingleBottle          = TIB({{GameItem::NOTHING,               "tingle_bottle_gray.png"},
                                            {GameItem::TingleBottle,          "tingle_bottle_color.png"}});
    TIB trackerProgressivePictoBox   = TIB({{GameItem::NOTHING,               "picto_box_gray.png"},
                                            {GameItem::ProgressivePictoBox,   "picto_box_color.png",                 "Picto Box"},
                                            {GameItem::ProgressivePictoBox,   "picto_box_deluxe_color.png",          "Deluxe Picto Box"}});
    TIB trackerIronBoots             = TIB({{GameItem::NOTHING,               "iron_boots_gray.png"},
                                            {GameItem::IronBoots,             "iron_boots_color.png"}});
    TIB trackerMagicArmor            = TIB({{GameItem::NOTHING,               "magic_armor_gray.png"},
                                            {GameItem::MagicArmor,            "magic_armor_color.png"}});
    TIB trackerBaitBag               = TIB({{GameItem::NOTHING,               "bait_bag_gray.png"},
                                            {GameItem::BaitBag,               "bait_bag_color.png"}});
    TIB trackerProgressiveBow        = TIB({{GameItem::NOTHING,               "bow_gray.png"},
                                            {GameItem::ProgressiveBow,        "bow_color.png",                       "Hero's Bow"},
                                            {GameItem::ProgressiveBow,        "bow_fire_ice_color.png",              "Hero's Bow\n(Fire & Ice Arrows)"},
                                            {GameItem::ProgressiveBow,        "bow_light_color.png",                 "Hero's Bow\n(All Arrows)"}});
    TIB trackerBombs                 = TIB({{GameItem::NOTHING,               "bombs_gray.png"},
                                            {GameItem::Bombs,                 "bombs_color.png"}});
    TIB trackerProgressiveShield     = TIB({{GameItem::NOTHING,               "shield_gray.png"},
                                            {GameItem::ProgressiveShield,     "shield_color.png",                    "Hero's Shield"},
                                            {GameItem::ProgressiveShield,     "shield_mirror_color.png",             "Mirror Shield"}});
    TIB trackerCabanaDeed            = TIB({{GameItem::NOTHING,               "cabana_deed_gray.png"},
                                            {GameItem::CabanaDeed,            "cabana_deed_color.png"}});
    TIB trackerMaggiesLetter         = TIB({{GameItem::NOTHING,               "maggies_letter_gray.png"},
                                            {GameItem::MaggiesLetter,         "maggies_letter_color.png"}});
    TIB trackerMoblinsLetter         = TIB({{GameItem::NOTHING,               "moblins_letter_gray.png"},
                                            {GameItem::MoblinsLetter,         "moblins_letter_color.png"}});
    TIB trackerNoteToMom             = TIB({{GameItem::NOTHING,               "note_to_mom_gray.png"},
                                            {GameItem::NoteToMom,             "note_to_mom_color.png"}});
    TIB trackerDeliveryBag           = TIB({{GameItem::NOTHING,               "delivery_bag_gray.png"},
                                            {GameItem::DeliveryBag,           "delivery_bag_color.png"}});
    TIB trackerHookshot              = TIB({{GameItem::NOTHING,               "hookshot_gray.png"},
                                            {GameItem::Hookshot,              "hookshot_color.png"}});
    TIB trackerSkullHammer           = TIB({{GameItem::NOTHING,               "skull_hammer_gray.png"},
                                            {GameItem::SkullHammer,           "skull_hammer_color.png"}});
    TIB trackerPowerBracelets        = TIB({{GameItem::NOTHING,               "power_bracelets_gray.png"},
                                            {GameItem::PowerBracelets,        "power_bracelets_color.png"}});
    TIB trackerEmptyBottle           = TIB({{GameItem::NOTHING,               "bottle_gray.png",                     "Empty Bottle (0/4)"},
                                            {GameItem::EmptyBottle,           "bottle_color.png",                    "Empty Bottle (1/4)"},
                                            {GameItem::EmptyBottle,           "bottle_2_color.png",                  "Empty Bottle (2/4)"},
                                            {GameItem::EmptyBottle,           "bottle_3_color.png",                  "Empty Bottle (3/4)"},
                                            {GameItem::EmptyBottle,           "bottle_4_color.png",                  "Empty Bottle (4/4)"}});
    TIB trackerWindsRequiem          = TIB({{GameItem::NOTHING,               "winds_requiem_gray.png"},
                                            {GameItem::WindsRequiem,          "winds_requiem_color.png"}});
    TIB trackerBalladOfGales         = TIB({{GameItem::NOTHING,               "ballad_of_gales_gray.png"},
                                            {GameItem::BalladOfGales,         "ballad_of_gales_color.png"}});
    TIB trackerCommandMelody         = TIB({{GameItem::NOTHING,               "command_melody_gray.png"},
                                            {GameItem::CommandMelody,         "command_melody_color.png"}});
    TIB trackerEarthGodsLyric        = TIB({{GameItem::NOTHING,               "earth_gods_lyric_gray.png"},
                                            {GameItem::EarthGodsLyric,        "earth_gods_lyric_color.png"}});
    TIB trackerWindGodsAria          = TIB({{GameItem::NOTHING,               "wind_gods_aria_gray.png"},
                                            {GameItem::WindGodsAria,          "wind_gods_aria_color.png"}});
    TIB trackerSongOfPassing         = TIB({{GameItem::NOTHING,               "song_of_passing_gray.png"},
                                            {GameItem::SongOfPassing,         "song_of_passing_color.png"}});
    TIB trackerHerosCharm            = TIB({{GameItem::NOTHING,               "heros_charm_gray.png"},
                                            {GameItem::HerosCharm,            "heros_charm_color.png"}});
    TIB trackerDinsPearl             = TIB({{GameItem::NOTHING,               "pearl_dins_gray.png"},
                                            {GameItem::DinsPearl,             "pearl_dins_color.png"}});
    TIB trackerFaroresPearl          = TIB({{GameItem::NOTHING,               "pearl_farores_gray.png"},
                                            {GameItem::FaroresPearl,          "pearl_farores_color.png"}});
    TIB trackerNayrusPearl           = TIB({{GameItem::NOTHING,               "pearl_nayrus_gray.png"},
                                            {GameItem::NayrusPearl,           "pearl_nayrus_color.png"}});
    TIB trackerTriforceShards        = TIB({{GameItem::NOTHING,               "triforce_gray.png",                       "Triforce Shard (0/8)"},
                                            {GameItem::TriforceShard1,        "triforce_1b.png",                       "Triforce Shard (1/8)"},
                                            {GameItem::TriforceShard2,        "triforce_2b.png",                       "Triforce Shard (2/8)"},
                                            {GameItem::TriforceShard3,        "triforce_3b.png",                       "Triforce Shard (3/8)"},
                                            {GameItem::TriforceShard4,        "triforce_4b.png",                       "Triforce Shard (4/8)"},
                                            {GameItem::TriforceShard5,        "triforce_5b.png",                       "Triforce Shard (5/8)"},
                                            {GameItem::TriforceShard6,        "triforce_6b.png",                       "Triforce Shard (6/8)"},
                                            {GameItem::TriforceShard7,        "triforce_7b.png",                       "Triforce Shard (7/8)"},
                                            {GameItem::TriforceShard8,        "triforce8.png",                       "Triforce of Courage"}});
    TIB trackerTingleStatues         = TIB({{GameItem::NOTHING,               "tingle_statue_gray.png",              "Tingle Statue (0/5)"},
                                            {GameItem::DragonTingleStatue,    "tingle_statue_1_color.png",           "Tingle Statue (1/5)"},
                                            {GameItem::ForbiddenTingleStatue, "tingle_statue_2_color.png",           "Tingle Statue (2/5)"},
                                            {GameItem::GoddessTingleStatue,   "tingle_statue_3_color.png",           "Tingle Statue (3/5)"},
                                            {GameItem::EarthTingleStatue,     "tingle_statue_4_color.png",           "Tingle Statue (4/5)"},
                                            {GameItem::WindTingleStatue,      "tingle_statue_5_color.png",           "Tingle Statue (5/5)"}});
    TIB trackerGhostShipChart        = TIB({{GameItem::NOTHING,               "ghost_ship_chart_gray.png"},
                                            {GameItem::GhostShipChart,        "ghost_ship_chart_color.png"}});
    TIB trackerHurricaneSpin         = TIB({{GameItem::NOTHING,               "hurricane_spin_gray.png"},
                                            {GameItem::HurricaneSpin,         "hurricane_spin_color.png"}});
    TIB trackerProgressiveBombBag    = TIB({{GameItem::NOTHING,               "bigger_bomb_bag_gray.png",            "Bomb Bag (60 Bombs)"},
                                            {GameItem::ProgressiveBombBag,    "bigger_bomb_bag_color.png",           "Bomb Bag (60 Bombs)"},
                                            {GameItem::ProgressiveBombBag,    "biggest_bomb_bag_color.png",          "Bomb Bag (99 Bombs)"}});
    TIB trackerProgressiveQuiver     = TIB({{GameItem::NOTHING,               "bigger_quiver_gray.png",              "Quiver (60 Arrows)"},
                                            {GameItem::ProgressiveQuiver,     "bigger_quiver_color.png",             "Quiver (60 Arrows)"},
                                            {GameItem::ProgressiveQuiver,     "biggest_quiver_color.png",            "Quiver (99 Arrows)"}});
    TIB trackerProgressiveWallet     = TIB({{GameItem::NOTHING,               "bigger_wallet_gray.png",              "Wallet (1000 Rupees)"},
                                            {GameItem::ProgressiveWallet,     "bigger_wallet_color.png",             "Wallet (1000 Rupees)"},
                                            {GameItem::ProgressiveWallet,     "biggest_wallet_color.png",            "Wallet (5000 Rupees)"}});
    TIB trackerProgressiveMagicMeter = TIB({{GameItem::NOTHING,               "magic_bottle_gray.png",               "Magic Meter"},
                                            {GameItem::ProgressiveMagicMeter, "magic_bottle_color.png",              "Magic Meter"},
                                            {GameItem::ProgressiveMagicMeter, "magic_double_bottle_color.png",       "Magic Meter Upgrade"}});

    // Dungeon Item Inventory Buttons
    TIB trackerWTSmallKeys           = TIB({{GameItem::NOTHING,               "small_key_gray.png",        "WT Small Key (0/2)"},
                                            {GameItem::WTSmallKey,            "small_key_1_color.png",     "WT Small Key (1/2)"},
                                            {GameItem::WTSmallKey,            "small_key_2_color.png",     "WT Small Key (2/2)"}});
    TIB trackerWTBigKey              = TIB({{GameItem::NOTHING,               "big_key_gray.png",          "WT Big Key"},
                                            {GameItem::WTBigKey,              "big_key_color.png",         "WT Big Key"}});
    TIB trackerWTDungeonMap          = TIB({{GameItem::NOTHING,               "map_gray.png",              "WT Dungeon Map"},
                                            {GameItem::WTDungeonMap,          "map_color.png",             "WT Dungeon Map"}});
    TIB trackerWTCompass             = TIB({{GameItem::NOTHING,               "compass_gray.png",          "WT Compass"},
                                            {GameItem::WTCompass,             "compass_color.png",         "WT Compass"}});
    TIB trackerETSmallKeys           = TIB({{GameItem::NOTHING,               "small_key_gray.png",        "ET Small Key (0/3)"},
                                            {GameItem::ETSmallKey,            "small_key_1_color.png",     "ET Small Key (1/3)"},
                                            {GameItem::ETSmallKey,            "small_key_2_color.png",     "ET Small Key (2/3)"},
                                            {GameItem::ETSmallKey,            "small_key_3_color.png",     "ET Small Key (3/3)"}});
    TIB trackerETBigKey              = TIB({{GameItem::NOTHING,               "big_key_gray.png",          "ET Big Key"},
                                            {GameItem::ETBigKey,              "big_key_color.png",         "ET Big Key"}});
    TIB trackerETDungeonMap          = TIB({{GameItem::NOTHING,               "map_gray.png",              "ET Dungeon Map"},
                                            {GameItem::ETDungeonMap,          "map_color.png",             "ET Dungeon Map"}});
    TIB trackerETCompass             = TIB({{GameItem::NOTHING,               "compass_gray.png",          "ET Compass"},
                                            {GameItem::ETCompass,             "compass_color.png",         "ET Compass"}});
    TIB trackerFFDungeonMap          = TIB({{GameItem::NOTHING,               "map_gray.png",              "FF Dungeon Map"},
                                            {GameItem::FFDungeonMap,          "map_color.png",             "FF Dungeon Map"}});
    TIB trackerFFCompass             = TIB({{GameItem::NOTHING,               "compass_gray.png",          "FF Compass"},
                                            {GameItem::FFCompass,             "compass_color.png",         "FF Compass"}});
    TIB trackerTOTGSmallKeys         = TIB({{GameItem::NOTHING,               "small_key_gray.png",        "TotG Small Key (0/2)"},
                                            {GameItem::TotGSmallKey,          "small_key_1_color.png",     "TotG Small Key (1/2)"},
                                            {GameItem::TotGSmallKey,          "small_key_2_color.png",     "TotG Small Key (2/2)"}});
    TIB trackerTOTGBigKey            = TIB({{GameItem::NOTHING,               "big_key_gray.png",          "TotG Big Key"},
                                            {GameItem::TotGBigKey,            "big_key_color.png",         "TotG Big Key"}});
    TIB trackerTOTGDungeonMap        = TIB({{GameItem::NOTHING,               "map_gray.png",              "TotG Dungeon Map"},
                                            {GameItem::TotGDungeonMap,        "map_color.png",             "TotG Dungeon Map"}});
    TIB trackerTOTGCompass           = TIB({{GameItem::NOTHING,               "compass_gray.png",          "TotG Compass"},
                                            {GameItem::TotGCompass,           "compass_color.png",         "TotG Compass"}});
    TIB trackerFWSmallKeys           = TIB({{GameItem::NOTHING,               "small_key_gray.png",        "FW Small Key"},
                                            {GameItem::FWSmallKey,            "small_key_1_color.png",     "FW Small Key"}});
    TIB trackerFWBigKey              = TIB({{GameItem::NOTHING,               "big_key_gray.png",          "FW Big Key"},
                                            {GameItem::FWBigKey,              "big_key_color.png",         "FW Big Key"}});
    TIB trackerFWDungeonMap          = TIB({{GameItem::NOTHING,               "map_gray.png",              "FW Dungeon Map"},
                                            {GameItem::FWDungeonMap,          "map_color.png",             "FW Dungeon Map"}});
    TIB trackerFWCompass             = TIB({{GameItem::NOTHING,               "compass_gray.png",          "FW Compass"},
                                            {GameItem::FWCompass,             "compass_color.png",         "FW Compass"}});
    TIB trackerDRCSmallKeys          = TIB({{GameItem::NOTHING,               "small_key_gray.png",        "DRC Small Key (0/4)"},
                                            {GameItem::DRCSmallKey,           "small_key_1_color.png",     "DRC Small Key (1/4)"},
                                            {GameItem::DRCSmallKey,           "small_key_2_color.png",     "DRC Small Key (2/4)"},
                                            {GameItem::DRCSmallKey,           "small_key_3_color.png",     "DRC Small Key (3/4)"},
                                            {GameItem::DRCSmallKey,           "small_key_4_color.png",     "DRC Small Key (4/4)"}});
    TIB trackerDRCBigKey             = TIB({{GameItem::NOTHING,               "big_key_gray.png",          "DRC Big Key"},
                                            {GameItem::DRCBigKey,             "big_key_color.png",         "DRC Big Key"}});
    TIB trackerDRCDungeonMap         = TIB({{GameItem::NOTHING,               "map_gray.png",              "DRC Dungeon Map"},
                                            {GameItem::DRCDungeonMap,         "map_color.png",             "DRC Dungeon Map"}});
    TIB trackerDRCCompass            = TIB({{GameItem::NOTHING,               "compass_gray.png",          "DRC Compass"},
                                            {GameItem::DRCCompass,            "compass_color.png",         "DRC Compass"}});
};
