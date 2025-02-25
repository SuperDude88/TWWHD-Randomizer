#include "mainwindow.hpp"

#include <algorithm>
#include <filesystem>

#include <QMessageBox>
#include <QFileDialog>
#include <QResource>
#include <QDirIterator>
#include <QFile>
#include <QDesktopServices>
#include <QClipboard>

#include <ui_mainwindow.h>
#include <gui/desktop/randomizer_thread.hpp>
#include <gui/desktop/option_descriptions.hpp>

#include <version.hpp>
#include <libs/yaml.hpp>
#include <seedgen/seed.hpp>
#include <utility/string.hpp>
#include <utility/file.hpp>
#include <utility/color.hpp>

#define UPDATE_CONFIG_STATE(config, ui, name) config.settings.name = ui->name->isChecked(); update_permalink_and_seed_hash(); update_progress_locations_text();
#define UPDATE_CONFIG_STATE_MIXED_POOLS(config, name) config.settings.name = (name.checkState() == Qt::Checked); update_permalink_and_seed_hash();
#define APPLY_CHECKBOX_SETTING(config, ui, name) if(config.settings.name) {ui->name->setCheckState(Qt::Checked);} else {ui->name->setCheckState(Qt::Unchecked);}
#define APPLY_CONFIG_CHECKBOX_SETTING(config, ui, name) if(config.name) {ui->name->setCheckState(Qt::Checked);} else {ui->name->setCheckState(Qt::Unchecked);}
#define APPLY_COMBOBOX_SETTING(config, ui, name) ui->name->setCurrentIndex(static_cast<int>(config.settings.name));

#define APPLY_SPINBOX_SETTING(config, ui, name, min, max) \
    auto& name = config.settings.name;                    \
    name = std::clamp(name, min, max);                    \
    ui->name->setValue(static_cast<int>(name));

#define APPLY_MIXED_POOLS_SETTING(config, ui, name)                         \
    name.setCheckState(config.settings.name ? Qt::Checked : Qt::Unchecked); \
    update_mixed_pools_combobox_option();

#define DEFINE_STATE_CHANGE_FUNCTION(name)                \
    void MainWindow::on_##name##_stateChanged(int arg1) { \
        UPDATE_CONFIG_STATE(config, ui, name);            \
    }

#define DEFINE_SPINBOX_VALUE_CHANGE_FUNCTION(name)        \
    void MainWindow::on_##name##_valueChanged(int arg1) { \
        config.settings.name = arg1;                      \
        update_permalink_and_seed_hash();                               \
    }

void delete_and_create_default_config()
{
    std::filesystem::remove(Utility::get_app_save_path() / "config.yaml");

    ConfigError err = Config::writeDefault(Utility::get_app_save_path() / "config.yaml", Utility::get_app_save_path() / "preferences.yaml");
    if(err != ConfigError::NONE) {
        QPointer<QMessageBox> messageBox = new QMessageBox();
        auto message = "Failed to create default configuration file\ncode " + std::to_string(static_cast<uint32_t>(err));
        messageBox->setText(message.c_str());
        messageBox->setIcon(QMessageBox::Critical);
    }
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    if (std::string(RANDOMIZER_VERSION).empty()) {
        show_warning_dialog("Could not determine Randomizer version. Please tell a dev if you see this message.");
    }

    // Check for existing config file
    if (!std::filesystem::is_regular_file(Utility::get_app_save_path() / "config.yaml") || !std::filesystem::is_regular_file(Utility::get_app_save_path() / "preferences.yaml"))
    {
        // No config file, create default
        delete_and_create_default_config();
    }
    
    // Load in config
    setup_mixed_pools_combobox();
    load_locations();
    load_config_into_ui();

    // Set some variables
    encounteredError = false;
    defaultWindowTitle = "Wind Waker HD Randomizer " RANDOMIZER_VERSION;
    this->setWindowTitle(defaultWindowTitle.c_str());
    update_option_description_text();
    currentPermalink = ui->permalink->text();

    // Set the event filter that updates the option description
    // for all child widgets
    for (auto child : this->findChildren<QWidget*>())
    {
        child->installEventFilter(this);
    }

    // Switch to first tab
    ui->tabWidget->setCurrentIndex(0);

    // Hide options which won't exist for a while
    ui->randomize_enemies->setVisible(false);
    ui->randomize_enemy_palettes->setVisible(false);
    ui->randomize_music->setVisible(false);
    //ui->update_checker_label->setVisible(false);
    ui->disable_custom_player_items->setVisible(false);
    ui->disable_custom_player_voice->setVisible(false);
    ui->install_custom_model->setVisible(false);

    // Setup Tracker
    initialize_tracker();
    load_tracker_autosave();
}

MainWindow::~MainWindow()
{
    delete ui;
}


Ui::MainWindow* MainWindow::getUI()
{
    return ui;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    // Write settings to file when user closes the program
    ConfigError err = config.writeToFile(Utility::get_app_save_path() / "config.yaml", Utility::get_app_save_path() / "preferences.yaml");
    if (err != ConfigError:: NONE)
    {
        show_error_dialog("Settings could not be saved\nCode: " + ConfigErrorGetName(err));
    }
}

void MainWindow::clear_layout(QLayout* layout) {

    // Recursively clear child layouts
    for (auto nestedLayout : layout->findChildren<QLayout*>())
    {
        clear_layout(nestedLayout);
    }

    while (QLayoutItem* item = layout->takeAt(0))
    {
        if (QWidget* widget = item->widget())
            widget->deleteLater();
        delete item;
    }
}

void MainWindow::load_config_into_ui()
{
    // Ignore errors and just load in whatever we can. The gui will write a proper config file
    // when the user begins randomization
    ConfigError err = config.loadFromFile(Utility::get_app_save_path() / "config.yaml", Utility::get_app_save_path() / "preferences.yaml", true);
    if (err != ConfigError::NONE)
    {
        show_error_dialog("Failed to load settings file\ncode " + ConfigErrorGetName(err));
    }
    else
    {
        apply_config_settings();
    }
}

void MainWindow::show_error_dialog(const std::string& msg, const std::string& title /*= "An error has occured!"*/)
{
    encounteredError = true;
    QPointer<QMessageBox> messageBox = new QMessageBox();
    messageBox->setWindowTitle(title.c_str());
    messageBox->setText(msg.c_str());
    messageBox->setIcon(QMessageBox::Critical);
    messageBox->exec();
}

void MainWindow::show_warning_dialog(const std::string& msg, const std::string& title /*= ""*/)
{
    QPointer<QMessageBox> messageBox = new QMessageBox();
    messageBox->setWindowTitle(title == "" ? defaultWindowTitle.c_str() : title.c_str());
    messageBox->setText(msg.c_str());
    messageBox->setIcon(QMessageBox::Warning);
    messageBox->exec();
}

void MainWindow::show_info_dialog(const std::string& msg, const std::string& title /*= ""*/)
{
    QPointer<QMessageBox> messageBox = new QMessageBox();
    messageBox->setWindowTitle(title == "" ? defaultWindowTitle.c_str() : title.c_str());
    messageBox->setText(msg.c_str());
    messageBox->setIcon(QMessageBox::NoIcon);
    messageBox->exec();
}

