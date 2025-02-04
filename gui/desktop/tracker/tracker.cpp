#include "../mainwindow.hpp"

#include <QAbstractButton>
#include <QMouseEvent>
#include <QMessageBox>
#include <QColorDialog>

#include <../ui_mainwindow.h>

#include <filesystem>

#include <../ui_mainwindow.h>
#include <gui/desktop/tracker/tracker_inventory_button.hpp>
#include <gui/desktop/tracker/tracker_area_widget.hpp>
#include <gui/desktop/tracker/tracker_preferences_dialog.hpp>
#include <gui/desktop/tracker/tracker_label.hpp>
#include <gui/desktop/tracker/set_font.hpp>

#include <logic/Fill.hpp>
#include <logic/Search.hpp>
#include <logic/PoolFunctions.hpp>
#include <logic/EntranceShuffle.hpp>
#include <logic/flatten/flatten.hpp>
#include <utility/path.hpp>
#include <utility/file.hpp>
#include <utility/string.hpp>
#include <utility/color.hpp>

void MainWindow::initialize_tracker_world(Settings& settings,
                                          const GameItemPool& markedItems,
                                          const std::vector<std::string>& markedLocations,
                                          const std::unordered_map<std::string, std::string>& connectedEntrances,
                                          const std::map<GameItem, uint8_t>& chartMappings)
{
    trackerStarted = true;

    // Build the world used for the tracker
    auto& trackerWorld = trackerWorlds[0];
    trackerLocations.clear();
    trackerInventory.clear();

    trackerWorld = World();
    trackerWorld.setWorldId(0);

    // Copy settings to modify them
    trackerSettings = settings;

    selectedChartIsland = 0;
    if (settings.randomize_charts)
    {
        // With charts shuffled the required ones can be on any island
        // So for tracker purposes, treat all 49 charts as potentially progress
        if (trackerSettings.progression_treasure_charts || trackerSettings.progression_triforce_charts)
        {
            trackerSettings.progression_treasure_charts = true;
            trackerSettings.progression_triforce_charts = true;
        }
    }

    // Give 3 hearts so that all heart checks pass logic
    trackerSettings.starting_hcs = 3;

    trackerWorld.setSettings(trackerSettings);

    if(trackerWorld.getSettings().plandomizer)
    {
        std::vector<Plandomizer> plandos(1);
        PlandomizerError err = loadPlandomizer(trackerWorld.getSettings().plandomizerFile, plandos, 1);
        if (err != PlandomizerError::NONE)
        {
            show_warning_dialog("Could not load provided plandomizer file. Continuing without plandomizer data.");
        }
        else {
            trackerWorld.plandomizer = plandos[0];
        }
    }

    if (trackerWorld.loadWorld(Utility::get_data_path() / "logic/world.yaml", Utility::get_data_path() / "logic/macros.yaml", Utility::get_data_path() / "logic/location_data.yaml", Utility::get_data_path() / "logic/item_data.yaml", Utility::get_data_path() / "logic/area_names.yaml"))
    {
        show_error_dialog("Could not build world for app tracker");
        return;
    }

    trackerWorld.determineChartMappings();
    // Use loaded chart mappings
    mappedCharts.clear();
    if(trackerSettings.randomize_charts) {
        for(const auto& [chart, island] : chartMappings) {
            mapChart(chart, island);
        }
    }
    // Otherwise map like vanilla
    else {
        for(size_t i = 1; i < 50; i++) {
            mapChart(roomNumToDefaultChart(i), i);
        }
    }

    trackerWorld.determineProgressionLocations();
    trackerWorld.setItemPools();

    // Tingle Statues have to be set manually since they can
    // be started with in any order
    auto& startingInventory = trackerWorld.getStartingItemsReference();
    std::vector<Item> tingleStatues = {
        Item(GameItem::DragonTingleStatue, &trackerWorld),
        Item(GameItem::ForbiddenTingleStatue, &trackerWorld),
        Item(GameItem::GoddessTingleStatue, &trackerWorld),
        Item(GameItem::EarthTingleStatue, &trackerWorld),
        Item(GameItem::WindTingleStatue, &trackerWorld),
    };

    auto startingTingleStatues = std::count_if(startingInventory.begin(), startingInventory.end(), [&](Item& item){
        return elementInPool(item, tingleStatues);
    });
    removeElementsFromPool(startingInventory, tingleStatues);
    for (int i = 0; i < startingTingleStatues; i++)
    {
        addElementToPool(startingInventory, tingleStatues[i]);
    }

    placeVanillaItems(trackerWorlds);

    setup_tracker_entrances();

    // Connect autosaved entrances
    auto shuffledEntrances = trackerWorld.getShuffledEntrances(EntranceType::ALL);
    for (auto entrance : shuffledEntrances)
    {
        auto entranceStr = entrance->getOriginalName();
        if (connectedEntrances.contains(entrance->getOriginalName()))
        {
            for (auto target : targetEntrancePools[entrance->getEntranceType()])
            {
                auto targetStr = target->getReplaces()->getOriginalName();
                if (targetStr == connectedEntrances.at(entranceStr))
                {
                    changeConnections(entrance, target);
                    connectedTargets[target] = entrance;
                }
            }
        }
    }

    trackerLocations = trackerWorld.getProgressionLocations();

    set_areas_locations();

    for (auto& gameItemId : markedItems)
    {
        trackerInventory.emplace_back(gameItemId, &trackerWorld);
    }

    for (auto& locName : markedLocations)
    {
        if (trackerWorld.locationTable.contains(locName))
        {
            trackerWorld.locationTable[locName]->marked = true;
        }
    }

    calculate_own_dungeon_key_locations();

    // Update inventory gui to have starting items
    auto startingInventoryCopy = startingInventory;
    auto trackerInventoryCopy = trackerInventory;

    // Make sure our buttons start at 0
    for (auto inventoryButton : ui->tracker_tab->findChildren<TrackerInventoryButton*>())
    {
        inventoryButton->setState(0);
        inventoryButton->clearForbiddenStates();
    }

    // Update buttons with starting inventory items
    for (auto inventoryButton : ui->tracker_tab->findChildren<TrackerInventoryButton*>())
    {
        // Update button with starting items first to set forbidden states
        for (auto& itemState : inventoryButton->itemStates)
        {
            auto item = Item(itemState.gameItem, &trackerWorld);
            if (itemState.gameItem != GameItem::NOTHING && elementInPool(item, startingInventoryCopy))
            {
                inventoryButton->addForbiddenState(inventoryButton->getState());
                inventoryButton->setState(inventoryButton->getState() + 1);
                removeElementFromPool(startingInventoryCopy, item);
            }
        }

        // Then update buttons with the current inventory
        for (auto& itemState : inventoryButton->itemStates)
        {
            auto item = Item(itemState.gameItem, &trackerWorld);
            if (itemState.gameItem != GameItem::NOTHING && elementInPool(item, trackerInventoryCopy))
            {
                inventoryButton->setState(inventoryButton->getState() + 1);
                removeElementFromPool(trackerInventoryCopy, item);
            }
        }

        // Then update the icon
        inventoryButton->updateIcon();
    }

    // Set locations/entrances for each area
    for (auto area : ui->tracker_tab->findChildren<TrackerAreaWidget*>())
    {
        area->setLocations(&areaLocations);
        area->setEntrances(&areaEntrances);

        std::string areaName = area->getPrefix();
        // TODO: figure out some better way to do this than just if/else
        if (areaName == "Dragon Roost Cavern")
        {
            area->setBossLocation(trackerWorld.locationTable["Dragon Roost Cavern - Gohma Heart Container"].get());
        }
        else if (areaName == "Forbidden Woods")
        {
            area->setBossLocation(trackerWorld.locationTable["Forbidden Woods - Kalle Demos Heart Container"].get());
        }
        else if (areaName == "Tower of the Gods")
        {
            area->setBossLocation(trackerWorld.locationTable["Tower of the Gods - Gohdan Heart Container"].get());
        }
        else if (areaName == "Forsaken Fortress")
        {
            area->setBossLocation(trackerWorld.locationTable["Forsaken Fortress - Helmaroc King Heart Container"].get());
        }
        else if (areaName == "Earth Temple")
        {
            area->setBossLocation(trackerWorld.locationTable["Earth Temple - Jalhalla Heart Container"].get());
        }
        else if (areaName == "Wind Temple")
        {
            area->setBossLocation(trackerWorld.locationTable["Wind Temple - Molgera Heart Container"].get());
        }
        else if (areaName == "Ganon's Tower")
        {
            area->setBossLocation(trackerWorld.locationTable["Ganon's Tower - Defeat Ganondorf"].get());
        }
    }

    // Update areas and entrances for all islands
    set_areas_locations();
    set_areas_entrances();

    // Setup the display of randomized entrances
    auto allShuffleableEntrances = trackerWorld.getShuffledEntrances(EntranceType::ALL, false);

    int currentPointSize = 12;
    for (auto& entrance : shuffledEntrances)
    {
        // New Horizontal layout to add the label and the disconnect button
        auto hLayout = new QHBoxLayout();

        auto newLabel = new TrackerLabel(TrackerLabelType::EntranceSource, currentPointSize, this, nullptr, entrance);
        hLayout->addWidget(newLabel);

        hLayout->setContentsMargins(7, 0, 0, 0);
        auto disconnectButton = new QPushButton("X");
        set_font(disconnectButton, "fira_sans", currentPointSize);
        disconnectButton->setCursor(Qt::PointingHandCursor);
        disconnectButton->setMaximumWidth(20);
        disconnectButton->setMaximumHeight(15);
        connect(disconnectButton, &QPushButton::clicked, this, [&,entrance=entrance](){MainWindow::tracker_disconnect_entrance(entrance);});
        hLayout->addWidget(disconnectButton);
        newLabel->set_disconnect_button(disconnectButton);
        // Hide the disconnect button if this entrance is not connected
        if (!entrance->getReplaces())
        {
            disconnectButton->setVisible(false);
        }

        ui->entrance_scroll_layout->addLayout(hLayout);
        connect(newLabel, &TrackerLabel::entrance_source_label_clicked, this, &MainWindow::tracker_show_available_target_entrances);
        connect(newLabel, &TrackerLabel::entrance_source_label_disconnect, this, &MainWindow::tracker_disconnect_entrance);
        connect(newLabel, &TrackerLabel::mouse_over_entrance_label, this, &MainWindow::tracker_display_current_entrance);
        connect(newLabel, &TrackerLabel::mouse_left_entrance_label, this, &MainWindow::tracker_clear_current_area_text);
    }

    // Add vertical spacer to push labels up
    ui->entrance_scroll_layout->addSpacerItem(new QSpacerItem(40, 20, QSizePolicy::Minimum, QSizePolicy::Expanding));
}

