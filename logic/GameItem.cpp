
#include "GameItem.hpp"
#include <unordered_map>
#include <array>

GameItem nameToGameItem(const std::string& name)
{
	static std::unordered_map<std::string, GameItem> nameItemMap = {
		{"INVALID", GameItem::INVALID},
		{"GreenRupee", GameItem::GreenRupee},
		{"BlueRupee", GameItem::BlueRupee},
		{"YellowRupee", GameItem::YellowRupee},
		{"RedRupee", GameItem::RedRupee},
		{"PurpleRupee", GameItem::PurpleRupee},
		{"OrangeRupee", GameItem::OrangeRupee},
		{"PieceOfHeart", GameItem::PieceOfHeart},
		{"HeartContainer", GameItem::HeartContainer},
		{"WindWaker", GameItem::WindWaker},
		{"SpoilsBag", GameItem::SpoilsBag},
		{"HurricaneSpin", GameItem::HurricaneSpin},
		{"GrapplingHook", GameItem::GrapplingHook},
		{"PowerBracelets", GameItem::PowerBracelets},
		{"IronBoots", GameItem::IronBoots},
		{"BaitBag", GameItem::BaitBag},
		{"Boomerang", GameItem::Boomerang},
		{"Hookshot", GameItem::Hookshot},
		{"DeliveryBag", GameItem::DeliveryBag},
		{"Bombs", GameItem::Bombs},
		{"SkullHammer", GameItem::SkullHammer},
		{"DekuLeaf", GameItem::DekuLeaf},
		{"TriforceShard", GameItem::TriforceShard},
		{"NayrusPearl", GameItem::NayrusPearl},
		{"DinsPearl", GameItem::DinsPearl},
		{"FaroresPearl", GameItem::FaroresPearl},
		{"WindsRequiem", GameItem::WindsRequiem},
		{"BalladOfGales", GameItem::BalladOfGales},
		{"CommandMelody", GameItem::CommandMelody},
		{"EarthGodsLyric", GameItem::EarthGodsLyric},
		{"WindGodsAria", GameItem::WindGodsAria},
		{"SongofPassing", GameItem::SongofPassing},
		{"Sail", GameItem::Sail},
		{"NoteToMom", GameItem::NoteToMom},
		{"MaggiesLetter", GameItem::MaggiesLetter},
		{"MoblinsLetter", GameItem::MoblinsLetter},
		{"CabanaDeed", GameItem::CabanaDeed},
		{"DragonTingleStatue", GameItem::DragonTingleStatue},
		{"ForbiddenTingleStatue", GameItem::ForbiddenTingleStatue},
		{"GoddessTingleStatue", GameItem::GoddessTingleStatue},
		{"EarthTingleStatue", GameItem::EarthTingleStatue},
		{"WindTingleStatue", GameItem::WindTingleStatue},
		{"BombBagUpgrade", GameItem::BombBagUpgrade},
		{"QuiverUpgrade", GameItem::QuiverUpgrade},
		{"MagicMeter", GameItem::MagicMeter},
		{"GhostShipChart", GameItem::GhostShipChart},
		{"Sword", GameItem::Sword},
		{"Shield", GameItem::Shield},
		{"Shield", GameItem::Shield},
		{"Bow", GameItem::Bow},
		{"Wallet", GameItem::Wallet},
		{"PictoBox", GameItem::PictoBox},
		{"DeluxePictoBox", GameItem::DeluxePictoBox},
		{"Bottle", GameItem::Bottle},
		{"TriforceChart1", GameItem::TriforceChart1},
		{"TriforceChart2", GameItem::TriforceChart2},
		{"TriforceChart3", GameItem::TriforceChart3},
		{"TreasureChart1", GameItem::TreasureChart1},
		{"TreasureChart2", GameItem::TreasureChart2},
		{"TreasureChart3", GameItem::TreasureChart3},
		{"TreasureChart4", GameItem::TreasureChart4},
		{"TreasureChart5", GameItem::TreasureChart5},
		{"TreasureChart6", GameItem::TreasureChart6},
		{"TreasureChart7", GameItem::TreasureChart7},
		{"TreasureChart8", GameItem::TreasureChart8},
		{"TreasureChart9", GameItem::TreasureChart9},
		{"TreasureChart10", GameItem::TreasureChart10},
		{"TreasureChart11", GameItem::TreasureChart11},
		{"TreasureChart12", GameItem::TreasureChart12},
		{"TreasureChart13", GameItem::TreasureChart13},
		{"TreasureChart14", GameItem::TreasureChart14},
		{"TreasureChart15", GameItem::TreasureChart15},
		{"TreasureChart16", GameItem::TreasureChart16},
		{"TreasureChart17", GameItem::TreasureChart17},
		{"TreasureChart18", GameItem::TreasureChart18},
		{"TreasureChart19", GameItem::TreasureChart19},
		{"TreasureChart20", GameItem::TreasureChart20},
		{"TreasureChart21", GameItem::TreasureChart21},
		{"TreasureChart22", GameItem::TreasureChart22},
		{"TreasureChart23", GameItem::TreasureChart23},
		{"TreasureChart24", GameItem::TreasureChart24},
		{"TreasureChart25", GameItem::TreasureChart25},
		{"TreasureChart26", GameItem::TreasureChart26},
		{"TreasureChart27", GameItem::TreasureChart27},
		{"TreasureChart28", GameItem::TreasureChart28},
		{"TreasureChart29", GameItem::TreasureChart29},
		{"TreasureChart30", GameItem::TreasureChart30},
		{"TreasureChart31", GameItem::TreasureChart31},
		{"TreasureChart32", GameItem::TreasureChart32},
		{"TreasureChart33", GameItem::TreasureChart33},
		{"TreasureChart34", GameItem::TreasureChart34},
		{"TreasureChart35", GameItem::TreasureChart35},
		{"TreasureChart36", GameItem::TreasureChart36},
		{"TreasureChart37", GameItem::TreasureChart37},
		{"TreasureChart38", GameItem::TreasureChart38},
		{"TreasureChart39", GameItem::TreasureChart39},
		{"TreasureChart40", GameItem::TreasureChart40},
		{"TreasureChart41", GameItem::TreasureChart41},
		{"TreasureChart42", GameItem::TreasureChart42},
		{"TreasureChart43", GameItem::TreasureChart43},
		{"TreasureChart44", GameItem::TreasureChart44},
		{"TreasureChart45", GameItem::TreasureChart45},
		{"TreasureChart46", GameItem::TreasureChart46},
		{"Nothing", GameItem::Nothing}
	};

	if (nameItemMap.count(name) == 0)
	{
		return GameItem::INVALID;
	}
	return nameItemMap.at(name);
}