void MainWindow::setup_mixed_pools_combobox()
{
    eventsByName.setModel(&poolModel);
    poolNames << "Dungeons" << "Bosses" << "Minibosses" << "Caves" << "Doors" << "Misc";
    poolCheckBoxes << &mix_dungeons << &mix_bosses << &mix_minibosses << &mix_caves << &mix_doors << &mix_misc;
    for (size_t i = 0; i < poolNames.size(); i++)
    {
        poolCheckBoxes[i]->setText(poolNames[i]);
        poolModel.setItem(i, 0, poolCheckBoxes[i]);
        poolCheckBoxes[i]->setFlags(Qt::ItemIsEnabled);
        poolCheckBoxes[i]->setData(Qt::Unchecked, Qt::CheckStateRole);
    }

    // Dummy row to allow changing the mix_pools_combobox text without
    // making it editable. The dummy row text will be changed to reflect
    // what we want the combobox text to be.
    poolCheckBoxes << new QStandardItem();
    poolCheckBoxes.back()->setText("None");
    poolCheckBoxes.back()->setFlags(Qt::NoItemFlags);
    poolCheckBoxes.back()->setData(Qt::Unchecked, Qt::CheckStateRole);
    poolModel.setItem(poolNames.size(), 0, poolCheckBoxes[poolNames.size()]);
    eventsByName.setRowHidden(poolNames.size(), true);

    ui->mix_pools_combobox->setModel(&poolModel);
    ui->mix_pools_combobox->setView(&eventsByName);
    ui->mix_pools_combobox->setCurrentText("None");

    connect(&eventsByName, &QListView::clicked, this, &MainWindow::update_mixed_pools_on_text_click);
}

void MainWindow::update_mixed_pools_combobox_text(const QString& text)
{
    poolCheckBoxes.back()->setText(text);
    ui->mix_pools_combobox->setCurrentText(text);
}

void MainWindow::update_mixed_pools_on_text_click(const QModelIndex& index)
{
    QString clickedOption = index.data(0).toString();
    update_mixed_pools_combobox_option(clickedOption);
}

void MainWindow::update_mixed_pools_combobox_option(const QString& pool /*= ""*/)
{
    QStringList enabledPools;

    for (size_t i = 0; i < poolCheckBoxes.size() - 1; i++)
    {
        auto& poolOption = poolCheckBoxes[i];
        if (pool != "" && poolOption->text() == pool)
        {
            poolOption->setCheckState(poolOption->checkState() == Qt::Checked ? Qt::Unchecked : Qt::Checked);
        }

        if (poolOption->checkState() == Qt::Checked)
        {
            enabledPools << poolOption->text();
        }
    }

    if (enabledPools.size() == poolCheckBoxes.size() - 1)
    {
        update_mixed_pools_combobox_text("All");
    }
    else if (enabledPools.empty())
    {
        update_mixed_pools_combobox_text("None");
    }
    else if (enabledPools.size() <= 3)
    {
        QString text = "";
        for (size_t i = 0; i < enabledPools.size(); i++)
        {
            text += enabledPools[i];
            if (i != enabledPools.size() - 1)
            {
                text += ", ";
            }
        }
        update_mixed_pools_combobox_text(text);
    }
    else
    {
        std::string text = std::to_string(enabledPools.size()) + " Selected";
        update_mixed_pools_combobox_text(text.c_str());
    }

    // Only update the config if the user manually selected an option
    if (pool != "")
    {
        UPDATE_CONFIG_STATE_MIXED_POOLS(config, mix_dungeons);
        UPDATE_CONFIG_STATE_MIXED_POOLS(config, mix_minibosses);
        UPDATE_CONFIG_STATE_MIXED_POOLS(config, mix_bosses);
        UPDATE_CONFIG_STATE_MIXED_POOLS(config, mix_caves);
        UPDATE_CONFIG_STATE_MIXED_POOLS(config, mix_doors);
        UPDATE_CONFIG_STATE_MIXED_POOLS(config, mix_misc);
    }

}

void MainWindow::setup_gear_menus()
{
    randomizedGearModel = new QStringListModel(this);
    startingGearModel = new QStringListModel(this);

    QStringList randomizedList;
    randomizedList << "Ballad of Gales"
                   << "Bait Bag"
                   << "Bombs"
                   << "Boomerang"
                   << "Cabana Deed"
                   << "Command Melody"
                   << "Deku Leaf"
                   << "Delivery Bag"
                   << "Din's Pearl"
                   << "Dragon Roost Cavern Big Key"
                   << "Dragon Roost Cavern Compass"
                   << "Dragon Roost Cavern Dungeon Map"
                   << "Dragon Roost Cavern Small Key"
                   << "Dragon Roost Cavern Small Key"
                   << "Dragon Roost Cavern Small Key"
                   << "Dragon Roost Cavern Small Key"
                   << "Dragon Tingle Statue"
                   << "Earth God's Lyric"
                   << "Earth Temple Big Key"
                   << "Earth Temple Compass"
                   << "Earth Temple Dungeon Map"
                   << "Earth Temple Small Key"
                   << "Earth Temple Small Key"
                   << "Earth Temple Small Key"
                   << "Earth Tingle Statue"
                   << "Empty Bottle"
                   << "Farore's Pearl"
                   << "Forbidden Tingle Statue"
                   << "Forbidden Woods Big Key"
                   << "Forbidden Woods Compass"
                   << "Forbidden Woods Dungeon Map"
                   << "Forbidden Woods Small Key"
                   << "Forsaken Fortress Compass"
                   << "Forsaken Fortress Dungeon Map"
                   << "Ghost Ship Chart"
                   << "Goddess Tingle Statue"
                   << "Grappling Hook"
                   << "Hero's Charm"
                   << "Hookshot"
                   << "Hurricane Spin"
                   << "Iron Boots"
                   << "Maggie's Letter"
                   << "Magic Armor"
                   << "Moblin's Letter"
                   << "Nayru's Pearl"
                   << "Note to Mom"
                   << "Power Bracelets"
                   << "Progressive Bomb Bag"
                   << "Progressive Bomb Bag"
                   << "Progressive Bow"
                   << "Progressive Bow"
                   << "Progressive Bow"
                   << "Progressive Picto Box"
                   << "Progressive Picto Box"
                   << "Progressive Magic Meter"
                   << "Progressive Magic Meter"
                   << "Progressive Quiver"
                   << "Progressive Quiver"
                   << "Progressive Sail"
                   << "Progressive Shield"
                   << "Progressive Shield"
                   << "Progressive Sword"
                   << "Progressive Sword"
                   << "Progressive Sword"
                   << "Progressive Sword"
                   << "Progressive Wallet"
                   << "Progressive Wallet"
                   << "Skull Hammer"
                   << "Song of Passing"
                   << "Spoils Bag"
                   << "Telescope"
                   << "Tingle Bottle"
                   << "Tower of the Gods Big Key"
                   << "Tower of the Gods Compass"
                   << "Tower of the Gods Dungeon Map"
                   << "Tower of the Gods Small Key"
                   << "Tower of the Gods Small Key"
                   << "Triforce Shard 1"
                   << "Triforce Shard 2"
                   << "Triforce Shard 3"
                   << "Triforce Shard 4"
                   << "Triforce Shard 5"
                   << "Triforce Shard 6"
                   << "Triforce Shard 7"
                   << "Triforce Shard 8"
                   << "Wind God's Aria"
                   << "Wind Temple Big Key"
                   << "Wind Temple Compass"
                   << "Wind Temple Dungeon Map"
                   << "Wind Temple Small Key"
                   << "Wind Temple Small Key"
                   << "Wind Tingle Statue";

    randomizedGearModel->setStringList(randomizedList);
    ui->randomized_gear->setModel(randomizedGearModel);
    ui->starting_gear->setModel(startingGearModel);
    ui->randomized_gear->setSelectionMode(QAbstractItemView::ExtendedSelection);
    ui->starting_gear->setSelectionMode(QAbstractItemView::ExtendedSelection);
}