void MainWindow::on_start_tracker_button_clicked()
{
    // Make sure the user wants to start a new tracker session
    QMessageBox confirmDialog;
    confirmDialog.setText("Reset the tracker with your current settings?");
    confirmDialog.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    if (confirmDialog.exec() == QMessageBox::No)
    {
        return;
    }

    trackerPreferences.autosaveFilePath = Utility::get_app_save_path() / "tracker_autosave.yaml";

    initialize_tracker_world(config.settings);

    // Get the first search iteration
    update_tracker();

    switch_to_overworld_tracker();
}

void MainWindow::autosave_current_tracker()
{
    if(!autosave_current_tracker_config()) {
        // error?
    }
    if(!autosave_current_tracker_preferences()) {
        // error?
    }
}

bool MainWindow::autosave_current_tracker_config()
{
    auto& trackerWorld = trackerWorlds[0];

    Config trackerConfig;
    trackerConfig.settings = trackerSettings;
    // Save current config
    auto configErr = trackerConfig.writeSettings(trackerPreferences.autosaveFilePath);
    if (configErr != ConfigError::NONE)
    {
        show_error_dialog("Could not save tracker config to file\n Error: " + ConfigErrorGetName(configErr));
        return false;
    }

    // Read it back and add extra tracker data
    std::string autosave;
    Utility::getFileContents(trackerPreferences.autosaveFilePath, autosave);
    YAML::Node root = YAML::Load(autosave);

    // Save which locations have been marked
    for (auto loc : trackerWorld.getLocations())
    {
        if (loc->marked)
        {
            root["marked_locations"].push_back(loc->getName());
        }
    }

    if (root["marked_locations"].size() == 0)
    {
        root["marked_locations"] = "None";
    }

    // Save which items have been marked
    for (auto& item : trackerInventory)
    {
        root["marked_items"].push_back(gameItemToName(item.getGameItemId()));
    }

    if (root["marked_items"].size() == 0)
    {
        root["marked_items"] = "None";
    }

    // Save which entrances have been connected
    for (auto& [target, entrance] : connectedTargets)
    {
        root["connected_entrances"][entrance->getOriginalName()] = target->getReplaces()->getOriginalName();
    }

    // Save which charts have been mapped
    if(trackerSettings.randomize_charts) {
        for (auto& [chart, island] : mappedCharts)
        {
            root["mapped_charts"][gameItemToName(chart)] = roomNumToIslandName(island);
        }
    }

    std::ofstream autosave_file(trackerPreferences.autosaveFilePath);
    if (autosave_file.is_open() == false)
    {
        show_error_dialog("Failed to open " + Utility::toUtf8String(trackerPreferences.autosaveFilePath));
        return false;
    }

    autosave_file << root;
    autosave_file.close();

    return true;
}

bool MainWindow::autosave_current_tracker_preferences()
{
    Config trackerConfig;
    trackerConfig.settings = trackerSettings;
    // Save current config
    auto configErr = trackerConfig.writePreferences(Utility::get_app_save_path() / "tracker_preferences.yaml");
    if (configErr != ConfigError::NONE)
    {
        show_error_dialog("Could not save tracker preferences to file\n Error: " + ConfigErrorGetName(configErr));
        return false;
    }

    // Save preferences back to tracker_preferences
    std::string preferences;
    Utility::getFileContents(Utility::get_app_save_path() / "tracker_preferences.yaml", preferences);
    YAML::Node pref = YAML::Load(preferences);

    pref["show_location_logic"] = trackerPreferences.showLocationLogic;
    pref["show_nonprogress_locations"] = trackerPreferences.showNonProgressLocations;
    pref["right_click_clear_all"] = trackerPreferences.rightClickClearAll;
    pref["clear_all_includes_dungeon_mail"] = trackerPreferences.clearAllIncludesDungeonMail;
    pref["override_items_color"] = trackerPreferences.overrideItemsColor;
    pref["override_locations_color"] = trackerPreferences.overrideLocationsColor;
    pref["override_stats_color"] = trackerPreferences.overrideStatsColor;
    pref["items_color"] = trackerPreferences.itemsColor.name().toStdString();
    pref["locations_color"] = trackerPreferences.locationsColor.name().toStdString();
    pref["stats_color"] = trackerPreferences.statsColor.name().toStdString();
    pref["autosave_file_override"] = Utility::toUtf8String(trackerPreferences.autosaveFilePath);

    std::ofstream preferences_file(Utility::get_app_save_path() / "tracker_preferences.yaml");
    if (preferences_file.is_open() == false)
    {
        show_error_dialog("Failed to open tracker_preferences.yaml");
        return false;
    }

    preferences_file << pref;
    preferences_file.close();

    return true;
}

void MainWindow::load_tracker_autosave()
{
    // Load tracker preferences
    std::string preferences;
    if(Utility::getFileContents(Utility::get_app_save_path() / "tracker_preferences.yaml", preferences) != 0) {
        // No autosave file, don't try to do anything
        return;
    }
    YAML::Node pref = YAML::Load(preferences);

    // Last saved tracker location
    if(pref["autosave_file_override"]) {
        trackerPreferences.autosaveFilePath = Utility::Str::toUTF16(pref["autosave_file_override"].as<std::string>());
    }
    else {
        trackerPreferences.autosaveFilePath = Utility::get_app_save_path() / "tracker_autosave.yaml";
    }

    // Color override preferences
    if (pref["items_color"])
    {
        trackerPreferences.itemsColor = QColor::fromString(QString::fromStdString(pref["items_color"].as<std::string>()));
    }

    if (pref["locations_color"])
    {
        trackerPreferences.locationsColor = QColor::fromString(QString::fromStdString(pref["locations_color"].as<std::string>()));
    }

    if (pref["stats_color"])
    {
        trackerPreferences.statsColor = QColor::fromString(QString::fromStdString(pref["stats_color"].as<std::string>()));
    }

    if (pref["override_items_color"])
    {
        trackerPreferences.overrideItemsColor = pref["override_items_color"].as<bool>();
        update_items_color();
    }

    if (pref["override_locations_color"])
    {
        trackerPreferences.overrideLocationsColor = pref["override_locations_color"].as<bool>();
        update_locations_color();
    }

    if (pref["override_stats_color"])
    {
        trackerPreferences.overrideStatsColor = pref["override_stats_color"].as<bool>();
        update_stats_color();
    }

    // Boolean preferences
    if (pref["show_location_logic"])
    {
        trackerPreferences.showLocationLogic = pref["show_location_logic"].as<bool>();
    }

    if (pref["show_nonprogress_locations"])
    {
        trackerPreferences.showNonProgressLocations = pref["show_nonprogress_locations"].as<bool>();
    }

    if (pref["right_click_clear_all"])
    {
        trackerPreferences.rightClickClearAll = pref["right_click_clear_all"].as<bool>();
    }

    if (pref["clear_all_includes_dungeon_mail"])
    {
        trackerPreferences.clearAllIncludesDungeonMail = pref["clear_all_includes_dungeon_mail"].as<bool>();
    }

    if (!std::filesystem::exists(trackerPreferences.autosaveFilePath) || !std::filesystem::exists(Utility::get_app_save_path() / "tracker_preferences.yaml"))
    {
        // No autosave file, don't try to do anything
        return;
    }

    Config trackerConfig;
    auto configErr = trackerConfig.loadFromFile(trackerPreferences.autosaveFilePath, Utility::get_app_save_path() / "tracker_preferences.yaml", true);
    if (configErr != ConfigError::NONE)
    {
        show_warning_dialog("Could not load tracker autosave config\nError: " + ConfigErrorGetName(configErr));
        return;
    }

    std::string autosave;
    Utility::getFileContents(trackerPreferences.autosaveFilePath, autosave);
    YAML::Node root = YAML::Load(autosave);

    // Load marked locations
    std::vector<std::string> markedLocations = {};
    if (root["marked_locations"].IsSequence())
    {
        for (const auto& locationName : root["marked_locations"])
        {
            markedLocations.push_back(locationName.as<std::string>());
        }
    }
    else if (root["marked_locations"] && root["marked_locations"].as<std::string>() != "None")
    {
        show_warning_dialog("Unable to load marked locations from tracker autosave");
    }


    // Load marked items
    GameItemPool markedItems = {};
    if (root["marked_items"].IsSequence())
    {
        for (const auto& item : root["marked_items"])
        {
            const std::string itemName = item.as<std::string>();
            if (nameToGameItem(itemName) != GameItem::INVALID)
            {
                markedItems.push_back(nameToGameItem(itemName));
            }
            else
            {
                show_warning_dialog("Unknown item \"" + itemName + "\" in tracker autosave file");
            }
        }
    }
    else if (root["marked_items"] && root["marked_items"].as<std::string>() != "None")
    {
        show_warning_dialog("Unable to load marked items from tracker autosave");
    }

    std::unordered_map<std::string, std::string> entranceConnections = {};
    const auto& connectedEntrances = root["connected_entrances"];
    if (!connectedEntrances.IsNull())
    {
        for (auto entranceItr = connectedEntrances.begin(); entranceItr != connectedEntrances.end(); entranceItr++)
        {
            auto entranceConnection = *entranceItr;
            entranceConnections[entranceConnection.first.as<std::string>()] = entranceConnection.second.as<std::string>();
        }
    }

    // Load saved mappings
    std::map<GameItem, uint8_t> chartMappings = {};
    const auto& charts = root["mapped_charts"];
    if(charts.IsMap())
    {
        for (auto chartItr = charts.begin(); chartItr != charts.end(); chartItr++)
        {
            auto chartMapping = *chartItr;
            chartMappings[nameToGameItem(chartMapping.first.as<std::string>())] = islandNameToRoomNum(chartMapping.second.as<std::string>());
        }
    }

    initialize_tracker_world(trackerConfig.settings, markedItems, markedLocations, entranceConnections, chartMappings);

    update_tracker();
}

