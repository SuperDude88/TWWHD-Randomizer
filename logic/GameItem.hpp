#pragma once

#include <string>
#include <set>
#include <unordered_set>
#include <list>
#include <unordered_map>
#include <utility/text.hpp>

enum struct GameItem : uint8_t
{
    HeartDrop = 0x00,
    GreenRupee,
    BlueRupee,
    YellowRupee,
    RedRupee,
    PurpleRupee,
    OrangeRupee,
    PieceOfHeart,
    HeartContainer,
    SmallMagicDrop,
    LargeMagicDrop,
    FiveBombs,
    TenBombs,
    TwentyBombs,
    ThirtyBombs,
    SilverRupee,
    TenArrows,
    TwentyArrows,
    ThirtyArrows,
    DRCSmallKey,
    DRCBigKey,
    SmallKey,
    Fairy,

    NOTHING, //not an item, uses a free space to represent no item (but not invalid)
    GameBeatable, // Dummy item to check for game beatability
    HINT, // Dummy item to represent placing a hint at a hint location

    YellowRupee2 = 0x1A, //joke message
    DRCDungeonMap,
    DRCCompass,
    FWSmallKey,
    ThreeHearts,
    JoyPendant,
    Telescope,
    TingleBottle,
    WindWaker,
    ProgressivePictoBox,
    SpoilsBag,
    GrapplingHook,
    DeluxePicto,
    ProgressiveBow,
    PowerBracelets,
    IronBoots,
    MagicArmor,

    BaitBag = 0x2C,
    Boomerang,

    Hookshot = 0x2F,
    DeliveryBag,
    Bombs,
    HerosClothes,
    SkullHammer,
    DekuLeaf,
    FireIceArrows,
    LightArrow,
    HerosNewClothes,
    ProgressiveSword,
    MasterSwordPowerless,
    MasterSwordHalf,
    ProgressiveShield,
    MirrorShield,
    RecoveredHerosSword,
    MasterSwordFull,
    PieceOfHeart2, //alternate message
    FWBigKey,
    FWDungeonMap,
    PiratesCharm,
    HerosCharm,

    SkullNecklace = 0x45,
    BokoBabaSeed,
    GoldenFeather,
    KnightsCrest,
    RedChuJelly,
    GreenChuJelly,
    BlueChuJelly,
    DungeonMap,
    Compass,
    BigKey,

    EmptyBottle = 0x50,
    RedPotion,
    GreenPotion,
    BluePotion,
    ElixirSoupHalf,
    ElixirSoup,
    BottledWater,
    FairyInBottle,
    ForestFirefly,
    ForestWater,
    FWCompass,
    TotGSmallKey,
    TotGBigKey,
    TotGDungeonMap,
    TotGCompass,
    FFDungeonMap,
    FFCompass,
    TriforceShard1,
    TriforceShard2,
    TriforceShard3,
    TriforceShard4,
    TriforceShard5,
    TriforceShard6,
    TriforceShard7,
    TriforceShard8,
    NayrusPearl,
    DinsPearl,
    FaroresPearl,

    WindsRequiem = 0x6D,
    BalladOfGales,
    CommandMelody,
    EarthGodsLyric,
    WindGodsAria,
    SongOfPassing,
    ETSmallKey,
    ETBigKey,
    ETDungeonMap,
    ETCompass,
    SwiftSail,
    ProgressiveSail,
    TriforceChart1Deciphered,
    TriforceChart2Deciphered,
    TriforceChart3Deciphered,
    TriforceChart4Deciphered,
    TriforceChart5Deciphered,
    TriforceChart6Deciphered,
    TriforceChart7Deciphered,
    TriforceChart8Deciphered,
    WTSmallKey,
    AllPurposeBait,
    HyoiPear,
    WTBigKey,
    WTDungeonMap,
    WTCompass,

    TownFlower = 0x8C,
    SeaFlower,
    ExoticFlower,
    HerosFlag,
    BigCatchFlag,
    BigSaleFlag,
    Pinwheel,
    SickleMoonFlag,
    SkullTowerIdol,
    FountainIdol,
    PostmanStatue,
    ShopGuruStatue,
    FathersLetter,
    NoteToMom,
    MaggiesLetter,
    MoblinsLetter,
    CabanaDeed,
    ComplimentaryID,
    FillUpCoupon,
    LegendaryPictograph,