void MainWindow::setup_location_menus()
{
    progressionLocationsModel = new QStringListModel(this);
    excludedLocationsModel = new QStringListModel(this);

    QStringList progressLocations;
    const auto& allLocations = getAllLocationsNames();
    if(allLocations.empty()) {
        show_error_dialog("Failed to load location names, check the error log for details.");
    }

    for (const auto& locName : allLocations)
    {
        // Don't expose certain locations to the user
        if (locName.ends_with("Defeat Ganondorf"))
        {
            continue;
        }
        progressLocations << QString::fromStdString(locName);
    }

    progressionLocationsModel->setStringList(progressLocations);
    ui->progression_locations->setModel(progressionLocationsModel);
    ui->excluded_locations->setModel(excludedLocationsModel);
    ui->progression_locations->setSelectionMode(QAbstractItemView::ExtendedSelection);
    ui->excluded_locations->setSelectionMode(QAbstractItemView::ExtendedSelection);
}

void MainWindow::update_plandomizer_widget_visbility()
{
    // Hide plandomizer input widgets when plandomizer isn't checked
    if (ui->plandomizer->isChecked())
    {
        ui->plandomizer_path->setVisible(true);
        ui->plandomizer_path_browse_button->setVisible(true);
        ui->plandomizer_label->setVisible(true);
    }
    else
    {
        ui->plandomizer_path->setVisible(false);
        ui->plandomizer_path_browse_button->setVisible(false);
        ui->plandomizer_label->setVisible(false);
    }
}