#define SET_BUTTON_TO_LAYOUT(trackerItem, layout, row, col) ui->layout->addWidget(&trackerItem, row, col);

void MainWindow::initialize_tracker()
{

    // Setup Fira Sans font for tracker UI elements
    set_font(ui->location_list_close_button,     "fira_sans", 14);
    set_font(ui->clear_all_button,               "fira_sans", 14);
    set_font(ui->entrance_list_close_button,     "fira_sans", 14);
    set_font(ui->entrance_list_locations_button, "fira_sans", 14);
    set_font(ui->source_entrance_filter_label,   "fira_sans", 14);
    set_font(ui->source_entrance_filter_lineedit,"fira_sans", 11);
    set_font(ui->entrance_destination_back_button, "fira_sans", 14);
    set_font(ui->target_entrance_filter_label,   "fira_sans", 14);
    set_font(ui->target_entrance_filter_lineedit,"fira_sans", 11);
    set_font(ui->chart_list_back_button,         "fira_sans", 14);
    set_font(ui->where_did_lead_to_label,        "fira_sans", 14);
    set_font(ui->current_area_name_label,        "fira_sans", 15);
    set_font(ui->current_area_accessible_number, "fira_sans", 11);
    set_font(ui->current_area_accessible_label,  "fira_sans", 9);
    set_font(ui->current_area_remaining_number,  "fira_sans", 11);
    set_font(ui->current_area_remaining_label,   "fira_sans", 9);
    set_font(ui->current_item_label,             "fira_sans", 14);
    set_font(ui->locations_accessible_label,     "fira_sans", 12);
    set_font(ui->locations_accessible_number,    "fira_sans", 12);
    set_font(ui->locations_checked_label,        "fira_sans", 12);
    set_font(ui->locations_checked_number,       "fira_sans", 12);
    set_font(ui->locations_remaining_label,      "fira_sans", 12);
    set_font(ui->locations_remaining_number,     "fira_sans", 12);

    // Hide certain elements
    ui->current_area_accessible_label->setVisible(false);
    ui->current_area_remaining_label->setVisible(false);

    if (trackerWorlds.empty())
    {
        trackerWorlds = WorldPool(1);
    }
    trackerWorlds[0].setWorldId(0);

    // Setup inventory buttons
    SET_BUTTON_TO_LAYOUT(trackerTelescope,             inventory_layout_top, 0, 0);
    SET_BUTTON_TO_LAYOUT(trackerProgressiveSail,       inventory_layout_top, 0, 1);
    SET_BUTTON_TO_LAYOUT(trackerWindWaker,             inventory_layout_top, 0, 2);
    SET_BUTTON_TO_LAYOUT(trackerGrapplingHook,         inventory_layout_top, 0, 3);
    SET_BUTTON_TO_LAYOUT(trackerSpoilsBag,             inventory_layout_top, 0, 4);
    SET_BUTTON_TO_LAYOUT(trackerBoomerang,             inventory_layout_top, 0, 5);
    SET_BUTTON_TO_LAYOUT(trackerDekuLeaf,              inventory_layout_top, 0, 6);
    SET_BUTTON_TO_LAYOUT(trackerProgressiveSword,      inventory_layout_top, 0, 7);
    SET_BUTTON_TO_LAYOUT(trackerTingleBottle,          inventory_layout_top, 1, 0);
    SET_BUTTON_TO_LAYOUT(trackerProgressivePictoBox,   inventory_layout_top, 1, 1);
    SET_BUTTON_TO_LAYOUT(trackerIronBoots,             inventory_layout_top, 1, 2);
    SET_BUTTON_TO_LAYOUT(trackerMagicArmor,            inventory_layout_top, 1, 3);
    SET_BUTTON_TO_LAYOUT(trackerBaitBag,               inventory_layout_top, 1, 4);
    SET_BUTTON_TO_LAYOUT(trackerProgressiveBow,        inventory_layout_top, 1, 5);
    SET_BUTTON_TO_LAYOUT(trackerBombs,                 inventory_layout_top, 1, 6);
    SET_BUTTON_TO_LAYOUT(trackerProgressiveShield,     inventory_layout_top, 1, 7);
    SET_BUTTON_TO_LAYOUT(trackerCabanaDeed,            inventory_layout_middle, 0, 0);
    SET_BUTTON_TO_LAYOUT(trackerMaggiesLetter,         inventory_layout_middle, 0, 1);
    SET_BUTTON_TO_LAYOUT(trackerMoblinsLetter,         inventory_layout_middle, 0, 2);
    SET_BUTTON_TO_LAYOUT(trackerNoteToMom,             inventory_layout_middle, 0, 3);
    SET_BUTTON_TO_LAYOUT(trackerDeliveryBag,           inventory_layout_middle, 0, 4);
    SET_BUTTON_TO_LAYOUT(trackerHookshot,              inventory_layout_middle, 0, 5);
    SET_BUTTON_TO_LAYOUT(trackerSkullHammer,           inventory_layout_middle, 0, 6);
    SET_BUTTON_TO_LAYOUT(trackerPowerBracelets,        inventory_layout_middle, 0, 7);
    SET_BUTTON_TO_LAYOUT(trackerEmptyBottle,           inventory_layout_middle, 1, 0);
    SET_BUTTON_TO_LAYOUT(trackerWindsRequiem,          inventory_layout_middle, 1, 1);
    SET_BUTTON_TO_LAYOUT(trackerBalladOfGales,         inventory_layout_middle, 1, 2);
    SET_BUTTON_TO_LAYOUT(trackerCommandMelody,         inventory_layout_middle, 1, 3);
    SET_BUTTON_TO_LAYOUT(trackerEarthGodsLyric,        inventory_layout_middle, 1, 4);
    SET_BUTTON_TO_LAYOUT(trackerWindGodsAria,          inventory_layout_middle, 1, 5);
    SET_BUTTON_TO_LAYOUT(trackerSongOfPassing,         inventory_layout_middle, 1, 6);
    SET_BUTTON_TO_LAYOUT(trackerHerosCharm,            inventory_layout_middle, 1, 7);
    SET_BUTTON_TO_LAYOUT(trackerDinsPearl,             inventory_layout_dins_farores_pearl, 0, 0);
    SET_BUTTON_TO_LAYOUT(trackerFaroresPearl,          inventory_layout_dins_farores_pearl, 0, 1);
    SET_BUTTON_TO_LAYOUT(trackerNayrusPearl,           inventory_layout_nayrus_pearl, 0, 0);
    SET_BUTTON_TO_LAYOUT(trackerTriforceShards,        inventory_layout_triforce, 0, 0);
    SET_BUTTON_TO_LAYOUT(trackerTingleStatues,         inventory_layout_bottom_right, 0, 1);
    SET_BUTTON_TO_LAYOUT(trackerGhostShipChart,        inventory_layout_bottom_right, 0, 2);
    SET_BUTTON_TO_LAYOUT(trackerHurricaneSpin,         inventory_layout_bottom_right, 0, 3);
    SET_BUTTON_TO_LAYOUT(trackerProgressiveBombBag,    inventory_layout_bottom_right, 1, 0);
    SET_BUTTON_TO_LAYOUT(trackerProgressiveQuiver,     inventory_layout_bottom_right, 1, 1);
    SET_BUTTON_TO_LAYOUT(trackerProgressiveWallet,     inventory_layout_bottom_right, 1, 2);
    SET_BUTTON_TO_LAYOUT(trackerProgressiveMagicMeter, inventory_layout_bottom_right, 1, 3);

    // Set charts in the chart list
    for (auto i = 1; i <= 49; i++)
    {
        auto chartName = i <= 46 ? "Treasure Chart " + std::to_string(i) : "Triforce Chart " + std::to_string(i - 46);
        // This technically results in dangling pointers, but they're still kept by the widgets and this function
        // will only ever be run once per execution of the application, so it should never result in any memory leaks
        auto chartButton = new TrackerLabel(TrackerLabelType::Chart, 9, this, nullptr, nullptr, nameToGameItem(chartName));
        set_font(chartButton, "fira_sans", 9);
        SET_BUTTON_TO_LAYOUT(*chartButton, chart_list_layout, (i - 1) % 17, (i - 1) / 17);
        connect(chartButton, &TrackerLabel::chart_label_clicked, this, &MainWindow::tracker_give_and_map_chart);
        connect(chartButton, &TrackerLabel::mouse_over_chart_label, this, &MainWindow::tracker_display_current_item_text);
        connect(chartButton, &TrackerLabel::mouse_left_chart_label, this, &MainWindow::tracker_clear_current_item_text);
    }

    // Add Background Images and Colors (can't do this in Qt Designer since the DATA_PATH changes
    // depending on if we embed data or not)
    ui->tracker_tab->setStyleSheet("QWidget#tracker_tab {background-image: url(" + getTrackerAssetPath("background.png") + ");}");
    ui->inventory_widget->setStyleSheet("QWidget#inventory_widget {border-image: url(" + getTrackerAssetPath("trackerbg.png") + ");}");
    ui->inventory_widget_pearls->setStyleSheet("QWidget#inventory_widget_pearls {"
                                                   "background-image: url(" + getTrackerAssetPath("pearl_holder.png") + ");"
                                                   "background-repeat: none;"
                                                   "background-position: center;"
                                               "}");
    ui->overworld_map_widget->setStyleSheet("QWidget#overworld_map_widget {background-image: url(" + getTrackerAssetPath("sea_chart.png") + ");}");
    ui->chart_list_widget->setStyleSheet("QWidget#chart_list_widget {border-image: url(" + getTrackerAssetPath("area_empty.png") + ");}");
    set_location_list_widget_background("empty");
    ui->entrance_list_widget->setStyleSheet("QWidget#entrance_list_widget {border-image: url(" + getTrackerAssetPath("area_empty.png") + ");}");
    ui->entrance_destination_widget->setStyleSheet("QWidget#entrance_destination_widget {border-image: url(" + getTrackerAssetPath("area_empty.png") + ");}");
    ui->other_areas_widget->setStyleSheet("QWidget#other_areas_widget {background-color: rgba(160, 160, 160, 0.85);}");
    ui->stat_box->setStyleSheet("QWidget#stat_box {background-color: rgba(79, 79, 79, 0.85);}");

    // Add area widgets to the overworld
    using TAW = TrackerAreaWidget;
    for(size_t row = 0; row < 7; row++) {
        for(size_t col = 0; col < 7; col++) {
            uint8_t islandNum = row * 7 + col + 1;
            TrackerChartButton* chartButton = new TrackerChartButton(islandNum, this);
            ui->overworld_map_layout_2->addWidget(new TAW(roomNumToIslandName(islandNum), chartButton), row, col);
            connect(chartButton, &TrackerChartButton::chart_map_button_pressed, this, &MainWindow::open_chart_mapping_list);
            connect(chartButton, &TrackerChartButton::mouse_over_item, this, &MainWindow::tracker_display_current_item_text);
            connect(chartButton, &TrackerChartButton::mouse_left_item, this, &MainWindow::tracker_clear_current_item_text);
        }
    }

    ui->other_areas_layout->addWidget(new TAW("Dragon Roost Cavern", "gohma",         &trackerDRCSmallKeys,  &trackerDRCBigKey,  &trackerDRCDungeonMap,  &trackerDRCCompass),  0, 0);
    ui->other_areas_layout->addWidget(new TAW("Forbidden Woods",     "kalle_demos",   &trackerFWSmallKeys,   &trackerFWBigKey,   &trackerFWDungeonMap,   &trackerFWCompass),   0, 1);
    ui->other_areas_layout->addWidget(new TAW("Tower of the Gods",   "gohdan",        &trackerTOTGSmallKeys, &trackerTOTGBigKey, &trackerTOTGDungeonMap, &trackerTOTGCompass), 0, 2);
    ui->other_areas_layout->addWidget(new TAW("Forsaken Fortress",   "helmaroc_king", nullptr,               nullptr,            &trackerFFDungeonMap,   &trackerFFCompass),   0, 3);
    ui->other_areas_layout->addWidget(new TAW("Earth Temple",        "jalhalla",      &trackerETSmallKeys,   &trackerETBigKey,   &trackerETDungeonMap,   &trackerETCompass),   0, 4);
    ui->other_areas_layout->addWidget(new TAW("Wind Temple",         "molgera",       &trackerWTSmallKeys,   &trackerWTBigKey,   &trackerWTDungeonMap,   &trackerWTCompass),   0, 5);
    ui->other_areas_layout->addWidget(new TAW("The Great Sea",       "great_sea"),    0, 6);
    ui->other_areas_layout->addWidget(new TAW("Mailbox",             "mailbox"),      0, 7);
    ui->other_areas_layout->addWidget(new TAW("Hyrule",              "hyrule" ),      0, 8);
    ui->other_areas_layout->addWidget(new TAW("Ganon's Tower",       "ganondorf"),    0, 9);

    // Set world and inventory and connect inventory button actions to updating the tracker
    for (auto inventoryButton : ui->tracker_tab->findChildren<TrackerInventoryButton*>())
    {
        inventoryButton->trackerInventory = &trackerInventory;
        inventoryButton->trackerWorld = &trackerWorlds[0];
        inventoryButton->mainWindow = this;
        connect(inventoryButton, &TrackerInventoryButton::inventory_button_pressed, this, &MainWindow::update_tracker);
        connect(inventoryButton, &TrackerInventoryButton::mouse_over_item, this, &MainWindow::tracker_display_current_item_text);
    }

    // Connect left-clicking area widgets to showing the checks in that area
    // and right-clicking area widgets to clearing all that areas locations
    for (auto area : ui->tracker_tab->findChildren<TrackerAreaLabel*>())
    {
        connect(area, &TrackerAreaLabel::area_label_clicked, this, &MainWindow::tracker_show_specific_area);
        connect(area, &TrackerAreaLabel::area_label_right_clicked, this, &MainWindow::tracker_area_right_clicked);
    }

    // Connect hovering over an area widget to showing the areas info in the side label
    for (auto area : ui->tracker_tab->findChildren<TrackerAreaWidget*>())
    {
        connect(area, &TrackerAreaWidget::mouse_over_area, this, &MainWindow::tracker_display_current_area_text);
        connect(area, &TrackerAreaWidget::mouse_left_area, this, &MainWindow::tracker_clear_current_area_text);
    }

    switch_to_overworld_tracker();
}

