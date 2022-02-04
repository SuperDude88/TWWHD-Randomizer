
#include "GameItem.hpp"
#include <unordered_map>
#include <array>

GameItem nameToGameItem(const std::string& name)
{
	static std::unordered_map<std::string, GameItem> nameItemMap = {
		{"HeartDrop", GameItem::HeartDrop},
		{"GreenRupee", GameItem::GreenRupee},
		{"BlueRupee", GameItem::BlueRupee},
		{"YellowRupee", GameItem::YellowRupee},
		{"RedRupee", GameItem::RedRupee},
		{"PurpleRupee", GameItem::PurpleRupee},
		{"OrangeRupee", GameItem::OrangeRupee},
		{"PieceOfHeart", GameItem::PieceOfHeart},
		{"HeartContainer", GameItem::HeartContainer},
		{"SmallMagicDrop", GameItem::SmallMagicDrop},
		{"LargeMagicDrop", GameItem::LargeMagicDrop},
		{"FiveBombs", GameItem::FiveBombs},
		{"TenBombs", GameItem::TenBombs},
		{"TwentyBombs", GameItem::TwentyBombs},
		{"ThirtyBombs", GameItem::ThirtyBombs},
		{"SilverRupee", GameItem::SilverRupee},
		{"TenArrows", GameItem::TenArrows},
		{"TwentyArrows", GameItem::TwentyArrows},
		{"ThirtyArrows", GameItem::ThirtyArrows},
		{"DRCSmallKey", GameItem::DRCSmallKey},
		{"DRCBigKey", GameItem::DRCBigKey},
		{"SmallKey", GameItem::SmallKey},
		{"Fairy", GameItem::Fairy},
		{"YellowRupee2", GameItem::YellowRupee2},
		{"DRCDungeonMap", GameItem::DRCDungeonMap},
		{"DRCCompass", GameItem::DRCCompass},
		{"FWSmallKey", GameItem::FWSmallKey},
		{"ThreeHearts", GameItem::ThreeHearts},
		{"JoyPendant", GameItem::JoyPendant},
		{"Telescope", GameItem::Telescope},
		{"TingleBottle", GameItem::TingleBottle},
		{"WindWaker", GameItem::WindWaker},
		{"ProgressivePictoBox", GameItem::ProgressivePictoBox},
		{"SpoilsBag", GameItem::SpoilsBag},
		{"GrapplingHook", GameItem::GrapplingHook},
		{"DeluxePicto", GameItem::DeluxePicto},
		{"ProgressiveBow", GameItem::ProgressiveBow},
		{"PowerBracelets", GameItem::PowerBracelets},
		{"IronBoots", GameItem::IronBoots},
		{"MagicArmor", GameItem::MagicArmor},
		{"BaitBag", GameItem::BaitBag},
		{"Boomerang", GameItem::Boomerang},
		{"Hookshot", GameItem::Hookshot},
		{"DeliveryBag", GameItem::DeliveryBag},
		{"Bombs", GameItem::Bombs},
		{"HerosClothes", GameItem::HerosClothes},
		{"SkullHammer", GameItem::SkullHammer},
		{"DekuLeaf", GameItem::DekuLeaf},
		{"FireIceArrows", GameItem::FireIceArrows},
		{"LightArrow", GameItem::LightArrow},
		{"HerosNewClothes", GameItem::HerosNewClothes},
		{"ProgressiveSword", GameItem::ProgressiveSword},
		{"MasterSwordPowerless", GameItem::MasterSwordPowerless},
		{"MasterSwordHalf", GameItem::MasterSwordHalf},
		{"ProgressiveShield", GameItem::ProgressiveShield},
		{"MirrorShield", GameItem::MirrorShield},
		{"RecoveredHerosSword", GameItem::RecoveredHerosSword},
		{"MasterSwordFull", GameItem::MasterSwordFull},
		{"PieceOfHeart2", GameItem::PieceOfHeart2},
		{"FWBigKey", GameItem::FWBigKey},
		{"FWDungeonMap", GameItem::FWDungeonMap},
		{"PiratesCharm", GameItem::PiratesCharm},
		{"HerosCharm", GameItem::HerosCharm},
		{"SkullNecklace", GameItem::SkullNecklace},
		{"BokoBabaSeed", GameItem::BokoBabaSeed},
		{"GoldenFeather", GameItem::GoldenFeather},
		{"KnightsCrest", GameItem::KnightsCrest},
		{"RedChuJelly", GameItem::RedChuJelly},
		{"GreenChuJelly", GameItem::GreenChuJelly},
		{"BlueChuJelly", GameItem::BlueChuJelly},
		{"DungeonMap", GameItem::DungeonMap},
		{"Compass", GameItem::Compass},
		{"BigKey", GameItem::BigKey},
		{"EmptyBottle", GameItem::EmptyBottle},
		{"RedPotion", GameItem::RedPotion},
		{"GreenPotion", GameItem::GreenPotion},
		{"BluePotion", GameItem::BluePotion},
		{"ElixirSoupHalf", GameItem::ElixirSoupHalf},
		{"ElixirSoup", GameItem::ElixirSoup},
		{"BottledWater", GameItem::BottledWater},
		{"FairyInBottle", GameItem::FairyInBottle},
		{"ForestFirefly", GameItem::ForestFirefly},
		{"ForestWater", GameItem::ForestWater},
		{"FWCompass", GameItem::FWCompass},
		{"TotGSmallKey", GameItem::TotGSmallKey},
		{"TotGBigKey", GameItem::TotGBigKey},
		{"TotGDungeonMap", GameItem::TotGDungeonMap},
		{"TotGCompass", GameItem::TotGCompass},
		{"FFDungeonMap", GameItem::FFDungeonMap},
		{"FFCompass", GameItem::FFCompass},
		{"TriforceShard1", GameItem::TriforceShard1},
		{"TriforceShard2", GameItem::TriforceShard2},
		{"TriforceShard3", GameItem::TriforceShard3},
		{"TriforceShard4", GameItem::TriforceShard4},
		{"TriforceShard5", GameItem::TriforceShard5},
		{"TriforceShard6", GameItem::TriforceShard6},
		{"TriforceShard7", GameItem::TriforceShard7},
		{"TriforceShard8", GameItem::TriforceShard8},
		{"NayrusPearl", GameItem::NayrusPearl},
		{"DinsPearl", GameItem::DinsPearl},
		{"FaroresPearl", GameItem::FaroresPearl},
		{"WindsRequiem", GameItem::WindsRequiem},
		{"BalladOfGales", GameItem::BalladOfGales},
		{"CommandMelody", GameItem::CommandMelody},
		{"EarthGodsLyric", GameItem::EarthGodsLyric},
		{"WindGodsAria", GameItem::WindGodsAria},
		{"SongOfPassing", GameItem::SongOfPassing},
		{"ETSmallKey", GameItem::ETSmallKey},
		{"ETBigKey", GameItem::ETBigKey},
		{"ETDungeonMap", GameItem::ETDungeonMap},
		{"ETCompass", GameItem::ETCompass},
		{"SwiftSail", GameItem::SwiftSail},
		{"BoatsSail", GameItem::BoatsSail},
		{"TriforceChart1Deciphered", GameItem::TriforceChart1Deciphered},
		{"TriforceChart2Deciphered", GameItem::TriforceChart2Deciphered},
		{"TriforceChart3Deciphered", GameItem::TriforceChart3Deciphered},
		{"TriforceChart4Deciphered", GameItem::TriforceChart4Deciphered},
		{"TriforceChart5Deciphered", GameItem::TriforceChart5Deciphered},
		{"TriforceChart6Deciphered", GameItem::TriforceChart6Deciphered},
		{"TriforceChart7Deciphered", GameItem::TriforceChart7Deciphered},
		{"TriforceChart8Deciphered", GameItem::TriforceChart8Deciphered},
		{"WTSmallKey", GameItem::WTSmallKey},
		{"AllPurposeBait", GameItem::AllPurposeBait},
		{"HyoiPear", GameItem::HyoiPear},
		{"WTBigKey", GameItem::WTBigKey},
		{"WTDungeonMap", GameItem::WTDungeonMap},
		{"WTCompass", GameItem::WTCompass},
		{"TownFlower", GameItem::TownFlower},
		{"SeaFlower", GameItem::SeaFlower},
		{"ExoticFlower", GameItem::ExoticFlower},
		{"HerosFlag", GameItem::HerosFlag},
		{"BigCatchFlag", GameItem::BigCatchFlag},
		{"BigSaleFlag", GameItem::BigSaleFlag},
		{"Pinwheel", GameItem::Pinwheel},
		{"SickleMoonFlag", GameItem::SickleMoonFlag},
		{"SkullTowerIdol", GameItem::SkullTowerIdol},
		{"FountainIdol", GameItem::FountainIdol},
		{"PostmanStatue", GameItem::PostmanStatue},
		{"ShopGuruStatue", GameItem::ShopGuruStatue},
		{"FathersLetter", GameItem::FathersLetter},
		{"NoteToMom", GameItem::NoteToMom},
		{"MaggiesLetter", GameItem::MaggiesLetter},
		{"MoblinsLetter", GameItem::MoblinsLetter},
		{"CabanaDeed", GameItem::CabanaDeed},
		{"ComplimentaryID", GameItem::ComplimentaryID},
		{"FillUpCoupon", GameItem::FillUpCoupon},
		{"LegendaryPictograph", GameItem::LegendaryPictograph},
		{"DragonTingleStatue", GameItem::DragonTingleStatue},
		{"ForbiddenTingleStatue", GameItem::ForbiddenTingleStatue},
		{"GoddessTingleStatue", GameItem::GoddessTingleStatue},
		{"EarthTingleStatue", GameItem::EarthTingleStatue},
		{"WindTingleStatue", GameItem::WindTingleStatue},
		{"HurricaneSpin", GameItem::HurricaneSpin},
		{"ProgressiveWallet", GameItem::ProgressiveWallet},
		{"FiveThousandWallet", GameItem::FiveThousandWallet},
		{"ProgressiveBombBag", GameItem::ProgressiveBombBag},
		{"NinetyNineBombBag", GameItem::NinetyNineBombBag},
		{"ProgressiveQuiver", GameItem::ProgressiveQuiver},
		{"NinetyNineQuiver", GameItem::NinetyNineQuiver},
		{"MagicMeterUpgrade", GameItem::MagicMeterUpgrade},
		{"FiftyRupees", GameItem::FiftyRupees},
		{"HundredRupees", GameItem::HundredRupees},
		{"HundredFiftyRupees", GameItem::HundredFiftyRupees},
		{"TwoHundredRupees", GameItem::TwoHundredRupees},
		{"TwoHundredFiftyRupees", GameItem::TwoHundredFiftyRupees},
		{"RainbowRupee", GameItem::RainbowRupee},
		{"SubmarineChart", GameItem::SubmarineChart},
		{"BeedlesChart", GameItem::BeedlesChart},
		{"PlatformChart", GameItem::PlatformChart},
		{"LightRingChart", GameItem::LightRingChart},
		{"SecretCaveChart", GameItem::SecretCaveChart},
		{"SeaHeartsChart", GameItem::SeaHeartsChart},
		{"IslandHeartsChart", GameItem::IslandHeartsChart},
		{"GreatFairyChart", GameItem::GreatFairyChart},
		{"OctoChart", GameItem::OctoChart},
		{"INcredibleChart", GameItem::INcredibleChart},
		{"TreasureChart7", GameItem::TreasureChart7},
		{"TreasureChart27", GameItem::TreasureChart27},
		{"TreasureChart21", GameItem::TreasureChart21},
		{"TreasureChart13", GameItem::TreasureChart13},
		{"TreasureChart32", GameItem::TreasureChart32},
		{"TreasureChart19", GameItem::TreasureChart19},
		{"TreasureChart41", GameItem::TreasureChart41},
		{"TreasureChart26", GameItem::TreasureChart26},
		{"TreasureChart8", GameItem::TreasureChart8},
		{"TreasureChart37", GameItem::TreasureChart37},
		{"TreasureChart25", GameItem::TreasureChart25},
		{"TreasureChart17", GameItem::TreasureChart17},
		{"TreasureChart36", GameItem::TreasureChart36},
		{"TreasureChart22", GameItem::TreasureChart22},
		{"TreasureChart9", GameItem::TreasureChart9},
		{"GhostShipChart", GameItem::GhostShipChart},
		{"TinglesChart", GameItem::TinglesChart},
		{"TreasureChart14", GameItem::TreasureChart14},
		{"TreasureChart10", GameItem::TreasureChart10},
		{"TreasureChart40", GameItem::TreasureChart40},
		{"TreasureChart3", GameItem::TreasureChart3},
		{"TreasureChart4", GameItem::TreasureChart4},
		{"TreasureChart28", GameItem::TreasureChart28},
		{"TreasureChart16", GameItem::TreasureChart16},
		{"TreasureChart18", GameItem::TreasureChart18},
		{"TreasureChart34", GameItem::TreasureChart34},
		{"TreasureChart29", GameItem::TreasureChart29},
		{"TreasureChart1", GameItem::TreasureChart1},
		{"TreasureChart35", GameItem::TreasureChart35},
		{"TreasureChart12", GameItem::TreasureChart12},
		{"TreasureChart6", GameItem::TreasureChart6},
		{"TreasureChart24", GameItem::TreasureChart24},
		{"TreasureChart39", GameItem::TreasureChart39},
		{"TreasureChart38", GameItem::TreasureChart38},
		{"TreasureChart2", GameItem::TreasureChart2},
		{"TreasureChart33", GameItem::TreasureChart33},
		{"TreasureChart31", GameItem::TreasureChart31},
		{"TreasureChart23", GameItem::TreasureChart23},
		{"TreasureChart5", GameItem::TreasureChart5},
		{"TreasureChart20", GameItem::TreasureChart20},
		{"TreasureChart30", GameItem::TreasureChart30},
		{"TreasureChart15", GameItem::TreasureChart15},
		{"TreasureChart11", GameItem::TreasureChart11},
		{"TreasureChart46", GameItem::TreasureChart46},
		{"TreasureChart45", GameItem::TreasureChart45},
		{"TreasureChart44", GameItem::TreasureChart44},
		{"TriforceChart3", GameItem::TriforceChart3},
		{"TreasureChart43", GameItem::TreasureChart43},
		{"TriforceChart2", GameItem::TriforceChart2},
		{"TreasureChart42", GameItem::TreasureChart42},
		{"TriforceChart1", GameItem::TriforceChart1},
		{"GameBeatable", GameItem::GameBeatable},
		{"Nothing", GameItem::NOTHING}
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
		{GameItem::HeartDrop, "HeartDrop"},
		{GameItem::GreenRupee, "GreenRupee"},
		{GameItem::BlueRupee, "BlueRupee"},
		{GameItem::YellowRupee, "YellowRupee"},
		{GameItem::RedRupee, "RedRupee"},
		{GameItem::PurpleRupee, "PurpleRupee"},
		{GameItem::OrangeRupee, "OrangeRupee"},
		{GameItem::PieceOfHeart, "PieceOfHeart"},
		{GameItem::HeartContainer, "HeartContainer"},
		{GameItem::SmallMagicDrop, "SmallMagicDrop"},
		{GameItem::LargeMagicDrop, "LargeMagicDrop"},
		{GameItem::FiveBombs, "FiveBombs"},
		{GameItem::TenBombs, "TenBombs"},
		{GameItem::TwentyBombs, "TwentyBombs"},
		{GameItem::ThirtyBombs, "ThirtyBombs"},
		{GameItem::SilverRupee, "SilverRupee"},
		{GameItem::TenArrows, "TenArrows"},
		{GameItem::TwentyArrows, "TwentyArrows"},
		{GameItem::ThirtyArrows, "ThirtyArrows"},
		{GameItem::DRCSmallKey, "DRCSmallKey"},
		{GameItem::DRCBigKey, "DRCBigKey"},
		{GameItem::SmallKey, "SmallKey"},
		{GameItem::Fairy, "Fairy"},
		{GameItem::YellowRupee2, "YellowRupee2"},
		{GameItem::DRCDungeonMap, "DRCDungeonMap"},
		{GameItem::DRCCompass, "DRCCompass"},
		{GameItem::FWSmallKey, "FWSmallKey"},
		{GameItem::ThreeHearts, "ThreeHearts"},
		{GameItem::JoyPendant, "JoyPendant"},
		{GameItem::Telescope, "Telescope"},
		{GameItem::TingleBottle, "TingleBottle"},
		{GameItem::WindWaker, "WindWaker"},
		{GameItem::ProgressivePictoBox, "ProgressivePictoBox"},
		{GameItem::SpoilsBag, "SpoilsBag"},
		{GameItem::GrapplingHook, "GrapplingHook"},
		{GameItem::DeluxePicto, "DeluxePicto"},
		{GameItem::ProgressiveBow, "ProgressiveBow"},
		{GameItem::PowerBracelets, "PowerBracelets"},
		{GameItem::IronBoots, "IronBoots"},
		{GameItem::MagicArmor, "MagicArmor"},
		{GameItem::BaitBag, "BaitBag"},
		{GameItem::Boomerang, "Boomerang"},
		{GameItem::Hookshot, "Hookshot"},
		{GameItem::DeliveryBag, "DeliveryBag"},
		{GameItem::Bombs, "Bombs"},
		{GameItem::HerosClothes, "HerosClothes"},
		{GameItem::SkullHammer, "SkullHammer"},
		{GameItem::DekuLeaf, "DekuLeaf"},
		{GameItem::FireIceArrows, "FireIceArrows"},
		{GameItem::LightArrow, "LightArrow"},
		{GameItem::HerosNewClothes, "HerosNewClothes"},
		{GameItem::ProgressiveSword, "ProgressiveSword"},
		{GameItem::MasterSwordPowerless, "MasterSwordPowerless"},
		{GameItem::MasterSwordHalf, "MasterSwordHalf"},
		{GameItem::ProgressiveShield, "ProgressiveShield"},
		{GameItem::MirrorShield, "MirrorShield"},
		{GameItem::RecoveredHerosSword, "RecoveredHerosSword"},
		{GameItem::MasterSwordFull, "MasterSwordFull"},
		{GameItem::PieceOfHeart2, "PieceOfHeart2"},
		{GameItem::FWBigKey, "FWBigKey"},
		{GameItem::FWDungeonMap, "FWDungeonMap"},
		{GameItem::PiratesCharm, "PiratesCharm"},
		{GameItem::HerosCharm, "HerosCharm"},
		{GameItem::SkullNecklace, "SkullNecklace"},
		{GameItem::BokoBabaSeed, "BokoBabaSeed"},
		{GameItem::GoldenFeather, "GoldenFeather"},
		{GameItem::KnightsCrest, "KnightsCrest"},
		{GameItem::RedChuJelly, "RedChuJelly"},
		{GameItem::GreenChuJelly, "GreenChuJelly"},
		{GameItem::BlueChuJelly, "BlueChuJelly"},
		{GameItem::DungeonMap, "DungeonMap"},
		{GameItem::Compass, "Compass"},
		{GameItem::BigKey, "BigKey"},
		{GameItem::EmptyBottle, "EmptyBottle"},
		{GameItem::RedPotion, "RedPotion"},
		{GameItem::GreenPotion, "GreenPotion"},
		{GameItem::BluePotion, "BluePotion"},
		{GameItem::ElixirSoupHalf, "ElixirSoupHalf"},
		{GameItem::ElixirSoup, "ElixirSoup"},
		{GameItem::BottledWater, "BottledWater"},
		{GameItem::FairyInBottle, "FairyInBottle"},
		{GameItem::ForestFirefly, "ForestFirefly"},
		{GameItem::ForestWater, "ForestWater"},
		{GameItem::FWCompass, "FWCompass"},
		{GameItem::TotGSmallKey, "TotGSmallKey"},
		{GameItem::TotGBigKey, "TotGBigKey"},
		{GameItem::TotGDungeonMap, "TotGDungeonMap"},
		{GameItem::TotGCompass, "TotGCompass"},
		{GameItem::FFDungeonMap, "FFDungeonMap"},
		{GameItem::FFCompass, "FFCompass"},
		{GameItem::TriforceShard1, "TriforceShard1"},
		{GameItem::TriforceShard2, "TriforceShard2"},
		{GameItem::TriforceShard3, "TriforceShard3"},
		{GameItem::TriforceShard4, "TriforceShard4"},
		{GameItem::TriforceShard5, "TriforceShard5"},
		{GameItem::TriforceShard6, "TriforceShard6"},
		{GameItem::TriforceShard7, "TriforceShard7"},
		{GameItem::TriforceShard8, "TriforceShard8"},
		{GameItem::NayrusPearl, "NayrusPearl"},
		{GameItem::DinsPearl, "DinsPearl"},
		{GameItem::FaroresPearl, "FaroresPearl"},
		{GameItem::WindsRequiem, "WindsRequiem"},
		{GameItem::BalladOfGales, "BalladOfGales"},
		{GameItem::CommandMelody, "CommandMelody"},
		{GameItem::EarthGodsLyric, "EarthGodsLyric"},
		{GameItem::WindGodsAria, "WindGodsAria"},
		{GameItem::SongOfPassing, "SongOfPassing"},
		{GameItem::ETSmallKey, "ETSmallKey"},
		{GameItem::ETBigKey, "ETBigKey"},
		{GameItem::ETDungeonMap, "ETDungeonMap"},
		{GameItem::ETCompass, "ETCompass"},
		{GameItem::SwiftSail, "SwiftSail"},
		{GameItem::BoatsSail, "BoatsSail"},
		{GameItem::TriforceChart1Deciphered, "TriforceChart1Deciphered"},
		{GameItem::TriforceChart2Deciphered, "TriforceChart2Deciphered"},
		{GameItem::TriforceChart3Deciphered, "TriforceChart3Deciphered"},
		{GameItem::TriforceChart4Deciphered, "TriforceChart4Deciphered"},
		{GameItem::TriforceChart5Deciphered, "TriforceChart5Deciphered"},
		{GameItem::TriforceChart6Deciphered, "TriforceChart6Deciphered"},
		{GameItem::TriforceChart7Deciphered, "TriforceChart7Deciphered"},
		{GameItem::TriforceChart8Deciphered, "TriforceChart8Deciphered"},
		{GameItem::WTSmallKey, "WTSmallKey"},
		{GameItem::AllPurposeBait, "AllPurposeBait"},
		{GameItem::HyoiPear, "HyoiPear"},
		{GameItem::WTBigKey, "WTBigKey"},
		{GameItem::WTDungeonMap, "WTDungeonMap"},
		{GameItem::WTCompass, "WTCompass"},
		{GameItem::TownFlower, "TownFlower"},
		{GameItem::SeaFlower, "SeaFlower"},
		{GameItem::ExoticFlower, "ExoticFlower"},
		{GameItem::HerosFlag, "HerosFlag"},
		{GameItem::BigCatchFlag, "BigCatchFlag"},
		{GameItem::BigSaleFlag, "BigSaleFlag"},
		{GameItem::Pinwheel, "Pinwheel"},
		{GameItem::SickleMoonFlag, "SickleMoonFlag"},
		{GameItem::SkullTowerIdol, "SkullTowerIdol"},
		{GameItem::FountainIdol, "FountainIdol"},
		{GameItem::PostmanStatue, "PostmanStatue"},
		{GameItem::ShopGuruStatue, "ShopGuruStatue"},
		{GameItem::FathersLetter, "FathersLetter"},
		{GameItem::NoteToMom, "NoteToMom"},
		{GameItem::MaggiesLetter, "MaggiesLetter"},
		{GameItem::MoblinsLetter, "MoblinsLetter"},
		{GameItem::CabanaDeed, "CabanaDeed"},
		{GameItem::ComplimentaryID, "ComplimentaryID"},
		{GameItem::FillUpCoupon, "FillUpCoupon"},
		{GameItem::LegendaryPictograph, "LegendaryPictograph"},
		{GameItem::DragonTingleStatue, "DragonTingleStatue"},
		{GameItem::ForbiddenTingleStatue, "ForbiddenTingleStatue"},
		{GameItem::GoddessTingleStatue, "GoddessTingleStatue"},
		{GameItem::EarthTingleStatue, "EarthTingleStatue"},
		{GameItem::WindTingleStatue, "WindTingleStatue"},
		{GameItem::HurricaneSpin, "HurricaneSpin"},
		{GameItem::ProgressiveWallet, "ProgressiveWallet"},
		{GameItem::FiveThousandWallet, "FiveThousandWallet"},
		{GameItem::ProgressiveBombBag, "ProgressiveBombBag"},
		{GameItem::NinetyNineBombBag, "NinetyNineBombBag"},
		{GameItem::ProgressiveQuiver, "ProgressiveQuiver"},
		{GameItem::NinetyNineQuiver, "NinetyNineQuiver"},
		{GameItem::MagicMeterUpgrade, "MagicMeterUpgrade"},
		{GameItem::FiftyRupees, "FiftyRupees"},
		{GameItem::HundredRupees, "HundredRupees"},
		{GameItem::HundredFiftyRupees, "HundredFiftyRupees"},
		{GameItem::TwoHundredRupees, "TwoHundredRupees"},
		{GameItem::TwoHundredFiftyRupees, "TwoHundredFiftyRupees"},
		{GameItem::RainbowRupee, "RainbowRupee"},
		{GameItem::SubmarineChart, "SubmarineChart"},
		{GameItem::BeedlesChart, "BeedlesChart"},
		{GameItem::PlatformChart, "PlatformChart"},
		{GameItem::LightRingChart, "LightRingChart"},
		{GameItem::SecretCaveChart, "SecretCaveChart"},
		{GameItem::SeaHeartsChart, "SeaHeartsChart"},
		{GameItem::IslandHeartsChart, "IslandHeartsChart"},
		{GameItem::GreatFairyChart, "GreatFairyChart"},
		{GameItem::OctoChart, "OctoChart"},
		{GameItem::INcredibleChart, "INcredibleChart"},
		{GameItem::TreasureChart7, "TreasureChart7"},
		{GameItem::TreasureChart27, "TreasureChart27"},
		{GameItem::TreasureChart21, "TreasureChart21"},
		{GameItem::TreasureChart13, "TreasureChart13"},
		{GameItem::TreasureChart32, "TreasureChart32"},
		{GameItem::TreasureChart19, "TreasureChart19"},
		{GameItem::TreasureChart41, "TreasureChart41"},
		{GameItem::TreasureChart26, "TreasureChart26"},
		{GameItem::TreasureChart8, "TreasureChart8"},
		{GameItem::TreasureChart37, "TreasureChart37"},
		{GameItem::TreasureChart25, "TreasureChart25"},
		{GameItem::TreasureChart17, "TreasureChart17"},
		{GameItem::TreasureChart36, "TreasureChart36"},
		{GameItem::TreasureChart22, "TreasureChart22"},
		{GameItem::TreasureChart9, "TreasureChart9"},
		{GameItem::GhostShipChart, "GhostShipChart"},
		{GameItem::TinglesChart, "TinglesChart"},
		{GameItem::TreasureChart14, "TreasureChart14"},
		{GameItem::TreasureChart10, "TreasureChart10"},
		{GameItem::TreasureChart40, "TreasureChart40"},
		{GameItem::TreasureChart3, "TreasureChart3"},
		{GameItem::TreasureChart4, "TreasureChart4"},
		{GameItem::TreasureChart28, "TreasureChart28"},
		{GameItem::TreasureChart16, "TreasureChart16"},
		{GameItem::TreasureChart18, "TreasureChart18"},
		{GameItem::TreasureChart34, "TreasureChart34"},
		{GameItem::TreasureChart29, "TreasureChart29"},
		{GameItem::TreasureChart1, "TreasureChart1"},
		{GameItem::TreasureChart35, "TreasureChart35"},
		{GameItem::TreasureChart12, "TreasureChart12"},
		{GameItem::TreasureChart6, "TreasureChart6"},
		{GameItem::TreasureChart24, "TreasureChart24"},
		{GameItem::TreasureChart39, "TreasureChart39"},
		{GameItem::TreasureChart38, "TreasureChart38"},
		{GameItem::TreasureChart2, "TreasureChart2"},
		{GameItem::TreasureChart33, "TreasureChart33"},
		{GameItem::TreasureChart31, "TreasureChart31"},
		{GameItem::TreasureChart23, "TreasureChart23"},
		{GameItem::TreasureChart5, "TreasureChart5"},
		{GameItem::TreasureChart20, "TreasureChart20"},
		{GameItem::TreasureChart30, "TreasureChart30"},
		{GameItem::TreasureChart15, "TreasureChart15"},
		{GameItem::TreasureChart11, "TreasureChart11"},
		{GameItem::TreasureChart46, "TreasureChart46"},
		{GameItem::TreasureChart45, "TreasureChart45"},
		{GameItem::TreasureChart44, "TreasureChart44"},
		{GameItem::TriforceChart3, "TriforceChart3"},
		{GameItem::TreasureChart43, "TreasureChart43"},
		{GameItem::TriforceChart2, "TriforceChart2"},
		{GameItem::TreasureChart42, "TreasureChart42"},
		{GameItem::TriforceChart1, "TriforceChart1"},
		{GameItem::GameBeatable, "GameBeatable"},
		{GameItem::NOTHING, "Nothing"}
	};

	if (itemNameMap.count(item) == 0)
	{
		return "INVALID";
	}
	return itemNameMap.at(item);
}

