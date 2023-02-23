#include "../mainwindow.h"

#include "../ui_mainwindow.h"
#include <tracker/tracker_inventory_button.h>
#include <tracker/tracker_area_widget.h>
#include <tracker/set_font.h>

#include <logic/Fill.hpp>
#include <logic/Search.hpp>
#include <logic/PoolFunctions.hpp>

#include <QAbstractButton>
#include <QMouseEvent>
#include <QFontDatabase>

#define LOCATION_TRACKER_OVERWORLD 0
#define LOCATION_TRACKER_SPECIFIC_AREA 1

void MainWindow::on_start_tracker_button_clicked()
{
    // Build the world used for the tracker
    auto& trackerWorld = trackerWorlds[0];

    trackerWorld = World();
    trackerWorld.setWorldId(0);

    // Copy settings to potentially modify them
    auto settingsCopy = config.settings;

    settingsCopy.randomize_charts = false;

    trackerWorld.setSettings(settingsCopy);
    if (trackerWorld.loadWorld(DATA_PATH "logic/data/world.yaml", DATA_PATH "logic/data/macros.yaml", DATA_PATH "logic/data/location_data.yaml", DATA_PATH "logic/data/item_data.yaml", DATA_PATH "logic/data/area_names.yaml"))
    {
        show_error_dialog("Could not build world for app tracker");
        return;
    }
    trackerWorld.determineChartMappings();
    trackerWorld.determineProgressionLocations();
    trackerWorld.setItemPools();
    placeVanillaItems(trackerWorlds);

    // TODO: Handle entrance randomizer stuff here

    trackerLocations = trackerWorld.getLocations(true);
    std::sort(trackerLocations.begin(), trackerLocations.end(), [](Location* loc1, Location* loc2){return loc1->sortPriority < loc2->sortPriority;});
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
    for (size_t i = 0; i < startingTingleStatues; i++)
    {
        addElementToPool(startingInventory, tingleStatues[i]);
    }

    auto trackerInventoryCopy = startingInventory;

    for (auto inventoryButton : ui->tracker_tab->findChildren<TrackerInventoryButton*>())
    {
        inventoryButton->state = 0;
        inventoryButton->forbiddenStates.clear();
        for (auto& itemState : inventoryButton->itemStates)
        {
            auto item = Item(itemState.gameItem, &trackerWorld);
            if (itemState.gameItem != GameItem::NOTHING && elementInPool(item, trackerInventoryCopy))
            {
                inventoryButton->addForbiddenState(inventoryButton->state);
                inventoryButton->state++;
                removeElementFromPool(trackerInventoryCopy, item);
            }
        }
        inventoryButton->updateIcon();
    }

    // Set locations for each area
    for (auto area : ui->tracker_tab->findChildren<TrackerAreaWidget*>())
    {
        std::string areaName = area->getPrefix() + " - ";
        auto areaLocations = filterFromPool(trackerLocations, [&](Location* loc){return loc->getName().starts_with(areaName);});
        area->setLocations(areaLocations);
    }

    // Get the first search iteration
    update_tracker();
}

#define SET_BUTTON_TO_LAYOUT(trackerItem, layout, row, col) ui->layout->addWidget(&trackerItem, row, col);