void MainWindow::update_tracker()
{
    // Don't allow any interaction with the tracker unless it's been started
    if (!trackerStarted)
    {
        return;
    }

    update_tracker_areas_and_autosave();

    auto& trackerWorld = trackerWorlds[0];

    // Update all the chart icons
    for(size_t islandNum = 1; islandNum < 50; islandNum++) {
        TrackerAreaWidget* w1 = dynamic_cast<TrackerAreaWidget*>(ui->overworld_map_layout_2->itemAtPosition((islandNum - 1) / 7, (islandNum - 1) % 7)->widget());
        dynamic_cast<TrackerChartButton*>(w1->stackedLayout.itemAt(0)->widget())->updateIcon();
    }

    // Only update the widget we're displaying
    if (ui->tracker_locations_widget->currentIndex() == LOCATION_TRACKER_OVERWORLD)
    {
        return;
    }

    // Clear previous labels from the locations widget
    auto right_layout = ui->specific_locations_right_layout;
    auto left_layout = ui->specific_locations_left_layout;
    clear_tracker_labels(right_layout);
    clear_tracker_labels(left_layout);

    // Only try displaying locations if the current area has locations
    if (areaLocations.contains(currentTrackerArea))
    {
        // If we have more than 14 locations to list, we'll split the listing
        // into 2 columns
        auto numLocations = areaLocations[currentTrackerArea].size();
        int currentPointSize = 12;
        if (numLocations > 14)
        {
            currentPointSize = 10;
        }

        int counter = 0;
        for (auto& loc : areaLocations[currentTrackerArea])
        {
            auto newLabel = new TrackerLabel(TrackerLabelType::Location, currentPointSize, this, loc);
            newLabel->updateShowLogic(trackerPreferences.showLocationLogic, trackerStarted);
            connect(newLabel, &TrackerLabel::location_label_clicked, this, &MainWindow::update_tracker);
            connect(newLabel, &TrackerLabel::mouse_over_location_label, this, &MainWindow::tracker_display_current_location);
            connect(newLabel, &TrackerLabel::mouse_left_location_label, this, &MainWindow::tracker_clear_current_area_text);

            if (numLocations > 14 && counter > numLocations / 2)
            {
                right_layout->addWidget(newLabel);
            }
            else
            {
                left_layout->addWidget(newLabel);
            }

            counter++;
        }
        left_layout->addSpacerItem(new QSpacerItem(40, 20, QSizePolicy::Minimum, QSizePolicy::Expanding));
        if (numLocations > 14)
        {
            right_layout->addSpacerItem(new QSpacerItem(40, 20, QSizePolicy::Minimum, QSizePolicy::Expanding));
        }
    }

    // Pare down which entrances are shown to be more intuitive
    std::list<std::list<Entrance*>> shuffledEntrances = {};
    if (areaEntrances.contains(currentTrackerArea))
    {
        // Gather all of the regions which have the current area as their
        // hardcoded region. These are the area's entrance parents.
        std::list<Area*> areaEntranceParents = {};
        for (auto e : areaEntrances[currentTrackerArea])
        {
            if (e->getParentArea()->getRegion() == currentTrackerArea && !elementInPool(e->getParentArea(), areaEntranceParents))
            {
                areaEntranceParents.push_front(e->getParentArea());
            }
        }

        // Use the area entrance parents to find all shuffled entrances in this area
        if (trackerWorld.areaTable.contains(currentTrackerArea))
        {
            shuffledEntrances = trackerWorld.getArea(currentTrackerArea)->findShuffledEntrances(areaEntranceParents);
        }
        else if(areaEntranceParents.size() > 0) {
            shuffledEntrances = areaEntranceParents.front()->findShuffledEntrances(areaEntranceParents);
        }
    }

    // If the current tracker area is empty, then show all entrances
    if (currentTrackerArea == "")
    {
        auto allShuffleableEntrances = trackerWorld.getShuffledEntrances(EntranceType::ALL, false);
        shuffledEntrances = {{}};
        for (auto& entrance : allShuffleableEntrances)
        {
            shuffledEntrances.front().push_back(entrance);
        }
    }

    // First, hide all entrances
    auto entranceLabels = ui->entrance_scroll_widget->findChildren<TrackerLabel*>();
    for (auto& entranceLabel : entranceLabels)
    {
        auto entrance = entranceLabel->get_entrance();
        if (entrance)
        {
            entrance->setHidden(true);
        }
    }

    // Then set all collected entrances as unhidden
    for (auto& sphere : shuffledEntrances)
    {
        for (auto& entrance : sphere)
        {
            entrance->setHidden(false);
        }
    }

    // Only show the unhidden entrances
    for (auto& entranceLabel : entranceLabels)
    {
        auto entrance = entranceLabel->get_entrance();
        if (entrance)
        {
            if (entrance->isHidden())
            {
                entranceLabel->hideAll();
            }
            else
            {
                entranceLabel->showAll();
                entranceLabel->update_colors();
            }
        }
    }
}

