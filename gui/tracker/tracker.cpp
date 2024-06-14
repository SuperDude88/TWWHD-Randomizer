#include "../mainwindow.hpp"
#include "../ui_mainwindow.h"

#include <gui/tracker/tracker_inventory_button.hpp>
#include <gui/tracker/tracker_area_widget.hpp>
#include <gui/tracker/set_font.hpp>

#include <logic/Fill.hpp>
#include <logic/Search.hpp>
#include <logic/PoolFunctions.hpp>
#include <logic/EntranceShuffle.hpp>
#include <utility/file.hpp>

#include <iostream>

#include <QAbstractButton>
#include <QMouseEvent>
#include <QMessageBox>
#include <QColorDialog>

void MainWindow::initialize_tracker_world(Settings& settings,
                                          const GameItemPool& markedItems,
                                          const std::vector<std::string>& markedLocations,
                                          const std::unordered_map<std::string, std::string>& connectedEntrances)
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

    // Turn off randomize charts when creating the world
    // and also set chart imaages for sectors which normally
    // have sunken treasure from triforce charts
    if (settings.randomize_charts)
    {
        trackerSettings.randomize_charts = false;
        if (trackerSettings.progression_treasure_charts || trackerSettings.progression_triforce_charts)
        {
            trackerSettings.progression_treasure_charts = true;
            trackerSettings.progression_triforce_charts = true;
        }

        for (auto trackerTriforceChart : {&trackerTriforceChart1, &trackerTriforceChart2, &trackerTriforceChart3})
        {
            trackerTriforceChart->itemStates[0].filename = "treasure_chart_closed.png";
            trackerTriforceChart->itemStates[1].filename = "treasure_chart_open.png";
        }
    }
    else
    {
        for (auto trackerTriforceChart : {&trackerTriforceChart1, &trackerTriforceChart2, &trackerTriforceChart3})
        {
            trackerTriforceChart->itemStates[0].filename = "triforce_chart_closed.png";
            trackerTriforceChart->itemStates[1].filename = "triforce_chart_open.png";
        }
    }

    // Give 3 hearts so that all heart checks pass logic
    trackerSettings.starting_hcs = 3;

    trackerWorld.setSettings(trackerSettings);
    if (trackerWorld.loadWorld(DATA_PATH "logic/world.yaml", DATA_PATH "logic/macros.yaml", DATA_PATH "logic/location_data.yaml", DATA_PATH "logic/item_data.yaml", DATA_PATH "logic/area_names.yaml"))
    {
        show_error_dialog("Could not build world for app tracker");
        return;
    }
    trackerWorld.determineChartMappings();
    trackerWorld.determineProgressionLocations();
    trackerWorld.setItemPools();
    placeVanillaItems(trackerWorlds);

    // Reset trackerSettings.randomize_charts so we can check it later
    trackerSettings.randomize_charts = settings.randomize_charts;

    setup_tracker_entrances();

    // Connect autosaved entrances
    auto shuffledEntrances = trackerWorld.getShuffledEntrances(EntranceType::ALL, !trackerWorld.getSettings().decouple_entrances);
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

    trackerLocations = trackerWorld.getLocations(true);

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

    auto startingInventory = trackerWorld.getStartingItems();

    // Update inventory gui to have starting items
    //
    // Tingle Statues have to be set manually since they can
    // be obtained in any order
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

    auto startingInventoryCopy = startingInventory;
    auto trackerInventoryCopy = trackerInventory;

    // Update buttons with starting inventory items
    for (auto inventoryButton : ui->tracker_tab->findChildren<TrackerInventoryButton*>())
    {
        inventoryButton->state = 0;
        inventoryButton->forbiddenStates.clear();

        // Update button with starting items first to set forbidden states
        for (auto& itemState : inventoryButton->itemStates)
        {
            auto item = Item(itemState.gameItem, &trackerWorld);
            if (itemState.gameItem != GameItem::NOTHING && elementInPool(item, startingInventoryCopy))
            {
                inventoryButton->addForbiddenState(inventoryButton->state);
                inventoryButton->state++;
                removeElementFromPool(startingInventoryCopy, item);
            }
        }

        // Then update buttons with the current inventory
        for (auto& itemState : inventoryButton->itemStates)
        {
            auto item = Item(itemState.gameItem, &trackerWorld);
            if (itemState.gameItem != GameItem::NOTHING && elementInPool(item, trackerInventoryCopy))
            {
                inventoryButton->state++;
                removeElementFromPool(trackerInventoryCopy, item);
            }
        }

        // Then update the icon
        inventoryButton->updateIcon();
    }


    // Set locations for each area
    for (auto area : ui->tracker_tab->findChildren<TrackerAreaWidget*>())
    {
        area->setLocations(&areaLocations);

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

    initialize_tracker_world(config.settings);

    // Get the first search iteration
    update_tracker();

    switch_to_overworld_tracker();
}