void MainWindow::apply_config_settings()
{
    // Validate the state of plando options to avoid seed hash errors
    if(config.settings.plandomizer && !std::filesystem::is_regular_file(config.settings.plandomizerFile)) {
        QMessageBox confirmDialog;
        confirmDialog.setText("No valid plandomizer file was found. Select a new one?");
        confirmDialog.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        switch(confirmDialog.exec()) {
            case QMessageBox::Yes:
            default:
                on_plandomizer_path_browse_button_clicked();
                break;
            case QMessageBox::No:
                config.settings.plandomizer = false;
                config.settings.plandomizerFile.clear();
        }
    }

    // Directories and Seed
    ui->base_game_path->setText(Utility::toQString(config.gameBaseDir));
    ui->output_folder->setText(Utility::toQString(config.outputDir));
    ui->seed->setText(QString::fromStdString(config.seed));

    // Progression settings
    APPLY_CHECKBOX_SETTING(config, ui, progression_battlesquid);
    APPLY_CHECKBOX_SETTING(config, ui, progression_big_octos_gunboats);
    APPLY_CHECKBOX_SETTING(config, ui, progression_combat_secret_caves);
    APPLY_COMBOBOX_SETTING(config, ui, progression_dungeons);
    APPLY_CHECKBOX_SETTING(config, ui, progression_dungeon_secrets);
    APPLY_CHECKBOX_SETTING(config, ui, progression_expensive_purchases);
    APPLY_CHECKBOX_SETTING(config, ui, progression_eye_reef_chests);
    APPLY_CHECKBOX_SETTING(config, ui, progression_free_gifts);
    APPLY_CHECKBOX_SETTING(config, ui, progression_great_fairies);
    APPLY_CHECKBOX_SETTING(config, ui, progression_island_puzzles);
    APPLY_CHECKBOX_SETTING(config, ui, progression_long_sidequests);
    APPLY_CHECKBOX_SETTING(config, ui, progression_mail);
    APPLY_CHECKBOX_SETTING(config, ui, progression_minigames);
    APPLY_CHECKBOX_SETTING(config, ui, progression_misc);
    APPLY_CHECKBOX_SETTING(config, ui, progression_obscure);
    APPLY_CHECKBOX_SETTING(config, ui, progression_platforms_rafts);
    APPLY_CHECKBOX_SETTING(config, ui, progression_puzzle_secret_caves);
    APPLY_CHECKBOX_SETTING(config, ui, progression_savage_labyrinth);
    APPLY_CHECKBOX_SETTING(config, ui, progression_short_sidequests);
    APPLY_CHECKBOX_SETTING(config, ui, progression_spoils_trading);
    APPLY_CHECKBOX_SETTING(config, ui, progression_submarines);
    APPLY_CHECKBOX_SETTING(config, ui, progression_tingle_chests);
    APPLY_CHECKBOX_SETTING(config, ui, progression_treasure_charts);
    APPLY_CHECKBOX_SETTING(config, ui, progression_triforce_charts);

    setup_gear_menus();
    setup_location_menus();

    // Starting Gear
    // Set this up before changing the remove_swords option since that function
    // will emit a signal to change the randomized/starting gear models
    QStringList randomizedGear = randomizedGearModel->stringList();
    QStringList startingGear =  startingGearModel->stringList();
    // For each item in the starting items list, remove it from the randomized items list
    for (GameItem& item : config.settings.starting_gear)
    {
        auto itemName = gameItemToName(item);
        auto index = randomizedGear.indexOf(itemName.c_str());
        if (index != -1)
        {
             randomizedGear.removeAt(index);
        }
        // Also add the item to the starting gear list
        startingGear << itemName.c_str();
    }
    startingGear.sort();
    randomizedGearModel->setStringList(randomizedGear);
    startingGearModel->setStringList(startingGear);
    ui->randomized_gear->setModel(randomizedGearModel);
    ui->starting_gear->setModel(startingGearModel);


    // Excluded Locations
    QStringList progressionLocations = progressionLocationsModel->stringList();
    QStringList excludedLocations = excludedLocationsModel->stringList();
    // Remove each excluded location from the progression locations
    for (const auto& locName : config.settings.excluded_locations)
    {
        auto index = progressionLocations.indexOf(locName.c_str());
        if (index != -1)
        {
            progressionLocations.removeAt(index);
        }
        // Also add the location to the excluded locations
        excludedLocations << locName.c_str();
    }
    progressionLocationsModel->setStringList(progressionLocations);
    excludedLocationsModel->setStringList(excludedLocations);
    ui->progression_locations->setModel(progressionLocationsModel);
    ui->excluded_locations->setModel(excludedLocationsModel);


    APPLY_CHECKBOX_SETTING(config, ui, remove_swords);
    APPLY_COMBOBOX_SETTING(config, ui, dungeon_small_keys);
    APPLY_COMBOBOX_SETTING(config, ui, dungeon_big_keys);
    APPLY_COMBOBOX_SETTING(config, ui, dungeon_maps_compasses);

    APPLY_SPINBOX_SETTING(config, ui, damage_multiplier, float(2.0f), float(MAXIMUM_DAMAGE_MULTIPLIER));

    auto& num_required_dungeons = config.settings.num_required_dungeons;
    // Race mode dungeons must be between 1 and 6 if race mode is enabled
    if (config.settings.progression_dungeons == ProgressionDungeons::RaceMode)
    {
        num_required_dungeons = std::clamp(num_required_dungeons, uint8_t(1), uint8_t(MAXIMUM_NUM_DUNGEONS));
    }
    else
    {
        num_required_dungeons = std::clamp(num_required_dungeons, uint8_t(0), uint8_t(MAXIMUM_NUM_DUNGEONS));
    }

    ui->num_required_dungeons->setCurrentIndex(num_required_dungeons);

    APPLY_CHECKBOX_SETTING(config, ui, randomize_charts);
    APPLY_CHECKBOX_SETTING(config, ui, chest_type_matches_contents);

    // Convenience Tweaks
    APPLY_CHECKBOX_SETTING(config, ui, invert_sea_compass_x_axis);
    APPLY_CHECKBOX_SETTING(config, ui, instant_text_boxes);
    APPLY_CHECKBOX_SETTING(config, ui, quiet_swift_sail);
    APPLY_CHECKBOX_SETTING(config, ui, reveal_full_sea_chart);
    APPLY_CHECKBOX_SETTING(config, ui, skip_rematch_bosses);
    APPLY_CHECKBOX_SETTING(config, ui, add_shortcut_warps_between_dungeons);
    APPLY_CHECKBOX_SETTING(config, ui, remove_music);

    // Starting Health
    APPLY_SPINBOX_SETTING(config, ui, starting_hcs, uint16_t(1), uint16_t(MAXIMUM_STARTING_HC));
    APPLY_SPINBOX_SETTING(config, ui, starting_pohs, uint16_t(0), uint16_t(MAXIMUM_STARTING_HP));
    APPLY_SPINBOX_SETTING(config, ui, starting_joy_pendants, uint16_t(0), uint16_t(MAXIMUM_STARTING_JOY_PENDANTS));
    APPLY_SPINBOX_SETTING(config, ui, starting_skull_necklaces, uint16_t(0), uint16_t(MAXIMUM_STARTING_SKULL_NECKLACES));
    APPLY_SPINBOX_SETTING(config, ui, starting_boko_baba_seeds, uint16_t(0), uint16_t(MAXIMUM_STARTING_BOKO_BABA_SEEDS));
    APPLY_SPINBOX_SETTING(config, ui, starting_golden_feathers, uint16_t(0), uint16_t(MAXIMUM_STARTING_GOLDEN_FEATHERS));
    APPLY_SPINBOX_SETTING(config, ui, starting_knights_crests, uint16_t(0), uint16_t(MAXIMUM_STARTING_KNIGHTS_CRESTS));
    APPLY_SPINBOX_SETTING(config, ui, starting_red_chu_jellys, uint16_t(0), uint16_t(MAXIMUM_STARTING_RED_CHU_JELLYS));
    APPLY_SPINBOX_SETTING(config, ui, starting_green_chu_jellys, uint16_t(0), uint16_t(MAXIMUM_STARTING_GREEN_CHU_JELLYS));
    APPLY_SPINBOX_SETTING(config, ui, starting_blue_chu_jellys, uint16_t(0), uint16_t(MAXIMUM_STARTING_BLUE_CHU_JELLYS));

    // Player Customization
    // Block signal so we don't setup colors twice
    ui->player_in_casual_clothes->blockSignals(true);
    if (config.settings.selectedModel.casual) {
        ui->player_in_casual_clothes->setCheckState(Qt::Checked);
    } else {
        ui->player_in_casual_clothes->setCheckState(Qt::Unchecked);
    }
    setup_color_options();
    ui->player_in_casual_clothes->blockSignals(false);

    // Advanced Options
    APPLY_CHECKBOX_SETTING(config, ui, do_not_generate_spoiler_log);
    APPLY_CHECKBOX_SETTING(config, ui, start_with_random_item);
    APPLY_CHECKBOX_SETTING(config, ui, random_item_slide_item);
    APPLY_CHECKBOX_SETTING(config, ui, classic_mode);
    APPLY_CHECKBOX_SETTING(config, ui, performance);
    APPLY_CHECKBOX_SETTING(config, ui, fix_rng);
    APPLY_CHECKBOX_SETTING(config, ui, plandomizer);
    update_plandomizer_widget_visbility();
    ui->plandomizer_path->setText(Utility::toQString(config.settings.plandomizerFile));

    // Hints
    APPLY_CHECKBOX_SETTING(config, ui, ho_ho_hints);
    APPLY_CHECKBOX_SETTING(config, ui, korl_hints);
    APPLY_CHECKBOX_SETTING(config, ui, use_always_hints);
    APPLY_CHECKBOX_SETTING(config, ui, clearer_hints);
    APPLY_CHECKBOX_SETTING(config, ui, hint_importance);
    APPLY_SPINBOX_SETTING(config, ui, path_hints, uint8_t(0), uint8_t(MAXIMUM_PATH_HINT_COUNT));
    APPLY_SPINBOX_SETTING(config, ui, barren_hints, uint8_t(0), uint8_t(MAXIMUM_BARREN_HINT_COUNT));
    APPLY_SPINBOX_SETTING(config, ui, location_hints, uint8_t(0), uint8_t(MAXIMUM_LOCATION_HINT_COUNT));
    APPLY_SPINBOX_SETTING(config, ui, item_hints, uint8_t(0), uint8_t(MAXIMUM_ITEM_HINT_COUNT));

    // Entrance Randomizer
    APPLY_CHECKBOX_SETTING(config, ui, randomize_dungeon_entrances);
    APPLY_CHECKBOX_SETTING(config, ui, randomize_boss_entrances);
    APPLY_CHECKBOX_SETTING(config, ui, randomize_miniboss_entrances);
    APPLY_COMBOBOX_SETTING(config, ui, randomize_cave_entrances);
    APPLY_CHECKBOX_SETTING(config, ui, randomize_door_entrances);
    APPLY_CHECKBOX_SETTING(config, ui, randomize_misc_entrances);
    APPLY_MIXED_POOLS_SETTING(config, ui, mix_dungeons);
    APPLY_MIXED_POOLS_SETTING(config, ui, mix_bosses);
    APPLY_MIXED_POOLS_SETTING(config, ui, mix_minibosses);
    APPLY_MIXED_POOLS_SETTING(config, ui, mix_caves);
    APPLY_MIXED_POOLS_SETTING(config, ui, mix_doors);
    APPLY_MIXED_POOLS_SETTING(config, ui, mix_misc);
    APPLY_CHECKBOX_SETTING(config, ui, decouple_entrances);
    APPLY_CHECKBOX_SETTING(config, ui, randomize_starting_island);

    // In-Game Preferences
    APPLY_COMBOBOX_SETTING(config, ui, target_type);
    APPLY_COMBOBOX_SETTING(config, ui, camera);
    APPLY_COMBOBOX_SETTING(config, ui, first_person_camera);
    APPLY_COMBOBOX_SETTING(config, ui, gyroscope);
    APPLY_COMBOBOX_SETTING(config, ui, ui_display);

    update_permalink_and_seed_hash();
}

void MainWindow::on_base_game_path_browse_button_clicked()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Choose Base Game Folder"), QDir::current().absolutePath(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (!dir.isEmpty() && !dir.isNull())
    {
        ui->base_game_path->setText(dir);
        config.gameBaseDir = Utility::fromQString(dir);
    }
}

