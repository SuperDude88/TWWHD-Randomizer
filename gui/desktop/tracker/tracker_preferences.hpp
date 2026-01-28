#pragma once

#include <QColor>

#include <utility/path.hpp>

struct TrackerPreferences {
    fspath autosaveFilePath;
    bool showLocationLogic = true;
    bool showNonProgressLocations = false;
    bool rightClickClearAll = true;
    bool clearAllIncludesDependentLocations = true;
    bool showCharts = true;
    bool overrideItemsColor = false;
    bool overrideLocationsColor = false;
    bool overrideStatsColor = false;
    QColor itemsColor = {105, 137, 28, 255};
    QColor locationsColor = {160, 160, 160, 255};
    QColor statsColor = {79, 79, 79, 255};
};
