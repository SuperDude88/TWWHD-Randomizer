#include "Structs.hpp"
#include "json.hpp"
#include <algorithm>
#include <unordered_set>
#include <random>

//temp section - remove later
#include "parse.hpp"
#include "Location.hpp"
#include <iostream>


std::vector<std::string> ProgressItems = {"WindWaker", "SpoilsBag", "GrapplingHook", "PowerBracelets", "IronBoots", "BaitBag", "Boomerang", "Hookshot", "DeliveryBag", "Bombs", "SkullHammer", "DekuLeaf",
"Shard", "Shard", "Shard", "Shard", "Shard", "Shard", "Shard", "Shard", "NayrusPearl", "DinsPearl", "FaroresPearl", "WindsRequiem", "BalladofGales", "CommandMelody", "EarthGodsLyric", "WindGodsAria", "SongofPassing",
"Sail", "NotetoMom", "MaggiesLetter", "MoblinsLetter", "CabanaDeed", "DragonTingleStatue", "ForbiddenTingleStatue", "GoddessTingleStatue", "EarthTingleStatue", "WindTingleStatue", "BombBag", "BombBag", "Quiver", "Quiver",
"MagicMeter", "GhostShipChart", "Sword", "Sword", "Sword", "Sword", "Shield", "Shield", "Bow", "Bow", "Bow", "Wallet", "Wallet", "PictoBox", "PictoBox", "Bottle"};

std::vector<std::string> PermanentItems; //These are starting items that you always own, this ensures they arent randomized accidentally

std::vector<std::string> DRCProgressItems = {"DRCBigKey", "DRCSmallKey", "DRCSmallKey", "DRCSmallKey", "DRCSmallKey"};
std::vector<std::string> FWProgressItems = {"FWBigKey", "FWSmallKey"};
std::vector<std::string> TOTGProgressItems = {"TOTGBigKey", "TOTGSmallKey", "TOTGSmallKey"};
std::vector<std::string> ETProgressItems = {"ETBigKey", "ETSmallKey", "ETSmallKey", "ETSmallKey"};
std::vector<std::string> WTProgressItems = {"WTBigKey", "WTSmallKey", "WTSmallKey"};

std::vector<std::string> DRCNonProgressItems = {"DungeonMap", "Compass"};
std::vector<std::string> FWNonProgressItems = {"DungeonMap", "Compass"};
std::vector<std::string> TOTGNonProgressItems = {"DungeonMap", "Compass"};
std::vector<std::string> FFNonProgressItems = {"DungeonMap", "Compass"};
std::vector<std::string> ETNonProgressItems = {"DungeonMap", "Compass"};
std::vector<std::string> WTNonProgressItems = {"DungeonMap", "Compass"};

bool has_item(std::vector<std::string> OwnedItems, std::string Item) {
    return std::find(OwnedItems.begin(), OwnedItems.end(), Item) != OwnedItems.end();
}

//Add "can_access" condition to both accessibility checks to allow randomized entrances and make certain things easier
//Also probably add macros so that certain things, like farming knight's crests, aren't terrible to implement each time
//This won't be added for a bit because its complex and I'm lazy (even though it would make things easier), plus core features come first
bool isAccessible(std::vector<std::string> OwnedItems, nlohmann::json expression) {

    //item is the input (location.needs)

    std::string item_typ = expression["type"];
    auto& item_args = expression["args"];
    if (item_typ == "or") {
        return std::any_of(item_args.begin(), item_args.end(), [&](auto arg) {return isAccessible(OwnedItems, arg); });
    }
    else if (item_typ == "and") {
        return std::all_of(item_args.begin(), item_args.end(), [&](auto arg) {return isAccessible(OwnedItems, arg); });
    }
    else if (item_typ == "count") {
        std::string count = item_args[0]["count"];
        //Add something to deal with small keys. Likely searching through the accessible dungeon locations to see what keys should be owned. This doesnt really follow the rest of assumed fill but I cant think of a better solution that is as effective
        return std::count(OwnedItems.begin(), OwnedItems.end(), item_args[0]["args"]) >= stoi(count, nullptr);
    }
    else if (item_typ == "has_item") {
        if (item_args[0] == "Nothing") return true;
        else {
            return has_item(OwnedItems, item_args[0]);
        }
    }
    else {
        printf("unknown type!");
        return false;
    }
}

bool isAccessibleDungeon(std::vector<std::string> OwnedDungeonItems, nlohmann::json expression) {

    std::vector<std::string> OwnedItems;
    OwnedItems.assign(ProgressItems.begin(), ProgressItems.end());

    std::string item_typ = expression["type"];
    auto& item_args = expression["args"];
    if (item_typ == "or") {
        return std::any_of(item_args.begin(), item_args.end(), [&](auto arg) {return isAccessible(OwnedItems, arg); });
    }
    else if (item_typ == "and") {
        return std::all_of(item_args.begin(), item_args.end(), [&](auto arg) {return isAccessible(OwnedItems, arg); });
    }
    else if (item_typ == "count") {
        std::string count = item_args[0]["count"];
        std::string itemtocount = item_args[0]["args"]; //This exists entirely for the find function to work, otherwise it just gets mad and says you cant use find on a json thing
        if (itemtocount.find("SmallKey") != std::string::npos) {
            return std::count(OwnedDungeonItems.begin(), OwnedDungeonItems.end(), item_args[0]["args"]) >= stoi(count, nullptr);
        }
        else {
            return std::count(OwnedItems.begin(), OwnedItems.end(), item_args[0]["args"]) >= stoi(count, nullptr);
        }
    }
    else if (item_typ == "has_item") {
        std::string itemtocheck = item_args[0]; //This exists entirely for the find function to work, otherwise it just gets mad and says you cant use find on a json thing
        if (itemtocheck.find("SmallKey") != std::string::npos || itemtocheck.find("BigKey") != std::string::npos) {
            return has_item(OwnedDungeonItems, item_args[0]);
        }
        else if (item_args[0] == "Nothing") return true;
        else {
            return has_item(OwnedItems, item_args[0]);
        }
    }
    else {
        printf("unknown type!");
        return false;
    }
}