void MainWindow::on_base_game_path_textChanged(const QString &arg1)
{
    config.gameBaseDir = Utility::fromQString(arg1);
}

void MainWindow::on_output_folder_browse_button_clicked()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Choose Output Folder"), QDir::current().absolutePath(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (!dir.isEmpty() && !dir.isNull())
    {
        ui->output_folder->setText(dir);
        config.outputDir = Utility::fromQString(dir);
    }
}

void MainWindow::on_output_folder_textChanged(const QString &arg1)
{
    config.outputDir = Utility::fromQString(arg1);
}

void MainWindow::on_generate_seed_button_clicked()
{
    config.seed = generate_seed();
    ui->seed->setText(config.seed.c_str());
}

void MainWindow::on_seed_textChanged(const QString &arg1)
{
    config.seed = arg1.toStdString();
    update_permalink_and_seed_hash();
}

int MainWindow::calculate_total_progress_locations()
{
    int totalProgressionLocations = 0;
    std::list<QListView*> views = {ui->excluded_locations, ui->progression_locations};
    for (auto view : views)
    {
        auto model = view->model();
        if (model == nullptr)
        {
            continue;
        }
        for (auto row = 0; row < model->rowCount(); row++)
        {
            auto locName = model->data(model->index(row, 0)).toString().toStdString();
            auto& categorySet = locationCategories[locName];
            if (std::all_of(categorySet.begin(), categorySet.end(), [this](LocationCategory category)
                {
                    if (category == LocationCategory::AlwaysProgression) return true;
                    return ( category == LocationCategory::Dungeon           && config.settings.progression_dungeons != ProgressionDungeons::Disabled)        ||
                           ( category == LocationCategory::DungeonSecret     && config.settings.progression_dungeon_secrets)                                  ||
                           ( category == LocationCategory::GreatFairy        && config.settings.progression_great_fairies)                                    ||
                           ( category == LocationCategory::PuzzleSecretCave  && config.settings.progression_puzzle_secret_caves)                              ||
                           ( category == LocationCategory::CombatSecretCave  && config.settings.progression_combat_secret_caves)                              ||
                           ( category == LocationCategory::ShortSideQuest    && config.settings.progression_short_sidequests)                                 ||
                           ( category == LocationCategory::LongSideQuest     && config.settings.progression_long_sidequests)                                  ||
                           ( category == LocationCategory::SpoilsTrading     && config.settings.progression_spoils_trading)                                   ||
                           ( category == LocationCategory::Minigame          && config.settings.progression_minigames)                                        ||
                           ( category == LocationCategory::FreeGift          && config.settings.progression_free_gifts)                                       ||
                           ( category == LocationCategory::Mail              && config.settings.progression_mail)                                             ||
                           ( category == LocationCategory::Submarine         && config.settings.progression_submarines)                                       ||
                           ( category == LocationCategory::EyeReefChests     && config.settings.progression_eye_reef_chests)                                  ||
                           ( category == LocationCategory::SunkenTreasure    && config.settings.progression_triforce_charts)                                  ||
                           ( category == LocationCategory::SunkenTreasure    && config.settings.progression_treasure_charts)                                  ||
                           ( category == LocationCategory::ExpensivePurchase && config.settings.progression_expensive_purchases)                              ||
                           ( category == LocationCategory::Misc              && config.settings.progression_misc)                                             ||
                           ( category == LocationCategory::TingleChest       && config.settings.progression_tingle_chests)                                    ||
                           ( category == LocationCategory::BattleSquid       && config.settings.progression_battlesquid)                                      ||
                           ( category == LocationCategory::SavageLabyrinth   && config.settings.progression_savage_labyrinth)                                 ||
                           ( category == LocationCategory::IslandPuzzle      && config.settings.progression_island_puzzles)                                   ||
                           ( category == LocationCategory::Obscure           && config.settings.progression_obscure)                                          ||
                           ((category == LocationCategory::Platform || category == LocationCategory::Raft)    && config.settings.progression_platforms_rafts) ||
                           ((category == LocationCategory::BigOcto  || category == LocationCategory::Gunboat) && config.settings.progression_big_octos_gunboats);
                }))
            {
                totalProgressionLocations++;
                view->setRowHidden(row, false);
            }
            else
            {
                view->setRowHidden(row, true);
            }
        }
    }

    // subtract 3 or 46 locations depending on if only one of triforce charts and treasure charts was chosen
    if (config.settings.progression_triforce_charts && !config.settings.progression_treasure_charts)
    {
        totalProgressionLocations -= 46;
    }
    else if (!config.settings.progression_triforce_charts && config.settings.progression_treasure_charts)
    {
        totalProgressionLocations -= 3;
    }

    return totalProgressionLocations;
}

void MainWindow::update_progress_locations_text()
{
    auto totalProgressionLocations = calculate_total_progress_locations();
    ui->groupBox->setTitle(std::string("Where Should Progress Items Appear? (Selected: " + std::to_string(totalProgressionLocations) + " Possible Progression Locations)").c_str());
}

// Macros which expand into functions for changing the state of config on checkboxes

// Progression Locations
DEFINE_STATE_CHANGE_FUNCTION(progression_battlesquid)
DEFINE_STATE_CHANGE_FUNCTION(progression_big_octos_gunboats)
DEFINE_STATE_CHANGE_FUNCTION(progression_combat_secret_caves)

void MainWindow::on_progression_dungeons_currentTextChanged(const QString &arg1)
{
    config.settings.progression_dungeons = nameToProgressionDungeons(arg1.toStdString());
    // Grey out the race mode dungeons combobox if race mode/standard is not selected
    if (config.settings.progression_dungeons == ProgressionDungeons::RaceMode)
    {
        ui->num_required_dungeons->setEnabled(true);
        // Change from 0 to 1 if necessary
        ui->num_required_dungeons->setItemData(0, 0, Qt::UserRole - 1);
        if (config.settings.num_required_dungeons == 0)
        {
            ui->num_required_dungeons->setCurrentIndex(1);
        }
        ui->label_for_num_required_dungeons->setEnabled(true);
        ui->label_for_num_required_dungeons->setText("Number of Race Mode Dungeons");

    }
    else if (config.settings.progression_dungeons == ProgressionDungeons::Standard)
    {
        ui->num_required_dungeons->setEnabled(true);
        ui->num_required_dungeons->setItemData(0, 33, Qt::UserRole - 1);
        ui->label_for_num_required_dungeons->setEnabled(true);
        ui->label_for_num_required_dungeons->setText("Number of Required Bosses");
    }
    else
    {
        ui->num_required_dungeons->setEnabled(false);
        ui->label_for_num_required_dungeons->setEnabled(false);
    }
    update_progress_locations_text();
    update_permalink_and_seed_hash();
}