// Under certain circumstances we want to mark certain
// locations as accessible even if they logically aren't (and vice versa)
void MainWindow::check_special_accessibility_conditions()
{
    // If the user has marked a boss location that has chain
    // locations in the mail, then set those locations as accessible
    // so the user knows they can go pick them up

    auto& trackerWorld = trackerWorlds[0];
    auto startingItems = trackerWorld.getStartingItems();

    if (trackerWorld.locationTable["Forsaken Fortress - Helmaroc King Heart Container"]->marked)
    {
        auto songOfPassing = Item(GameItem::SongOfPassing, &trackerWorld);
        if (elementInPool(songOfPassing, trackerInventory) || elementInPool(songOfPassing, startingItems)) {
            trackerWorld.locationTable["Mailbox - Letter from Aryll"]->hasBeenFound = true;

            if (trackerWorld.areaTable["Windfall Jail"]->isAccessible) {
                trackerWorld.locationTable["Mailbox - Letter from Tingle"]->hasBeenFound = true;
            }
        }
    }

    if (trackerWorld.locationTable["Forbidden Woods - Kalle Demos Heart Container"]->marked)
    {
        trackerWorld.locationTable["Mailbox - Letter from Orca"]->hasBeenFound = true;
    }

    if (trackerWorld.locationTable["Earth Temple - Jalhalla Heart Container"]->marked)
    {
        auto noteToMom = Item(GameItem::NoteToMom, &trackerWorld);
        auto deliveryBag = Item(GameItem::DeliveryBag, &trackerWorld);
        if ((elementInPool(noteToMom, trackerInventory)   || elementInPool(noteToMom, startingItems)) &&
            (elementInPool(deliveryBag, trackerInventory) || elementInPool(deliveryBag, startingItems)))
        {
            trackerWorld.locationTable["Mailbox - Letter from Baito"]->hasBeenFound = true;
        }
    }

    // If chart randomization is on, the tracker logic uses the world's chart mappings and not just the tracked ones
    // That can leak the mappings, so set the location as inaccessible as long as its chart mapping is not tracked
    if(trackerSettings.randomize_charts) {
        for(size_t i = 1; i < 50; i++) {
            if(!isIslandMappedToChart(i)) {
                trackerWorld.locationTable[roomNumToIslandName(i) + " - Sunken Treasure"]->hasBeenFound = false;
            }
        }
    }
}

void MainWindow::update_tracker_areas_and_autosave()
{
    getAccessibleLocations(trackerWorlds, trackerInventory, trackerLocations);
    check_special_accessibility_conditions();

    // Apply any own dungeon items after we get the accessible locations
    auto trackerInventoryExtras = trackerInventory;
    bool addedItems = false;
    do
    {
        addedItems = false;
        for (auto& [item, locationPools] : ownDungeonKeyLocations)
        {
            auto itemCount = elementCountInPool(item, trackerInventoryExtras);

            for (auto i = itemCount; i < locationPools.size(); i++)
            {
                // If all the locations in the pool are either marked or accessible
                // then add the key to the inventory calculation
                auto& locPool = locationPools[i];
                if (std::all_of(locPool.begin(), locPool.end(), [](Location* loc){return loc->marked || loc->hasBeenFound;}))
                {
                    addElementToPool(trackerInventoryExtras, item);
                    addedItems = true;
                }
                else
                {
                    break;
                }
            }
        }

        if (addedItems)
        {
            getAccessibleLocations(trackerWorlds, trackerInventoryExtras, trackerLocations);
            check_special_accessibility_conditions();
        }
    }
    while (addedItems);

    // Set computed requirements for each location
    auto flattenSearch = FlattenSearch(&trackerWorlds[0]);
    flattenSearch.doSearch();

    // Update each areas information
    for (auto area : ui->tracker_tab->findChildren<TrackerAreaWidget*>())
    {
        area->updateArea();
    }

    // Update stat box labels
    int checkedLocations = 0;
    int accessibleLocations = 0;
    int remainingLocations = 0;
    for (auto loc : trackerWorlds[0].getLocations(!trackerPreferences.showNonProgressLocations))
    {
        // Don't do anything with hint locations
        if (loc->categories.contains(LocationCategory::HoHoHint) || loc->categories.contains(LocationCategory::BlueChuChu))
        {
            continue;
        }
        if (loc->marked)
        {
            checkedLocations++;
        }
        else if (loc->hasBeenFound)
        {
            accessibleLocations++;
            remainingLocations++;
        }
        else
        {
            remainingLocations++;
        }
    }

    ui->locations_checked_number->setText(std::to_string(checkedLocations).c_str());
    ui->locations_accessible_number->setText(std::to_string(accessibleLocations).c_str());
    ui->locations_remaining_number->setText(std::to_string(remainingLocations).c_str());

    calculate_entrance_paths();

    autosave_current_tracker();
}

void MainWindow::switch_to_overworld_tracker()
{
    ui->tracker_locations_widget->setCurrentIndex(LOCATION_TRACKER_OVERWORLD);
    currentTrackerArea = "";
}

void MainWindow::switch_to_area_tracker()
{
    ui->tracker_locations_widget->setCurrentIndex(LOCATION_TRACKER_SPECIFIC_AREA);
}

