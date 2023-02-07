#include "mainwindow.h"

#include <ui_mainwindow.h>

#include <logic/World.hpp>
#include <logic/Fill.hpp>
#include <logic/Search.hpp>

static WorldPool trackerWorlds(1);
static ItemPool trackerInventory;
static LocationPool trackerLocations;


void MainWindow::on_start_tracker_button_clicked()
{
    // Build the world used for the tracker
    auto& trackerWorld = trackerWorlds[0];

    trackerWorld = World();
    trackerWorld.setWorldId(0);

    // Copy settings to potentially modify things
    auto settingsCopy = config.settings;

    trackerWorld.setSettings(settingsCopy);
    if (trackerWorld.loadWorld(DATA_PATH "logic/data/world.yaml", DATA_PATH "logic/data/macros.yaml", DATA_PATH "logic/data/location_data.yaml", DATA_PATH "logic/data/item_data.yaml", DATA_PATH "logic/data/area_names.yaml"))
    {
        show_error_dialog("Could not build world for app tracker");
        return;
    }
    trackerWorld.determineProgressionLocations();
    placeVanillaItems(trackerWorlds);

    // TODO: Handle entrance randomizer stuff here

    // Get the first search iteration
    trackerLocations = trackerWorld.getLocations(true);
    auto accessibleLocations = getAccessibleLocations(trackerWorlds, trackerInventory, trackerLocations);

    for (auto& loc : accessibleLocations)
    {
        std::cout << loc->getName() << std::endl;
    }
}