DEFINE_STATE_CHANGE_FUNCTION(progression_dungeon_secrets)
DEFINE_STATE_CHANGE_FUNCTION(progression_expensive_purchases)
DEFINE_STATE_CHANGE_FUNCTION(progression_eye_reef_chests)
DEFINE_STATE_CHANGE_FUNCTION(progression_free_gifts)
DEFINE_STATE_CHANGE_FUNCTION(progression_great_fairies)
DEFINE_STATE_CHANGE_FUNCTION(progression_island_puzzles)
DEFINE_STATE_CHANGE_FUNCTION(progression_long_sidequests)
DEFINE_STATE_CHANGE_FUNCTION(progression_mail)
DEFINE_STATE_CHANGE_FUNCTION(progression_minigames)
DEFINE_STATE_CHANGE_FUNCTION(progression_misc)
DEFINE_STATE_CHANGE_FUNCTION(progression_obscure)
DEFINE_STATE_CHANGE_FUNCTION(progression_platforms_rafts)
DEFINE_STATE_CHANGE_FUNCTION(progression_puzzle_secret_caves)
DEFINE_STATE_CHANGE_FUNCTION(progression_savage_labyrinth)
DEFINE_STATE_CHANGE_FUNCTION(progression_short_sidequests)
DEFINE_STATE_CHANGE_FUNCTION(progression_spoils_trading)
DEFINE_STATE_CHANGE_FUNCTION(progression_submarines)
DEFINE_STATE_CHANGE_FUNCTION(progression_tingle_chests)
DEFINE_STATE_CHANGE_FUNCTION(progression_treasure_charts)
DEFINE_STATE_CHANGE_FUNCTION(progression_triforce_charts)

// Additional Randomization Options
void MainWindow::on_remove_swords_stateChanged(int arg1)
{
    UPDATE_CONFIG_STATE(config, ui, remove_swords);

    // Update randomized/starting gear lists to reflect sword mode
    auto randomizedGear = randomizedGearModel->stringList();
    auto startingGear = startingGearModel->stringList();

    // Remove all swords
    randomizedGear.removeAll("Progressive Sword");
    startingGear.removeAll("Progressive Sword");
    
    // Also make sure the hurricane spin is removed with swords
    randomizedGear.removeAll("Hurricane Spin");
    startingGear.removeAll("Hurricane Spin");

    // If the player disabled the option to remove swords
    if (!config.settings.remove_swords) {
        // Add swords back to the randomized and starting gear
        for (int i = 0; i < 3; i++)
        {
            randomizedGear.append("Progressive Sword");
        }
        startingGear.append("Progressive Sword");

        // Add back Hurricane Spin
        randomizedGear.append("Hurricane Spin");
    }

    startingGear.sort();
    randomizedGear.sort();
    randomizedGearModel->setStringList(randomizedGear);
    startingGearModel->setStringList(startingGear);
    update_starting_gear();
}

DEFINE_STATE_CHANGE_FUNCTION(randomize_charts)
DEFINE_STATE_CHANGE_FUNCTION(chest_type_matches_contents)

void MainWindow::on_damage_multiplier_valueChanged(int multiplier)
{
    config.settings.damage_multiplier = static_cast<float>(multiplier);
    update_permalink_and_seed_hash();
}

void MainWindow::on_dungeon_small_keys_currentTextChanged(const QString &arg1)
{
    config.settings.dungeon_small_keys = nameToPlacementOption(arg1.toStdString());
    update_permalink_and_seed_hash();
}


void MainWindow::on_dungeon_big_keys_currentTextChanged(const QString &arg1)
{
    config.settings.dungeon_big_keys = nameToPlacementOption(arg1.toStdString());
    update_permalink_and_seed_hash();
}


void MainWindow::on_dungeon_maps_compasses_currentTextChanged(const QString &arg1)
{
    config.settings.dungeon_maps_compasses = nameToPlacementOption(arg1.toStdString());
    update_permalink_and_seed_hash();
}

void MainWindow::on_num_required_dungeons_currentIndexChanged(int index)
{
    config.settings.num_required_dungeons = ui->num_required_dungeons->currentIndex();
    update_permalink_and_seed_hash();
}

// Convenience Tweaks
DEFINE_STATE_CHANGE_FUNCTION(invert_sea_compass_x_axis)
DEFINE_STATE_CHANGE_FUNCTION(instant_text_boxes)
DEFINE_STATE_CHANGE_FUNCTION(quiet_swift_sail)
DEFINE_STATE_CHANGE_FUNCTION(reveal_full_sea_chart)
DEFINE_STATE_CHANGE_FUNCTION(skip_rematch_bosses)
DEFINE_STATE_CHANGE_FUNCTION(add_shortcut_warps_between_dungeons)
DEFINE_STATE_CHANGE_FUNCTION(remove_music)

//Advanced Options
DEFINE_STATE_CHANGE_FUNCTION(do_not_generate_spoiler_log)
DEFINE_STATE_CHANGE_FUNCTION(start_with_random_item)
DEFINE_STATE_CHANGE_FUNCTION(random_item_slide_item)
DEFINE_STATE_CHANGE_FUNCTION(classic_mode)
DEFINE_STATE_CHANGE_FUNCTION(performance)
DEFINE_STATE_CHANGE_FUNCTION(fix_rng)
void MainWindow::on_plandomizer_stateChanged(int arg1)
{
    config.settings.plandomizer = ui->plandomizer->isChecked();

    update_plandomizer_widget_visbility();

    if(config.settings.plandomizer && config.settings.plandomizerFile.empty()) {
        on_plandomizer_path_browse_button_clicked();
    }

    update_permalink_and_seed_hash();
    update_progress_locations_text();
}

void MainWindow::validate_plandomizer_path() {
    if(!std::filesystem::is_regular_file(config.settings.plandomizerFile)) {
        // Can't rely just on setCheckState to update the config
        // If the box was already unchecked (i.e. when loading)
        // it won't actually "change" -> config won't update
        ui->plandomizer->setCheckState(Qt::Unchecked);
        config.settings.plandomizer = false;
        config.settings.plandomizerFile.clear();
        update_plandomizer_widget_visbility();
    }
}

void MainWindow::on_plandomizer_path_browse_button_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Plandomzier File"), QDir::current().absolutePath());
    if (!fileName.isEmpty() && !fileName.isNull())
    {
        ui->plandomizer_path->setText(fileName);
        config.settings.plandomizerFile = Utility::fromQString(fileName);
    }
    validate_plandomizer_path();
    update_permalink_and_seed_hash();
}

void MainWindow::on_plandomizer_path_editingFinished() {
    config.settings.plandomizerFile = Utility::fromQString(ui->plandomizer_path->text());
    validate_plandomizer_path();
    update_permalink_and_seed_hash();
}

// Starting Gear
void MainWindow::swap_selected_gear(QListView* gearFrom, QStringListModel* gearTo)
{
    // Add selected items to the gear and
    // put all selected rows into a list for sorting
    QStringList list = gearTo->stringList();
    std::list<int> selectedRows = {};
    for (auto& index : gearFrom->selectionModel()->selectedIndexes())
    {
        list << index.data().toString();
        selectedRows.push_back(index.row());
    }
    list.sort();

    // Set the sorted strings as the new starting gear list
    gearTo->setStringList(list);

    // Remove rows from the randomized gear list in reverse order
    // to not invalidate row positions as we go
    selectedRows.sort();
    selectedRows.reverse();
    for (auto& row : selectedRows)
    {
        gearFrom->model()->removeRow(row);
    }
}

void MainWindow::update_starting_gear()
{
    // Clear config starting gear and re-add everything in the starting_gear QListView
    config.settings.starting_gear.clear();
    auto startingItemsList = startingGearModel->stringList();
    for (auto& item : startingItemsList)
    {
        config.settings.starting_gear.push_back(nameToGameItem(item.toStdString()));
    }
    update_permalink_and_seed_hash();
}