void MainWindow::switch_to_entrances_tracker()
{
    ui->tracker_locations_widget->setCurrentIndex(LOCATION_TRACKER_SPECIFIC_AREA_ENTRANCES);
    ui->entrance_list_locations_button->setVisible(currentTrackerArea != "");
    ui->source_entrance_filter_lineedit->setFocus();
}

void MainWindow::switch_to_entrance_destinations_tracker()
{
    ui->tracker_locations_widget->setCurrentIndex(LOCATION_TRACKER_ENTRANCE_DESTINATIONS);
    ui->target_entrance_filter_lineedit->setText("");
    ui->target_entrance_filter_lineedit->setFocus();
}

void MainWindow::switch_to_chart_list_tracker()
{
    for(auto label : ui->chart_list_widget->findChildren<TrackerLabel*>()) {
        label->update_colors();
    }
    ui->tracker_locations_widget->setCurrentIndex(LOCATION_TRACKER_CHART_LIST);
}

void MainWindow::on_location_list_entrances_button_released()
{
    switch_to_entrances_tracker();
}

void MainWindow::on_location_list_close_button_released()
{
    switch_to_overworld_tracker();
}

void MainWindow::on_entrance_list_close_button_released()
{
    switch_to_overworld_tracker();
}

void MainWindow::on_entrance_list_locations_button_released()
{
    switch_to_area_tracker();
}

void MainWindow::on_source_entrance_filter_lineedit_textChanged(const QString &arg1)
{
    filter_entrance_list(arg1.toLower(), true);
}


void MainWindow::on_entrance_destination_back_button_released()
{
    switch_to_entrances_tracker();
    selectedEntrance = nullptr;
}

void MainWindow::on_target_entrance_filter_lineedit_textChanged(const QString &arg1)
{
    filter_entrance_list(arg1.toLower(), false);
}

void MainWindow::filter_entrance_list(const QString& filter, const bool arenaExitNameChange)
{
    // Hide entrance labels which don't fit the filter
    // The labels should be shown if any part of the entrance's original name
    // or the area it's connected to, fit the filter
    for (auto targetLabel : ui->tracker_locations_widget->findChildren<TrackerLabel*>())
    {
        auto entrance = targetLabel->get_entrance();
        if (entrance)
        {
            auto entranceName = QString::fromStdString(entrance->getOriginalName(arenaExitNameChange)).toLower();
            QString connectedAreaName = "";

            // If this is a target entrance, use the original name of the entrance this target
            // was created from to filter for. This allows us to more easily filter for target
            // entrances that all lead to the same area
            if (entranceName.startsWith("root -> "))
            {
                entranceName = QString::fromStdString(entrance->getReplaces()->getOriginalName(arenaExitNameChange)).toLower();
            }

            // If this entrance is connected to another area, include that in the filter.
            // This allows us to filter for which entrances are connected to an area
            if (entrance->getConnectedArea())
            {
                connectedAreaName = QString::fromStdString(entrance->getConnectedArea()->name);
            }

            // Check the filter
            if (!entrance->isHidden() && (entranceName.contains(filter) || connectedAreaName.contains(filter)))
            {
                targetLabel->showAll();
            }
            else
            {
                targetLabel->hideAll();
            }
        }
    }
}

bool MainWindow::isMappingChart() {
    return selectedChartIsland != 0;
}

bool MainWindow::isChartMapped(GameItem chart) {
    return mappedCharts.contains(chart);
}

void MainWindow::mapChart(GameItem chart, uint8_t islandNum) {
    if(isIslandMappedToChart(islandNum)) {
        std::erase_if(mappedCharts, [islandNum](const auto& mapping) { return mapping.second == islandNum; });
        trackerWorlds[0].macros[trackerWorlds[0].macroNameMap.at("Chart For Island " + std::to_string(islandNum))].args[0] = Item(trackerWorlds[0].chartMappings[islandNum], &trackerWorlds[0]);
    }

    mappedCharts[chart] = islandNum;
    trackerWorlds[0].macros[trackerWorlds[0].macroNameMap.at("Chart For Island " + std::to_string(islandNum))].args[0] = Item(chart, &trackerWorlds[0]);
}

bool MainWindow::isIslandMappedToChart(uint8_t island) {
    return std::ranges::find_if(mappedCharts, [island](const auto& mapping) { return mapping.second == island; }) != mappedCharts.end();
}

uint8_t MainWindow::islandForChart(GameItem chart) { 
    if(mappedCharts.contains(chart)) {
        return mappedCharts.at(chart);
    }

    return 0;
}

GameItem MainWindow::chartForIsland(uint8_t islandNum) { 
    if(!isIslandMappedToChart(islandNum)) {
        return GameItem::INVALID;
    }

    return std::ranges::find_if(mappedCharts, [islandNum](const auto& mapping) { return mapping.second == islandNum; })->first;
}

void MainWindow::on_clear_all_button_released()
{
    tracker_clear_specific_area(currentTrackerArea);
}

void MainWindow::on_chart_list_back_button_released()
{
    if(selectedChartIsland != 0) {
        selectedChartIsland = 0;
    }
    switch_to_overworld_tracker();
}

void MainWindow::update_items_color() {
    if(trackerPreferences.overrideItemsColor) {
        ui->inventory_widget->setStyleSheet("QWidget#inventory_widget {background-color: " + trackerPreferences.itemsColor.name() + "}");
        ui->inventory_widget_pearls->setStyleSheet("");
    }
    else {
        ui->inventory_widget->setStyleSheet("QWidget#inventory_widget {border-image: url(" + getTrackerAssetPath("trackerbg.png") + ");}");
        // Only display the pearl holder if we're using the default background
        ui->inventory_widget_pearls->setStyleSheet("QWidget#inventory_widget_pearls {"
                                                   "background-image: url(" + getTrackerAssetPath("pearl_holder.png") + ");"
                                                   "background-repeat: none;"
                                                   "background-position: center;"
                                                   "}");
    }
    autosave_current_tracker_preferences();
}

void MainWindow::update_locations_color()
{
    if(trackerPreferences.overrideLocationsColor) {
        ui->other_areas_widget->setStyleSheet("QWidget#other_areas_widget {background-color: " + trackerPreferences.locationsColor.name() + "}");
    }
    else {
        ui->other_areas_widget->setStyleSheet("QWidget#other_areas_widget {background-color: rgba(160, 160, 160, 0.85);}");
    }
    autosave_current_tracker_preferences();
}

void MainWindow::update_stats_color()
{
    if(trackerPreferences.overrideStatsColor) {
        ui->stat_box->setStyleSheet("QWidget#stat_box {background-color: " + trackerPreferences.statsColor.name() + "}");
    }
    else {
        ui->stat_box->setStyleSheet("QWidget#stat_box {background-color: rgba(79, 79, 79, 0.85);}");
    }
    autosave_current_tracker_preferences();
}

void MainWindow::on_open_chart_list_button_clicked()
{
    switch_to_chart_list_tracker();
}

void MainWindow::on_open_tracker_settings_button_clicked()
{
    // Make sure we can only open one settings window at a time
    if (!prefDialog)
    {
        prefDialog = new TrackerPreferencesDialog(this);
        prefDialog->setWindowTitle("Tracker Settings");
        prefDialog->setAttribute(Qt::WA_DeleteOnClose);
        // Reset our pointer when the dialog closes
        connect(prefDialog, &QDialog::finished, this, &MainWindow::tracker_preferences_closed);
        prefDialog->show();
    }
    else
    {
        // If the window already exists, raise it above the main window
        prefDialog->raise();
    }
}

void MainWindow::tracker_preferences_closed(int resault)
{
    prefDialog = nullptr;
}

void MainWindow::on_view_all_entrances_button_clicked()
{
    set_current_tracker_area("");
    switch_to_entrances_tracker();
    update_tracker();
}

void MainWindow::open_chart_mapping_list(uint8_t islandNum) {
    if(!trackerSettings.randomize_charts) {
        const Item chart = Item(roomNumToDefaultChart(islandNum), &trackerWorlds[0]);
        if(!elementInPool(chart, trackerInventory)) {
            addElementToPool(trackerInventory, chart);
        }
        else {
            removeElementFromPool(trackerInventory, chart);
        }

        update_tracker();

        return;
    }

    selectedChartIsland = islandNum;
    switch_to_chart_list_tracker();
}

void MainWindow::tracker_give_and_map_chart(TrackerLabel* label, GameItem chart) {
    // if mapping a chart
    if(selectedChartIsland != 0) {
        if(!elementInPool(Item(chart, &trackerWorlds[0]), trackerInventory)) {
            addElementToPool(trackerInventory, Item(chart, &trackerWorlds[0]));
        }

        if(isIslandMappedToChart(selectedChartIsland)) {
            std::erase_if(mappedCharts, [selectedChartIsland = this->selectedChartIsland](const auto& mapping) { return mapping.second == selectedChartIsland; });
        }

        mapChart(chart, selectedChartIsland);
        on_chart_list_back_button_released();
    }
    // normal chart list
    else {
        if(!elementInPool(Item(chart, &trackerWorlds[0]), trackerInventory)) {
            addElementToPool(trackerInventory, Item(chart, &trackerWorlds[0]));
        }
        else {
            removeElementFromPool(trackerInventory, Item(chart, &trackerWorlds[0]));
            if(trackerSettings.randomize_charts && mappedCharts.contains(chart)) {
                const uint8_t oldIsland = mappedCharts.at(chart);
                mappedCharts.erase(chart);
                trackerWorlds[0].macros[trackerWorlds[0].macroNameMap.at("Chart For Island " + std::to_string(oldIsland))].args[0] = Item(trackerWorlds[0].chartMappings[oldIsland], &trackerWorlds[0]);
            }
        }
    }

    label->update_colors();
    update_tracker();
}

