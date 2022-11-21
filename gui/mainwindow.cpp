#include "mainwindow.h"

#include <algorithm>
#include <iostream>

#include <QMessageBox>
#include <QFileDialog>
#include <QResource>
#include <QDirIterator>
#include <QFile>

#include <ui_mainwindow.h>
#include <randomizer_thread.hpp>
#include <option_descriptions.hpp>

#include <libs/Yaml.hpp>
#include <seedgen/seed.hpp>
#include <seedgen/permalink.hpp>
#include <seedgen/tracker_permalink.hpp>
#include <utility/string.hpp>
#include <utility/file.hpp>

#define UPDATE_CONFIG_STATE(config, ui, name) config.settings.name = ui->name->isChecked(); update_permalink(); update_progress_locations_text();
#define UPDATE_CONFIG_STATE_MIXED_POOLS(config, name) config.settings.name = (name.checkState() == Qt::Checked); update_permalink();
#define APPLY_CHECKBOX_SETTING(config, ui, name) if(config.settings.name) {ui->name->setCheckState(Qt::Checked);} else {ui->name->setCheckState(Qt::Unchecked);}
#define APPLY_CONFIG_CHECKBOX_SETTING(config, ui, name) if(config.name) {ui->name->setCheckState(Qt::Checked);} else {ui->name->setCheckState(Qt::Unchecked);}

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

void delete_and_create_default_config()
{
    std::filesystem::remove(APP_SAVE_PATH "config.yaml");

    ConfigError err = createDefaultConfig(APP_SAVE_PATH "config.yaml");
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
    setup_mixed_pools_combobox();
    load_locations();
    load_config_into_ui();
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

    // Hide options which won't exist for a while
    ui->randomize_enemies->setVisible(false);
    ui->randomize_enemy_palettes->setVisible(false);
    ui->randomize_music->setVisible(false);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    // Write settings to file when user closes the program
    ConfigError err = writeToFile(APP_SAVE_PATH "config.yaml", config);
    if (err != ConfigError:: NONE)
    {
        show_error_dialog("Settings could not be saved\nCode: " + errorToName(err));
    }

    update_encryption_files();
}