GameItem idToGameItem(uint8_t id)
{
	return static_cast<GameItem>(id);
}

Item::Item(GameItem gameItemId_, int worldId_)
{
		gameItemId = gameItemId_;
		worldId = worldId_;

		if (junkItems.count(gameItemId) > 0)
		{
				junkItem = true;
		}
		else if (gameItemId >= GameItem::TreasureChart7 && gameItemId <= GameItem::TriforceChart1 &&
						 gameItemId != GameItem::GhostShipChart && gameItemId != GameItem::TinglesChart)
		{
				chartForSunkenTreasure = true;
		}
}

void Item::setWorldId(int newWorldId)
{
		worldId = newWorldId;
}

int Item::getWorldId() const
{
		return worldId;
}

void Item::setGameItemId(GameItem newGameItemId)
{
		gameItemId = newGameItemId;
}

GameItem Item::getGameItemId() const
{
		return gameItemId;
}

void Item::setDelayedItemId(GameItem newDelayedItemId)
{
		delayedGameItemId = newDelayedItemId;
}

void Item::saveDelayedItemId()
{
		gameItemId = delayedGameItemId;
		delayedGameItemId = GameItem::INVALID;
}

std::string Item::getName() const
{
		return gameItemToName(gameItemId) + " for Player " + std::to_string(worldId + 1);
}

void Item::setAsMajorItem()
{
		majorItem = true;
}

bool Item::isMajorItem() const
{
		return majorItem;
}

bool Item::isChartForSunkenTreasure() const
{
		return chartForSunkenTreasure;
}

bool Item::isJunkItem() const
{
		return junkItem;
}

bool Item::operator==(const Item& rhs) const
{
		return gameItemId == rhs.gameItemId && worldId == rhs.worldId;
}

bool Item::operator<(const Item& rhs) const
{
		return (worldId == rhs.worldId) ? gameItemId < rhs.gameItemId : worldId < rhs.worldId;
}