    DragonTingleStatue = 0xA3,
    ForbiddenTingleStatue,
    GoddessTingleStatue,
    EarthTingleStatue,
    WindTingleStatue,

    HurricaneSpin = 0xAA,
    ProgressiveWallet,
    FiveThousandWallet,
    ProgressiveBombBag,
    NinetyNineBombBag,
    ProgressiveQuiver,
    NinetyNineQuiver,
    MagicMeter, //Added by rando
    ProgressiveMagicMeter,  //Replaces magic upgrade
    FiftyRupees,
    HundredRupees,
    HundredFiftyRupees,
    TwoHundredRupees,
    TwoHundredFiftyRupees,
    RainbowRupee,

    SubmarineChart = 0xC2,
    BeedlesChart,
    PlatformChart,
    LightRingChart,
    SecretCaveChart,
    SeaHeartsChart,
    IslandHeartsChart,
    GreatFairyChart,
    OctoChart,
    INcredibleChart,
    TreasureChart7,
    TreasureChart27,
    TreasureChart21,
    TreasureChart13,
    TreasureChart32,
    TreasureChart19,
    TreasureChart41,
    TreasureChart26,
    TreasureChart8,
    TreasureChart37,
    TreasureChart25,
    TreasureChart17,
    TreasureChart36,
    TreasureChart22,
    TreasureChart9,
    GhostShipChart,
    TinglesChart,
    TreasureChart14,
    TreasureChart10,
    TreasureChart40,
    TreasureChart3,
    TreasureChart4,
    TreasureChart28,
    TreasureChart16,
    TreasureChart18,
    TreasureChart34,
    TreasureChart29,
    TreasureChart1,
    TreasureChart35,
    TreasureChart12,
    TreasureChart6,
    TreasureChart24,
    TreasureChart39,
    TreasureChart38,
    TreasureChart2,
    TreasureChart33,
    TreasureChart31,
    TreasureChart23,
    TreasureChart5,
    TreasureChart20,
    TreasureChart30,
    TreasureChart15,
    TreasureChart11,
    TreasureChart46,
    TreasureChart45,
    TreasureChart44,
    TriforceChart3,
    TreasureChart43,
    TriforceChart2,
    TreasureChart42,
    TriforceChart1,

    INVALID
};

GameItem nameToGameItem(const std::string& name);

std::string gameItemToName(GameItem item);

GameItem idToGameItem(uint8_t id);

uint32_t maxItemCount(GameItem item);

std::unordered_multiset<GameItem> getSupportedStartingItems();

static const std::set<GameItem> junkItems = {
    GameItem::HeartDrop,
    GameItem::GreenRupee,
    GameItem::BlueRupee,
    GameItem::YellowRupee,
    GameItem::RedRupee,
    GameItem::PurpleRupee,
    GameItem::OrangeRupee,
    GameItem::PieceOfHeart,
    GameItem::HeartContainer,
    GameItem::SmallMagicDrop,
    GameItem::LargeMagicDrop,
    GameItem::FiveBombs,
    GameItem::TenBombs,
    GameItem::TwentyBombs,
    GameItem::ThirtyBombs,
    GameItem::SilverRupee,
    GameItem::TenArrows,
    GameItem::TwentyArrows,
    GameItem::ThirtyArrows,
    GameItem::Fairy,
    GameItem::YellowRupee2, //joke message
    GameItem::DRCDungeonMap,
    GameItem::DRCCompass,
    GameItem::ThreeHearts,
    GameItem::JoyPendant,
    GameItem::Telescope,
    GameItem::TingleBottle,
    GameItem::MagicArmor,
    GameItem::HerosClothes,
    GameItem::HerosNewClothes,
    GameItem::PieceOfHeart2, //alternate message
    GameItem::FWDungeonMap,
    GameItem::PiratesCharm,
    GameItem::HerosCharm,
    GameItem::SkullNecklace,
    GameItem::BokoBabaSeed,
    GameItem::GoldenFeather,
    GameItem::KnightsCrest,
    GameItem::RedChuJelly,
    GameItem::GreenChuJelly,
    // GameItem::BlueChuJelly,
    GameItem::DungeonMap,
    GameItem::Compass,
    GameItem::FWCompass,
    GameItem::TotGDungeonMap,
    GameItem::TotGCompass,
    GameItem::FFDungeonMap,
    GameItem::FFCompass,
    GameItem::ETDungeonMap,
    GameItem::ETCompass,
    GameItem::AllPurposeBait,
    GameItem::HyoiPear,
    GameItem::WTDungeonMap,
    GameItem::WTCompass,
    GameItem::ComplimentaryID,
    GameItem::FillUpCoupon,
    GameItem::LegendaryPictograph,
    GameItem::HurricaneSpin,
    GameItem::FiftyRupees,
    GameItem::HundredRupees,
    GameItem::HundredFiftyRupees,
    GameItem::TwoHundredRupees,
    GameItem::TwoHundredFiftyRupees,
    GameItem::RainbowRupee,
    GameItem::SubmarineChart,
    GameItem::BeedlesChart,
    GameItem::PlatformChart,
    GameItem::LightRingChart,
    GameItem::SecretCaveChart,
    GameItem::SeaHeartsChart,
    GameItem::IslandHeartsChart,
    GameItem::GreatFairyChart,
    GameItem::OctoChart,
    GameItem::INcredibleChart,
    GameItem::TinglesChart
};