void MainWindow::load_config_into_ui()
{
    // Ignore errors and just load in whatever we can. The gui will write a proper config file
    // when the user begins randomization
    ConfigError err = loadFromFile(APP_SAVE_PATH "config.yaml", config, true);
    if (err != ConfigError::NONE)
    {
        show_error_dialog("Failed to load settings file\ncode " + errorToName(err));
    }
    else
    {
        apply_config_settings();
    }

    // Load encryption keys if the files exists
    auto encryptionKeyPath = "./encryption.txt";
    std::string encryptionKeyStr;
    if (Utility::getFileContents(encryptionKeyPath, encryptionKeyStr) != 1)
    {
        ui->encryption_key->setText(encryptionKeyStr.c_str());
    }

    auto commonKeyPath = "./common.txt";
    std::string commonKeyStr;
    if (Utility::getFileContents(commonKeyPath, commonKeyStr) != 1)
    {
        ui->wii_u_common_key->setText(commonKeyStr.c_str());
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
    poolNames << "Dungeons" << "Caves" << "Doors" << "Misc";
    poolCheckBoxes << &mix_dungeons << &mix_caves << &mix_doors << &mix_misc;
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
    else if (enabledPools.size() <= 2)
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
                   << "Earth God's Lyric"
                   << "Earth Temple Big Key"
                   << "Earth Temple Compass"
                   << "Earth Temple Dungeon Map"
                   << "Earth Temple Small Key"
                   << "Earth Temple Small Key"
                   << "Earth Temple Small Key"
                   << "Empty Bottle"
                   << "Farore's Pearl"
                   << "Forbidden Woods Big Key"
                   << "Forbidden Woods Compass"
                   << "Forbidden Woods Dungeon Map"
                   << "Forbidden Woods Small Key"
                   << "Forsaken Fortress Compass"
                   << "Forsaken Fortress Dungeon Map"
                   << "Ghost Ship Chart"
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
                   << "Wind God's Aria"
                   << "Wind Temple Big Key"
                   << "Wind Temple Compass"
                   << "Wind Temple Dungeon Map"
                   << "Wind Temple Small Key"
                   << "Wind Temple Small Key";

    randomizedGearModel->setStringList(randomizedList);
    ui->randomized_gear->setModel(randomizedGearModel);
    ui->starting_gear->setModel(startingGearModel);
    ui->randomized_gear->setSelectionMode(QAbstractItemView::ExtendedSelection);
    ui->starting_gear->setSelectionMode(QAbstractItemView::ExtendedSelection);
}

void MainWindow::update_plandomizer_widget_visbility()
{
    // Hide plandomizer input widgets when plandomizer isn't check
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
    // Directories and Seed
    ui->base_game_path->setText(config.gameBaseDir.c_str());
    ui->output_folder->setText(config.outputDir.c_str());
    APPLY_CONFIG_CHECKBOX_SETTING(config, ui, repack_for_console);
    on_repack_for_console_stateChanged(0); // hide console repacking elements if necessary
    ui->console_output->setText(config.consoleOutputDir.c_str());
    ui->seed->setText(config.seed.c_str());

    // Progression settings
    APPLY_CHECKBOX_SETTING(config, ui, progression_battlesquid);
    APPLY_CHECKBOX_SETTING(config, ui, progression_big_octos_gunboats);
    APPLY_CHECKBOX_SETTING(config, ui, progression_combat_secret_caves);
    APPLY_CHECKBOX_SETTING(config, ui, progression_dungeons);
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

    // Starting Gear
    // Set this up before changing the sword_mode option since that function
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

    // Additional Randomizer Settings
    auto& sword_mode = config.settings.sword_mode;
    if (sword_mode == SwordMode::INVALID)
    {
        show_warning_dialog("Unknown sword mode.\nSetting as default value Start with Hero's Sword");
        sword_mode = SwordMode::StartWithSword;
    }
    auto index = static_cast<int>(sword_mode);
    ui->sword_mode->setCurrentIndex(index);

    // Call the index change slot function if the option didn't change from the default
    if (sword_mode == SwordMode::StartWithSword)
    {
        on_sword_mode_currentIndexChanged(index);
    }

    APPLY_SPINBOX_SETTING(config, ui, damage_multiplier, float(2.0f), float(80.0f));
    APPLY_CHECKBOX_SETTING(config, ui, keylunacy);
    APPLY_CHECKBOX_SETTING(config, ui, race_mode);
    // Call the slot for the race_mode state change incase it didn't change
    // to potentially grey out the num_race_mode_dungeons combobox
    on_race_mode_stateChanged(0);

    auto& num_race_mode_dungeons = config.settings.num_race_mode_dungeons;
    // Race mode dungeons must be between 1 and 6
    num_race_mode_dungeons = std::clamp(num_race_mode_dungeons, uint8_t(1), uint8_t(6));
    ui->num_race_mode_dungeons->setCurrentIndex(num_race_mode_dungeons - 1);

    auto& num_starting_triforce_shards = config.settings.num_starting_triforce_shards;
    // Number of starting triforce shards must be between 0 and 8
    num_starting_triforce_shards = std::clamp(num_starting_triforce_shards, uint8_t(0), uint8_t(8));
    ui->num_starting_triforce_shards->setCurrentIndex(num_starting_triforce_shards);

    APPLY_CHECKBOX_SETTING(config, ui, randomize_charts);
    APPLY_CHECKBOX_SETTING(config, ui, chest_type_matches_contents);

    // Convenience Tweaks
    APPLY_CHECKBOX_SETTING(config, ui, invert_sea_compass_x_axis);
    APPLY_CHECKBOX_SETTING(config, ui, instant_text_boxes);
    APPLY_CHECKBOX_SETTING(config, ui, reveal_full_sea_chart);
    APPLY_CHECKBOX_SETTING(config, ui, skip_rematch_bosses);
    APPLY_CHECKBOX_SETTING(config, ui, add_shortcut_warps_between_dungeons);
    APPLY_CHECKBOX_SETTING(config, ui, remove_music);

    // Starting Health
    APPLY_SPINBOX_SETTING(config, ui, starting_hcs, uint16_t(0), uint16_t(6));
    APPLY_SPINBOX_SETTING(config, ui, starting_pohs, uint16_t(0), uint16_t(44));

    // Player Customization
    APPLY_CHECKBOX_SETTING(config, ui, player_in_casual_clothes);

    // Advanced Options
    APPLY_CHECKBOX_SETTING(config, ui, do_not_generate_spoiler_log);
    APPLY_CHECKBOX_SETTING(config, ui, start_with_random_item);
    APPLY_CHECKBOX_SETTING(config, ui, plandomizer);
    update_plandomizer_widget_visbility();
    ui->plandomizer_path->setText(config.settings.plandomizerFile.c_str());

    // Hints
    APPLY_CHECKBOX_SETTING(config, ui, ho_ho_hints);
    APPLY_CHECKBOX_SETTING(config, ui, korl_hints);
    APPLY_CHECKBOX_SETTING(config, ui, use_always_hints);
    APPLY_CHECKBOX_SETTING(config, ui, clearer_hints);
    APPLY_SPINBOX_SETTING(config, ui, path_hints, uint8_t(0), uint8_t(7));
    APPLY_SPINBOX_SETTING(config, ui, barren_hints, uint8_t(0), uint8_t(7));
    APPLY_SPINBOX_SETTING(config, ui, location_hints, uint8_t(0), uint8_t(7));
    APPLY_SPINBOX_SETTING(config, ui, item_hints, uint8_t(0), uint8_t(7));

    // Entrance Randomizer
    APPLY_CHECKBOX_SETTING(config, ui, randomize_dungeon_entrances);
    APPLY_CHECKBOX_SETTING(config, ui, randomize_cave_entrances);
    APPLY_CHECKBOX_SETTING(config, ui, randomize_door_entrances);
    APPLY_CHECKBOX_SETTING(config, ui, randomize_misc_entrances);
    APPLY_MIXED_POOLS_SETTING(config, ui, mix_dungeons);
    APPLY_MIXED_POOLS_SETTING(config, ui, mix_caves);
    APPLY_MIXED_POOLS_SETTING(config, ui, mix_doors);
    APPLY_MIXED_POOLS_SETTING(config, ui, mix_misc);
    APPLY_CHECKBOX_SETTING(config, ui, decouple_entrances);
    APPLY_CHECKBOX_SETTING(config, ui, randomize_starting_island);

    // Permalink
    ui->permalink->setText(create_permalink(config.settings, config.seed).c_str());
}

void MainWindow::on_base_game_path_browse_button_clicked()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Folder"), QDir::current().absolutePath(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (!dir.isEmpty() && !dir.isNull())
    {
        ui->base_game_path->setText(dir);
        config.gameBaseDir = dir.toStdString();
    }
}

