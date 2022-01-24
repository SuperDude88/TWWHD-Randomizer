#pragma once

#include <string>

// TODO: go back and make the values here match item id's in game?
enum struct GameItem
{
    INVALID = 0,
    GreenRupee,
    BlueRupee,
    YellowRupee,
    RedRupee,
    PurpleRupee,
    OrangeRupee,
    PieceOfHeart,
    HeartContainer,
    WindWaker,
    SpoilsBag,
    HurricaneSpin,
    GrapplingHook,
    PowerBracelets,
    IronBoots,
    BaitBag,
    Boomerang,
    Hookshot,
    DeliveryBag,
    Bombs,
    SkullHammer,
    DekuLeaf,
    TriforceShard,
    NayrusPearl,
    DinsPearl,
    FaroresPearl,
    WindsRequiem,
    BalladOfGales,
    CommandMelody,
    EarthGodsLyric,
    WindGodsAria,
    SongofPassing,
    Sail,
    NoteToMom,
    MaggiesLetter,
    MoblinsLetter,
    CabanaDeed,
    DragonTingleStatue,
    ForbiddenTingleStatue,
    GoddessTingleStatue,
    EarthTingleStatue,
    WindTingleStatue,
    BombBagUpgrade,
    QuiverUpgrade,
    MagicMeter,
    GhostShipChart,
    Sword,
    Shield,
    Bow,
    Wallet,
    PictoBox,
    DeluxePictoBox,
    Bottle,
    DRCSmallKey,
    DRCBigKey,
    FWSmallKey,
    FWBigKey,
    TOTGSmallKey,
    TOTGBigKey,
    ETSmallKey,
    ETBigKey,
    WTSmallKey,
    WTBigKey,
    TriforceChart1,
    TriforceChart2,
    TriforceChart3,
    TreasureChart1,
    TreasureChart2,
    TreasureChart3,
    TreasureChart4,
    TreasureChart5,
    TreasureChart6,
    TreasureChart7,
    TreasureChart8,
    TreasureChart9,
    TreasureChart10,
    TreasureChart11,
    TreasureChart12,
    TreasureChart13,
    TreasureChart14,
    TreasureChart15,
    TreasureChart16,
    TreasureChart17,
    TreasureChart18,
    TreasureChart19,
    TreasureChart20,
    TreasureChart21,
    TreasureChart22,
    TreasureChart23,
    TreasureChart24,
    TreasureChart25,
    TreasureChart26,
    TreasureChart27,
    TreasureChart28,
    TreasureChart29,
    TreasureChart30,
    TreasureChart31,
    TreasureChart32,
    TreasureChart33,
    TreasureChart34,
    TreasureChart35,
    TreasureChart36,
    TreasureChart37,
    TreasureChart38,
    TreasureChart39,
    TreasureChart40,
    TreasureChart41,
    TreasureChart42,
    TreasureChart43,
    TreasureChart44,
    TreasureChart45,
    TreasureChart46,
    Nothing,
    GAME_ITEM_COUNT
};

GameItem nameToGameItem(const std::string& name);

std::string gameItemToName(GameItem item);

GameItem indexToGameItem(uint32_t index);

uint32_t maxItemCount(GameItem item);

class Item
{
public:
    Item() = default;
    Item(GameItem gameItemId_, int worldId_);

    void setWorldId(int newWorldId);
    int getWorldId() const;
    GameItem getGameItemId() const;
    std::string getName() const;
    bool operator==(const Item& rhs) const;
    bool isMajorItem() const;

private:
    GameItem gameItemId = GameItem::INVALID;
    int worldId = -1; // The world that this item is *FOR*
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