void MainWindow::autosave_current_tracker()
{
    auto& trackerWorld = trackerWorlds[0];

    Config trackerConfig;
    trackerConfig.settings = trackerSettings;
    // Save current config
    auto configErr = trackerConfig.writeToFile(Utility::get_app_save_path() +  "tracker_autosave.yaml", Utility::get_app_save_path() +  "autosave_preferences.yaml");
    if (configErr != ConfigError::NONE)
    {
        show_error_dialog("Could not save tracker config to file\n Error: " + errorToName(configErr));
        return;
    }

    // Read it back and add extra tracker data
    std::string autosave;
    Utility::getFileContents(Utility::get_app_save_path() +  "tracker_autosave.yaml", autosave);
    YAML::Node root = YAML::Load(autosave);

    // Save which locations have been marked
    for (auto loc : trackerWorld.getLocations(true))
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

    std::ofstream f(Utility::get_app_save_path() +  "tracker_autosave.yaml");
    if (f.is_open() == false)
    {
        show_error_dialog("Failed to open tracker_autosave.yaml");
        return;
    }

    f << root;
}

void MainWindow::load_tracker_autosave()
{
    if (!std::filesystem::exists(Utility::get_app_save_path() +  "tracker_autosave.yaml") || !std::filesystem::exists(Utility::get_app_save_path() +  "autosave_preferences.yaml"))
    {
        // No autosave file, don't try to do anything
        return;
    }

    Config trackerConfig;
    auto configErr = trackerConfig.loadFromFile(Utility::get_app_save_path() +  "tracker_autosave.yaml", Utility::get_app_save_path() +  "autosave_preferences.yaml", true);
    if (configErr != ConfigError::NONE)
    {
        show_warning_dialog("Could not load tracker autosave config\nError: " + errorToName(configErr));
        return;
    }

    std::string autosave;
    Utility::getFileContents(Utility::get_app_save_path() +  "tracker_autosave.yaml", autosave);
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

    initialize_tracker_world(trackerConfig.settings, markedItems, markedLocations, entranceConnections);

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
    set_font(ui->entrance_destination_back_button, "fira_sans", 14);
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

    // Add Background Images and Colors (can't do this in Qt Designer since the DATA_PATH changes
    // depending on if we embed data or not)
    ui->tracker_tab->setStyleSheet("QWidget#tracker_tab {background-image: url(" DATA_PATH "tracker/background.png);}");
    ui->inventory_widget->setStyleSheet("QWidget#inventory_widget {border-image: url(" DATA_PATH "tracker/trackerbg.png);}");
    ui->inventory_widget_pearls->setStyleSheet("QWidget#inventory_widget_pearls {"
                                                   "background-image: url(" DATA_PATH "tracker/pearl_holder.png);"
                                                   "background-repeat: none;"
                                                   "background-position: center;"
                                               "}");
    ui->overworld_map_widget->setStyleSheet("QWidget#overworld_map_widget {background-image: url(" DATA_PATH "tracker/sea_chart.png);}");
    set_location_list_widget_background("empty");
    ui->entrance_list_widget->setStyleSheet("QWidget#entrance_list_widget {border-image: url(" DATA_PATH "tracker/area_empty.png);}");
    ui->entrance_destination_widget->setStyleSheet("QWidget#entrance_destination_widget {border-image: url(" DATA_PATH "tracker/area_empty.png);}");
    ui->other_areas_widget->setStyleSheet("QWidget#other_areas_widget {background-color: rgba(160, 160, 160, 0.85);}");
    ui->stat_box->setStyleSheet("QWidget#stat_box {background-color: rgba(79, 79, 79, 0.85);}");

    // Add area widgets to the overworld
    using TAW = TrackerAreaWidget;
    ui->overworld_map_layout_2->addWidget(new TAW("Forsaken Fortress Sector", &trackerTreasureChart25), 0, 0);
    ui->overworld_map_layout_2->addWidget(new TAW("Star Island",              &trackerTreasureChart7),  0, 1);
    ui->overworld_map_layout_2->addWidget(new TAW("Northern Fairy Island",    &trackerTreasureChart24), 0, 2);
    ui->overworld_map_layout_2->addWidget(new TAW("Gale Isle",                &trackerTreasureChart42), 0, 3);
    ui->overworld_map_layout_2->addWidget(new TAW("Crescent Moon Island",     &trackerTreasureChart11), 0, 4);
    ui->overworld_map_layout_2->addWidget(new TAW("Seven Star Isles",         &trackerTreasureChart45), 0, 5);
    ui->overworld_map_layout_2->addWidget(new TAW("Overlook Island",          &trackerTreasureChart13), 0, 6);
    ui->overworld_map_layout_2->addWidget(new TAW("Four Eye Reef",            &trackerTreasureChart41), 1, 0);
    ui->overworld_map_layout_2->addWidget(new TAW("Mother & Child Isles",     &trackerTreasureChart29), 1, 1);
    ui->overworld_map_layout_2->addWidget(new TAW("Spectacle Island",         &trackerTreasureChart22), 1, 2);
    ui->overworld_map_layout_2->addWidget(new TAW("Windfall Island",          &trackerTreasureChart18), 1, 3);
    ui->overworld_map_layout_2->addWidget(new TAW("Pawprint Isle",            &trackerTreasureChart30), 1, 4);
    ui->overworld_map_layout_2->addWidget(new TAW("Dragon Roost Island",      &trackerTreasureChart39), 1, 5);
    ui->overworld_map_layout_2->addWidget(new TAW("Flight Control Platform",  &trackerTreasureChart19), 1, 6);
    ui->overworld_map_layout_2->addWidget(new TAW("Western Fairy Island",     &trackerTreasureChart8),  2, 0);
    ui->overworld_map_layout_2->addWidget(new TAW("Rock Spire Isle",          &trackerTreasureChart2),  2, 1);
    ui->overworld_map_layout_2->addWidget(new TAW("Tingle Island",            &trackerTreasureChart10), 2, 2);
    ui->overworld_map_layout_2->addWidget(new TAW("Northern Triangle Island", &trackerTreasureChart26), 2, 3);
    ui->overworld_map_layout_2->addWidget(new TAW("Eastern Fairy Island",     &trackerTreasureChart3),  2, 4);
    ui->overworld_map_layout_2->addWidget(new TAW("Fire Mountain",            &trackerTreasureChart37), 2, 5);
    ui->overworld_map_layout_2->addWidget(new TAW("Star Belt Archipelago",    &trackerTreasureChart27), 2, 6);
    ui->overworld_map_layout_2->addWidget(new TAW("Three Eye Reef",           &trackerTreasureChart38), 3, 0);
    ui->overworld_map_layout_2->addWidget(new TAW("Greatfish Isle",           &trackerTriforceChart1),  3, 1);
    ui->overworld_map_layout_2->addWidget(new TAW("Cyclops Reef",             &trackerTreasureChart21), 3, 2);
    ui->overworld_map_layout_2->addWidget(new TAW("Six Eye Reef",             &trackerTreasureChart6),  3, 3);
    ui->overworld_map_layout_2->addWidget(new TAW("Tower of the Gods Sector", &trackerTreasureChart14), 3, 4);
    ui->overworld_map_layout_2->addWidget(new TAW("Eastern Triangle Island",  &trackerTreasureChart34), 3, 5);
    ui->overworld_map_layout_2->addWidget(new TAW("Thorned Fairy Island",     &trackerTreasureChart5),  3, 6);
    ui->overworld_map_layout_2->addWidget(new TAW("Needle Rock Isle",         &trackerTreasureChart28), 4, 0);
    ui->overworld_map_layout_2->addWidget(new TAW("Islet of Steel",           &trackerTreasureChart35), 4, 1);
    ui->overworld_map_layout_2->addWidget(new TAW("Stone Watcher Island",     &trackerTriforceChart2),  4, 2);
    ui->overworld_map_layout_2->addWidget(new TAW("Southern Triangle Island", &trackerTreasureChart44), 4, 3);
    ui->overworld_map_layout_2->addWidget(new TAW("Private Oasis",            &trackerTreasureChart1),  4, 4);
    ui->overworld_map_layout_2->addWidget(new TAW("Bomb Island",              &trackerTreasureChart20), 4, 5);
    ui->overworld_map_layout_2->addWidget(new TAW("Birds Peak Rock",          &trackerTreasureChart36), 4, 6);
    ui->overworld_map_layout_2->addWidget(new TAW("Diamond Steppe Island",    &trackerTreasureChart23), 5, 0);
    ui->overworld_map_layout_2->addWidget(new TAW("Five Eye Reef",            &trackerTreasureChart12), 5, 1);
    ui->overworld_map_layout_2->addWidget(new TAW("Shark Island",             &trackerTreasureChart16), 5, 2);
    ui->overworld_map_layout_2->addWidget(new TAW("Southern Fairy Island",    &trackerTreasureChart4),  5, 3);
    ui->overworld_map_layout_2->addWidget(new TAW("Ice Ring Isle",            &trackerTreasureChart17), 5, 4);
    ui->overworld_map_layout_2->addWidget(new TAW("Forest Haven",             &trackerTreasureChart31), 5, 5);
    ui->overworld_map_layout_2->addWidget(new TAW("Cliff Plateau Isles",      &trackerTriforceChart3),  5, 6);
    ui->overworld_map_layout_2->addWidget(new TAW("Horseshoe Island",         &trackerTreasureChart9),  6, 0);
    ui->overworld_map_layout_2->addWidget(new TAW("Outset Island",            &trackerTreasureChart43), 6, 1);
    ui->overworld_map_layout_2->addWidget(new TAW("Headstone Island",         &trackerTreasureChart40), 6, 2);
    ui->overworld_map_layout_2->addWidget(new TAW("Two Eye Reef",             &trackerTreasureChart46), 6, 3);
    ui->overworld_map_layout_2->addWidget(new TAW("Angular Isles",            &trackerTreasureChart15), 6, 4);
    ui->overworld_map_layout_2->addWidget(new TAW("Boating Course",           &trackerTreasureChart32), 6, 5);
    ui->overworld_map_layout_2->addWidget(new TAW("Five Star Isles",          &trackerTreasureChart33), 6, 6);

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
        connect(inventoryButton, &TrackerInventoryButton::inventory_button_pressed, this, &MainWindow::update_tracker);
        connect(inventoryButton, &TrackerInventoryButton::mouse_over_item, this, &MainWindow::tracker_display_current_item_text);
    }

    // Connect clicking area widgets to showing the checks in that area
    for (auto area : ui->tracker_tab->findChildren<TrackerAreaLabel*>())
    {
        connect(area, &TrackerAreaLabel::area_label_clicked, this, &MainWindow::tracker_show_specific_area);
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

    // Only update the widget we're displaying
    if (ui->tracker_locations_widget->currentIndex() == LOCATION_TRACKER_OVERWORLD)
    {
        return;
    }

    // Clear previous labels from the locations widget
    auto location_list_layout = ui->specific_locations_layout;
    clear_tracker_labels(location_list_layout);

    int row = 0;
    int col = 0;
    int currentPointSize = 12;
    for (auto& loc : areaLocations[currentTrackerArea])
    {
        auto newLabel = new TrackerLabel(TrackerLabelType::Location, currentPointSize, loc);
        // newLabel->set_location(loc);
        location_list_layout->addWidget(newLabel, row, col);
        connect(newLabel, &TrackerLabel::location_label_clicked, this, &MainWindow::update_tracker);
        connect(newLabel, &TrackerLabel::mouse_over_location_label, this, &MainWindow::tracker_display_current_location);
        connect(newLabel, &TrackerLabel::mouse_left_location_label, this, &MainWindow::tracker_clear_current_area_text);
        row++;

        // Maximum of 13 labels per coloumn
        if (row > 13)
        {
            col++;
            row = 0;

            // When a new column is created subtract 2 from the font point size
            for (auto label : ui->location_list_widget->findChildren<TrackerLabel*>())
            {
                auto labelFont = label->font();
                labelFont.setPointSize(labelFont.pointSize() - 2);
                label->setFont(labelFont);
            }
            currentPointSize -= 2;
        }
    }

    // Clear previous labels from the entrances widget
    clear_tracker_labels(ui->entrance_scroll_layout);

    currentPointSize = 12;
    for (auto& entrance : areaEntrances[currentTrackerArea])
    {
        // New Horizontal layout to add the label and the disconnect button
        // if the entrance is connected
        auto hLayout = new QHBoxLayout();

        auto newLabel = new TrackerLabel(TrackerLabelType::EntranceSource, currentPointSize, nullptr, entrance);
        hLayout->addWidget(newLabel);
        // If the entrance is connected, give the user a disconnect button
        if (entrance->getReplaces())
        {
            hLayout->setContentsMargins(7, 0, 0, 0);
            auto disconnectButton = new QPushButton("X");
            set_font(disconnectButton, "fira_sans", currentPointSize);
            disconnectButton->setCursor(Qt::PointingHandCursor);
            disconnectButton->setMaximumWidth(20);
            disconnectButton->setMaximumHeight(15);
            connect(disconnectButton, &QPushButton::clicked, this, [&](){MainWindow::tracker_disconnect_entrance(entrance);});
            hLayout->addWidget(disconnectButton);
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

// Under certain circumstances we want to mark certain
// locations as accessible even if they logically aren't
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

    // Update each areas information
    for (auto area : ui->tracker_tab->findChildren<TrackerAreaWidget*>())
    {
        area->updateArea();
    }

    // Update stat box labels
    int checkedLocations = 0;
    int accessibleLocations = 0;
    int remainingLocations = 0;
    for (auto loc : trackerLocations)
    {
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

    autosave_current_tracker();
}

void MainWindow::switch_to_overworld_tracker()
{
    ui->tracker_locations_widget->setCurrentIndex(LOCATION_TRACKER_OVERWORLD);
}

void MainWindow::switch_to_area_tracker()
{
    ui->tracker_locations_widget->setCurrentIndex(LOCATION_TRACKER_SPECIFIC_AREA);
}

void MainWindow::switch_to_entrances_tracker()
{
    ui->tracker_locations_widget->setCurrentIndex(LOCATION_TRACKER_SPECIFIC_AREA_ENTRANCES);
}

void MainWindow::switch_to_entrance_destinations_tracker()
{
    ui->tracker_locations_widget->setCurrentIndex(LOCATION_TRACKER_ENTRANCE_DESTINATIONS);
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

void MainWindow::on_entrance_destination_back_button_released()
{
    switch_to_entrances_tracker();
    selectedEntrance = nullptr;
}

void MainWindow::on_clear_all_button_released()
{
    auto& trackerWorld = trackerWorlds[0];
    for (auto locLabel : ui->location_list_widget->findChildren<TrackerLabel*>())
    {
        auto loc = locLabel->get_location();
        loc->marked = true;

        // Clear certain mail locations associated with bosses
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

        locLabel->update_colors();
    }
    update_tracker();
}

void MainWindow::update_items_color() {
    if(ui->override_items_color->isChecked()) {
        const std::string r = std::to_string(itemsColor.red());
        const std::string g = std::to_string(itemsColor.green());
        const std::string b = std::to_string(itemsColor.blue());

        ui->inventory_widget->setStyleSheet(
            QString::fromStdString("QWidget#inventory_widget {background-color: rgb(" + r + ", " + g + ", " + b + ");}")
        );
    }
    else {
        ui->inventory_widget->setStyleSheet("QWidget#inventory_widget {border-image: url(" DATA_PATH "tracker/trackerbg.png);}");
    }
}

void MainWindow::on_override_items_color_stateChanged(int arg1)
{
    update_items_color();
}

void MainWindow::on_items_color_clicked()
{
    QColor color = QColorDialog::getColor(itemsColor, this, "Select color");
    if (!color.isValid()) {
        return;
    }

    itemsColor = color;
    update_items_color();
}

void MainWindow::update_locations_color()
{
    if(ui->override_locations_color->isChecked()) {
        const std::string r = std::to_string(locationsColor.red());
        const std::string g = std::to_string(locationsColor.green());
        const std::string b = std::to_string(locationsColor.blue());

        ui->other_areas_widget->setStyleSheet(
            QString::fromStdString("QWidget#other_areas_widget {background-color: rgb(" + r + ", " + g + ", " + b + ");}")
        );
    }
    else {
        ui->other_areas_widget->setStyleSheet("QWidget#other_areas_widget {background-color: rgba(160, 160, 160, 0.85);}");
    }
}

void MainWindow::on_override_locations_color_stateChanged(int arg1)
{
    update_locations_color();
}

void MainWindow::on_locations_color_clicked()
{
    QColor color = QColorDialog::getColor(locationsColor, this, "Select color");
    if (!color.isValid()) {
        return;
    }

    locationsColor = color;
    update_locations_color();
}

void MainWindow::update_stats_color()
{
    if(ui->override_stats_color->isChecked()) {
        const std::string r = std::to_string(statsColor.red());
        const std::string g = std::to_string(statsColor.green());
        const std::string b = std::to_string(statsColor.blue());

        ui->stat_box->setStyleSheet(
            QString::fromStdString("QWidget#stat_box {background-color: rgb(" + r + ", " + g + ", " + b + ");}")
        );
    }
    else {
        ui->stat_box->setStyleSheet("QWidget#stat_box {background-color: rgba(79, 79, 79, 0.85);}");
    }
}

void MainWindow::on_override_stats_color_stateChanged(int arg1)
{
    update_stats_color();
}

void MainWindow::on_stats_color_clicked()
{
    QColor color = QColorDialog::getColor(statsColor, this, "Select color");
    if (!color.isValid()) {
        return;
    }

    statsColor = color;
    update_stats_color();
}

void MainWindow::set_current_tracker_area(const std::string& areaPrefix)
{
    currentTrackerArea = areaPrefix;
}

void MainWindow::tracker_show_specific_area(std::string areaPrefix)
{
    set_current_tracker_area(areaPrefix);
    switch_to_area_tracker();
    update_tracker();

    // Only show the Clear All option if this is a dungeon
    ui->clear_all_button->setVisible(trackerWorlds[0].dungeons.contains(areaPrefix));

    // Only show the Entrances button if the island has randomized entrances
    ui->location_list_entrances_button->setVisible(!areaEntrances[areaPrefix].empty());

    // Update the image used for the background of the area location labels
    // Currently this lags the tracker a little bit, so maybe revisit later
    // std::replace(areaPrefix.begin(), areaPrefix.end(), ' ', '_');
    // std::transform(areaPrefix.begin(), areaPrefix.end(), areaPrefix.begin(), [](char& c){return std::tolower(c);});
    // std::erase(areaPrefix, '\'');
    // set_location_list_widget_background(areaPrefix);
}

void MainWindow::set_location_list_widget_background(const std::string& area)
{
    ui->location_list_widget->setStyleSheet(std::string("QWidget#location_list_widget {border-image: url(" DATA_PATH "tracker/area_" + area + ".png);}").c_str());
}

void MainWindow::tracker_display_current_item_text(const std::string& currentItem)
{
    if (trackerSettings.randomize_charts && (currentItem.starts_with("Treasure Chart") || currentItem.starts_with("Triforce Chart")))
    {
        ui->current_item_label->setText(std::string("Chart for " + ui->current_area_name_label->text().toStdString()).c_str());;
    }
    else
    {
        ui->current_item_label->setText(currentItem.c_str());
    }

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
        auto newLabel = new TrackerLabel(TrackerLabelType::EntranceDestination, currentPointSize, nullptr, target);
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

    selectedEntrance = nullptr;
    switch_to_entrances_tracker();
    update_tracker();
}

void MainWindow::tracker_disconnect_entrance(Entrance* connectedEntrance)
{
    for (auto& [target, entrance] : connectedTargets)
    {
        if (entrance == connectedEntrance)
        {
            restoreConnections(entrance, target);
            connectedTargets.erase(target);
            set_areas_locations();
            set_areas_entrances();
            update_tracker();
            break;
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

    bool smallKeys = trackerWorld.getSettings().dungeon_big_keys == PlacementOption::OwnDungeon;
    bool bigKeys = trackerWorld.getSettings().dungeon_small_keys == PlacementOption::OwnDungeon;

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
        for (auto& locAccess : area->locations)
        {
            if (locAccess.location->progression)
            {
                if (area->dungeon != "")
                {
                    areaLocations[area->dungeon].insert(locAccess.location);
                }
                else if (area->hintRegion != "")
                {
                    areaLocations[area->hintRegion].insert(locAccess.location);
                }
                else
                {
                    auto islands = area->findIslands();
                    auto dungeons = area->findDungeons();

                    if (dungeons.size() > 0)
                    {
                        areaLocations[dungeons.front()].insert(locAccess.location);
                    }
                    else
                    {
                        for (auto& island : islands)
                        {
                            areaLocations[island].insert(locAccess.location);
                        }
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
    // Only list non-primary entrances if entrances are decoupled
    for (auto& entrance : trackerWorld.getShuffledEntrances(EntranceType::ALL, !trackerWorld.getSettings().decouple_entrances))
    {
        if (entrance->getEntranceType() == EntranceType::MISC_RESTRICTIVE)
        {
            entrance->setEntranceType(EntranceType::MISC);
        }
        auto islands = entrance->getParentArea()->findIslands();
        auto dungeons = entrance->getParentArea()->findDungeons();

        if (dungeons.size() > 0)
        {
            areaEntrances[dungeons.front()].push_back(entrance);
        }
        else
        {
            for (auto& island : islands)
            {
                areaEntrances[island].push_back(entrance);
            }
        }
    }
}

void MainWindow::clear_tracker_labels(QLayout* layout)
{
    clear_layout(layout);
}
