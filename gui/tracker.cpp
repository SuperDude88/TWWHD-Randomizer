#include "mainwindow.h"

#include <ui_mainwindow.h>
#include <tracker_inventory_button.h>

#include <logic/Fill.hpp>
#include <logic/Search.hpp>
#include <logic/PoolFunctions.hpp>

#include <QAbstractButton>
#include <QMouseEvent>

void MainWindow::on_start_tracker_button_clicked()
{
    // Build the world used for the tracker
    if (trackerWorlds.empty())
    {
        trackerWorlds = WorldPool(1);
    }
    auto& trackerWorld = trackerWorlds[0];

    initialize_tracker_buttons();

    trackerWorld = World();
    trackerWorld.setWorldId(0);

    // Copy settings to potentially modify them
    auto settingsCopy = config.settings;

    trackerWorld.setSettings(settingsCopy);
    if (trackerWorld.loadWorld(DATA_PATH "logic/data/world.yaml", DATA_PATH "logic/data/macros.yaml", DATA_PATH "logic/data/location_data.yaml", DATA_PATH "logic/data/item_data.yaml", DATA_PATH "logic/data/area_names.yaml"))
    {
        show_error_dialog("Could not build world for app tracker");
        return;
    }
    trackerWorld.setItemPools();
    trackerWorld.determineProgressionLocations();
    placeVanillaItems(trackerWorlds);

    // TODO: Handle entrance randomizer stuff here

    trackerLocations = trackerWorld.getLocations(true);
    trackerInventory = trackerWorld.getStartingItems();

    // Modify starting tingle statues to match order of tracker items


    // Update inventory gui to have starting items
    //
    // Tingle Statues have to be set manually since they can
    // obtained in any order



    auto trackerInventoryCopy = trackerInventory;


    for (auto inventoryButton : ui->inventory_widget->findChildren<TrackerInventoryButton*>())
    {
        for (auto& itemState : inventoryButton->itemStates)
        {
            auto item = Item(itemState.gameItem, &trackerWorld);
            if (itemState.gameItem != GameItem::NOTHING && elementInPool(item, trackerInventoryCopy))
            {
                inventoryButton->state++;
                removeElementFromPool(trackerInventoryCopy, item);
            }
        }
    }

    // Get the first search iteration
    update_tracker();
}

#define INITIALIZE_TRACKER_BUTTON(trackerItem, layout, row, col) \
    ui->layout->addWidget(&trackerItem, row, col);               \
    trackerItem.trackerInventory = &trackerInventory;            \
    trackerItem.trackerWorld = &trackerWorlds[0];                \
    trackerItem.show();                                          \
    trackerItem.setFlat(true);

