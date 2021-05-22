#include <vector>
#include "Structs.hpp"
#include "EditLocation.hpp"
#include <fstream>
#include "Import.hpp"
#include "Parse.hpp"
#include "Location.hpp"

//temp, remove later
#include <iostream>

std::map<std::string, int> ItemId = {
	{"HeartPickup", 0x00},
	{"GreenRupee", 0x01},
	{"BlueRupee", 0x02},
	{"YellowRupee", 0x03},
	{"RedRupee", 0x04},
	{"PurpleRupee", 0x05},
	{"OrangeRupee", 0x06},
	{"HeartPiece", 0x07},
	{"HeartContainer", 0x08},
	{"SmallMagicJar", 0x09},
	{"LargeMagicJar", 0x0A},
	{"FiveBombs", 0x0B},
	{"TenBombs", 0x0C},
	{"TwentyBombs", 0x0D},
	{"ThirtyBombs", 0x0E},
	{"SilverRupee", 0x0F},
	{"TenArrows", 0x10},
	{"TwentyArrows", 0x11},
	{"ThirtyArrows", 0x12},
	{"SmallKey", 0x15},
	{"Fairy", 0x16},
	{"YellowRupeeAlt", 0x1A},
	{"ThreeHearts", 0x1E},
	{"JoyPendant", 0x1F},
	{"Telescope", 0x20},
	{"TingleBottle", 0x21},
	{"WindWaker", 0x22},
	{"PictoBox", 0x23},
	{"SpoilsBag", 0x24},
	{"GrapplingHook", 0x25},
	{"DeluxePicto", 0x26},
	{"HerosBow", 0x27},
	{"PowerBracelets", 0x28},
	{"IronBoots", 0x29},
	{"MagicArmor", 0x2A},
	{"BaitBag", 0x2C},
	{"Boomerang", 0x2D},
	{"Hookshot", 0x2F},
	{"DeliveryBag", 0x30},
	{"Bombs", 0x31},
	{"HerosClothes", 0x32},
	{"SkullHammer", 0x33},
	{"DekuLeaf", 0x34},
	{"FireIceArrows", 0x35},
	{"LightArrows", 0x36},
	{"NewHerosClothes", 0x37},
	{"HerosSword", 0x38},
	{"MS", 0x39},
	{"MSHalf", 0x3A},
	{"HerosShield", 0x3B},
	{"MirrorShield", 0x3C},
	{"HerosSwordRecovered", 0x3D},
	{"MSFull", 0x3E},
	{"HeartPieceAlt", 0x3F},
	{"GossipStone", 0x42},
	{"HerosCharm", 0x43},
	{"SkullNecklace", 0x45},
	{"BabaSeed", 0x46},
	{"GoldenFeather", 0x47},
	{"KnightsCrest", 0x48},
	{"RedJelly", 0x49},
	{"GreenJelly", 0x4A},
	{"BlueJelly", 0x4B},
	{"DungeonMap", 0x4C},
	{"Compass", 0x4D},
	{"BigKey", 0x4E},
	{"EmptyBottle", 0x50},
	{"RedPotion", 0x51},
	{"GreenPotion", 0x52},
	{"BluePotion", 0x53},
	{"HalfSoup", 0x54},
	{"FullSoup", 0x55},
	{"WaterBottle", 0x56},
	{"FairyBottle", 0x57},
	{"Firefly", 0x58},
	{"ForestWater", 0x59},
	//Incomplete, start at 0x61 with shards
};

LocationLists FindPossibleProgressLocations(std::vector<Location> Locations, std::unordered_set<std::string> settings) {

	//Does not include race mode, this is handled in PlaceDungeonItems

	LocationLists list;
	list.ProgressLocations = Locations;

	for (unsigned int i = 0; i < list.ProgressLocations.size(); i++) {
		for (unsigned int x = 0; x < list.ProgressLocations[i].Category.size(); x++) {
			if (settings.count(list.ProgressLocations[i].Category[x]) == 0) {
				list.NonprogressLocations.push_back(list.ProgressLocations[i]);
				list.ProgressLocations.erase(list.ProgressLocations.begin() + i);
				i = i - 1;
				break;
			}
			
		}

	}
	return list;
}

int WriteLocations(LocationLists list, std::fstream fptr) {

	for (unsigned int i = 0; i < list.PlacedLocations.size(); i++) {

		//Add section to convert dungeon-specific keys into generic "small key" items. This is to make the item ids work while simplifying some checks in the fill algorithms

		if (list.PlacedLocations[i].Type == "Chest") {
			for (unsigned int x = 0; x < list.PlacedLocations[i].Offsets.size(); x++) {
				ACTR chest = ReadChest(fptr, std::stoi(list.PlacedLocations[i].Offsets[x], nullptr, 16));
				EditChest(fptr, chest, ItemId[list.PlacedLocations[x].Item], std::stoi(list.PlacedLocations[i].Offsets[x], nullptr, 16));
			}
		}
		else if (list.PlacedLocations[i].Type == "Actor") {
			for (unsigned int x = 0; x < list.PlacedLocations[i].Offsets.size(); x++) {
				ACTR actor = ReadActor(fptr, std::stoi(list.PlacedLocations[i].Offsets[x], nullptr, 16));
				EditActor(fptr, actor, ItemId[list.PlacedLocations[x].Item], std::stoi(list.PlacedLocations[i].Offsets[x], nullptr, 16));
			}
		}
		else if (list.PlacedLocations[i].Type == "SCOB") {
			for (unsigned int x = 0; x < list.PlacedLocations[i].Offsets.size(); x++) {
				SCOB scob = ReadScob(fptr, std::stoi(list.PlacedLocations[i].Offsets[x], nullptr, 16));
				EditScob(fptr, scob, ItemId[list.PlacedLocations[x].Item], std::stoi(list.PlacedLocations[i].Offsets[x], nullptr, 16));
			}
		}
		else if (list.PlacedLocations[i].Type == "REL" || list.PlacedLocations[i].Type == "RPX") {
			for (unsigned int x = 0; x < list.PlacedLocations[i].Offsets.size(); x++) {
				EditRPX(fptr, ItemId[list.PlacedLocations[x].Item], std::stoi(list.PlacedLocations[i].Offsets[x], nullptr, 16));
			}
		}
		else if (list.PlacedLocations[i].Type == "Event") {
			for (unsigned int x = 0; x < list.PlacedLocations[i].Offsets.size(); x++) {
				EditEvent(fptr, ItemId[list.PlacedLocations[x].Item], std::stoi(list.PlacedLocations[i].Offsets[x], nullptr, 16), std::stoi(list.PlacedLocations[i].Extra, nullptr, 16));
			}
		}
		//Need to add some special cases for weird types such as giving Orca 10 KC (Event + RPX data). This could probably be done differently but its the simplest/easiest way I can think of
	}
return 1;
}