void MainWindow::on_add_gear_clicked()
{
    swap_selected_gear(ui->randomized_gear, startingGearModel);
    update_starting_gear();
}

void MainWindow::on_remove_gear_clicked()
{
    swap_selected_gear(ui->starting_gear, randomizedGearModel);
    update_starting_gear();
}

void MainWindow::update_starting_health_text()
{
    int health = config.settings.starting_hcs * 4;
    health += config.settings.starting_pohs;

    int containers =  health / 4;
    int pieces = health % 4;

    std::string containersStr = std::to_string(containers) + " Hearts";
    std::string piecesStr = std::to_string(pieces) + (pieces == 1 ? " Piece" : " Pieces");

    std::string message = "Starting Health: " + containersStr + (pieces != 0 ? (" and " + piecesStr) : "");
    ui->current_health->setText(message.c_str());

    update_permalink_and_seed_hash();
}

void MainWindow::on_starting_hcs_valueChanged(int starting_hcs)
{
    config.settings.starting_hcs = starting_hcs;
    update_starting_health_text();
}

void MainWindow::on_starting_pohs_valueChanged(int starting_pohs)
{
    config.settings.starting_pohs = starting_pohs;
    update_starting_health_text();
}

DEFINE_SPINBOX_VALUE_CHANGE_FUNCTION(starting_blue_chu_jellys)
DEFINE_SPINBOX_VALUE_CHANGE_FUNCTION(starting_green_chu_jellys)
DEFINE_SPINBOX_VALUE_CHANGE_FUNCTION(starting_red_chu_jellys)
DEFINE_SPINBOX_VALUE_CHANGE_FUNCTION(starting_knights_crests)
DEFINE_SPINBOX_VALUE_CHANGE_FUNCTION(starting_golden_feathers)
DEFINE_SPINBOX_VALUE_CHANGE_FUNCTION(starting_boko_baba_seeds)
DEFINE_SPINBOX_VALUE_CHANGE_FUNCTION(starting_skull_necklaces)
DEFINE_SPINBOX_VALUE_CHANGE_FUNCTION(starting_joy_pendants)

// Excluded Locations
void MainWindow::swap_selected_locations(QListView* locsFrom, QStringListModel* locsTo)
{
    // Add selected items to the gear and
    // put all selected rows into a list for sorting
    QStringList list = locsTo->stringList();
    std::list<int> selectedRows = {};
    for (auto& index : locsFrom->selectionModel()->selectedIndexes())
    {
        list << index.data().toString();
        selectedRows.push_back(index.row());
    }
    list.sort();

    // Set the sorted strings as the new starting gear list
    locsTo->setStringList(list);

    // Remove rows from the randomized gear list in reverse order
    // to not invalidate row positions as we go
    selectedRows.sort();
    selectedRows.reverse();
    for (auto& row : selectedRows)
    {
        locsFrom->model()->removeRow(row);
    }
}

void MainWindow::update_excluded_locations()
{
    // Clear config excluded locations and re-add everything in the excluded locations QListView
    config.settings.excluded_locations.clear();
    auto excludedLocationsList = excludedLocationsModel->stringList();
    for (auto& locName : excludedLocationsList)
    {
        config.settings.excluded_locations.insert(locName.toStdString());
    }
    update_permalink_and_seed_hash();
    calculate_total_progress_locations();
}

void MainWindow::on_remove_locations_clicked()
{
    swap_selected_locations(ui->excluded_locations, progressionLocationsModel);
    update_excluded_locations();
}

void MainWindow::on_add_locations_clicked()
{
    swap_selected_locations(ui->progression_locations, excludedLocationsModel);
    update_excluded_locations();
}

// Player Customization
void MainWindow::on_player_in_casual_clothes_stateChanged(int arg1) {
    config.settings.selectedModel.casual = ui->player_in_casual_clothes->isChecked();
    setup_color_options();
}

// Advanced Options
DEFINE_STATE_CHANGE_FUNCTION(ho_ho_hints)
DEFINE_STATE_CHANGE_FUNCTION(korl_hints)
DEFINE_STATE_CHANGE_FUNCTION(use_always_hints)
DEFINE_STATE_CHANGE_FUNCTION(clearer_hints)
DEFINE_STATE_CHANGE_FUNCTION(hint_importance)
DEFINE_SPINBOX_VALUE_CHANGE_FUNCTION(path_hints)
DEFINE_SPINBOX_VALUE_CHANGE_FUNCTION(barren_hints)
DEFINE_SPINBOX_VALUE_CHANGE_FUNCTION(location_hints)
DEFINE_SPINBOX_VALUE_CHANGE_FUNCTION(item_hints)

DEFINE_STATE_CHANGE_FUNCTION(randomize_dungeon_entrances)
DEFINE_STATE_CHANGE_FUNCTION(randomize_boss_entrances)
DEFINE_STATE_CHANGE_FUNCTION(randomize_miniboss_entrances)

void MainWindow::on_randomize_cave_entrances_currentTextChanged(const QString &arg1)
{
    config.settings.randomize_cave_entrances = nameToShuffleCaveEntrances(arg1.toStdString());
    update_permalink_and_seed_hash();
}

DEFINE_STATE_CHANGE_FUNCTION(randomize_door_entrances)
DEFINE_STATE_CHANGE_FUNCTION(randomize_misc_entrances)
// Mixed pools options' states are handled in update_mixed_pools_combobox_option()
DEFINE_STATE_CHANGE_FUNCTION(decouple_entrances)
DEFINE_STATE_CHANGE_FUNCTION(randomize_starting_island)

void MainWindow::on_target_type_currentTextChanged(const QString &arg1)
{
    config.settings.target_type = nameToTargetTypePreference(arg1.toStdString());
    update_permalink_and_seed_hash();
}


void MainWindow::on_camera_currentTextChanged(const QString &arg1)
{
    config.settings.camera = nameToCameraPreference(arg1.toStdString());
    update_permalink_and_seed_hash();
}


void MainWindow::on_first_person_camera_currentTextChanged(const QString &arg1)
{
    config.settings.first_person_camera = nameToFirstPersonCameraPreference(arg1.toStdString());
    update_permalink_and_seed_hash();
}


void MainWindow::on_gyroscope_currentTextChanged(const QString &arg1)
{
    config.settings.gyroscope = nameToGyroscopePreference(arg1.toStdString());
    update_permalink_and_seed_hash();
}


void MainWindow::on_ui_display_currentTextChanged(const QString &arg1)
{
    config.settings.ui_display = nameToUIDisplayPreference(arg1.toStdString());
    update_permalink_and_seed_hash();
}

void MainWindow::update_option_description_text(const std::string& description /*= ""*/)
{
    if (description == "")
    {
        ui->option_description->setText("(Hover over an option to see a description of what it does.)");
        ui->option_description->setStyleSheet("color: grey;");
    }
    else
    {
        ui->option_description->setText(description.c_str());
        ui->option_description->setStyleSheet("");
        ui->option_description->setTextFormat(Qt::RichText);
    }
}