static const std::set<GameItem> dungeonItems = {
    GameItem::DRCSmallKey,
    GameItem::DRCBigKey,
    GameItem::DRCDungeonMap,
    GameItem::DRCCompass,
    GameItem::FWSmallKey,
    GameItem::FWBigKey,
    GameItem::FWDungeonMap,
    GameItem::FWCompass,
    GameItem::TotGSmallKey,
    GameItem::TotGBigKey,
    GameItem::TotGDungeonMap,
    GameItem::TotGCompass,
    GameItem::FFDungeonMap,
    GameItem::FFCompass,
    GameItem::ETSmallKey,
    GameItem::ETBigKey,
    GameItem::ETDungeonMap,
    GameItem::ETCompass,
    GameItem::WTSmallKey,
    GameItem::WTBigKey,
    GameItem::WTDungeonMap,
    GameItem::WTCompass,
};

struct Location;
class World;
class Item
{
public:
    Item() = default;
    Item(GameItem gameItemId_, World* world_);
    Item(std::string itemName_, World* world_);

    World* getWorld();
    int getWorldId() const;
    void setGameItemId(GameItem newGameItemId);
    GameItem getGameItemId() const;
    void setDelayedItemId(GameItem delayedItemId);
    void saveDelayedItemId();
    void setAsMajorItem();
    bool isMajorItem() const;
    bool isChartForSunkenTreasure() const;
    void addChainLocation(Location* location);
    std::list<Location*>& getChainLocations();
    std::string getName() const;
    std::string getUTF8Name(const std::string& language = "English", const Text::Type& type = Text::Type::STANDARD, const Text::Color& color = Text::Color::RAW, const bool& showWorld = false) const;
    std::u16string getUTF16Name(const std::string& language = "English", const Text::Type& type = Text::Type::STANDARD, const Text::Color& color = Text::Color::RED, const bool& showWorld = false) const;
    void setName(const std::string& language, const Text::Type& type, const std::string& name_);
    void setAsJunkItem();
    bool isJunkItem() const;
    bool isDungeonItem() const;
    bool isMap() const;
    bool isCompass() const;
    bool isSmallKey() const;
    bool isBigKey() const;
    bool operator==(const Item& rhs) const;
    bool operator<(const Item& rhs) const;

private:
    GameItem gameItemId = GameItem::INVALID;
    GameItem delayedGameItemId = GameItem::INVALID;
    bool majorItem = false;
    bool chartForSunkenTreasure = false;
    std::list<Location*> chainLocations = {};
    bool dungeonItem = false;
    bool junkItem = false;
    World* world = nullptr; // The world that this item is *FOR*
};

// Hash function for Item class, copied from
// https://en.cppreference.com/w/cpp/utility/hash
template<>
struct std::hash<Item>
{
    size_t operator()(Item const& i) const noexcept
    {
        size_t h1 = std::hash<GameItem>{}(i.getGameItemId());
        size_t h2 = std::hash<int>{}(i.getWorldId());
        return h1 ^ (h2 << 1);
    }
};