void MainWindow::on_output_folder_browse_button_clicked()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Folder"), QDir::current().absolutePath(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (!dir.isEmpty() && !dir.isNull())
    {
        ui->output_folder->setText(dir);
        config.outputDir = dir.toStdString();
    }
}

void MainWindow::on_repack_for_console_stateChanged(int arg1)
{
    if (ui->repack_for_console->isChecked())
    {
        ui->label_for_console_output->setVisible(true);
        ui->console_output_browse_button->setVisible(true);
        ui->console_output->setVisible(true);
        ui->wii_u_common_key->setVisible(true);
        ui->label_for_wii_u_common_key->setVisible(true);
        ui->encryption_key->setVisible(true);
        ui->label_for_encryption_key->setVisible(true);
        config.repack_for_console = true;
    }
    else
    {
        ui->label_for_console_output->setVisible(false);
        ui->console_output_browse_button->setVisible(false);
        ui->console_output->setVisible(false);
        ui->wii_u_common_key->setVisible(false);
        ui->label_for_wii_u_common_key->setVisible(false);
        ui->encryption_key->setVisible(false);
        ui->label_for_encryption_key->setVisible(false);
        config.repack_for_console = false;
    }
}

void MainWindow::on_console_output_browse_button_clicked()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Folder"), QDir::current().absolutePath(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (!dir.isEmpty() && !dir.isNull())
    {
        ui->console_output->setText(dir);
        config.consoleOutputDir = dir.toStdString();
    }
}