void MainWindow::initialize_tracker()
{

    // Setup Fira Sans font for close button
    SET_FONT(ui->location_list_close_button, "fira_sans", 14);

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

    // Add Background Images
    ui->tracker_tab->setStyleSheet("QWidget#tracker_tab {background-image: url(" DATA_PATH "assets/tracker/background.png);}");
    ui->inventory_widget->setStyleSheet("QWidget#inventory_widget {border-image: url(" DATA_PATH "assets/tracker/trackerbg.png);}");
    ui->inventory_widget_pearls->setStyleSheet("QWidget#inventory_widget_pearls {"
                                                   "background-image: url(" DATA_PATH "assets/tracker/pearl_holder.png);"
                                                   "background-repeat: none;"
                                                   "background-position: center;"
                                               "}");
    ui->overworld_map_widget->setStyleSheet("QWidget#overworld_map_widget {background-image: url(" DATA_PATH "assets/tracker/sea_chart.png);}");
    ui->location_list_widget->setStyleSheet("QWidget#location_list_widget {border-image: url(" DATA_PATH "assets/tracker/area_empty.png) 0 0 0 0 stretch stretch;}");
    ui->other_areas_widget->setStyleSheet("QWidget#other_areas_widget {background-color: gray;}");

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

    ui->other_areas_layout->addWidget(new TAW("Dragon Roost Cavern", "gohma.png",         &trackerDRCSmallKeys,  &trackerDRCBigKey,  &trackerDRCDungeonMap,  &trackerDRCCompass),  0, 0);
    ui->other_areas_layout->addWidget(new TAW("Forbidden Woods",     "kalle_demos.png",   &trackerFWSmallKeys,   &trackerFWBigKey,   &trackerFWDungeonMap,   &trackerFWCompass),   0, 1);
    ui->other_areas_layout->addWidget(new TAW("Tower of the Gods",   "gohdan.png",        &trackerTOTGSmallKeys, &trackerTOTGBigKey, &trackerTOTGDungeonMap, &trackerTOTGCompass), 0, 2);
    ui->other_areas_layout->addWidget(new TAW("Forsaken Fortress",   "helmaroc_king.png", nullptr,               nullptr,            &trackerFFDungeonMap,   &trackerFFCompass),   0, 3);
    ui->other_areas_layout->addWidget(new TAW("Earth Temple",        "jalhalla.png",      &trackerETSmallKeys,   &trackerETBigKey,   &trackerETDungeonMap,   &trackerETCompass),   0, 4);
    ui->other_areas_layout->addWidget(new TAW("Wind Temple",         "molgera.png",       &trackerWTSmallKeys,   &trackerWTBigKey,   &trackerWTDungeonMap,   &trackerWTCompass),   0, 5);
    ui->other_areas_layout->addWidget(new TAW("Great Sea",           "great_sea.png",     nullptr,               nullptr,            nullptr,                nullptr),             0, 6);
    ui->other_areas_layout->addWidget(new TAW("Mailbox",             "mailbox.png",       nullptr,               nullptr,            nullptr,                nullptr),             0, 7);
    ui->other_areas_layout->addWidget(new TAW("Hyrule Castle",       "hyrule.png",        nullptr,               nullptr,            nullptr,                nullptr),             0, 8);
    ui->other_areas_layout->addWidget(new TAW("Ganon's Tower",       "ganondorf.png",     nullptr,               nullptr,            nullptr,                nullptr),             0, 9);

    // Set world and inventory and connect inventory button clicks to updating the tracker
    for (auto inventoryButton : ui->tracker_tab->findChildren<TrackerInventoryButton*>())
    {
        inventoryButton->trackerInventory = &trackerInventory;
        inventoryButton->trackerWorld = &trackerWorlds[0];
        connect(inventoryButton, &TrackerInventoryButton::inventory_button_pressed, this, &MainWindow::update_tracker);
    }

    // Connect clicking area widgets to showing the checks in that area
    for (auto area : ui->tracker_tab->findChildren<TrackerAreaLabel*>())
    {
        connect(area, &TrackerAreaLabel::area_label_clicked, this, &MainWindow::tracker_show_specific_area);
    }
}


void MainWindow::update_tracker()
{
    getAccessibleLocations(trackerWorlds, trackerInventory, trackerLocations);

    update_tracker_areas();

    // No reason to update labels if we're displaying the overworld
    if (ui->tracker_locations_widget->currentIndex() == LOCATION_TRACKER_OVERWORLD)
    {
        return;
    }

    // Clear previous labels from the widget
    auto location_list_layout = ui->specific_locations_layout;
    while (location_list_layout->count() != 0)
    {
        auto label = location_list_layout->itemAt(0);
        location_list_layout->removeItem(label);
    }

    for (auto label : currentlyDisplayedLocations)
    {
        delete label;
    }

    currentlyDisplayedLocations.clear();

    int row = 0;
    int col = 0;
    int currentPointSize = 12;
    for (auto& loc : trackerLocations)
    {
        if (loc->getName().starts_with(currentTrackerArea + " - "))
        {
            auto newLabel = new TrackerLocationLabel(currentPointSize);
            newLabel->set_location(loc);
            location_list_layout->addWidget(newLabel, row, col);
            connect(newLabel, &TrackerLocationLabel::location_label_clicked, this, &MainWindow::update_tracker_areas);
            currentlyDisplayedLocations.push_back(newLabel);
            row++;

            // Maximum of 13 labels per coloumn
            if (row == 13)
            {
                col++;
                row = 0;

                // When a new column is created subtract 2 from the font point size
                for (auto label : currentlyDisplayedLocations)
                {
                    auto labelFont = label->font();
                    labelFont.setPointSize(labelFont.pointSize() - 2);
                    label->setFont(labelFont);
                }
                currentPointSize -= 2;
            }
        }
    }
}

void MainWindow::update_tracker_areas()
{
    // Update each areas information
    for (auto area : ui->tracker_tab->findChildren<TrackerAreaWidget*>())
    {
        area->updateArea();
    }
}

void MainWindow::switch_location_tracker_widgets()
{
    if (ui->tracker_locations_widget->currentIndex() == LOCATION_TRACKER_OVERWORLD)
    {
        ui->tracker_locations_widget->setCurrentIndex(LOCATION_TRACKER_SPECIFIC_AREA);
    }
    else
    {
        ui->tracker_locations_widget->setCurrentIndex(LOCATION_TRACKER_OVERWORLD);
    }
}

void MainWindow::on_location_list_close_button_released()
{
    switch_location_tracker_widgets();
}

void MainWindow::set_current_tracker_area(const std::string& areaPrefix)
{
    currentTrackerArea = areaPrefix;
}

void MainWindow::tracker_show_specific_area(std::string areaPrefix)
{
    set_current_tracker_area(areaPrefix);
    switch_location_tracker_widgets();
    update_tracker();
}