void MainWindow::set_current_tracker_area(const std::string& areaPrefix)
{
    currentTrackerArea = areaPrefix;
}

void MainWindow::tracker_show_specific_area(const std::string& areaPrefix)
{
    set_current_tracker_area(areaPrefix);
    switch_to_area_tracker();
    update_tracker();

    // Only show the Entrances button if the island has randomized entrances
    ui->location_list_entrances_button->setVisible(!areaEntrances[areaPrefix].empty());

    // Update the image used for the background of the area location labels
    // Currently this lags the tracker a little bit, so maybe revisit later
    // std::replace(areaPrefix.begin(), areaPrefix.end(), ' ', '_');
    // std::transform(areaPrefix.begin(), areaPrefix.end(), areaPrefix.begin(), [](char& c){return std::tolower(c);});
    // std::erase(areaPrefix, '\'');
    // set_location_list_widget_background(areaPrefix);
}

void MainWindow::tracker_clear_specific_area(const std::string& areaPrefix)
{
    auto& trackerWorld = trackerWorlds[0];
    for (auto loc : areaLocations[areaPrefix])
    {
        loc->marked = true;
        // Clear certain mail locations associated with bosses
        if (trackerPreferences.clearAllIncludesDungeonMail)
        {
            if (loc->getName() == "Forsaken Fortress - Helmaroc King Heart Container")
            {
                trackerWorld.locationTable["Mailbox - Letter from Aryll"]->marked = true;
                trackerWorld.locationTable["Mailbox - Letter from Tingle"]->marked = true;
            }
            else if (loc->getName() == "Forbidden Woods - Kalle Demos Heart Container")
            {
                trackerWorld.locationTable["Mailbox - Letter from Orca"]->marked = true;
            }
            else if (loc->getName() == "Earth Temple - Jalhalla Heart Container")
            {
                trackerWorld.locationTable["Mailbox - Letter from Baito"]->marked = true;
            }
        }
    }

    // Update any labels currently being shown
    for (auto locLabel : ui->location_list_widget->findChildren<TrackerLabel*>())
    {
        locLabel->update_colors();
    }

    update_tracker();
}

void MainWindow::tracker_area_right_clicked(const std::string& areaPrefix)
{
    if (trackerPreferences.rightClickClearAll)
    {
        tracker_clear_specific_area(areaPrefix);
    }
}

void MainWindow::set_location_list_widget_background(const std::string& area)
{
    ui->location_list_widget->setStyleSheet("QWidget#location_list_widget {border-image: url(" + getTrackerAssetPath("area_" + area + ".png") + ");}");
}

void MainWindow::tracker_display_current_item_text(const std::string& currentItem)
{
    ui->current_item_label->setText(currentItem.c_str());
}

void MainWindow::tracker_clear_current_item_text()
{
    ui->current_item_label->setText("");
}

void MainWindow::tracker_display_current_area_text(TrackerAreaWidget* currentArea)
{
    ui->current_area_name_label->setText(currentArea->getPrefix().c_str());
    ui->current_area_accessible_number->setText(std::to_string(currentArea->totalAccessibleLocations).c_str());
    ui->current_area_remaining_number->setText(std::to_string(currentArea->totalRemainingLocations).c_str());
    ui->current_area_accessible_label->setVisible(true);
    ui->current_area_remaining_label->setVisible(true);
}

void MainWindow::tracker_clear_current_area_text()
{
    ui->current_area_name_label->setText("");
    ui->current_area_accessible_number->setText("");
    ui->current_area_remaining_number->setText("");
    ui->current_area_accessible_label->setVisible(false);
    ui->current_area_remaining_label->setVisible(false);
}

void MainWindow::tracker_display_current_location(Location* location)
{
    // Re-use the area name label for location names also
    ui->current_area_name_label->setText(location->getName().c_str());
}

void MainWindow::tracker_display_current_entrance(Entrance* entrance)
{
    auto entranceName = entrance->getOriginalName(true);
    if (entranceName.starts_with("Root"))
    {
        entranceName = entrance->getReplaces()->getOriginalName();
    }

    // Re-use the area name label for entrance names also
    ui->current_area_name_label->setText(entranceName.c_str());
}

void MainWindow::tracker_show_available_target_entrances(Entrance* entrance)
{
    selectedEntrance = entrance;

    auto entranceName = entrance->getOriginalName(true);
    if (entranceName.find("Battle Arena Exit") != std::string::npos)
    {
        entranceName = entranceName.substr(0, entranceName.find(" -> ")) + " Exit";
    }
    else
    {
        entranceName = entrance->getOriginalConnectedArea()->name;
    }

    ui->where_did_lead_to_label->setText(std::string("Where did " + entranceName + " lead to?").c_str());

    auto entrance_destination_list_layout = ui->entrance_destination_list_layout;
    clear_tracker_labels(entrance_destination_list_layout);
    clear_tracker_labels(ui->entrance_targets_scroll_layout);

    int currentPointSize = 12;
    for (auto target : targetEntrancePools[entrance->getEntranceType()])
    {
        // Don't list targets that have already been connected somewhere else
        if (target->getConnectedArea() == nullptr)
        {
            continue;
        }
        // Don't list the reverse of the target if it's the reverse of the selected entrance
        // since the entrance randomization algorithm forbids these connections
        if (target->getReplaces() == selectedEntrance->getReverse())
        {
            continue;
        }
        auto newLabel = new TrackerLabel(TrackerLabelType::EntranceDestination, currentPointSize, this, nullptr, target);
        ui->entrance_targets_scroll_layout->addWidget(newLabel);
        connect(newLabel, &TrackerLabel::entrance_destination_label_clicked, this, &MainWindow::tracker_change_entrance_connections);
        connect(newLabel, &TrackerLabel::mouse_over_entrance_label, this, &MainWindow::tracker_display_current_entrance);
        connect(newLabel, &TrackerLabel::mouse_left_entrance_label, this, &MainWindow::tracker_clear_current_area_text);
    }
    switch_to_entrance_destinations_tracker();
}

void MainWindow::tracker_change_entrance_connections(Entrance* target)
{
    // Disconnect the selected entrance incase it was
    // previously connected to another target
    tracker_disconnect_entrance(selectedEntrance);

    changeConnections(selectedEntrance, target);
    connectedTargets[target] = selectedEntrance;

    // Update areas and entrances for all islands after changing a connection
    set_areas_locations();
    set_areas_entrances();
    update_tracker_areas_and_autosave();

    // Change the text of the label for entrance we just connected
    for (auto entranceLabel : ui->entrance_scroll_widget->findChildren<TrackerLabel*>())
    {
        if (entranceLabel->get_entrance() == selectedEntrance)
        {
            entranceLabel->update_entrance_text();
            break;
        }
    }

    selectedEntrance = nullptr;
    switch_to_entrances_tracker();
    update_tracker();
}

void MainWindow::tracker_disconnect_entrance(Entrance* connectedEntrance)
{
    for (auto& [target, entrance] : connectedTargets)
    {
        if (entrance == connectedEntrance || (!entrance->isDecoupled() && entrance->getReplaces()->getReverse() == connectedEntrance))
        {
            restoreConnections(entrance, target);
            connectedTargets.erase(target);
            set_areas_locations();
            set_areas_entrances();
            update_tracker();
            // Change the text of the label for entrance we just connected
            for (auto entranceLabel : ui->entrance_scroll_widget->findChildren<TrackerLabel*>())
            {
                if (entranceLabel->get_entrance() == connectedEntrance)
                {
                    entranceLabel->update_entrance_text();
                    // Hide and show the entrance to hide the disconnect button
                    entranceLabel->hideAll();
                    entranceLabel->showAll();
                    break;
                }
            }
            return;
        }
    }
}