void MainWindow::on_generate_seed_button_clicked()
{
    std::string seed = generate_seed();
    config.seed = seed;
    ui->seed->setText(seed.c_str());
    update_permalink();
}

void MainWindow::update_progress_locations_text()
{
    int totalProgressionLocations = 0;
    for (auto& categorySet : locationCategories)
    {
        if (std::all_of(categorySet.begin(), categorySet.end(), [this](LocationCategory category)
        {
            if (category == LocationCategory::Junk) return false;
            if (category == LocationCategory::AlwaysProgression) return true;
            return ( category == LocationCategory::Dungeon           && config.settings.progression_dungeons)            ||
                   ( category == LocationCategory::GreatFairy        && config.settings.progression_great_fairies)       ||
                   ( category == LocationCategory::PuzzleSecretCave  && config.settings.progression_puzzle_secret_caves) ||
                   ( category == LocationCategory::CombatSecretCave  && config.settings.progression_combat_secret_caves) ||
                   ( category == LocationCategory::ShortSideQuest    && config.settings.progression_short_sidequests)    ||
                   ( category == LocationCategory::LongSideQuest     && config.settings.progression_long_sidequests)     ||
                   ( category == LocationCategory::SpoilsTrading     && config.settings.progression_spoils_trading)      ||
                   ( category == LocationCategory::Minigame          && config.settings.progression_minigames)           ||
                   ( category == LocationCategory::FreeGift          && config.settings.progression_free_gifts)          ||
                   ( category == LocationCategory::Mail              && config.settings.progression_mail)                ||
                   ( category == LocationCategory::Submarine         && config.settings.progression_submarines)          ||
                   ( category == LocationCategory::EyeReefChests     && config.settings.progression_eye_reef_chests)     ||
                   ( category == LocationCategory::SunkenTreasure    && config.settings.progression_triforce_charts)     ||
                   ( category == LocationCategory::SunkenTreasure    && config.settings.progression_treasure_charts)     ||
                   ( category == LocationCategory::ExpensivePurchase && config.settings.progression_expensive_purchases) ||
                   ( category == LocationCategory::Misc              && config.settings.progression_misc)                ||
                   ( category == LocationCategory::TingleChest       && config.settings.progression_tingle_chests)       ||
                   ( category == LocationCategory::BattleSquid       && config.settings.progression_battlesquid)         ||
                   ( category == LocationCategory::SavageLabyrinth   && config.settings.progression_savage_labyrinth)    ||
                   ( category == LocationCategory::IslandPuzzle      && config.settings.progression_island_puzzles)      ||
                   ( category == LocationCategory::Obscure           && config.settings.progression_obscure)             ||
                   ((category == LocationCategory::Platform || category == LocationCategory::Raft)    && config.settings.progression_platforms_rafts) ||
                   ((category == LocationCategory::BigOcto  || category == LocationCategory::Gunboat) && config.settings.progression_big_octos_gunboats);
        }))
        {
            totalProgressionLocations++;
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

    ui->groupBox->setTitle(std::string("Where Should Progress Items Appear? (Selected: " + std::to_string(totalProgressionLocations) + " Possible Progression Locations)").c_str());
}

// Macros which expand into functions for changing the state of config on checkboxes

// Progression Locations
DEFINE_STATE_CHANGE_FUNCTION(progression_battlesquid)
DEFINE_STATE_CHANGE_FUNCTION(progression_big_octos_gunboats)
DEFINE_STATE_CHANGE_FUNCTION(progression_combat_secret_caves)
DEFINE_STATE_CHANGE_FUNCTION(progression_dungeons)
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
void MainWindow::on_sword_mode_currentIndexChanged(int index)
{
    config.settings.sword_mode = static_cast<SwordMode>(ui->sword_mode->currentIndex());

    // Update randomized/starting gear lists to reflect sword mode
    auto randomizedGear = randomizedGearModel->stringList();
    auto startingGear = startingGearModel->stringList();

    int numSwordsInGearMenus = randomizedGear.filter("Progressive Sword").size() + startingGear.filter("Progressive Sword").size();

    // If the player set the option to No Starting Sword or Swordless, remove all swords
    auto& sword_mode = config.settings.sword_mode;
    if (sword_mode == SwordMode::RandomSword || sword_mode == SwordMode::NoSword)
    {
        randomizedGear.removeAll("Progressive Sword");
        startingGear.removeAll("Progressive Sword");
    }
    // If the player set the option to Start with Hero's Sword
    else if (config.settings.sword_mode == SwordMode::StartWithSword)
    {
        // Add swords to the randomized gear until there are 3
        while (numSwordsInGearMenus < 3)
        {
            randomizedGear.append("Progressive Sword");
            numSwordsInGearMenus++;
        }
    }

    // Make sure the hurricane spin is removed when Swordless is enabled
    if (sword_mode == SwordMode::NoSword)
    {
        randomizedGear.removeAll("Hurricane Spin");
        startingGear.removeAll("Hurricane Spin");
    }
    else if (randomizedGear.filter("Hurricane Spin").size() + startingGear.filter("Hurricane Spin").size() == 0)
    {
        randomizedGear.append("Hurricane Spin");
    }
    startingGear.sort();
    randomizedGear.sort();
    randomizedGearModel->setStringList(randomizedGear);
    startingGearModel->setStringList(startingGear);
    update_permalink();
}

DEFINE_STATE_CHANGE_FUNCTION(randomize_charts)
DEFINE_STATE_CHANGE_FUNCTION(chest_type_matches_contents)

void MainWindow::on_damage_multiplier_valueChanged(int multiplier)
{
    config.settings.damage_multiplier = static_cast<float>(multiplier);
}

DEFINE_STATE_CHANGE_FUNCTION(keylunacy)

void MainWindow::on_race_mode_stateChanged(int arg1) {
    UPDATE_CONFIG_STATE(config, ui, race_mode);
    // Grey out the race mode dungeons combobox if race mode is off
    if (config.settings.race_mode)
    {
        ui->num_race_mode_dungeons->setEnabled(true);
        ui->label_for_num_race_mode_dungeons->setEnabled(true);
    }
    else
    {
        ui->num_race_mode_dungeons->setEnabled(false);
        ui->label_for_num_race_mode_dungeons->setEnabled(false);
    }
}

void MainWindow::on_num_starting_triforce_shards_currentIndexChanged(int index)
{
    config.settings.num_starting_triforce_shards = ui->num_starting_triforce_shards->currentIndex();
    update_permalink();
}

void MainWindow::on_num_race_mode_dungeons_currentIndexChanged(int index)
{
    config.settings.num_race_mode_dungeons = ui->num_race_mode_dungeons->currentIndex() + 1;
    update_permalink();
}

// Convenience Tweaks
DEFINE_STATE_CHANGE_FUNCTION(invert_sea_compass_x_axis)
DEFINE_STATE_CHANGE_FUNCTION(instant_text_boxes)
DEFINE_STATE_CHANGE_FUNCTION(reveal_full_sea_chart)
DEFINE_STATE_CHANGE_FUNCTION(skip_rematch_bosses)
DEFINE_STATE_CHANGE_FUNCTION(add_shortcut_warps_between_dungeons)
DEFINE_STATE_CHANGE_FUNCTION(remove_music)

//Advanced Options
DEFINE_STATE_CHANGE_FUNCTION(do_not_generate_spoiler_log)
DEFINE_STATE_CHANGE_FUNCTION(start_with_random_item)
void MainWindow::on_plandomizer_stateChanged(int arg1)
{
    UPDATE_CONFIG_STATE(config, ui, plandomizer);
    update_permalink();
    update_plandomizer_widget_visbility();
}

void MainWindow::on_plandomizer_path_browse_button_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Plandomzier File"), QDir::current().absolutePath());
    if (!fileName.isEmpty() && !fileName.isNull())
    {
        ui->plandomizer_path->setText(fileName);
        config.settings.plandomizerFile = fileName.toStdString();
    }
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
    update_permalink();
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
    int health = 12 + (config.settings.starting_hcs * 4);
    health += config.settings.starting_pohs;

    int containers =  health / 4;
    int pieces = health % 4;

    std::string containersStr = std::to_string(containers) + " Hearts";
    std::string piecesStr = std::to_string(pieces) + (pieces == 1 ? " Piece" : " Pieces");

    std::string message = "Current Starting Health: " + containersStr + " and " + piecesStr;
    ui->current_health->setText(message.c_str());

    update_permalink();
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

// Player Customization
DEFINE_STATE_CHANGE_FUNCTION(player_in_casual_clothes)

// Advanced Options
DEFINE_STATE_CHANGE_FUNCTION(ho_ho_hints)
DEFINE_STATE_CHANGE_FUNCTION(korl_hints)
DEFINE_STATE_CHANGE_FUNCTION(use_always_hints)
DEFINE_STATE_CHANGE_FUNCTION(clearer_hints)
void MainWindow::on_path_hints_valueChanged(int path_hints)
{
    config.settings.path_hints = path_hints;
    update_permalink();
}

void MainWindow::on_barren_hints_valueChanged(int barren_hints)
{
    config.settings.barren_hints = barren_hints;
    update_permalink();
}

void MainWindow::on_location_hints_valueChanged(int location_hints)
{
    config.settings.location_hints = location_hints;
    update_permalink();
}

void MainWindow::on_item_hints_valueChanged(int item_hints)
{
    config.settings.item_hints = item_hints;
    update_permalink();
}

DEFINE_STATE_CHANGE_FUNCTION(randomize_dungeon_entrances)
DEFINE_STATE_CHANGE_FUNCTION(randomize_cave_entrances)
DEFINE_STATE_CHANGE_FUNCTION(randomize_door_entrances)
DEFINE_STATE_CHANGE_FUNCTION(randomize_misc_entrances)
// Mixed pools options' states are handled in update_mixed_pools_combobox_option()
DEFINE_STATE_CHANGE_FUNCTION(decouple_entrances)
DEFINE_STATE_CHANGE_FUNCTION(randomize_starting_island)

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

void MainWindow::update_permalink()
{
    ui->permalink->setText(create_permalink(config.settings, config.seed).c_str());
    currentPermalink = ui->permalink->text();
    ui->tracker_permalink->setText(create_tracker_permalink(config.settings, config.seed).c_str());
}

void MainWindow::on_permalink_textEdited(const QString &newPermalink)
{
    Config oldConfig = config;

    PermalinkError err = parse_permalink(newPermalink.toStdString(), config.settings, config.seed);
    if (err == PermalinkError::INVALID_VERSION)
    {
        show_error_dialog("The permalink you pasted does not match the current version of the randomizer.");
        config = oldConfig;
        ui->permalink->setText(currentPermalink);
        return;
    }
    else if (err == PermalinkError::BAD_PERMALINK)
    {
        show_error_dialog("The permalink you pasted is invalid.");
        config = oldConfig;
        ui->permalink->setText(currentPermalink);
        return;
    }
    currentPermalink = newPermalink;
    apply_config_settings();
}


void MainWindow::on_reset_settings_to_default_clicked()
{
    delete_and_create_default_config();

    // Don't reset any paths the user has specified
    auto previousGameBaseDir = config.gameBaseDir;
    auto previousOutputDir = config.outputDir;
    auto previousSeed = config.seed;
    auto previousPlandomizerFile = config.settings.plandomizerFile;

    // Load new config
    load_config_into_ui();

    // Restore previous paths
    config.gameBaseDir = previousGameBaseDir;
    config.outputDir = previousOutputDir;
    config.seed = previousSeed;
    config.settings.plandomizerFile = previousPlandomizerFile;
    apply_config_settings();
}

void MainWindow::update_encryption_files()
{
    // Write encryption and common keys to separate files so users can share config files without sharing the keys
    if (ui->encryption_key->text() != "")
    {
        std::ofstream file("./encryption.txt");
        if (file.is_open())
        {
            file << ui->encryption_key->text().toStdString();
            file.close();
        }
    }

    if (ui->wii_u_common_key->text() != "")
    {
        std::ofstream file("./common.txt");
        if (file.is_open())
        {
            file << ui->wii_u_common_key->text().toStdString();
            file.close();
        }
    }
}

void MainWindow::on_randomize_button_clicked()
{
    // TODO: remove when korl text is fixed
    int totalNumHints = config.settings.item_hints + config.settings.barren_hints + config.settings.location_hints + config.settings.path_hints;
    if (config.settings.korl_hints && !config.settings.ho_ho_hints && totalNumHints > 10)
    {
        show_warning_dialog("Currently there is a bug where the King of Red Lions will softlock you if he has more than 10 hints. Please reduce the number of hints to 10 or less, or enable Ho-Ho hints as well", "Hint Warning");
        return;
    }

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
    // And check to make sure the plando path leads to a file
    if (config.settings.plandomizer && (!std::filesystem::exists(config.settings.plandomizerFile) || std::filesystem::is_directory(config.settings.plandomizerFile)))
    {
        show_warning_dialog("Cannot find specified plandomizer file.\nPlease check to make sure it's entered correctly.", "Bad plandomizer file path");
        return;
    }

    if (config.repack_for_console)
    {
        if (!std::filesystem::is_directory(config.consoleOutputDir))
        {
            show_warning_dialog("Must specify a valid output folder for the repacked console files.", "No console output folder specified");
            return;
        }
        update_encryption_files();
    }

    // Write config to file so that the main randomization algorithm can pick it up
    // and to keep compatibility with non-gui version
    ConfigError err = writeToFile(APP_SAVE_PATH "config.yaml", config);
    if(err != ConfigError::NONE) {
        show_error_dialog("Failed to write config.yaml\ncode " + errorToName(err));
        return;
    }

    // Setup the progress dialog for the randomization algorithm
    int progressTotal = config.repack_for_console ? 200 : 100;
    QPointer<QProgressDialog> progressDialog = new QProgressDialog("Initializing...", "", 0, progressTotal, this);
    progressDialog->setWindowTitle("Randomizing");
    progressDialog->setWindowModality(Qt::WindowModal);
    progressDialog->setWindowFlag(Qt::WindowCloseButtonHint, false);
    progressDialog->setCancelButton(0);
    progressDialog->setValue(0);
    progressDialog->setVisible(true);

    encounteredError = false;
    // Initialize custom thread for running the randomizer algorithm
    QPointer<RandomizerThread> thread = new RandomizerThread();
    // When the randomizer thread finishes, close the progress dialog
    connect(thread, &QThread::finished, progressDialog, &QProgressDialog::cancel);
    // Let the randomizer thread update the text and value of the progress dialog
    connect(thread, &RandomizerThread::dialogValueUpdate, progressDialog, &QProgressDialog::setValue);
    connect(thread, &RandomizerThread::dialogLabelUpdate, progressDialog, &QProgressDialog::setLabelText);
    // If an error happened, let the Main Window know so it can show an error message
    connect(thread, &RandomizerThread::errorUpdate, this, &MainWindow::show_error_dialog);
    thread->start();
    progressDialog->exec();

    // Only show the finish confirmation if there was no error
    if (!encounteredError)
    {
        show_info_dialog("Randomization complete.\n\nIf you get stuck, check the progression spoiler log in the folder where this program is.", "Randomization complete");
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
        else if (optionName == "qt_spinbox_lineedit")
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
    QResource file(":/logic/data/location_data.yaml");
    std::string locationDataStr = file.uncompressedData().toStdString();

    locationDataStr = Utility::Str::InsertUnicodeReplacements(locationDataStr);
    Yaml::Node locationDataTree;
    Yaml::Parse(locationDataTree, locationDataStr);
    for (auto locationObjectIt = locationDataTree.Begin(); locationObjectIt != locationDataTree.End(); locationObjectIt++)
    {
        Yaml::Node& locationObject = (*locationObjectIt).second;
        std::string name = Utility::Str::RemoveUnicodeReplacements(locationObject["Names"]["English"].As<std::string>());
        locationCategories.push_back({});
        for (auto categoryIt = locationObject["Category"].Begin(); categoryIt != locationObject["Category"].End(); categoryIt++)
        {
            Yaml::Node category = (*categoryIt).second;
            const std::string& categoryNameStr = category.As<std::string>();
            const auto& cat = nameToLocationCategory(categoryNameStr);
            if (cat == LocationCategory::INVALID)
            {
                show_warning_dialog("Location \"" + name + "\" has an invalid category name \"" + categoryNameStr + "\"");
            }
            locationCategories.back().insert(cat);
        }
    }
}
