
#include "GameItem.hpp"
#include "../server/command/Log.hpp"
#include <unordered_map>
#include <array>

GameItem nameToGameItem(const std::string& name)
{
	static std::unordered_map<std::string, GameItem> nameItemMap = {
		{"Heart Drop", GameItem::HeartDrop},
		{"Green Rupee", GameItem::GreenRupee},
		{"Blue Rupee", GameItem::BlueRupee},
		{"Yellow Rupee", GameItem::YellowRupee},
		{"Red Rupee", GameItem::RedRupee},
		{"Purple Rupee", GameItem::PurpleRupee},
		{"Orange Rupee", GameItem::OrangeRupee},
		{"Piece Of Heart", GameItem::PieceOfHeart},
		{"Heart Container", GameItem::HeartContainer},
		{"Small Magic Drop", GameItem::SmallMagicDrop},
		{"Large Magic Drop", GameItem::LargeMagicDrop},
		{"Five Bombs", GameItem::FiveBombs},
		{"Ten Bombs", GameItem::TenBombs},
		{"Twenty Bombs", GameItem::TwentyBombs},
		{"Thirty Bombs", GameItem::ThirtyBombs},
		{"Silver Rupee", GameItem::SilverRupee},
		{"Ten Arrows", GameItem::TenArrows},
		{"Twenty Arrows", GameItem::TwentyArrows},
		{"Thirty Arrows", GameItem::ThirtyArrows},
		{"DRC Small Key", GameItem::DRCSmallKey},
		{"DRC Big Key", GameItem::DRCBigKey},
		{"Small Key", GameItem::SmallKey},
		{"Fairy", GameItem::Fairy},
		{"Yellow Rupee 2", GameItem::YellowRupee2},
		{"DRC Dungeon Map", GameItem::DRCDungeonMap},
		{"DRC Compass", GameItem::DRCCompass},
		{"FW Small Key", GameItem::FWSmallKey},
		{"Three Hearts", GameItem::ThreeHearts},
		{"Joy Pendant", GameItem::JoyPendant},
		{"Telescope", GameItem::Telescope},
		{"Tingle Bottle", GameItem::TingleBottle},
		{"Wind Waker", GameItem::WindWaker},
		{"Progressive Picto Box", GameItem::ProgressivePictoBox},
		{"Spoils Bag", GameItem::SpoilsBag},
		{"Grappling Hook", GameItem::GrapplingHook},
		{"Deluxe Picto", GameItem::DeluxePicto},
		{"Progressive Bow", GameItem::ProgressiveBow},
		{"Power Bracelets", GameItem::PowerBracelets},
		{"Iron Boots", GameItem::IronBoots},
		{"Magic Armor", GameItem::MagicArmor},
		{"Bait Bag", GameItem::BaitBag},
		{"Boomerang", GameItem::Boomerang},
		{"Hookshot", GameItem::Hookshot},
		{"Delivery Bag", GameItem::DeliveryBag},
		{"Bombs", GameItem::Bombs},
		{"Heros Clothes", GameItem::HerosClothes},
		{"Skull Hammer", GameItem::SkullHammer},
		{"Deku Leaf", GameItem::DekuLeaf},
		{"Fire Ice Arrows", GameItem::FireIceArrows},
		{"Light Arrow", GameItem::LightArrow},
		{"Heros New Clothes", GameItem::HerosNewClothes},
		{"Progressive Sword", GameItem::ProgressiveSword},
		{"Master Sword Powerless", GameItem::MasterSwordPowerless},
		{"Master Sword Half", GameItem::MasterSwordHalf},
		{"Progressive Shield", GameItem::ProgressiveShield},
		{"Mirror Shield", GameItem::MirrorShield},
		{"Recovered Heros Sword", GameItem::RecoveredHerosSword},
		{"Master Sword Full", GameItem::MasterSwordFull},
		{"Piece Of Heart 2", GameItem::PieceOfHeart2},
		{"FW Big Key", GameItem::FWBigKey},
		{"FW Dungeon Map", GameItem::FWDungeonMap},
		{"Pirates Charm", GameItem::PiratesCharm},
		{"Heros Charm", GameItem::HerosCharm},
		{"Skull Necklace", GameItem::SkullNecklace},
		{"Boko Baba Seed", GameItem::BokoBabaSeed},
		{"Golden Feather", GameItem::GoldenFeather},
		{"Knights Crest", GameItem::KnightsCrest},
		{"Red Chu Jelly", GameItem::RedChuJelly},
		{"Green Chu Jelly", GameItem::GreenChuJelly},
		{"Blue Chu Jelly", GameItem::BlueChuJelly},
		{"Dungeon Map", GameItem::DungeonMap},
		{"Compass", GameItem::Compass},
		{"Big Key", GameItem::BigKey},
		{"Empty Bottle", GameItem::EmptyBottle},
		{"Red Potion", GameItem::RedPotion},
		{"Green Potion", GameItem::GreenPotion},
		{"Blue Potion", GameItem::BluePotion},
		{"Elixir Soup Half", GameItem::ElixirSoupHalf},
		{"Elixir Soup", GameItem::ElixirSoup},
		{"Bottled Water", GameItem::BottledWater},
		{"Fairy In Bottle", GameItem::FairyInBottle},
		{"Forest Firefly", GameItem::ForestFirefly},
		{"Forest Water", GameItem::ForestWater},
		{"FW Compass", GameItem::FWCompass},
		{"TotG Small Key", GameItem::TotGSmallKey},
		{"TotG Big Key", GameItem::TotGBigKey},
		{"TotG Dungeon Map", GameItem::TotGDungeonMap},
		{"TotG Compass", GameItem::TotGCompass},
		{"FF Dungeon Map", GameItem::FFDungeonMap},
		{"FF Compass", GameItem::FFCompass},
		{"Triforce Shard 1", GameItem::TriforceShard1},
		{"Triforce Shard 2", GameItem::TriforceShard2},
		{"Triforce Shard 3", GameItem::TriforceShard3},
		{"Triforce Shard 4", GameItem::TriforceShard4},
		{"Triforce Shard 5", GameItem::TriforceShard5},
		{"Triforce Shard 6", GameItem::TriforceShard6},
		{"Triforce Shard 7", GameItem::TriforceShard7},
		{"Triforce Shard 8", GameItem::TriforceShard8},
		{"Nayrus Pearl", GameItem::NayrusPearl},
		{"Dins Pearl", GameItem::DinsPearl},
		{"Farores Pearl", GameItem::FaroresPearl},
		{"Winds Requiem", GameItem::WindsRequiem},
		{"Ballad Of Gales", GameItem::BalladOfGales},
		{"Command Melody", GameItem::CommandMelody},
		{"Earth Gods Lyric", GameItem::EarthGodsLyric},
		{"Wind Gods Aria", GameItem::WindGodsAria},
		{"Song Of Passing", GameItem::SongOfPassing},
		{"ET Small Key", GameItem::ETSmallKey},
		{"ET Big Key", GameItem::ETBigKey},
		{"ET Dungeon Map", GameItem::ETDungeonMap},
		{"ET Compass", GameItem::ETCompass},
		{"Swift Sail", GameItem::SwiftSail},
		{"Progressive Sail", GameItem::ProgressiveSail},
		{"Triforce Chart 1 Deciphered", GameItem::TriforceChart1Deciphered},
		{"Triforce Chart 2 Deciphered", GameItem::TriforceChart2Deciphered},
		{"Triforce Chart 3 Deciphered", GameItem::TriforceChart3Deciphered},
		{"Triforce Chart 4 Deciphered", GameItem::TriforceChart4Deciphered},
		{"Triforce Chart 5 Deciphered", GameItem::TriforceChart5Deciphered},
		{"Triforce Chart 6 Deciphered", GameItem::TriforceChart6Deciphered},
		{"Triforce Chart 7 Deciphered", GameItem::TriforceChart7Deciphered},
		{"Triforce Chart 8 Deciphered", GameItem::TriforceChart8Deciphered},
		{"WT Small Key", GameItem::WTSmallKey},
		{"All Purpose Bait", GameItem::AllPurposeBait},
		{"Hyoi Pear", GameItem::HyoiPear},
		{"WT Big Key", GameItem::WTBigKey},
		{"WT Dungeon Map", GameItem::WTDungeonMap},
		{"WT Compass", GameItem::WTCompass},
		{"Town Flower", GameItem::TownFlower},
		{"Sea Flower", GameItem::SeaFlower},
		{"Exotic Flower", GameItem::ExoticFlower},
		{"Heros Flag", GameItem::HerosFlag},
		{"Big Catch Flag", GameItem::BigCatchFlag},
		{"Big Sale Flag", GameItem::BigSaleFlag},
		{"Pinwheel", GameItem::Pinwheel},
		{"Sickle Moon Flag", GameItem::SickleMoonFlag},
		{"Skull Tower Idol", GameItem::SkullTowerIdol},
		{"Fountain Idol", GameItem::FountainIdol},
		{"Postman Statue", GameItem::PostmanStatue},
		{"Shop Guru Statue", GameItem::ShopGuruStatue},
		{"Fathers Letter", GameItem::FathersLetter},
		{"Note To Mom", GameItem::NoteToMom},
		{"Maggies Letter", GameItem::MaggiesLetter},
		{"Moblins Letter", GameItem::MoblinsLetter},
		{"Cabana Deed", GameItem::CabanaDeed},
		{"Complimentary ID", GameItem::ComplimentaryID},
		{"Fill Up Coupon", GameItem::FillUpCoupon},
		{"Legendary Pictograph", GameItem::LegendaryPictograph},
		{"Dragon Tingle Statue", GameItem::DragonTingleStatue},
		{"Forbidden Tingle Statue", GameItem::ForbiddenTingleStatue},
		{"Goddess Tingle Statue", GameItem::GoddessTingleStatue},
		{"Earth Tingle Statue", GameItem::EarthTingleStatue},
		{"Wind Tingle Statue", GameItem::WindTingleStatue},
		{"Hurricane Spin", GameItem::HurricaneSpin},
		{"Progressive Wallet", GameItem::ProgressiveWallet},
		{"Five Thousand Wallet", GameItem::FiveThousandWallet},
		{"Progressive Bomb Bag", GameItem::ProgressiveBombBag},
		{"Ninety Nine Bomb Bag", GameItem::NinetyNineBombBag},
		{"Progressive Quiver", GameItem::ProgressiveQuiver},
		{"Ninety Nine Quiver", GameItem::NinetyNineQuiver},
		{"Magic Meter", GameItem::MagicMeter},
		{"Progressive Magic Meter", GameItem::ProgressiveMagicMeter},
		{"Fifty Rupees", GameItem::FiftyRupees},
		{"Hundred Rupees", GameItem::HundredRupees},
		{"Hundred Fifty Rupees", GameItem::HundredFiftyRupees},
		{"Two Hundred Rupees", GameItem::TwoHundredRupees},
		{"Two Hundred Fifty Rupees", GameItem::TwoHundredFiftyRupees},
		{"Rainbow Rupee", GameItem::RainbowRupee},
		{"Submarine Chart", GameItem::SubmarineChart},
		{"Beedles Chart", GameItem::BeedlesChart},
		{"Platform Chart", GameItem::PlatformChart},
		{"Light Ring Chart", GameItem::LightRingChart},
		{"Secret Cave Chart", GameItem::SecretCaveChart},
		{"Sea Hearts Chart", GameItem::SeaHeartsChart},
		{"Island Hearts Chart", GameItem::IslandHeartsChart},
		{"Great Fairy Chart", GameItem::GreatFairyChart},
		{"Octo Chart", GameItem::OctoChart},
		{"INcredible Chart", GameItem::INcredibleChart},
		{"Treasure Chart 7", GameItem::TreasureChart7},
		{"Treasure Chart 27", GameItem::TreasureChart27},
		{"Treasure Chart 21", GameItem::TreasureChart21},
		{"Treasure Chart 13", GameItem::TreasureChart13},
		{"Treasure Chart 32", GameItem::TreasureChart32},
		{"Treasure Chart 19", GameItem::TreasureChart19},
		{"Treasure Chart 41", GameItem::TreasureChart41},
		{"Treasure Chart 26", GameItem::TreasureChart26},
		{"Treasure Chart 8", GameItem::TreasureChart8},
		{"Treasure Chart 37", GameItem::TreasureChart37},
		{"Treasure Chart 25", GameItem::TreasureChart25},
		{"Treasure Chart 17", GameItem::TreasureChart17},
		{"Treasure Chart 36", GameItem::TreasureChart36},
		{"Treasure Chart 22", GameItem::TreasureChart22},
		{"Treasure Chart 9", GameItem::TreasureChart9},
		{"Ghost Ship Chart", GameItem::GhostShipChart},
		{"Tingles Chart", GameItem::TinglesChart},
		{"Treasure Chart 14", GameItem::TreasureChart14},
		{"Treasure Chart 10", GameItem::TreasureChart10},
		{"Treasure Chart 40", GameItem::TreasureChart40},
		{"Treasure Chart 3", GameItem::TreasureChart3},
		{"Treasure Chart 4", GameItem::TreasureChart4},
		{"Treasure Chart 28", GameItem::TreasureChart28},
		{"Treasure Chart 16", GameItem::TreasureChart16},
		{"Treasure Chart 18", GameItem::TreasureChart18},
		{"Treasure Chart 34", GameItem::TreasureChart34},
		{"Treasure Chart 29", GameItem::TreasureChart29},
		{"Treasure Chart 1", GameItem::TreasureChart1},
		{"Treasure Chart 35", GameItem::TreasureChart35},
		{"Treasure Chart 12", GameItem::TreasureChart12},
		{"Treasure Chart 6", GameItem::TreasureChart6},
		{"Treasure Chart 24", GameItem::TreasureChart24},
		{"Treasure Chart 39", GameItem::TreasureChart39},
		{"Treasure Chart 38", GameItem::TreasureChart38},
		{"Treasure Chart 2", GameItem::TreasureChart2},
		{"Treasure Chart 33", GameItem::TreasureChart33},
		{"Treasure Chart 31", GameItem::TreasureChart31},
		{"Treasure Chart 23", GameItem::TreasureChart23},
		{"Treasure Chart 5", GameItem::TreasureChart5},
		{"Treasure Chart 20", GameItem::TreasureChart20},
		{"Treasure Chart 30", GameItem::TreasureChart30},
		{"Treasure Chart 15", GameItem::TreasureChart15},
		{"Treasure Chart 11", GameItem::TreasureChart11},
		{"Treasure Chart 46", GameItem::TreasureChart46},
		{"Treasure Chart 45", GameItem::TreasureChart45},
		{"Treasure Chart 44", GameItem::TreasureChart44},
		{"Triforce Chart 3", GameItem::TriforceChart3},
		{"Treasure Chart 43", GameItem::TreasureChart43},
		{"Triforce Chart 2", GameItem::TriforceChart2},
		{"Treasure Chart 42", GameItem::TreasureChart42},
		{"Triforce Chart 1", GameItem::TriforceChart1},
		{"Game Beatable", GameItem::GameBeatable},
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
		{GameItem::HeartDrop, "Heart Drop"},
		{GameItem::GreenRupee, "Green Rupee"},
		{GameItem::BlueRupee, "Blue Rupee"},
		{GameItem::YellowRupee, "Yellow Rupee"},
		{GameItem::RedRupee, "Red Rupee"},
		{GameItem::PurpleRupee, "Purple Rupee"},
		{GameItem::OrangeRupee, "Orange Rupee"},
		{GameItem::PieceOfHeart, "Piece Of Heart"},
		{GameItem::HeartContainer, "Heart Container"},
		{GameItem::SmallMagicDrop, "Small Magic Drop"},
		{GameItem::LargeMagicDrop, "Large Magic Drop"},
		{GameItem::FiveBombs, "Five Bombs"},
		{GameItem::TenBombs, "Ten Bombs"},
		{GameItem::TwentyBombs, "Twenty Bombs"},
		{GameItem::ThirtyBombs, "Thirty Bombs"},
		{GameItem::SilverRupee, "Silver Rupee"},
		{GameItem::TenArrows, "Ten Arrows"},
		{GameItem::TwentyArrows, "Twenty Arrows"},
		{GameItem::ThirtyArrows, "Thirty Arrows"},
		{GameItem::DRCSmallKey, "DRC Small Key"},
		{GameItem::DRCBigKey, "DRC Big Key"},
		{GameItem::SmallKey, "Small Key"},
		{GameItem::Fairy, "Fairy"},
		{GameItem::YellowRupee2, "Yellow Rupee 2"},
		{GameItem::DRCDungeonMap, "DRC Dungeon Map"},
		{GameItem::DRCCompass, "DRC Compass"},
		{GameItem::FWSmallKey, "FW Small Key"},
		{GameItem::ThreeHearts, "Three Hearts"},
		{GameItem::JoyPendant, "Joy Pendant"},
		{GameItem::Telescope, "Telescope"},
		{GameItem::TingleBottle, "Tingle Bottle"},
		{GameItem::WindWaker, "Wind Waker"},
		{GameItem::ProgressivePictoBox, "Progressive Picto Box"},
		{GameItem::SpoilsBag, "Spoils Bag"},
		{GameItem::GrapplingHook, "Grappling Hook"},
		{GameItem::DeluxePicto, "Deluxe Picto Box"},
		{GameItem::ProgressiveBow, "Progressive Bow"},
		{GameItem::PowerBracelets, "Power Bracelets"},
		{GameItem::IronBoots, "Iron Boots"},
		{GameItem::MagicArmor, "Magic Armor"},
		{GameItem::BaitBag, "Bait Bag"},
		{GameItem::Boomerang, "Boomerang"},
		{GameItem::Hookshot, "Hookshot"},
		{GameItem::DeliveryBag, "Delivery Bag"},
		{GameItem::Bombs, "Bombs"},
		{GameItem::HerosClothes, "Heros Clothes"},
		{GameItem::SkullHammer, "Skull Hammer"},
		{GameItem::DekuLeaf, "Deku Leaf"},
		{GameItem::FireIceArrows, "Fire Ice Arrows"},
		{GameItem::LightArrow, "Light Arrow"},
		{GameItem::HerosNewClothes, "Heros New Clothes"},
		{GameItem::ProgressiveSword, "Progressive Sword"},
		{GameItem::MasterSwordPowerless, "Master Sword Powerless"},
		{GameItem::MasterSwordHalf, "Master Sword Half"},
		{GameItem::ProgressiveShield, "Progressive Shield"},
		{GameItem::MirrorShield, "Mirror Shield"},
		{GameItem::RecoveredHerosSword, "Recovered Heros Sword"},
		{GameItem::MasterSwordFull, "Master Sword Full"},
		{GameItem::PieceOfHeart2, "Piece Of Heart 2"},
		{GameItem::FWBigKey, "FW Big Key"},
		{GameItem::FWDungeonMap, "FW Dungeon Map"},
		{GameItem::PiratesCharm, "Pirates Charm"},
		{GameItem::HerosCharm, "Heros Charm"},
		{GameItem::SkullNecklace, "Skull Necklace"},
		{GameItem::BokoBabaSeed, "Boko Baba Seed"},
		{GameItem::GoldenFeather, "Golden Feather"},
		{GameItem::KnightsCrest, "Knights Crest"},
		{GameItem::RedChuJelly, "Red Chu Jelly"},
		{GameItem::GreenChuJelly, "Green Chu Jelly"},
		{GameItem::BlueChuJelly, "Blue Chu Jelly"},
		{GameItem::DungeonMap, " Dungeon Map"},
		{GameItem::Compass, "Compass"},
		{GameItem::BigKey, "Big Key"},
		{GameItem::EmptyBottle, "Empty Bottle"},
		{GameItem::RedPotion, "Red Potion"},
		{GameItem::GreenPotion, "Green Potion"},
		{GameItem::BluePotion, "Blue Potion"},
		{GameItem::ElixirSoupHalf, "Elixir Soup Half"},
		{GameItem::ElixirSoup, "Elixir Soup"},
		{GameItem::BottledWater, "Bottled Water"},
		{GameItem::FairyInBottle, "Fairy In Bottle"},
		{GameItem::ForestFirefly, "Forest Firefly"},
		{GameItem::ForestWater, "Forest Water"},
		{GameItem::FWCompass, "FW Compass"},
		{GameItem::TotGSmallKey, "TotG Small Key"},
		{GameItem::TotGBigKey, "TotG Big Key"},
		{GameItem::TotGDungeonMap, "TotG Dungeon Map"},
		{GameItem::TotGCompass, "TotG Compass"},
		{GameItem::FFDungeonMap, "FF Dungeon Map"},
		{GameItem::FFCompass, "FF Compass"},
		{GameItem::TriforceShard1, "Triforce Shard 1"},
		{GameItem::TriforceShard2, "Triforce Shard 2"},
		{GameItem::TriforceShard3, "Triforce Shard 3"},
		{GameItem::TriforceShard4, "Triforce Shard 4"},
		{GameItem::TriforceShard5, "Triforce Shard 5"},
		{GameItem::TriforceShard6, "Triforce Shard 6"},
		{GameItem::TriforceShard7, "Triforce Shard 7"},
		{GameItem::TriforceShard8, "Triforce Shard 8"},
		{GameItem::NayrusPearl, "Nayrus Pearl"},
		{GameItem::DinsPearl, "Dins Pearl"},
		{GameItem::FaroresPearl, "Farores Pearl"},
		{GameItem::WindsRequiem, "Winds Requiem"},
		{GameItem::BalladOfGales, "Ballad Of Gales"},
		{GameItem::CommandMelody, "Command Melody"},
		{GameItem::EarthGodsLyric, "Earth Gods Lyric"},
		{GameItem::WindGodsAria, "Wind Gods Aria"},
		{GameItem::SongOfPassing, "Song Of Passing"},
		{GameItem::ETSmallKey, "ET Small Key"},
		{GameItem::ETBigKey, "ET Big Key"},
		{GameItem::ETDungeonMap, "ET Dungeon Map"},
		{GameItem::ETCompass, "ET Compass"},
		{GameItem::SwiftSail, "Swift Sail"},
		{GameItem::ProgressiveSail, "Progressive Sail"},
		{GameItem::TriforceChart1Deciphered, "Triforce Chart 1 Deciphered"},
		{GameItem::TriforceChart2Deciphered, "Triforce Chart 2 Deciphered"},
		{GameItem::TriforceChart3Deciphered, "Triforce Chart 3 Deciphered"},
		{GameItem::TriforceChart4Deciphered, "Triforce Chart 4 Deciphered"},
		{GameItem::TriforceChart5Deciphered, "Triforce Chart 5 Deciphered"},
		{GameItem::TriforceChart6Deciphered, "Triforce Chart 6 Deciphered"},
		{GameItem::TriforceChart7Deciphered, "Triforce Chart 7 Deciphered"},
		{GameItem::TriforceChart8Deciphered, "Triforce Chart 8 Deciphered"},
		{GameItem::WTSmallKey, "WT Small Key"},
		{GameItem::AllPurposeBait, "All Purpose Bait"},
		{GameItem::HyoiPear, "Hyoi Pear"},
		{GameItem::WTBigKey, "WT Big Key"},
		{GameItem::WTDungeonMap, "WT Dungeon Map"},
		{GameItem::WTCompass, "WT Compass"},
		{GameItem::TownFlower, "Town Flower"},
		{GameItem::SeaFlower, "Sea Flower"},
		{GameItem::ExoticFlower, "Exotic Flower"},
		{GameItem::HerosFlag, "Heros Flag"},
		{GameItem::BigCatchFlag, "Big Catch Flag"},
		{GameItem::BigSaleFlag, "Big Sale Flag"},
		{GameItem::Pinwheel, "Pinwheel"},
		{GameItem::SickleMoonFlag, "Sickle Moon Flag"},
		{GameItem::SkullTowerIdol, "Skull Tower Idol"},
		{GameItem::FountainIdol, "Fountain Idol"},
		{GameItem::PostmanStatue, "Postman Statue"},
		{GameItem::ShopGuruStatue, "Shop Guru Statue"},
		{GameItem::FathersLetter, "Fathers Letter"},
		{GameItem::NoteToMom, "Note To Mom"},
		{GameItem::MaggiesLetter, "Maggies Letter"},
		{GameItem::MoblinsLetter, "Moblins Letter"},
		{GameItem::CabanaDeed, "Cabana Deed"},
		{GameItem::ComplimentaryID, "Complimentary ID"},
		{GameItem::FillUpCoupon, "Fill Up Coupon"},
		{GameItem::LegendaryPictograph, "Legendary Pictograph"},
		{GameItem::DragonTingleStatue, "Dragon Tingle Statue"},
		{GameItem::ForbiddenTingleStatue, "Forbidden Tingle Statue"},
		{GameItem::GoddessTingleStatue, "Goddess Tingle Statue"},
		{GameItem::EarthTingleStatue, "Earth Tingle Statue"},
		{GameItem::WindTingleStatue, "Wind Tingle Statue"},
		{GameItem::HurricaneSpin, "Hurricane Spin"},
		{GameItem::ProgressiveWallet, "Progressive Wallet"},
		{GameItem::FiveThousandWallet, "Five Thousand Wallet"},
		{GameItem::ProgressiveBombBag, "Progressive Bomb Bag"},
		{GameItem::NinetyNineBombBag, "Ninety Nine Bomb Bag"},
		{GameItem::ProgressiveQuiver, "Progressive Quiver"},
		{GameItem::NinetyNineQuiver, "Ninety Nine Quiver"},
		{GameItem::MagicMeter, "Magic Meter"},
		{GameItem::ProgressiveMagicMeter, "Progressive Magic Meter"},
		{GameItem::FiftyRupees, "Fifty Rupees"},
		{GameItem::HundredRupees, "Hundred Rupees"},
		{GameItem::HundredFiftyRupees, "Hundred Fifty Rupees"},
		{GameItem::TwoHundredRupees, "Two Hundred Rupees"},
		{GameItem::TwoHundredFiftyRupees, "Two Hundred Fifty Rupees"},
		{GameItem::RainbowRupee, "Rainbow Rupee"},
		{GameItem::SubmarineChart, "Submarine Chart"},
		{GameItem::BeedlesChart, "Beedles Chart"},
		{GameItem::PlatformChart, "Platform Chart"},
		{GameItem::LightRingChart, "Light Ring Chart"},
		{GameItem::SecretCaveChart, "Secret Cave Chart"},
		{GameItem::SeaHeartsChart, "Sea Hearts Chart"},
		{GameItem::IslandHeartsChart, "Island Hearts Chart"},
		{GameItem::GreatFairyChart, "Great Fairy Chart"},
		{GameItem::OctoChart, "Octo Chart"},
		{GameItem::INcredibleChart, "INcredible Chart"},
		{GameItem::TreasureChart7, "Treasure Chart 7"},
		{GameItem::TreasureChart27, "Treasure Chart 27"},
		{GameItem::TreasureChart21, "Treasure Chart 21"},
		{GameItem::TreasureChart13, "Treasure Chart 13"},
		{GameItem::TreasureChart32, "Treasure Chart 32"},
		{GameItem::TreasureChart19, "Treasure Chart 19"},
		{GameItem::TreasureChart41, "Treasure Chart 41"},
		{GameItem::TreasureChart26, "Treasure Chart 26"},
		{GameItem::TreasureChart8, "Treasure Chart 8"},
		{GameItem::TreasureChart37, "Treasure Chart 37"},
		{GameItem::TreasureChart25, "Treasure Chart 25"},
		{GameItem::TreasureChart17, "Treasure Chart 17"},
		{GameItem::TreasureChart36, "Treasure Chart 36"},
		{GameItem::TreasureChart22, "Treasure Chart 22"},
		{GameItem::TreasureChart9, "Treasure Chart 9"},
		{GameItem::GhostShipChart, "Ghost Ship Chart"},
		{GameItem::TinglesChart, "Tingles Chart"},
		{GameItem::TreasureChart14, "Treasure Chart 14"},
		{GameItem::TreasureChart10, "Treasure Chart 10"},
		{GameItem::TreasureChart40, "Treasure Chart 40"},
		{GameItem::TreasureChart3, "Treasure Chart 3"},
		{GameItem::TreasureChart4, "Treasure Chart 4"},
		{GameItem::TreasureChart28, "Treasure Chart 28"},
		{GameItem::TreasureChart16, "Treasure Chart 16"},
		{GameItem::TreasureChart18, "Treasure Chart 18"},
		{GameItem::TreasureChart34, "Treasure Chart 34"},
		{GameItem::TreasureChart29, "Treasure Chart 29"},
		{GameItem::TreasureChart1, "Treasure Chart 1"},
		{GameItem::TreasureChart35, "Treasure Chart 35"},
		{GameItem::TreasureChart12, "Treasure Chart 12"},
		{GameItem::TreasureChart6, "Treasure Chart 6"},
		{GameItem::TreasureChart24, "Treasure Chart 24"},
		{GameItem::TreasureChart39, "Treasure Chart 39"},
		{GameItem::TreasureChart38, "Treasure Chart 38"},
		{GameItem::TreasureChart2, "Treasure Chart 2"},
		{GameItem::TreasureChart33, "Treasure Chart 33"},
		{GameItem::TreasureChart31, "Treasure Chart 31"},
		{GameItem::TreasureChart23, "Treasure Chart 23"},
		{GameItem::TreasureChart5, "Treasure Chart 5"},
		{GameItem::TreasureChart20, "Treasure Chart 20"},
		{GameItem::TreasureChart30, "Treasure Chart 30"},
		{GameItem::TreasureChart15, "Treasure Chart 15"},
		{GameItem::TreasureChart11, "Treasure Chart 11"},
		{GameItem::TreasureChart46, "Treasure Chart 46"},
		{GameItem::TreasureChart45, "Treasure Chart 45"},
		{GameItem::TreasureChart44, "Treasure Chart 44"},
		{GameItem::TriforceChart3, "Triforce Chart 3"},
		{GameItem::TreasureChart43, "Treasure Chart 43"},
		{GameItem::TriforceChart2, "Triforce Chart 2"},
		{GameItem::TreasureChart42, "Treasure Chart 42"},
		{GameItem::TriforceChart1, "Triforce Chart 1"},
		{GameItem::GameBeatable, "Game Beatable"},
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
		else if (dungeonItems.count(gameItemId) > 0)
		{
				dungeonItem = true;
		}
}

Item::Item(std::string itemName_, int worldId_)
{
		gameItemId = nameToGameItem(itemName_);
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
		else if (dungeonItems.count(gameItemId) > 0)
		{
				dungeonItem = true;
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
		junkItem = false;
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

void Item::addChainLocation(Location* location)
{
		LOG_TO_DEBUG("Yes, we made it");
		chainLocations.push_back(location);
}

std::list<Location*>& Item::getChainLocations()
{
		return chainLocations;
}

void Item::setAsJunkItem()
{
		majorItem = false;
		junkItem = true;
}

bool Item::isJunkItem() const
{
		return junkItem;
}

bool Item::isDungeonItem() const
{
		return dungeonItem;
}

bool Item::operator==(const Item& rhs) const
{
		return gameItemId == rhs.gameItemId && worldId == rhs.worldId;
}

bool Item::operator<(const Item& rhs) const
{
		return (worldId == rhs.worldId) ? gameItemId < rhs.gameItemId : worldId < rhs.worldId;
}