LocationLists PlaceDungeonItems(LocationLists locations, std::unordered_set<std::string> settings, int NumRaceModeDungeons) {

    std::vector<std::string> Dungeons = {"DRC", "FW", "TOTG", "FF", "ET", "WT"};
    std::vector<std::string> RaceModeDungeons;

    for (int a = 0; a < NumRaceModeDungeons; a++) {
        std::shuffle(Dungeons.begin(), Dungeons.end(), std::default_random_engine(1234567)); //Replace with random seed later
        RaceModeDungeons.push_back(Dungeons[0]);
        Dungeons.erase(Dungeons.begin());
    }

    std::vector<Location> DRCLocations;
    std::vector<Location> FWLocations;
    std::vector<Location> TOTGLocations;
    std::vector<Location> FFLocations;
    std::vector<Location> ETLocations;
    std::vector<Location> WTLocations;

    //The "unplaced" vectors are used to store dungeon-specific unplaced locations for nonprogress item placement
    std::vector<Location> UnplacedDRCLocations;
    std::vector<Location> UnplacedFWLocations;
    std::vector<Location> UnplacedTOTGLocations;
    std::vector<Location> UnplacedFFLocations;
    std::vector<Location> UnplacedETLocations;
    std::vector<Location> UnplacedWTLocations;

    //Check if dungeons are in logic
    //This is used to determine which list to pull from
    if (settings.count("Dungeon") > 0) {
        for (unsigned int i = 0; i < locations.ProgressLocations.size(); i++) {
            if (locations.ProgressLocations[i].Name.find("Dragon Roost Cavern") != std::string::npos && locations.ProgressLocations[i].Name.find("Dragon Roost Cavern")) {
                DRCLocations.push_back(locations.ProgressLocations[i]);
                locations.ProgressLocations.erase(locations.ProgressLocations.begin() + i);
                i = i - 1;
            }
            else if (locations.ProgressLocations[i].Name.find("Forbidden Woods") != std::string::npos) {
                FWLocations.push_back(locations.ProgressLocations[i]);
                locations.ProgressLocations.erase(locations.ProgressLocations.begin() + i);
                i = i - 1;
            }
            else if (locations.ProgressLocations[i].Name.find("Tower of the Gods") != std::string::npos) {
                TOTGLocations.push_back(locations.ProgressLocations[i]);
                locations.ProgressLocations.erase(locations.ProgressLocations.begin() + i);
                i = i - 1;
            }
            else if (locations.ProgressLocations[i].Name.find("Forsaken Fortress") != std::string::npos) {
                FFLocations.push_back(locations.ProgressLocations[i]);
                locations.ProgressLocations.erase(locations.ProgressLocations.begin() + i);
                i = i - 1;
            }
            else if (locations.ProgressLocations[i].Name.find("Earth Temple") != std::string::npos) {
                ETLocations.push_back(locations.ProgressLocations[i]);
                locations.ProgressLocations.erase(locations.ProgressLocations.begin() + i);
                i = i - 1;
            }
            else if (locations.ProgressLocations[i].Name.find("Wind Temple") != std::string::npos) {
                WTLocations.push_back(locations.ProgressLocations[i]);
                locations.ProgressLocations.erase(locations.ProgressLocations.begin() + i);
                i = i - 1;
            }
        }
    }
    else {
        for (unsigned int i = 0; i < locations.NonprogressLocations.size(); i++) {
            if (locations.NonprogressLocations[i].Name.find("Dragon Roost Cavern") != std::string::npos) {
                DRCLocations.push_back(locations.NonprogressLocations[i]);
                locations.NonprogressLocations.erase(locations.NonprogressLocations.begin() + i);
                i = i - 1;
            }
            else if (locations.NonprogressLocations[i].Name.find("Forbidden Woods") != std::string::npos) {
                FWLocations.push_back(locations.NonprogressLocations[i]);
                locations.NonprogressLocations.erase(locations.NonprogressLocations.begin() + i);
                i = i - 1;
            }
            else if (locations.NonprogressLocations[i].Name.find("Tower of the Gods") != std::string::npos) {
                TOTGLocations.push_back(locations.NonprogressLocations[i]);
                locations.NonprogressLocations.erase(locations.NonprogressLocations.begin() + i);
                i = i - 1;
            }
            else if (locations.NonprogressLocations[i].Name.find("Forsaken Fortress") != std::string::npos) {
                FFLocations.push_back(locations.NonprogressLocations[i]);
                locations.NonprogressLocations.erase(locations.NonprogressLocations.begin() + i);
                i = i - 1;
            }
            else if (locations.NonprogressLocations[i].Name.find("Earth Temple") != std::string::npos) {
                ETLocations.push_back(locations.NonprogressLocations[i]);
                locations.NonprogressLocations.erase(locations.NonprogressLocations.begin() + i);
                i = i - 1;
            }
            else if (locations.NonprogressLocations[i].Name.find("Wind Temple") != std::string::npos) {
                WTLocations.push_back(locations.NonprogressLocations[i]);
                locations.NonprogressLocations.erase(locations.NonprogressLocations.begin() + i);
                i = i - 1;
            }
        }
    }

    std::vector<Location> AccessibleLocations;

    //Place DRC Items
  
    if (settings.count("RaceMode") > 0 && std::find(RaceModeDungeons.begin(), RaceModeDungeons.end(), "DRC") != RaceModeDungeons.end() && settings.count("Dungeon") > 0) {
        std::string RaceModeItemToPlace;
        bool item_placed = false;
        if (std::find(ProgressItems.begin(), ProgressItems.end(), "Shard") != ProgressItems.end()) {
            int firstshard = std::distance(ProgressItems.begin(), std::find(ProgressItems.begin(), ProgressItems.end(), "Shard"));
            RaceModeItemToPlace = "Shard";
            ProgressItems.erase(ProgressItems.begin() + firstshard);
            item_placed = true;
        }
        else if (std::find(ProgressItems.begin(), ProgressItems.end(), "Sword") != ProgressItems.end() && item_placed == false) {
            int firstsword = std::distance(ProgressItems.begin(), std::find(ProgressItems.begin(), ProgressItems.end(), "Sword"));
            ProgressItems.erase(ProgressItems.begin() + firstsword);
            if (isAccessible(ProgressItems, DRCLocations.back().Needs) != 1) {
                ProgressItems.push_back("Sword");
                item_placed = false;
            }
            else {
                RaceModeItemToPlace = "Sword";
                item_placed = true;
            }
        }
        else if (std::find(ProgressItems.begin(), ProgressItems.end(), "Bow") != ProgressItems.end() && item_placed == false) {
            int firstbow = std::distance(ProgressItems.begin(), std::find(ProgressItems.begin(), ProgressItems.end(), "Bow"));
            ProgressItems.erase(ProgressItems.begin() + firstbow);
            if (isAccessible(ProgressItems, DRCLocations.back().Needs) != 1) {
                ProgressItems.push_back("Bow");
                item_placed = false;
            }
            else {
                RaceModeItemToPlace = "Bow";
                item_placed = true;
            }
        }
        if (item_placed == true) {
            DRCLocations.back().Item = RaceModeItemToPlace;
            locations.PlacedLocations.push_back(DRCLocations.back());
            DRCLocations.pop_back();
        }
        else {
            std::cout << "Not enough items to fill boss location! Replacing with a nonprogress item" << std::endl;
            locations.NonprogressLocations.push_back(DRCLocations.back());
        }
    }
    
    AccessibleLocations.assign(DRCLocations.begin(), DRCLocations.end());
    for (unsigned int x = 0; DRCProgressItems.size() > 0; x++) {
        std::shuffle(DRCProgressItems.begin(), DRCProgressItems.end(), std::default_random_engine(123456));
        std::string itemtoplace = DRCProgressItems[0];
        //std::cout << "removed " << itemtoplace << std::endl;
        DRCProgressItems.erase(DRCProgressItems.begin());
        for (unsigned int y = 0; y < AccessibleLocations.size(); y++) {
            if (isAccessibleDungeon(DRCProgressItems, AccessibleLocations[y].Needs) == false) {
                UnplacedDRCLocations.push_back(AccessibleLocations[y]);
                AccessibleLocations.erase(AccessibleLocations.begin() + y);
                y = y - 1;
            }

        }
        std::shuffle(AccessibleLocations.begin(), AccessibleLocations.end(), std::default_random_engine(1234567)); //Change static seed to something random in all instances, this is just for testing
        int seed = 1234567;
        while (std::count(DRCProgressItems.begin(), DRCProgressItems.end(), "SmallKey") > 0 && AccessibleLocations[0].Name == "Dragon Roost Cavern - First Room") { //This is to make sure only the last small key is placed in the first room to avoid failed placements
            seed = seed + 1;
            std::shuffle(AccessibleLocations.begin(), AccessibleLocations.end(), std::default_random_engine(seed));
        }
        AccessibleLocations[0].Item = itemtoplace;
        std::cout << AccessibleLocations[0].Name << " contains " << AccessibleLocations[0].Item << std::endl;
        locations.PlacedLocations.push_back(AccessibleLocations[0]);
        AccessibleLocations.erase(AccessibleLocations.begin());
    }

    //Place DRC Nonprogress items
    std::shuffle(UnplacedDRCLocations.begin(), UnplacedDRCLocations.end(), std::default_random_engine(1234567));
    std::shuffle(DRCNonProgressItems.begin(), DRCNonProgressItems.end(), std::default_random_engine(1234567));
    UnplacedDRCLocations[0].Item = DRCNonProgressItems[0];
    std::cout << UnplacedDRCLocations[0].Name << " contains " << UnplacedDRCLocations[0].Item << std::endl;
    locations.PlacedLocations.push_back(UnplacedDRCLocations[0]);
    UnplacedDRCLocations.erase(UnplacedDRCLocations.begin());
    DRCNonProgressItems.erase(DRCNonProgressItems.begin());

    std::shuffle(UnplacedDRCLocations.begin(), UnplacedDRCLocations.end(), std::default_random_engine(1234567));
    UnplacedDRCLocations[0].Item = DRCNonProgressItems[0];
    std::cout << UnplacedDRCLocations[0].Name << " contains " << UnplacedDRCLocations[0].Item << std::endl;
    locations.PlacedLocations.push_back(UnplacedDRCLocations[0]);
    UnplacedDRCLocations.erase(UnplacedDRCLocations.begin());
    DRCNonProgressItems.erase(DRCNonProgressItems.begin());

    std::cout << std::endl;

    for (unsigned int z = 0; z < UnplacedDRCLocations.size(); z++) {
        if ((settings.count("RaceMode") == 0 || std::find(RaceModeDungeons.begin(), RaceModeDungeons.end(), "DRC") != RaceModeDungeons.end()) && settings.count("Dungeon") > 0) {
            locations.UnplacedLocations.push_back(UnplacedDRCLocations[z]);
        }
        else {
            locations.NonprogressLocations.push_back(UnplacedDRCLocations[z]);
        }
        std::cout << locations.UnplacedLocations[z].Name << std::endl;
    }

    //End of DRC Placement
    
    //FW Placement

    if (settings.count("RaceMode") > 0 && std::find(RaceModeDungeons.begin(), RaceModeDungeons.end(), "FW") != RaceModeDungeons.end() && settings.count("Dungeon") > 0) {
        std::string RaceModeItemToPlace;
        bool item_placed = false;
        if (std::find(ProgressItems.begin(), ProgressItems.end(), "Shard") != ProgressItems.end()) {
            int firstshard = std::distance(ProgressItems.begin(), std::find(ProgressItems.begin(), ProgressItems.end(), "Shard"));
            RaceModeItemToPlace = "Shard";
            ProgressItems.erase(ProgressItems.begin() + firstshard);
            item_placed = true;
        }
        else if (std::find(ProgressItems.begin(), ProgressItems.end(), "Sword") != ProgressItems.end() && item_placed == false) {
            int firstsword = std::distance(ProgressItems.begin(), std::find(ProgressItems.begin(), ProgressItems.end(), "Sword"));
            ProgressItems.erase(ProgressItems.begin() + firstsword);
            if (isAccessible(ProgressItems, FWLocations.back().Needs) != 1) {
                ProgressItems.push_back("Sword");
                item_placed = false;
            }
            else {
                RaceModeItemToPlace = "Sword";
                item_placed = true;
            }
        }
        else if (std::find(ProgressItems.begin(), ProgressItems.end(), "Bow") != ProgressItems.end() && item_placed == false) {
            int firstbow = std::distance(ProgressItems.begin(), std::find(ProgressItems.begin(), ProgressItems.end(), "Bow"));
            ProgressItems.erase(ProgressItems.begin() + firstbow);
            if (isAccessible(ProgressItems, FWLocations.back().Needs) != 1) {
                ProgressItems.push_back("Bow");
                item_placed = false;
            }
            else {
                RaceModeItemToPlace = "Bow";
                item_placed = true;
            }
        }
        if (item_placed == true) {
            FWLocations.back().Item = RaceModeItemToPlace;
            locations.PlacedLocations.push_back(FWLocations.back());
            FWLocations.pop_back();
        }
        else {
            std::cout << "Not enough items to fill boss location! Replacing with a nonprogress item" << std::endl;
            locations.NonprogressLocations.push_back(FWLocations.back());
        }
    }

    AccessibleLocations.assign(FWLocations.begin(), FWLocations.end());
    for (unsigned int x = 0; FWProgressItems.size() > 0; x++) {
        std::shuffle(FWProgressItems.begin(), FWProgressItems.end(), std::default_random_engine(123456));
        std::string itemtoplace = FWProgressItems[0];
        //std::cout << "removed " << itemtoplace << std::endl;
        FWProgressItems.erase(FWProgressItems.begin());
        for (unsigned int y = 0; y < AccessibleLocations.size(); y++) {
            if (isAccessibleDungeon(FWProgressItems, AccessibleLocations[y].Needs) == false) {
                UnplacedFWLocations.push_back(AccessibleLocations[y]);
                AccessibleLocations.erase(AccessibleLocations.begin() + y);
                y = y - 1;
            }

        }
        std::shuffle(AccessibleLocations.begin(), AccessibleLocations.end(), std::default_random_engine(1234567)); //Change static seed to something random in all instances, this is just for testing
        AccessibleLocations[0].Item = itemtoplace;
        std::cout << AccessibleLocations[0].Name << " contains " << AccessibleLocations[0].Item << std::endl;
        locations.PlacedLocations.push_back(AccessibleLocations[0]);
        AccessibleLocations.erase(AccessibleLocations.begin());
    }

    //Place FW Nonprogress items
    std::shuffle(UnplacedFWLocations.begin(), UnplacedFWLocations.end(), std::default_random_engine(1234567));
    std::shuffle(FWNonProgressItems.begin(), FWNonProgressItems.end(), std::default_random_engine(1234567));
    UnplacedFWLocations[0].Item = FWNonProgressItems[0];
    std::cout << UnplacedFWLocations[0].Name << " contains " << UnplacedFWLocations[0].Item << std::endl;
    locations.PlacedLocations.push_back(UnplacedFWLocations[0]);
    UnplacedFWLocations.erase(UnplacedFWLocations.begin());
    FWNonProgressItems.erase(FWNonProgressItems.begin());

    std::shuffle(UnplacedFWLocations.begin(), UnplacedFWLocations.end(), std::default_random_engine(1234567));
    UnplacedFWLocations[0].Item = FWNonProgressItems[0];
    std::cout << UnplacedFWLocations[0].Name << " contains " << UnplacedFWLocations[0].Item << std::endl;
    locations.PlacedLocations.push_back(UnplacedFWLocations[0]);
    UnplacedFWLocations.erase(UnplacedFWLocations.begin());
    FWNonProgressItems.erase(FWNonProgressItems.begin());

    std::cout << std::endl;

    for (unsigned int z = 0; z < UnplacedFWLocations.size(); z++) {
        if ((settings.count("RaceMode") == 0 || std::find(RaceModeDungeons.begin(), RaceModeDungeons.end(), "FW") != RaceModeDungeons.end()) && settings.count("Dungeon") > 0) {
            locations.UnplacedLocations.push_back(UnplacedFWLocations[z]);
        }
        else {
            locations.NonprogressLocations.push_back(UnplacedFWLocations[z]);
        }
        std::cout << locations.UnplacedLocations[z].Name << std::endl;
    }

    //End of FW Placement

    //TotG Placement

    if (settings.count("RaceMode") > 0 && std::find(RaceModeDungeons.begin(), RaceModeDungeons.end(), "TOTG") != RaceModeDungeons.end() && settings.count("Dungeon") > 0) {
        std::string RaceModeItemToPlace;
        bool item_placed = false;
        if (std::find(ProgressItems.begin(), ProgressItems.end(), "Shard") != ProgressItems.end()) {
            int firstshard = std::distance(ProgressItems.begin(), std::find(ProgressItems.begin(), ProgressItems.end(), "Shard"));
            RaceModeItemToPlace = "Shard";
            ProgressItems.erase(ProgressItems.begin() + firstshard);
            item_placed = true;
        }
        else if (std::find(ProgressItems.begin(), ProgressItems.end(), "Sword") != ProgressItems.end() && item_placed == false) {
            int firstsword = std::distance(ProgressItems.begin(), std::find(ProgressItems.begin(), ProgressItems.end(), "Sword"));
            ProgressItems.erase(ProgressItems.begin() + firstsword);
            if (isAccessible(ProgressItems, TOTGLocations.back().Needs) != 1) {
                ProgressItems.push_back("Sword");
                item_placed = false;
            }
            else {
                RaceModeItemToPlace = "Sword";
                item_placed = true;
            }
        }
        else if (std::find(ProgressItems.begin(), ProgressItems.end(), "Bow") != ProgressItems.end() && item_placed == false) {
            int firstbow = std::distance(ProgressItems.begin(), std::find(ProgressItems.begin(), ProgressItems.end(), "Bow"));
            ProgressItems.erase(ProgressItems.begin() + firstbow);
            if (isAccessible(ProgressItems, TOTGLocations.back().Needs) != 1) {
                ProgressItems.push_back("Bow");
                item_placed = false;
            }
            else {
                RaceModeItemToPlace = "Bow";
                item_placed = true;
            }
        }
        if (item_placed == true) {
            TOTGLocations.back().Item = RaceModeItemToPlace;
            locations.PlacedLocations.push_back(TOTGLocations.back());
            TOTGLocations.pop_back();
        }
        else {
            std::cout << "Not enough items to fill boss location! Replacing with a nonprogress item" << std::endl;
            locations.NonprogressLocations.push_back(TOTGLocations.back());
        }
    }

    AccessibleLocations.assign(TOTGLocations.begin(), TOTGLocations.end());
    for (unsigned int x = 0; TOTGProgressItems.size() > 0; x++) {
        std::shuffle(TOTGProgressItems.begin(), TOTGProgressItems.end(), std::default_random_engine(123456));
        std::string itemtoplace = TOTGProgressItems[0];
        //std::cout << "removed " << itemtoplace << std::endl;
        TOTGProgressItems.erase(TOTGProgressItems.begin());
        for (unsigned int y = 0; y < AccessibleLocations.size(); y++) {
            if (isAccessibleDungeon(TOTGProgressItems, AccessibleLocations[y].Needs) == false) {
                UnplacedTOTGLocations.push_back(AccessibleLocations[y]);
                AccessibleLocations.erase(AccessibleLocations.begin() + y);
                y = y - 1;
            }

        }
        std::shuffle(AccessibleLocations.begin(), AccessibleLocations.end(), std::default_random_engine(1234567)); //Change static seed to something random in all instances, this is just for testing
        AccessibleLocations[0].Item = itemtoplace;
        std::cout << AccessibleLocations[0].Name << " contains " << AccessibleLocations[0].Item << std::endl;
        locations.PlacedLocations.push_back(AccessibleLocations[0]);
        AccessibleLocations.erase(AccessibleLocations.begin());
    }

    //Place TotG Nonprogress items
    std::shuffle(UnplacedTOTGLocations.begin(), UnplacedTOTGLocations.end(), std::default_random_engine(1234567));
    std::shuffle(TOTGNonProgressItems.begin(), TOTGNonProgressItems.end(), std::default_random_engine(1234567));
    UnplacedTOTGLocations[0].Item = TOTGNonProgressItems[0];
    std::cout << UnplacedTOTGLocations[0].Name << " contains " << UnplacedTOTGLocations[0].Item << std::endl;
    locations.PlacedLocations.push_back(UnplacedTOTGLocations[0]);
    UnplacedTOTGLocations.erase(UnplacedTOTGLocations.begin());
    TOTGNonProgressItems.erase(TOTGNonProgressItems.begin());

    std::shuffle(UnplacedTOTGLocations.begin(), UnplacedTOTGLocations.end(), std::default_random_engine(1234567));
    UnplacedTOTGLocations[0].Item = TOTGNonProgressItems[0];
    std::cout << UnplacedTOTGLocations[0].Name << " contains " << UnplacedTOTGLocations[0].Item << std::endl;
    locations.PlacedLocations.push_back(UnplacedTOTGLocations[0]);
    UnplacedTOTGLocations.erase(UnplacedTOTGLocations.begin());
    TOTGNonProgressItems.erase(TOTGNonProgressItems.begin());

    std::cout << std::endl;

    for (unsigned int z = 0; z < UnplacedTOTGLocations.size(); z++) {
        if ((settings.count("RaceMode") == 0 || std::find(RaceModeDungeons.begin(), RaceModeDungeons.end(), "TOTG") != RaceModeDungeons.end()) && settings.count("Dungeon") > 0) {
            locations.UnplacedLocations.push_back(UnplacedTOTGLocations[z]);
        }
        else {
            locations.NonprogressLocations.push_back(UnplacedTOTGLocations[z]);
        }
        std::cout << locations.UnplacedLocations[z].Name << std::endl;
    }

    //End of TotG Placement

    //FF Placement (No small keys or big key)

    if (settings.count("RaceMode") > 0 && std::find(RaceModeDungeons.begin(), RaceModeDungeons.end(), "FF") != RaceModeDungeons.end() && settings.count("Dungeon") > 0) {
        std::string RaceModeItemToPlace;
        bool item_placed = false;
        if (std::find(ProgressItems.begin(), ProgressItems.end(), "Shard") != ProgressItems.end()) {
            int firstshard = std::distance(ProgressItems.begin(), std::find(ProgressItems.begin(), ProgressItems.end(), "Shard"));
            RaceModeItemToPlace = "Shard";
            ProgressItems.erase(ProgressItems.begin() + firstshard);
            item_placed = true;
        }
        else if (std::find(ProgressItems.begin(), ProgressItems.end(), "Sword") != ProgressItems.end() && item_placed == false) {
            int firstsword = std::distance(ProgressItems.begin(), std::find(ProgressItems.begin(), ProgressItems.end(), "Sword"));
            ProgressItems.erase(ProgressItems.begin() + firstsword);
            if (isAccessible(ProgressItems, FFLocations.back().Needs) != 1) {
                ProgressItems.push_back("Sword");
                item_placed = false;
            }
            else {
                RaceModeItemToPlace = "Sword";
                item_placed = true;
            }
        }
        else if (std::find(ProgressItems.begin(), ProgressItems.end(), "Bow") != ProgressItems.end() && item_placed == false) {
            int firstbow = std::distance(ProgressItems.begin(), std::find(ProgressItems.begin(), ProgressItems.end(), "Bow"));
            ProgressItems.erase(ProgressItems.begin() + firstbow);
            if (isAccessible(ProgressItems, FFLocations.back().Needs) != 1) {
                ProgressItems.push_back("Bow");
                item_placed = false;
            }
            else {
                RaceModeItemToPlace = "Bow";
                item_placed = true;
            }
        }
        if (item_placed == true) {
            FFLocations.back().Item = RaceModeItemToPlace;
            locations.PlacedLocations.push_back(FFLocations.back());
            FFLocations.pop_back();
        }
        else {
            std::cout << "Not enough items to fill boss location! Replacing with a nonprogress item" << std::endl;
            locations.NonprogressLocations.push_back(FFLocations.back());
        }
    }

    UnplacedFFLocations.assign(FFLocations.begin(), FFLocations.end());

    //Place FF Nonprogress items
    std::shuffle(UnplacedFFLocations.begin(), UnplacedFFLocations.end(), std::default_random_engine(1234567));
    std::shuffle(FFNonProgressItems.begin(), FFNonProgressItems.end(), std::default_random_engine(1234567));
    UnplacedFFLocations[0].Item = FFNonProgressItems[0];
    std::cout << UnplacedFFLocations[0].Name << " contains " << UnplacedFFLocations[0].Item << std::endl;
    locations.PlacedLocations.push_back(UnplacedFFLocations[0]);
    UnplacedFFLocations.erase(UnplacedFFLocations.begin());
    FFNonProgressItems.erase(FFNonProgressItems.begin());

    std::shuffle(UnplacedFFLocations.begin(), UnplacedFFLocations.end(), std::default_random_engine(1234567));
    UnplacedFFLocations[0].Item = FFNonProgressItems[0];
    std::cout << UnplacedFFLocations[0].Name << " contains " << UnplacedFFLocations[0].Item << std::endl;
    locations.PlacedLocations.push_back(UnplacedFFLocations[0]);
    UnplacedFFLocations.erase(UnplacedFFLocations.begin());
    FFNonProgressItems.erase(FFNonProgressItems.begin());

    std::cout << std::endl;

    for (unsigned int z = 0; z < UnplacedFFLocations.size(); z++) {
        if ((settings.count("RaceMode") == 0 || std::find(RaceModeDungeons.begin(), RaceModeDungeons.end(), "FF") != RaceModeDungeons.end()) && settings.count("Dungeon") > 0) {
            locations.UnplacedLocations.push_back(UnplacedFFLocations[z]);
        }
        else {
            locations.NonprogressLocations.push_back(UnplacedFFLocations[z]);
        }
        std::cout << locations.UnplacedLocations[z].Name << std::endl;
    }

    //End of FF Placement

    //ET Placement

    if (settings.count("RaceMode") > 0 && std::find(RaceModeDungeons.begin(), RaceModeDungeons.end(), "ET") != RaceModeDungeons.end() && settings.count("Dungeon") > 0) {
        std::string RaceModeItemToPlace;
        bool item_placed = false;
        if (std::find(ProgressItems.begin(), ProgressItems.end(), "Shard") != ProgressItems.end()) {
            int firstshard = std::distance(ProgressItems.begin(), std::find(ProgressItems.begin(), ProgressItems.end(), "Shard"));
            RaceModeItemToPlace = "Shard";
            ProgressItems.erase(ProgressItems.begin() + firstshard);
            item_placed = true;
        }
        else if (std::find(ProgressItems.begin(), ProgressItems.end(), "Sword") != ProgressItems.end() && item_placed == false) {
            int firstsword = std::distance(ProgressItems.begin(), std::find(ProgressItems.begin(), ProgressItems.end(), "Sword"));
            ProgressItems.erase(ProgressItems.begin() + firstsword);
            if (isAccessible(ProgressItems, ETLocations.back().Needs) != 1) {
                ProgressItems.push_back("Sword");
                item_placed = false;
            }
            else {
                RaceModeItemToPlace = "Sword";
                item_placed = true;
            }
        }
        else if (std::find(ProgressItems.begin(), ProgressItems.end(), "Bow") != ProgressItems.end() && item_placed == false) {
            int firstbow = std::distance(ProgressItems.begin(), std::find(ProgressItems.begin(), ProgressItems.end(), "Bow"));
            ProgressItems.erase(ProgressItems.begin() + firstbow);
            if (isAccessible(ProgressItems, ETLocations.back().Needs) != 1) {
                ProgressItems.push_back("Bow");
                item_placed = false;
            }
            else {
                RaceModeItemToPlace = "Bow";
                item_placed = true;
            }
        }
        if (item_placed == true) {
            ETLocations.back().Item = RaceModeItemToPlace;
            locations.PlacedLocations.push_back(ETLocations.back());
            ETLocations.pop_back();
        }
        else {
            std::cout << "Not enough items to fill boss location! Replacing with a nonprogress item" << std::endl;
            locations.NonprogressLocations.push_back(ETLocations.back());
        }
    }

    AccessibleLocations.assign(ETLocations.begin(), ETLocations.end());
    for (unsigned int x = 0; ETProgressItems.size() > 0; x++) {
        std::shuffle(ETProgressItems.begin(), ETProgressItems.end(), std::default_random_engine(123456));
        std::string itemtoplace = ETProgressItems[0];
        //std::cout << "removed " << itemtoplace << std::endl;
        ETProgressItems.erase(ETProgressItems.begin());
        for (unsigned int y = 0; y < AccessibleLocations.size(); y++) {
            if (isAccessibleDungeon(ETProgressItems, AccessibleLocations[y].Needs) == false) {
                UnplacedETLocations.push_back(AccessibleLocations[y]);
                AccessibleLocations.erase(AccessibleLocations.begin() + y);
                y = y - 1;
            }

        }
        std::shuffle(AccessibleLocations.begin(), AccessibleLocations.end(), std::default_random_engine(1234567)); //Change static seed to something random in all instances, this is just for testing
        AccessibleLocations[0].Item = itemtoplace;
        std::cout << AccessibleLocations[0].Name << " contains " << AccessibleLocations[0].Item << std::endl;
        locations.PlacedLocations.push_back(AccessibleLocations[0]);
        AccessibleLocations.erase(AccessibleLocations.begin());
    }

    //Place ET Nonprogress items
    std::shuffle(UnplacedETLocations.begin(), UnplacedETLocations.end(), std::default_random_engine(1234567));
    std::shuffle(ETNonProgressItems.begin(), ETNonProgressItems.end(), std::default_random_engine(1234567));
    UnplacedETLocations[0].Item = ETNonProgressItems[0];
    std::cout << UnplacedETLocations[0].Name << " contains " << UnplacedETLocations[0].Item << std::endl;
    locations.PlacedLocations.push_back(UnplacedETLocations[0]);
    UnplacedETLocations.erase(UnplacedETLocations.begin());
    ETNonProgressItems.erase(ETNonProgressItems.begin());

    std::shuffle(UnplacedETLocations.begin(), UnplacedETLocations.end(), std::default_random_engine(1234567));
    UnplacedETLocations[0].Item = ETNonProgressItems[0];
    std::cout << UnplacedETLocations[0].Name << " contains " << UnplacedETLocations[0].Item << std::endl;
    locations.PlacedLocations.push_back(UnplacedETLocations[0]);
    UnplacedETLocations.erase(UnplacedETLocations.begin());
    ETNonProgressItems.erase(ETNonProgressItems.begin());

    std::cout << std::endl;

    for (unsigned int z = 0; z < UnplacedETLocations.size(); z++) {
        if ((settings.count("RaceMode") == 0 || std::find(RaceModeDungeons.begin(), RaceModeDungeons.end(), "ET") != RaceModeDungeons.end()) && settings.count("Dungeon") > 0) {
            locations.UnplacedLocations.push_back(UnplacedETLocations[z]);
        }
        else {
            locations.NonprogressLocations.push_back(UnplacedETLocations[z]);
        }
        std::cout << locations.UnplacedLocations[z].Name << std::endl;
    }

    //End of ET Placement

    //WT Placement

    if (settings.count("RaceMode") > 0 && std::find(RaceModeDungeons.begin(), RaceModeDungeons.end(), "WT") != RaceModeDungeons.end() && settings.count("Dungeon") > 0) {
        std::string RaceModeItemToPlace;
        bool item_placed = false;
        if (std::find(ProgressItems.begin(), ProgressItems.end(), "Shard") != ProgressItems.end()) {
            int firstshard = std::distance(ProgressItems.begin(), std::find(ProgressItems.begin(), ProgressItems.end(), "Shard"));
            RaceModeItemToPlace = "Shard";
            ProgressItems.erase(ProgressItems.begin() + firstshard);
            item_placed = true;
        }
        else if (std::find(ProgressItems.begin(), ProgressItems.end(), "Sword") != ProgressItems.end() && item_placed == false) {
            int firstsword = std::distance(ProgressItems.begin(), std::find(ProgressItems.begin(), ProgressItems.end(), "Sword"));
            ProgressItems.erase(ProgressItems.begin() + firstsword);
            if (isAccessible(ProgressItems, WTLocations.back().Needs) != 1) {
                ProgressItems.push_back("Sword");
                item_placed = false;
            }
            else {
                RaceModeItemToPlace = "Sword";
                item_placed = true;
            }
        }
        else if (std::find(ProgressItems.begin(), ProgressItems.end(), "Bow") != ProgressItems.end() && item_placed == false) {
            int firstbow = std::distance(ProgressItems.begin(), std::find(ProgressItems.begin(), ProgressItems.end(), "Bow"));
            ProgressItems.erase(ProgressItems.begin() + firstbow);
            if (isAccessible(ProgressItems, WTLocations.back().Needs) != 1) {
                ProgressItems.push_back("Bow");
                item_placed = false;
            }
            else {
                RaceModeItemToPlace = "Bow";
                item_placed = true;
            }
        }
        if (item_placed == true) {
            WTLocations.back().Item = RaceModeItemToPlace;
            locations.PlacedLocations.push_back(WTLocations.back());
            WTLocations.pop_back();
        }
        else {
            std::cout << "Not enough items to fill boss location! Replacing with a nonprogress item" << std::endl;
            locations.NonprogressLocations.push_back(WTLocations.back());
        }
    }

    AccessibleLocations.assign(WTLocations.begin(), WTLocations.end());
    for (unsigned int x = 0; WTProgressItems.size() > 0; x++) {
        std::shuffle(WTProgressItems.begin(), WTProgressItems.end(), std::default_random_engine(123456));
        std::string itemtoplace = WTProgressItems[0];
        //std::cout << "removed " << itemtoplace << std::endl;
        WTProgressItems.erase(WTProgressItems.begin());
        for (unsigned int y = 0; y < AccessibleLocations.size(); y++) {
            if (isAccessibleDungeon(WTProgressItems, AccessibleLocations[y].Needs) == false) {
                UnplacedWTLocations.push_back(AccessibleLocations[y]);
                AccessibleLocations.erase(AccessibleLocations.begin() + y);
                y = y - 1;
            }

        }
        std::shuffle(AccessibleLocations.begin(), AccessibleLocations.end(), std::default_random_engine(1234567)); //Change static seed to something random in all instances, this is just for testing
        AccessibleLocations[0].Item = itemtoplace;
        std::cout << AccessibleLocations[0].Name << " contains " << AccessibleLocations[0].Item << std::endl;
        locations.PlacedLocations.push_back(AccessibleLocations[0]);
        AccessibleLocations.erase(AccessibleLocations.begin());
    }

    //Place WT Nonprogress items
    std::shuffle(UnplacedWTLocations.begin(), UnplacedWTLocations.end(), std::default_random_engine(1234567));
    std::shuffle(WTNonProgressItems.begin(), WTNonProgressItems.end(), std::default_random_engine(1234567));
    UnplacedWTLocations[0].Item = WTNonProgressItems[0];
    std::cout << UnplacedWTLocations[0].Name << " contains " << UnplacedWTLocations[0].Item << std::endl;
    locations.PlacedLocations.push_back(UnplacedWTLocations[0]);
    UnplacedWTLocations.erase(UnplacedWTLocations.begin());
    WTNonProgressItems.erase(WTNonProgressItems.begin());

    std::shuffle(UnplacedWTLocations.begin(), UnplacedWTLocations.end(), std::default_random_engine(1234567));
    UnplacedWTLocations[0].Item = WTNonProgressItems[0];
    std::cout << UnplacedWTLocations[0].Name << " contains " << UnplacedWTLocations[0].Item << std::endl;
    locations.PlacedLocations.push_back(UnplacedWTLocations[0]);
    UnplacedWTLocations.erase(UnplacedWTLocations.begin());
    WTNonProgressItems.erase(WTNonProgressItems.begin());

    std::cout << std::endl;

    for (unsigned int z = 0; z < UnplacedWTLocations.size(); z++) {
        if ((settings.count("RaceMode") == 0 || std::find(RaceModeDungeons.begin(), RaceModeDungeons.end(), "WT") != RaceModeDungeons.end()) && settings.count("Dungeon") > 0) {
            locations.UnplacedLocations.push_back(UnplacedWTLocations[z]);
        }
        else {
            locations.NonprogressLocations.push_back(UnplacedWTLocations[z]);
        }
        std::cout << locations.UnplacedLocations[z].Name << std::endl;
    }

    //End of WT Placement

    return locations;
}