std::string gameItemToName(GameItem item)
{
	static std::unordered_map<GameItem, std::string> itemNameMap = {
		{GameItem::INVALID, "INVALID"},
		{GameItem::GreenRupee, "GreenRupee"},
		{GameItem::BlueRupee, "BlueRupee"},
		{GameItem::YellowRupee, "YellowRupee"},
		{GameItem::RedRupee, "RedRupee"},
		{GameItem::PurpleRupee, "PurpleRupee"},
		{GameItem::OrangeRupee, "OrangeRupee"},
		{GameItem::PieceOfHeart, "PieceOfHeart"},
		{GameItem::HeartContainer, "HeartContainer"},
		{GameItem::WindWaker, "WindWaker"},
		{GameItem::SpoilsBag, "SpoilsBag"},
		{GameItem::HurricaneSpin, "HurricaneSpin"},
		{GameItem::GrapplingHook, "GrapplingHook"},
		{GameItem::PowerBracelets, "PowerBracelets"},
		{GameItem::IronBoots, "IronBoots"},
		{GameItem::BaitBag, "BaitBag"},
		{GameItem::Boomerang, "Boomerang"},
		{GameItem::Hookshot, "Hookshot"},
		{GameItem::DeliveryBag, "DeliveryBag"},
		{GameItem::Bombs, "Bombs"},
		{GameItem::SkullHammer, "SkullHammer"},
		{GameItem::DekuLeaf, "DekuLeaf"},
		{GameItem::TriforceShard, "TriforceShard"},
		{GameItem::NayrusPearl, "NayrusPearl"},
		{GameItem::DinsPearl, "DinsPearl"},
		{GameItem::FaroresPearl, "FaroresPearl"},
		{GameItem::WindsRequiem, "WindsRequiem"},
		{GameItem::BalladOfGales, "BalladOfGales"},
		{GameItem::CommandMelody, "CommandMelody"},
		{GameItem::EarthGodsLyric, "EarthGodsLyric"},
		{GameItem::WindGodsAria, "WindGodsAria"},
		{GameItem::SongofPassing, "SongofPassing"},
		{GameItem::Sail, "Sail"},
		{GameItem::NoteToMom, "NoteToMom"},
		{GameItem::MaggiesLetter, "MaggiesLetter"},
		{GameItem::MoblinsLetter, "MoblinsLetter"},
		{GameItem::CabanaDeed, "CabanaDeed"},
		{GameItem::DragonTingleStatue, "DragonTingleStatue"},
		{GameItem::ForbiddenTingleStatue, "ForbiddenTingleStatue"},
		{GameItem::GoddessTingleStatue, "GoddessTingleStatue"},
		{GameItem::EarthTingleStatue, "EarthTingleStatue"},
		{GameItem::WindTingleStatue, "WindTingleStatue"},
		{GameItem::BombBagUpgrade, "BombBagUpgrade"},
		{GameItem::QuiverUpgrade, "QuiverUpgrade"},
		{GameItem::MagicMeter, "MagicMeter"},
		{GameItem::GhostShipChart, "GhostShipChart"},
		{GameItem::Sword, "Sword"},
		{GameItem::Shield, "Shield"},
		{GameItem::Shield, "Shield"},
		{GameItem::Bow, "Bow"},
		{GameItem::Wallet, "Wallet"},
		{GameItem::PictoBox, "PictoBox"},
		{GameItem::DeluxePictoBox, "DeluxePictoBox"},
		{GameItem::Bottle, "Bottle"},
		{GameItem::TriforceChart1, "TriforceChart1"},
		{GameItem::TriforceChart2, "TriforceChart2"},
		{GameItem::TriforceChart3, "TriforceChart3"},
		{GameItem::TreasureChart1, "TreasureChart1"},
		{GameItem::TreasureChart2, "TreasureChart2"},
		{GameItem::TreasureChart3, "TreasureChart3"},
		{GameItem::TreasureChart4, "TreasureChart4"},
		{GameItem::TreasureChart5, "TreasureChart5"},
		{GameItem::TreasureChart6, "TreasureChart6"},
		{GameItem::TreasureChart7, "TreasureChart7"},
		{GameItem::TreasureChart8, "TreasureChart8"},
		{GameItem::TreasureChart9, "TreasureChart9"},
		{GameItem::TreasureChart10, "TreasureChart10"},
		{GameItem::TreasureChart11, "TreasureChart11"},
		{GameItem::TreasureChart12, "TreasureChart12"},
		{GameItem::TreasureChart13, "TreasureChart13"},
		{GameItem::TreasureChart14, "TreasureChart14"},
		{GameItem::TreasureChart15, "TreasureChart15"},
		{GameItem::TreasureChart16, "TreasureChart16"},
		{GameItem::TreasureChart17, "TreasureChart17"},
		{GameItem::TreasureChart18, "TreasureChart18"},
		{GameItem::TreasureChart19, "TreasureChart19"},
		{GameItem::TreasureChart20, "TreasureChart20"},
		{GameItem::TreasureChart21, "TreasureChart21"},
		{GameItem::TreasureChart22, "TreasureChart22"},
		{GameItem::TreasureChart23, "TreasureChart23"},
		{GameItem::TreasureChart24, "TreasureChart24"},
		{GameItem::TreasureChart25, "TreasureChart25"},
		{GameItem::TreasureChart26, "TreasureChart26"},
		{GameItem::TreasureChart27, "TreasureChart27"},
		{GameItem::TreasureChart28, "TreasureChart28"},
		{GameItem::TreasureChart29, "TreasureChart29"},
		{GameItem::TreasureChart30, "TreasureChart30"},
		{GameItem::TreasureChart31, "TreasureChart31"},
		{GameItem::TreasureChart32, "TreasureChart32"},
		{GameItem::TreasureChart33, "TreasureChart33"},
		{GameItem::TreasureChart34, "TreasureChart34"},
		{GameItem::TreasureChart35, "TreasureChart35"},
		{GameItem::TreasureChart36, "TreasureChart36"},
		{GameItem::TreasureChart37, "TreasureChart37"},
		{GameItem::TreasureChart38, "TreasureChart38"},
		{GameItem::TreasureChart39, "TreasureChart39"},
		{GameItem::TreasureChart40, "TreasureChart40"},
		{GameItem::TreasureChart41, "TreasureChart41"},
		{GameItem::TreasureChart42, "TreasureChart42"},
		{GameItem::TreasureChart43, "TreasureChart43"},
		{GameItem::TreasureChart44, "TreasureChart44"},
		{GameItem::TreasureChart45, "TreasureChart45"},
		{GameItem::TreasureChart46, "TreasureChart46"},
		{GameItem::Nothing, "Nothing"}
	};

	if (itemNameMap.count(item) == 0)
	{
		return "INVALID";
	}
	return itemNameMap.at(item);
}

