#include "mainwindow.h"

#include <ui_mainwindow.h>
#include <tracker_inventory_button.h>
#include <tracker_area_widget.h>

#include <logic/Fill.hpp>
#include <logic/Search.hpp>
#include <logic/PoolFunctions.hpp>

#include <QAbstractButton>
#include <QMouseEvent>
#include <QFontDatabase>

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

#define INITIALIZE_MAIN_TRACKER_BUTTON(trackerItem, layout, row, col) \
    ui->layout->addWidget(&trackerItem, row, col);                    \
    trackerItem.trackerInventory = &trackerInventory;                 \
    trackerItem.trackerWorld = &trackerWorlds[0];                     \
    trackerItem.show();                                               \

#define INITIALIZE_OTHER_TRACKER_BUTTON(trackerItem)                  \
    trackerItem.trackerInventory = &trackerInventory;                 \
    trackerItem.trackerWorld = &trackerWorlds[0];                     \
    trackerItem.show();                                               \

void MainWindow::initialize_tracker()
{

    // Setup Fira Sans font for close button
    int firaSansFontId = QFontDatabase::addApplicationFont(DATA_PATH "assets/tracker/fira_sans.ttf");
    QString family = QFontDatabase::applicationFontFamilies(firaSansFontId).at(0);
    QFont fira(family);
    fira.setPointSize(14);
    ui->location_list_close_button->setFont(fira);

    if (trackerWorlds.empty())
    {
        trackerWorlds = WorldPool(1);
    }
    trackerWorlds[0].setWorldId(0);

    // Setup inventory buttons
    INITIALIZE_MAIN_TRACKER_BUTTON(trackerTelescope,             inventory_layout_top, 0, 0);
    INITIALIZE_MAIN_TRACKER_BUTTON(trackerProgressiveSail,       inventory_layout_top, 0, 1);
    INITIALIZE_MAIN_TRACKER_BUTTON(trackerWindWaker,             inventory_layout_top, 0, 2);
    INITIALIZE_MAIN_TRACKER_BUTTON(trackerGrapplingHook,         inventory_layout_top, 0, 3);
    INITIALIZE_MAIN_TRACKER_BUTTON(trackerSpoilsBag,             inventory_layout_top, 0, 4);
    INITIALIZE_MAIN_TRACKER_BUTTON(trackerBoomerang,             inventory_layout_top, 0, 5);
    INITIALIZE_MAIN_TRACKER_BUTTON(trackerDekuLeaf,              inventory_layout_top, 0, 6);
    INITIALIZE_MAIN_TRACKER_BUTTON(trackerProgressiveSword,      inventory_layout_top, 0, 7);
    INITIALIZE_MAIN_TRACKER_BUTTON(trackerTingleBottle,          inventory_layout_top, 1, 0);
    INITIALIZE_MAIN_TRACKER_BUTTON(trackerProgressivePictoBox,   inventory_layout_top, 1, 1);
    INITIALIZE_MAIN_TRACKER_BUTTON(trackerIronBoots,             inventory_layout_top, 1, 2);
    INITIALIZE_MAIN_TRACKER_BUTTON(trackerMagicArmor,            inventory_layout_top, 1, 3);
    INITIALIZE_MAIN_TRACKER_BUTTON(trackerBaitBag,               inventory_layout_top, 1, 4);
    INITIALIZE_MAIN_TRACKER_BUTTON(trackerProgressiveBow,        inventory_layout_top, 1, 5);
    INITIALIZE_MAIN_TRACKER_BUTTON(trackerBombs,                 inventory_layout_top, 1, 6);
    INITIALIZE_MAIN_TRACKER_BUTTON(trackerProgressiveShield,     inventory_layout_top, 1, 7);
    INITIALIZE_MAIN_TRACKER_BUTTON(trackerCabanaDeed,            inventory_layout_middle, 0, 0);
    INITIALIZE_MAIN_TRACKER_BUTTON(trackerMaggiesLetter,         inventory_layout_middle, 0, 1);
    INITIALIZE_MAIN_TRACKER_BUTTON(trackerMoblinsLetter,         inventory_layout_middle, 0, 2);
    INITIALIZE_MAIN_TRACKER_BUTTON(trackerNoteToMom,             inventory_layout_middle, 0, 3);
    INITIALIZE_MAIN_TRACKER_BUTTON(trackerDeliveryBag,           inventory_layout_middle, 0, 4);
    INITIALIZE_MAIN_TRACKER_BUTTON(trackerHookshot,              inventory_layout_middle, 0, 5);
    INITIALIZE_MAIN_TRACKER_BUTTON(trackerSkullHammer,           inventory_layout_middle, 0, 6);
    INITIALIZE_MAIN_TRACKER_BUTTON(trackerPowerBracelets,        inventory_layout_middle, 0, 7);
    INITIALIZE_MAIN_TRACKER_BUTTON(trackerEmptyBottle,           inventory_layout_middle, 1, 0);
    INITIALIZE_MAIN_TRACKER_BUTTON(trackerWindsRequiem,          inventory_layout_middle, 1, 1);
    INITIALIZE_MAIN_TRACKER_BUTTON(trackerBalladOfGales,         inventory_layout_middle, 1, 2);
    INITIALIZE_MAIN_TRACKER_BUTTON(trackerCommandMelody,         inventory_layout_middle, 1, 3);
    INITIALIZE_MAIN_TRACKER_BUTTON(trackerEarthGodsLyric,        inventory_layout_middle, 1, 4);
    INITIALIZE_MAIN_TRACKER_BUTTON(trackerWindGodsAria,          inventory_layout_middle, 1, 5);
    INITIALIZE_MAIN_TRACKER_BUTTON(trackerSongOfPassing,         inventory_layout_middle, 1, 6);
    INITIALIZE_MAIN_TRACKER_BUTTON(trackerHerosCharm,            inventory_layout_middle, 1, 7);
    INITIALIZE_MAIN_TRACKER_BUTTON(trackerDinsPearl,             inventory_layout_dins_farores_pearl, 0, 0);
    INITIALIZE_MAIN_TRACKER_BUTTON(trackerFaroresPearl,          inventory_layout_dins_farores_pearl, 0, 1);
    INITIALIZE_MAIN_TRACKER_BUTTON(trackerNayrusPearl,           inventory_layout_nayrus_pearl, 0, 0);
    INITIALIZE_MAIN_TRACKER_BUTTON(trackerTriforceShards,        inventory_layout_triforce, 0, 0);
    INITIALIZE_MAIN_TRACKER_BUTTON(trackerTingleStatues,         inventory_layout_bottom_right, 0, 1);
    INITIALIZE_MAIN_TRACKER_BUTTON(trackerGhostShipChart,        inventory_layout_bottom_right, 0, 2);
    INITIALIZE_MAIN_TRACKER_BUTTON(trackerHurricaneSpin,         inventory_layout_bottom_right, 0, 3);
    INITIALIZE_MAIN_TRACKER_BUTTON(trackerProgressiveBombBag,    inventory_layout_bottom_right, 1, 0);
    INITIALIZE_MAIN_TRACKER_BUTTON(trackerProgressiveQuiver,     inventory_layout_bottom_right, 1, 1);
    INITIALIZE_MAIN_TRACKER_BUTTON(trackerProgressiveWallet,     inventory_layout_bottom_right, 1, 2);
    INITIALIZE_MAIN_TRACKER_BUTTON(trackerProgressiveMagicMeter, inventory_layout_bottom_right, 1, 3);

    // Chart buttons
    INITIALIZE_OTHER_TRACKER_BUTTON(trackerTreasureChart1);
    INITIALIZE_OTHER_TRACKER_BUTTON(trackerTreasureChart2);
    INITIALIZE_OTHER_TRACKER_BUTTON(trackerTreasureChart3);
    INITIALIZE_OTHER_TRACKER_BUTTON(trackerTreasureChart4);
    INITIALIZE_OTHER_TRACKER_BUTTON(trackerTreasureChart5);
    INITIALIZE_OTHER_TRACKER_BUTTON(trackerTreasureChart6);
    INITIALIZE_OTHER_TRACKER_BUTTON(trackerTreasureChart7);
    INITIALIZE_OTHER_TRACKER_BUTTON(trackerTreasureChart8);
    INITIALIZE_OTHER_TRACKER_BUTTON(trackerTreasureChart9);
    INITIALIZE_OTHER_TRACKER_BUTTON(trackerTreasureChart10);
    INITIALIZE_OTHER_TRACKER_BUTTON(trackerTreasureChart11);
    INITIALIZE_OTHER_TRACKER_BUTTON(trackerTreasureChart12);
    INITIALIZE_OTHER_TRACKER_BUTTON(trackerTreasureChart13);
    INITIALIZE_OTHER_TRACKER_BUTTON(trackerTreasureChart14);
    INITIALIZE_OTHER_TRACKER_BUTTON(trackerTreasureChart15);
    INITIALIZE_OTHER_TRACKER_BUTTON(trackerTreasureChart16);
    INITIALIZE_OTHER_TRACKER_BUTTON(trackerTreasureChart17);
    INITIALIZE_OTHER_TRACKER_BUTTON(trackerTreasureChart18);
    INITIALIZE_OTHER_TRACKER_BUTTON(trackerTreasureChart19);
    INITIALIZE_OTHER_TRACKER_BUTTON(trackerTreasureChart20);
    INITIALIZE_OTHER_TRACKER_BUTTON(trackerTreasureChart21);
    INITIALIZE_OTHER_TRACKER_BUTTON(trackerTreasureChart22);
    INITIALIZE_OTHER_TRACKER_BUTTON(trackerTreasureChart23);
    INITIALIZE_OTHER_TRACKER_BUTTON(trackerTreasureChart24);
    INITIALIZE_OTHER_TRACKER_BUTTON(trackerTreasureChart25);
    INITIALIZE_OTHER_TRACKER_BUTTON(trackerTreasureChart26);
    INITIALIZE_OTHER_TRACKER_BUTTON(trackerTreasureChart27);
    INITIALIZE_OTHER_TRACKER_BUTTON(trackerTreasureChart28);
    INITIALIZE_OTHER_TRACKER_BUTTON(trackerTreasureChart29);
    INITIALIZE_OTHER_TRACKER_BUTTON(trackerTreasureChart30);
    INITIALIZE_OTHER_TRACKER_BUTTON(trackerTreasureChart31);
    INITIALIZE_OTHER_TRACKER_BUTTON(trackerTreasureChart32);
    INITIALIZE_OTHER_TRACKER_BUTTON(trackerTreasureChart33);
    INITIALIZE_OTHER_TRACKER_BUTTON(trackerTreasureChart34);
    INITIALIZE_OTHER_TRACKER_BUTTON(trackerTreasureChart35);
    INITIALIZE_OTHER_TRACKER_BUTTON(trackerTreasureChart36);
    INITIALIZE_OTHER_TRACKER_BUTTON(trackerTreasureChart37);
    INITIALIZE_OTHER_TRACKER_BUTTON(trackerTreasureChart38);
    INITIALIZE_OTHER_TRACKER_BUTTON(trackerTreasureChart39);
    INITIALIZE_OTHER_TRACKER_BUTTON(trackerTreasureChart40);
    INITIALIZE_OTHER_TRACKER_BUTTON(trackerTreasureChart41);
    INITIALIZE_OTHER_TRACKER_BUTTON(trackerTreasureChart42);
    INITIALIZE_OTHER_TRACKER_BUTTON(trackerTreasureChart43);
    INITIALIZE_OTHER_TRACKER_BUTTON(trackerTreasureChart44);
    INITIALIZE_OTHER_TRACKER_BUTTON(trackerTreasureChart45);
    INITIALIZE_OTHER_TRACKER_BUTTON(trackerTreasureChart46);
    INITIALIZE_OTHER_TRACKER_BUTTON(trackerTriforceChart1);
    INITIALIZE_OTHER_TRACKER_BUTTON(trackerTriforceChart2);
    INITIALIZE_OTHER_TRACKER_BUTTON(trackerTriforceChart3);

    // Dungeon Items
    INITIALIZE_OTHER_TRACKER_BUTTON(trackerWTSmallKeys);
    INITIALIZE_OTHER_TRACKER_BUTTON(trackerWTBigKey);
    INITIALIZE_OTHER_TRACKER_BUTTON(trackerWTDungeonMap);
    INITIALIZE_OTHER_TRACKER_BUTTON(trackerWTCompass);
    INITIALIZE_OTHER_TRACKER_BUTTON(trackerETSmallKeys);
    INITIALIZE_OTHER_TRACKER_BUTTON(trackerETBigKey);
    INITIALIZE_OTHER_TRACKER_BUTTON(trackerETDungeonMap);
    INITIALIZE_OTHER_TRACKER_BUTTON(trackerETCompass);
    INITIALIZE_OTHER_TRACKER_BUTTON(trackerFFDungeonMap);
    INITIALIZE_OTHER_TRACKER_BUTTON(trackerFFCompass);
    INITIALIZE_OTHER_TRACKER_BUTTON(trackerTOTGSmallKeys);
    INITIALIZE_OTHER_TRACKER_BUTTON(trackerTOTGBigKey);
    INITIALIZE_OTHER_TRACKER_BUTTON(trackerTOTGDungeonMap);
    INITIALIZE_OTHER_TRACKER_BUTTON(trackerTOTGCompass);
    INITIALIZE_OTHER_TRACKER_BUTTON(trackerFWSmallKeys);
    INITIALIZE_OTHER_TRACKER_BUTTON(trackerFWBigKey);
    INITIALIZE_OTHER_TRACKER_BUTTON(trackerFWDungeonMap);
    INITIALIZE_OTHER_TRACKER_BUTTON(trackerFWCompass);
    INITIALIZE_OTHER_TRACKER_BUTTON(trackerDRCSmallKeys);
    INITIALIZE_OTHER_TRACKER_BUTTON(trackerDRCBigKey);
    INITIALIZE_OTHER_TRACKER_BUTTON(trackerDRCDungeonMap);
    INITIALIZE_OTHER_TRACKER_BUTTON(trackerDRCCompass);

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


    ui->overworld_map_layout_2->addWidget(&ForsakenFortressSectorWidget, 0, 0);
    ui->overworld_map_layout_2->addWidget(&StarIslandWidget,             0, 1);
    ui->overworld_map_layout_2->addWidget(&NorthernFairyIslandWidget,    0, 2);
    ui->overworld_map_layout_2->addWidget(&GaleIsleWidget,               0, 3);
    ui->overworld_map_layout_2->addWidget(&CrescentMoonIslandWidget,     0, 4);
    ui->overworld_map_layout_2->addWidget(&SevenStarIslesWidget,         0, 5);
    ui->overworld_map_layout_2->addWidget(&OverlookIslandWidget,         0, 6);
    ui->overworld_map_layout_2->addWidget(&FourEyeReefWidget,            1, 0);
    ui->overworld_map_layout_2->addWidget(&MotherAndChildIslesWidget,    1, 1);
    ui->overworld_map_layout_2->addWidget(&SpectacleIslandWidget,        1, 2);
    ui->overworld_map_layout_2->addWidget(&WindfallIslandWidget,         1, 3);
    ui->overworld_map_layout_2->addWidget(&PawprintIsleWidget,           1, 4);
    ui->overworld_map_layout_2->addWidget(&DragonRoostIslandWidget,      1, 5);
    ui->overworld_map_layout_2->addWidget(&FlightControlPlatformWidget,  1, 6);
    ui->overworld_map_layout_2->addWidget(&WesternFairyIslandWidget,     2, 0);
    ui->overworld_map_layout_2->addWidget(&RockSpireIsleWidget,          2, 1);
    ui->overworld_map_layout_2->addWidget(&TingleIslandWidget,           2, 2);
    ui->overworld_map_layout_2->addWidget(&NorthernTriangleIslandWidget, 2, 3);
    ui->overworld_map_layout_2->addWidget(&EasternFairyIslandWidget,     2, 4);
    ui->overworld_map_layout_2->addWidget(&FireMountainWidget,           2, 5);
    ui->overworld_map_layout_2->addWidget(&StarBeltArchipelagoWidget,    2, 6);
    ui->overworld_map_layout_2->addWidget(&ThreeEyeReefWidget,           3, 0);
    ui->overworld_map_layout_2->addWidget(&GreatfishIsleWidget,          3, 1);
    ui->overworld_map_layout_2->addWidget(&CyclopsReefWidget,            3, 2);
    ui->overworld_map_layout_2->addWidget(&SixEyeReefWidget,             3, 3);
    ui->overworld_map_layout_2->addWidget(&TowerOfTheGodsSectorWidget,   3, 4);
    ui->overworld_map_layout_2->addWidget(&EasternTriangleIslandWidget,  3, 5);
    ui->overworld_map_layout_2->addWidget(&ThornedFairyIslandWidget,     3, 6);
    ui->overworld_map_layout_2->addWidget(&NeedleRockIsleWidget,         4, 0);
    ui->overworld_map_layout_2->addWidget(&IsletofSteelWidget,           4, 1);
    ui->overworld_map_layout_2->addWidget(&StoneWatcherIslandWidget,     4, 2);
    ui->overworld_map_layout_2->addWidget(&SouthernTriangleIslandWidget, 4, 3);
    ui->overworld_map_layout_2->addWidget(&PrivateOasisWidget,           4, 4);
    ui->overworld_map_layout_2->addWidget(&BombIslandWidget,             4, 5);
    ui->overworld_map_layout_2->addWidget(&BirdsPeakRockWidget,          4, 6);
    ui->overworld_map_layout_2->addWidget(&DiamondSteppeIslandWidget,    5, 0);
    ui->overworld_map_layout_2->addWidget(&FiveEyeReefWidget,            5, 1);
    ui->overworld_map_layout_2->addWidget(&SharkIslandWidget,            5, 2);
    ui->overworld_map_layout_2->addWidget(&SouthernFairyIslandWidget,    5, 3);
    ui->overworld_map_layout_2->addWidget(&IceRingIsleWidget,            5, 4);
    ui->overworld_map_layout_2->addWidget(&ForestHavenWidget,            5, 5);
    ui->overworld_map_layout_2->addWidget(&CliffPlateauIslesWidget,      5, 6);
    ui->overworld_map_layout_2->addWidget(&HorseshoeIslandWidget,        6, 0);
    ui->overworld_map_layout_2->addWidget(&OutsetIslandWidget,           6, 1);
    ui->overworld_map_layout_2->addWidget(&HeadstoneIslandWidget,        6, 2);
    ui->overworld_map_layout_2->addWidget(&TwoEyeReefWidget,             6, 3);
    ui->overworld_map_layout_2->addWidget(&AngularIslesWidget,           6, 4);
    ui->overworld_map_layout_2->addWidget(&BoatingCourseWidget,          6, 5);
    ui->overworld_map_layout_2->addWidget(&FiveStarIslesWidget,          6, 6);

    ui->other_areas_layout->addWidget(&DRCWidget, 0, 0);
    ui->other_areas_layout->addWidget(&FWWidget, 0, 1);
    ui->other_areas_layout->addWidget(&TOTGWidget, 0, 2);
    ui->other_areas_layout->addWidget(&ETWidget, 0, 3);
    ui->other_areas_layout->addWidget(&WTWidget, 0, 4);
    ui->other_areas_layout->addWidget(&FFWidget, 0, 5);
    ui->other_areas_layout->addWidget(&GreatSeaWidget, 0, 6);
    ui->other_areas_layout->addWidget(&MailboxWidget, 0, 7);
    ui->other_areas_layout->addWidget(&HyruleWidget, 0, 8);
    ui->other_areas_layout->addWidget(&GanonsTowerWidget, 0, 9);

    // Connect inventory button clicks to updating the tracker
    for (auto inventoryButton : ui->tracker_tab->findChildren<TrackerInventoryButton*>())
    {
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

    if (ui->tracker_locations_widget->currentIndex() == 0)
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
    if (ui->tracker_locations_widget->currentIndex() == 0)
    {
        ui->tracker_locations_widget->setCurrentIndex(1);
    }
    else
    {
        ui->tracker_locations_widget->setCurrentIndex(0);
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