LocationLists AssumedFill(LocationLists locations, std::vector<std::string> StartingItems, std::unordered_set<std::string> settings, int NumRaceModeDungeons) {

    //Do something about triforce shards so they are numbered properly and the item ids dont overlap weirdly
    
    for (int i = 0; i < StartingItems.size(); i++) {
        ProgressItems.erase(ProgressItems.begin() + std::distance(ProgressItems.begin(), std::find(ProgressItems.begin(), ProgressItems.end(), StartingItems[i])));
        PermanentItems.push_back(StartingItems[i]);
    }

    LocationLists Locations = PlaceDungeonItems(locations, settings, NumRaceModeDungeons);

    for (int i = 0; i < Locations.UnplacedLocations.size(); i++) {
        Locations.ProgressLocations.push_back(Locations.UnplacedLocations[i]);
    }
    Locations.UnplacedLocations.clear();
    int i = 0;
    while(i < ProgressItems.size()) {
        std::shuffle(Locations.ProgressLocations.begin(), Locations.ProgressLocations.end(), std::default_random_engine(1234567)); //replace with random seed later
        std::shuffle(ProgressItems.begin(), ProgressItems.end(), std::default_random_engine(1234567)); //replace with random seed later
        std::string itemtoplace = ProgressItems[0];
        ProgressItems.erase(ProgressItems.begin());
        for (int y = 0; y < Locations.ProgressLocations.size(); y++) {
            if (isAccessible(ProgressItems, Locations.ProgressLocations[y].Needs) != 1) {
                Locations.NonprogressLocations.push_back(Locations.ProgressLocations[y]);
                Locations.ProgressLocations.erase(Locations.ProgressLocations.begin() + y);
                y = y - 1;
            }
        }
        std::shuffle(Locations.ProgressLocations.begin(), Locations.ProgressLocations.end(), std::default_random_engine(1234567));
        Locations.ProgressLocations[0].Item = itemtoplace;
        Locations.PlacedLocations.push_back(Locations.ProgressLocations[0]);
        Locations.ProgressLocations.erase(Locations.ProgressLocations.begin());
    }

    for (int i = 0; i < Locations.ProgressLocations.size(); i++) { //Remaining unused progress locations get added to the nonprogress pool
        Locations.NonprogressLocations.push_back(Locations.ProgressLocations[i]);
        Locations.ProgressLocations.erase(Locations.ProgressLocations.begin() + i);
        i = i - 1;
    }

    //Add nonprogress stuff



    return locations;
}

int main() { 

    //This is just some placeholder stuff im using for testing
    //I'll keep it in the code here for now for this reason and as some example stuff ig
    //Most of these comments are notes for things I need to do, and a couple things are explanations so I remember what they are for and don't delete them

    std::unordered_set<std::string> settings = {"Dungeon", "Great Fairy", "Misc", "Spoils Trading", "Tingle Chest"};

    std::vector<Location> Locations = ParseLocations("LocationsSmall.json");
    LocationLists Lists = FindPossibleProgressLocations(Locations, settings);

    int NumRaceModeDungeons = 4;

    std::vector<std::string> StartingItems = {"GrapplingHook", "HerosBow", "Bombs"};
    
    AssumedFill(Lists, StartingItems, settings, NumRaceModeDungeons);

}