void MainWindow::update_permalink_and_seed_hash()
{
    ui->permalink->setText(QString::fromStdString(config.getPermalink()));
    currentPermalink = ui->permalink->text();

    // Also update seed hash
    const std::string hash = hash_for_config(config);
    if(hash.empty()) {
        show_warning_dialog("Could not get seed hash.\nPlease tell a dev and provide the error log if you see this message.");
    }

    ui->seed_hash_label->setText(QString::fromStdString("Seed Hash: " + hash));
}

void MainWindow::on_permalink_textEdited(const QString &newPermalink)
{
    // loadPermalink keeps the old config if there is an error
    const PermalinkError err = config.loadPermalink(newPermalink.toStdString());
    if (err == PermalinkError::INVALID_VERSION)
    {
        show_error_dialog("The permalink you pasted does not match the current version of the randomizer.");
        ui->permalink->setText(currentPermalink);
        return;
    }
    else if (err != PermalinkError::NONE)
    {
        show_error_dialog("The permalink you pasted is invalid.");
        ui->permalink->setText(currentPermalink);
        return;
    }
    currentPermalink = newPermalink;
    apply_config_settings();
}


void MainWindow::on_reset_settings_to_default_clicked()
{
    config.resetDefaultSettings();
    apply_config_settings();
}

void MainWindow::on_randomize_button_clicked()
{
    // Check to make sure the base game and output are directories
    if (!std::filesystem::is_directory(config.gameBaseDir))
    {
        show_warning_dialog("Must specify path to your clean base game folder (USA).", "Clean base game path not specified");
        return;
    }

    if (!std::filesystem::is_directory(config.outputDir))
    {
        show_warning_dialog("Must specify a valid output folder for the randomized files.", "No output folder specified");
        return;
    }

    // Check to make sure the output directory is not the same or a sub directory of the base game
    if (config.outputDir.u32string().starts_with(config.gameBaseDir.u32string()))
    {
        show_warning_dialog("The output folder cannot be within the base game folder. Please select a different output folder.", "Bad output folder");
        return;
    }

    // And check to make sure the plando path leads to a file
    if (config.settings.plandomizer && (!std::filesystem::exists(config.settings.plandomizerFile) || std::filesystem::is_directory(config.settings.plandomizerFile)))
    {
        show_warning_dialog("Cannot find specified plandomizer file.\nPlease check to make sure it's entered correctly.", "Bad plandomizer file path");
        return;
    }

    // Write config to file so that the main randomization algorithm can pick it up
    // and to keep compatibility with non-gui version
    ConfigError err = config.writeToFile(Utility::get_app_save_path() / "config.yaml", Utility::get_app_save_path() / "preferences.yaml");
    if(err != ConfigError::NONE) {
        show_error_dialog("Failed to write config.yaml\ncode " + ConfigErrorGetName(err));
        return;
    }

    // Setup the progress dialog for the randomization algorithm
    QPointer<QProgressDialog> progressDialog = new QProgressDialog("Initializing...", "", 0, 100, this);
    progressDialog->setWindowTitle("Randomizing");
    progressDialog->setWindowModality(Qt::WindowModal);
    progressDialog->setWindowFlag(Qt::WindowCloseButtonHint, false);
    progressDialog->setCancelButton(0);
    progressDialog->setValue(0);
    progressDialog->setMinimumWidth(250);
    progressDialog->setVisible(true);

    encounteredError = false;
    // Initialize custom thread for running the randomizer algorithm
    QPointer<RandomizerThread> thread = new RandomizerThread();
    // When the randomizer thread finishes, close the progress dialog
    connect(thread, &QThread::finished, progressDialog, &QProgressDialog::cancel);
    // Let the randomizer thread update the text and value of the progress dialog
    connect(thread, &RandomizerThread::dialogValueUpdate, progressDialog, &QProgressDialog::setValue);
    connect(thread, &RandomizerThread::dialogLabelUpdate, progressDialog, &QProgressDialog::setLabelText);
    connect(thread, &RandomizerThread::dialogTitleUpdate, progressDialog, &QProgressDialog::setWindowTitle);
    // If an error happened, let the Main Window know so it can show an error message
    connect(thread, &RandomizerThread::errorUpdate, this, &MainWindow::show_error_dialog);
    thread->start();
    progressDialog->exec();

    // Only show the finish confirmation if there was no error
    if (!encounteredError)
    {
        show_info_dialog("Randomization complete.\n\nIf you get stuck, check the progression spoiler log in the logs folder.", "Randomization complete");
    }

}

bool MainWindow::eventFilter(QObject *target, QEvent *event)
{
    if (event->type() == QEvent::Enter)
    {
        auto optionName = target->objectName();
        if (optionName.startsWith("label_for_"))
        {
            optionName = optionName.replace("label_for_", "");
        }
        // This is the name of a line edit within a spinbox. Change the name to
        // it's parent's name so that we get the correct description.
        else if (optionName == "qt_spinbox_lineedit" || optionName == "qt_scrollarea_viewport")
        {
            optionName = target->parent()->objectName();
        }

        if (optionDescriptions.contains(optionName.toStdString()))
        {
            update_option_description_text(optionDescriptions[optionName.toStdString()]);
        }
        else
        {
            update_option_description_text();
        }
        return true;
    }
    else if (event->type() == QEvent::Leave)
    {
        update_option_description_text();
        return true;
    }

    return QMainWindow::eventFilter(target, event);
}

void MainWindow::load_locations()
{   
    std::string locationDataStr;
    Utility::getFileContents(Utility::get_data_path() / "logic/location_data.yaml", locationDataStr, true);
    YAML::Node locationDataTree = YAML::Load(locationDataStr);
    for (const auto& locationObject : locationDataTree)
    {
        auto locName = locationObject["Names"]["English"].as<std::string>();
        locationCategories[locName] = {};
        for (const auto& category : locationObject["Category"])
        {
            const auto& cat = nameToLocationCategory(category.as<std::string>());
            if (cat == LocationCategory::INVALID)
            {
                show_warning_dialog("Location \"" + locName + "\" has an invalid category name \"" + category.as<std::string>() + "\"");
            }
            locationCategories[locName].insert(cat);
        }
    }
}

void MainWindow::on_about_button_clicked()
{
    auto message = "Wind Waker HD Randomizer Version " RANDOMIZER_VERSION "<br><br>"
                   "Created by Superdude88, gymnast86, csunday95, and CrainWWR<br><br>"
                   "Report issues here:<br><a href=\"https://github.com/SuperDude88/TWWHD-Randomizer/issues\">https://github.com/SuperDude88/TWWHD-Randomizer/issues</a><br><br>"
                   "Source code:<br><a href=\"https://github.com/SuperDude88/TWWHD-Randomizer\">https://github.com/SuperDude88/TWWHD-Randomizer</a><br><br>"
                   "Discord Server: <br><a href=\"https://discord.com/invite/wPvdQ2Krrm\">https://discord.com/invite/wPvdQ2Krrm</a><br><br>";
    show_info_dialog(message, "About");
}


void MainWindow::on_open_logs_folder_button_clicked()
{
    QDesktopServices::openUrl(QUrl::fromLocalFile(Utility::toQString(Utility::get_logs_path())));
}

void MainWindow::on_copy_permalink_clicked()
{
    auto permalink = ui->permalink->text();
    QGuiApplication::clipboard()->setText(permalink);
}


void MainWindow::on_paste_permalink_clicked()
{
    auto permalink = QGuiApplication::clipboard()->text();
    on_permalink_textEdited(permalink);
}