void MainWindow::initialize_tracker_buttons()
{
    INITIALIZE_TRACKER_BUTTON(trackerTelescope,             inventory_layout_top, 0, 0);
    INITIALIZE_TRACKER_BUTTON(trackerProgressiveSail,       inventory_layout_top, 0, 1);
    INITIALIZE_TRACKER_BUTTON(trackerWindWaker,             inventory_layout_top, 0, 2);
    INITIALIZE_TRACKER_BUTTON(trackerGrapplingHook,         inventory_layout_top, 0, 3);
    INITIALIZE_TRACKER_BUTTON(trackerSpoilsBag,             inventory_layout_top, 0, 4);
    INITIALIZE_TRACKER_BUTTON(trackerBoomerang,             inventory_layout_top, 0, 5);
    INITIALIZE_TRACKER_BUTTON(trackerDekuLeaf,              inventory_layout_top, 0, 6);
    INITIALIZE_TRACKER_BUTTON(trackerProgressiveSword,      inventory_layout_top, 0, 7);
    INITIALIZE_TRACKER_BUTTON(trackerTingleBottle,          inventory_layout_top, 1, 0);
    INITIALIZE_TRACKER_BUTTON(trackerProgressivePictoBox,   inventory_layout_top, 1, 1);
    INITIALIZE_TRACKER_BUTTON(trackerIronBoots,             inventory_layout_top, 1, 2);
    INITIALIZE_TRACKER_BUTTON(trackerMagicArmor,            inventory_layout_top, 1, 3);
    INITIALIZE_TRACKER_BUTTON(trackerBaitBag,               inventory_layout_top, 1, 4);
    INITIALIZE_TRACKER_BUTTON(trackerProgressiveBow,        inventory_layout_top, 1, 5);
    INITIALIZE_TRACKER_BUTTON(trackerBombs,                 inventory_layout_top, 1, 6);
    INITIALIZE_TRACKER_BUTTON(trackerProgressiveShield,     inventory_layout_top, 1, 7);
    INITIALIZE_TRACKER_BUTTON(trackerCabanaDeed,            inventory_layout_middle, 0, 0);
    INITIALIZE_TRACKER_BUTTON(trackerMaggiesLetter,         inventory_layout_middle, 0, 1);
    INITIALIZE_TRACKER_BUTTON(trackerMoblinsLetter,         inventory_layout_middle, 0, 2);
    INITIALIZE_TRACKER_BUTTON(trackerNoteToMom,             inventory_layout_middle, 0, 3);
    INITIALIZE_TRACKER_BUTTON(trackerDeliveryBag,           inventory_layout_middle, 0, 4);
    INITIALIZE_TRACKER_BUTTON(trackerHookshot,              inventory_layout_middle, 0, 5);
    INITIALIZE_TRACKER_BUTTON(trackerSkullHammer,           inventory_layout_middle, 0, 6);
    INITIALIZE_TRACKER_BUTTON(trackerPowerBracelets,        inventory_layout_middle, 0, 7);
    INITIALIZE_TRACKER_BUTTON(trackerEmptyBottle,           inventory_layout_middle, 1, 0);
    INITIALIZE_TRACKER_BUTTON(trackerWindsRequiem,          inventory_layout_middle, 1, 1);
    INITIALIZE_TRACKER_BUTTON(trackerBalladOfGales,         inventory_layout_middle, 1, 2);
    INITIALIZE_TRACKER_BUTTON(trackerCommandMelody,         inventory_layout_middle, 1, 3);
    INITIALIZE_TRACKER_BUTTON(trackerEarthGodsLyric,        inventory_layout_middle, 1, 4);
    INITIALIZE_TRACKER_BUTTON(trackerWindGodsAria,          inventory_layout_middle, 1, 5);
    INITIALIZE_TRACKER_BUTTON(trackerSongOfPassing,         inventory_layout_middle, 1, 6);
    INITIALIZE_TRACKER_BUTTON(trackerHerosCharm,            inventory_layout_middle, 1, 7);
    INITIALIZE_TRACKER_BUTTON(trackerDinsPearl,             inventory_layout_dins_farores_pearl, 0, 0);
    INITIALIZE_TRACKER_BUTTON(trackerFaroresPearl,          inventory_layout_dins_farores_pearl, 0, 1);
    INITIALIZE_TRACKER_BUTTON(trackerNayrusPearl,           inventory_layout_nayrus_pearl, 0, 0);
    INITIALIZE_TRACKER_BUTTON(trackerTriforceShards,        inventory_layout_triforce, 0, 0);
    INITIALIZE_TRACKER_BUTTON(trackerTingleStatues,         inventory_layout_bottom_right, 0, 1);
    INITIALIZE_TRACKER_BUTTON(trackerGhostShipChart,        inventory_layout_bottom_right, 0, 2);
    INITIALIZE_TRACKER_BUTTON(trackerHurricaneSpin,         inventory_layout_bottom_right, 0, 3);
    INITIALIZE_TRACKER_BUTTON(trackerProgressiveBombBag,    inventory_layout_bottom_right, 1, 0);
    INITIALIZE_TRACKER_BUTTON(trackerProgressiveQuiver,     inventory_layout_bottom_right, 1, 1);
    INITIALIZE_TRACKER_BUTTON(trackerProgressiveWallet,     inventory_layout_bottom_right, 1, 2);
    INITIALIZE_TRACKER_BUTTON(trackerProgressiveMagicMeter, inventory_layout_bottom_right, 1, 3);

    // Triforce is twice as big as other items on tracker
    trackerTriforceShards.setIconSize(QSize(INVENTORY_BUTTON_SIZE * 2, INVENTORY_BUTTON_SIZE * 2));

    // Add Background Images
    ui->tracker_tab->setStyleSheet("QWidget#tracker_tab {background-image: url(" DATA_PATH "assets/tracker/background.png);}");
    ui->inventory_widget->setStyleSheet("QWidget#inventory_widget {border-image: url(" DATA_PATH "assets/tracker/trackerbg.png);}");
    ui->inventory_widget_pearls->setStyleSheet("QWidget#inventory_widget_pearls {"
                                                   "background-image: url(" DATA_PATH "assets/tracker/pearl_holder.png);"
                                                   "background-repeat: none;"
                                                   "background-position: center;"
                                               "}");
    ui->location_list_widget->setStyleSheet("QWidget#location_list_widget {background-image: url(" DATA_PATH "assets/tracker/sea_chart.png);}");
}


void MainWindow::update_tracker()
{
    auto accessibleLocations = getAccessibleLocations(trackerWorlds, trackerInventory, trackerLocations);

    // Clear previous labels from the widget
    auto location_list_layout = ui->location_list_layout;
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
    for (auto& loc : accessibleLocations)
    {
        currentlyDisplayedLocations.push_back(new TrackerLocationLabel(loc));
        location_list_layout->addWidget(currentlyDisplayedLocations.back(), row, col);



        row++;
    }


}