GameItem indexToGameItem(uint32_t index)
{
	if (index > static_cast<uint32_t>(GameItem::GAME_ITEM_COUNT))
	{
		return GameItem::INVALID;
	}
	return static_cast<GameItem>(index);
}

uint32_t maxItemCount(GameItem item)
{
	static std::unordered_map<GameItem, uint32_t> progressionItemCount = {
		{GameItem::Sword, 4},
		{GameItem::Shield, 2},
		{GameItem::Bow, 3},
		{GameItem::QuiverUpgrade, 2},
		{GameItem::BombBagUpgrade, 2},
		{GameItem::DRCSmallKey, 4},
		{GameItem::FWSmallKey, 1},
		{GameItem::TOTGSmallKey, 2},
		{GameItem::WTSmallKey, 2},
		{GameItem::ETSmallKey, 3},
		{GameItem::Wallet, 3},
		{GameItem::TriforceShard, 8}
	};

	if (progressionItemCount.count(item) == 0) return 1;
	return progressionItemCount.at(item);
}

Item::Item(GameItem gameItemId_, int worldId_)
{
		gameItemId = gameItemId_;
		worldId = worldId_;
}

void Item::setWorldId(int newWorldId)
{
		worldId = newWorldId;
}

int Item::getWorldId() const
{
		return worldId;
}

GameItem Item::getGameItemId() const
{
		return gameItemId;
}

std::string Item::getName() const
{
		return "Player " + std::to_string(worldId) + "'s " + gameItemToName(gameItemId);
}

bool Item::operator==(const Item& rhs) const
{
		return gameItemId == rhs.gameItemId && worldId == rhs.worldId;
}

bool Item::isMajorItem() const
{
		if (gameItemId == GameItem::GreenRupee)
		{
				return false;
		}
		return true;
}