// Calculates the potential locations for each dungeon key if the
// key has to be inside the dungeon. This helps users know how much
// of a dungeon they can actually do even if they haven't collected
// the keys yet
void MainWindow::calculate_own_dungeon_key_locations()
{
    ownDungeonKeyLocations.clear();
    auto& trackerWorld = trackerWorlds[0];

    bool smallKeys = trackerWorld.getSettings().dungeon_small_keys == PlacementOption::OwnDungeon;
    bool bigKeys = trackerWorld.getSettings().dungeon_big_keys == PlacementOption::OwnDungeon;

    auto itemPool = trackerWorld.getItemPool();
    auto keys = filterAndEraseFromPool(itemPool, [&](Item& item){return (item.isSmallKey() && smallKeys) || (item.isBigKey() && bigKeys);});

    for (auto& key : keys)
    {
        // Get the dungeon this key is for
        std::string dungeonName = "";
        for (auto& [name, dungeon] : trackerWorld.dungeons)
        {
            if (isAnyOf(key, dungeon.smallKey, dungeon.bigKey))
            {
                dungeonName = name + " - ";
                break;
            }
        }

        // Find all possible locations for this key in the dungeon
        auto accessibleLocations = getAccessibleLocations(trackerWorlds, itemPool, trackerLocations);
        auto potentialKeyLocations = filterFromPool(accessibleLocations, [&](Location* loc){return loc->getName().starts_with(dungeonName);});

        // Save the possible locations for this key
        ownDungeonKeyLocations[key].push_back(potentialKeyLocations);

        // Add the key back to the pool to get ready for the next one
        addElementToPool(itemPool, key);
    }
}

void MainWindow::setup_tracker_entrances()
{
    areaEntrances.clear();
    connectedTargets.clear();

    auto& trackerWorld = trackerWorlds[0];

    auto err = setAllEntrancesData(trackerWorld);
    if (err != EntranceShuffleError::NONE)
    {
        show_error_dialog("ERROR: Could not set entrance data for tracker world\nReason: " + errorToName(err));
        return;
    }

    std::set<EntranceType> poolsToMix;
    auto entrancePools = createEntrancePools(trackerWorld, poolsToMix);
    targetEntrancePools = createTargetEntrances(entrancePools);

    // If entrances are not decoupled, then create the reverse pools for each type in the case
    // that users want to connect some in the opposite direction
    auto& settings = trackerWorld.getSettings();
    if (!settings.decouple_entrances)
    {
        for (auto& [type, targetPool] : targetEntrancePools)
        {
            EntrancePool reverseTargetPool = {};
            for (auto target : targetEntrancePools[type])
            {
                reverseTargetPool.push_back(target->getReplaces()->getReverse()->getAssumed());
                target->getReplaces()->getReverse()->setEntranceType(entranceTypeToReverse(type));
            }
            targetEntrancePools[entranceTypeToReverse(type)] = reverseTargetPool;
        }
    }

    // Prevent access to target entrances so that some locations don't show
    // up as reachable
    for (auto& [type, targetPool] : targetEntrancePools)
    {
        for (auto target : targetPool)
        {
            target->setRequirement({RequirementType::IMPOSSIBLE, {}});
            if (target->getReverse() != nullptr) {
                target->getReverse()->setRequirement({RequirementType::IMPOSSIBLE, {}});
            }
        }
    }

    set_areas_entrances();
}

void MainWindow::set_areas_locations()
{
    auto& trackerWorld = trackerWorlds[0];
    areaLocations.clear();

    // Setup each islands/dungeons locations
    for (auto& [name, area] : trackerWorld.areaTable)
    {
        if (trackerWorld.dungeons.contains(name))
        {
            continue;
        }

        for (auto& locAccess : area->locations)
        {
            auto& location = locAccess.location;
            // Don't add Ho Ho locations or Blue Chus for now
            if ((location->progression || trackerPreferences.showNonProgressLocations) &&
                !location->categories.contains(LocationCategory::HoHoHint) &&
                !location->categories.contains(LocationCategory::BlueChuChu))
            {
                if (area->dungeon != "")
                {
                    areaLocations[area->dungeon].insert(location);
                }
                else if (area->hintRegion != "")
                {
                    areaLocations[area->hintRegion].insert(location);
                }
                else
                {
                    auto regions = area->findHintRegions();
                    for (auto& region : regions)
                    {
                        areaLocations[region].insert(location);
                    }
                }
            }
        }
    }
}

void MainWindow::set_areas_entrances()
{
    auto& trackerWorld = trackerWorlds[0];
    areaEntrances.clear();

    // Separate entrances into which islands/dungeons they belong to
    for (auto& entrance : trackerWorld.getShuffledEntrances(EntranceType::ALL))
    {    
        // Set Misc Restrictive Entrances as Misc so that they get properly picked up
        if (entrance->getEntranceType() == EntranceType::MISC_RESTRICTIVE)
        {
            entrance->setEntranceType(EntranceType::MISC);
        }
        // Same thing for Fairy Fountains as Caves
        if (entrance->getEntranceType() == EntranceType::FAIRY)
        {
            entrance->setEntranceType(EntranceType::CAVE);
        }
        // And for the Reverse
        if (entrance->getEntranceType() == EntranceType::FAIRY_REVERSE)
        {
            entrance->setEntranceType(EntranceType::CAVE_REVERSE);
        }

        auto regions = entrance->getParentArea()->findHintRegions();
        for (auto& region : regions)
        {
            areaEntrances[region].push_back(entrance);
        }
    }
}

void MainWindow::calculate_entrance_paths()
{
    auto& trackerWorld = trackerWorlds[0];
    // Recalculate entrance paths
    entrancePathsByLocation.clear();

    // Only calculate paths for areas that have
    // randomized entrances
    for (const auto& [region, entrances] : areaEntrances)
    {
        Area* area = nullptr;
        if (region == "Hyrule")
        {
            area = trackerWorld.getArea("Hyrule Castle Interior");
        }
        else if (region == "Forsaken Fortress")
        {
            area = trackerWorld.getArea("Forsaken Fortress Sector");
        }
        else if(trackerWorld.areaTable.contains(region)) {
            area = trackerWorld.getArea(region);
        }

        if (area != nullptr)
        {
            auto paths = area->findEntrancePaths();
            entrancePaths[region] = paths;
            for (auto& [subarea, curPath] : paths)
            {
                for (auto& locAcc : subarea->locations)
                {
                    auto loc = locAcc.location;
                    // Don't bother with non-progression locations if show
                    // nonprogress locations is off
                    if (!loc->progression && !trackerPreferences.showNonProgressLocations)
                    {
                        continue;
                    }
                    // If this location doesn't have a set path yet, always put
                    // one in
                    if (!entrancePathsByLocation.contains(loc))
                    {
                        entrancePathsByLocation[loc] = curPath;
                    }
                    // Otherwise, only replace the previous path if the current path
                    // is better
                    else
                    {
                        auto& prevPath = entrancePathsByLocation[loc];
                        if (curPath.isBetterThan(prevPath))
                        {
                            prevPath = curPath;
                        }
                    }
                }
            }
        }
    }
}

void MainWindow::clear_tracker_labels(QLayout* layout)
{
    clear_layout(layout);
}

QString prettyTrackerName(Item& item, const int& count, MainWindow* mainWindow)
{
    switch(item.getGameItemId())
    {
    case GameItem::ProgressiveSword:
        switch(count)
        {
        case 1:
            return "Hero's Sword";
        case 2:
            return "Master Sword";
        case 3:
            return "Master Sword (Half-Power)";
        case 4:
            return "Master Sword (Full-Power)";
        }
    case GameItem::ProgressiveSail:
        switch(count)
        {
        case 1:
            return "Sail";
        case 2:
            return "Swift Sail";
        }
    case GameItem::ProgressiveShield:
        switch(count)
        {
        case 1:
            return "Hero's Shield";
        case 2:
            return "Mirror Shield";
        }
    case GameItem::ProgressiveBow:
        switch(count)
        {
        case 1:
            return "Hero's Bow";
        case 2:
            return "Fire & Ice Arrows";
        case 3:
            return "Light Arrows";
        }
    case GameItem::ProgressiveMagicMeter:
        switch(count)
        {
        case 1:
            return "Magic";
        case 2:
            return "Double Magic";
        }
    case GameItem::ProgressiveWallet:
        switch(count)
        {
        case 1:
            return "Wallet (1000)";
        case 2:
            return "Wallet (5000)";
        }
    case GameItem::ProgressivePictoBox:
        switch(count)
        {
        case 1:
            return "Picto Box";
        case 2:
            return "Deluxe Picto Box";
        }
    case GameItem::ProgressiveBombBag:
        switch(count)
        {
        case 1:
            return "Bomb Bag (60)";
        case 2:
            return "Bomb Bag (99)";
        }
    case GameItem::ProgressiveQuiver:
        switch(count)
        {
        case 1:
            return "Quiver (60)";
        case 2:
            return "Quiver (99)";
        }
    default:
        switch(count)
        {
        case 1:
            // Change the name of the chart if randomize charts is on
            if (item.isChartForSunkenTreasure() && mainWindow->trackerSettings.randomize_charts)
            {
                if(mainWindow->isChartMapped(item.getGameItemId()))
                {
                    return std::string(item.getName() + " -> " + roomNumToIslandName(mainWindow->islandForChart(item.getGameItemId()))).c_str();
                }
                else
                {
                    uint8_t island = std::ranges::find_if(mainWindow->trackerWorlds[0].chartMappings, [item](const auto& mapping){ return mapping.second == item.getGameItemId(); })->first;
                    return std::string("Chart for " + roomNumToIslandName(island)).c_str();
                }
            }
            return QString(item.getName().c_str());
        default:
            return QString(item.getName().c_str()) + " x" + QString::number(count);
        }
    }
}
