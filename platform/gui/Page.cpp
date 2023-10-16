#include "Page.hpp"

#include <platform/gui/screen.hpp>
#include <platform/gui/TextWrap.hpp>

SeedPage::SeedPage() {}

void SeedPage::open() {}

bool SeedPage::update(const VPADStatus& stat) {
    if (stat.trigger & VPAD_BUTTON_A) {
        OptionCB::changeSeed();
        return true;
    }
    
    return false;
}

void SeedPage::drawTV() const {
    OSScreenPutFontEx(SCREEN_TV, 0, 3, ("The current seed is \"" + getSeed() + "\". Hash: " + getSeedHash()).c_str());
    OSScreenPutFontEx(SCREEN_TV, 0, 4, "Press A to generate a new seed");
}

void SeedPage::drawDRC() const {
    const std::vector<std::string>& descLines = wrap_string(getDesc(), ScreenSizeData::drc_line_length);

    const size_t startLine = ScreenSizeData::drc_num_lines - descLines.size();
    for(size_t i = 0; i < descLines.size(); i++) {
        OSScreenPutFontEx(SCREEN_DRC, 0, startLine + i, descLines[i].c_str());
    }
}



ProgressionPage::ProgressionPage() {
    using namespace std::literals::chrono_literals;

    buttonColumns[0][0] = std::make_unique<BasicButton>(Option::ProgressDungeons);
    buttonColumns[0][1] = std::make_unique<BasicButton>(Option::ProgressGreatFairies);
    buttonColumns[0][2] = std::make_unique<BasicButton>(Option::ProgressPuzzleCaves);
    buttonColumns[0][3] = std::make_unique<BasicButton>(Option::ProgressCombatCaves);
    buttonColumns[0][4] = std::make_unique<BasicButton>(Option::ProgressShortSidequests);
    buttonColumns[0][5] = std::make_unique<BasicButton>(Option::ProgressLongSidequests);
    buttonColumns[0][6] = std::make_unique<BasicButton>(Option::ProgressSpoilsTrading);
    buttonColumns[0][7] = std::make_unique<BasicButton>(Option::ProgressMinigames);
    buttonColumns[0][8] = std::make_unique<BasicButton>(Option::ProgressFreeGifts);
    buttonColumns[0][9] = std::make_unique<BasicButton>(Option::ProgressMail);
    buttonColumns[0][10] = std::make_unique<BasicButton>(Option::ProgressPlatformsRafts);
    buttonColumns[0][11] = std::make_unique<BasicButton>(Option::ProgressSubmarines);
    buttonColumns[0][12] = std::make_unique<BasicButton>(Option::ProgressEyeReefs);
    buttonColumns[0][13] = std::make_unique<BasicButton>(Option::ProgressOctosGunboats);
    buttonColumns[0][14] = std::make_unique<BasicButton>(Option::ProgressTriforceCharts);

    buttonColumns[1][0] = std::make_unique<BasicButton>(Option::ProgressTreasureCharts);
    buttonColumns[1][1] = std::make_unique<BasicButton>(Option::ProgressExpPurchases);
    buttonColumns[1][2] = std::make_unique<BasicButton>(Option::ProgressMisc);
    buttonColumns[1][3] = std::make_unique<BasicButton>(Option::ProgressTingleChests);
    buttonColumns[1][4] = std::make_unique<BasicButton>(Option::ProgressBattlesquid);
    buttonColumns[1][5] = std::make_unique<BasicButton>(Option::ProgressSavageLabyrinth);
    buttonColumns[1][6] = std::make_unique<BasicButton>(Option::ProgressIslandPuzzles);
    buttonColumns[1][7] = std::make_unique<BasicButton>(Option::ProgressDungeonSecrets);
    buttonColumns[1][8] = std::make_unique<BasicButton>(Option::ProgressObscure);
    buttonColumns[1][9] = std::make_unique<BasicButton>(Option::DungeonSmallKeys);
    buttonColumns[1][10] = std::make_unique<BasicButton>(Option::DungeonBigKeys);
    buttonColumns[1][11] = std::make_unique<BasicButton>(Option::DungeonMapsAndCompasses);
    buttonColumns[1][12] = std::make_unique<CounterButton>(Option::NumRequiredDungeons, 250ms, 300ms);
    buttonColumns[1][13] = std::make_unique<CounterButton>(Option::NumShards, 250ms, 300ms);
    buttonColumns[1][14] = std::make_unique<BasicButton>(Option::SwordMode);
}

void ProgressionPage::open() {
    curCol = 0;
    curRow = 0;
}

bool ProgressionPage::update(const VPADStatus& stat) {
    bool moved = false;

    if(stat.trigger & VPAD_BUTTON_LEFT) {
        if(curCol <= 0) {
            curCol = buttonColumns.size() - 1; //wrap on leftmost row
        }
        else {
            curCol -= 1; //left one row
        }
        moved = true;
    }
    else if(stat.trigger & VPAD_BUTTON_RIGHT) {
        if(curCol >= buttonColumns.size() - 1) {
            curCol = 0; //wrap on rightmost row
        }
        else {
            curCol += 1; //right one row
        }
        moved = true;
    }

    if(stat.trigger & VPAD_BUTTON_UP) {
        if(curRow <= 0) {
            curRow = buttonColumns[curCol].size() - 1; //wrap on top
        }
        else {
            curRow -= 1; //up one row
        }
        moved = true;
    }
    else if(stat.trigger & VPAD_BUTTON_DOWN) {
        if(curRow >= buttonColumns[curCol].size() - 1) {
            curRow = 0; //wrap on buttom row
        }
        else {
            curRow += 1; //down one row
        }
        moved = true;
    }
    
    return moved || buttonColumns[curCol][curRow]->update(stat);
}

void ProgressionPage::drawTV() const {
    for(size_t col = 0; col < buttonColumns.size(); col++) {
        const size_t startCol = (ScreenSizeData::tv_line_length / buttonColumns.size()) * col;
        for(size_t row = 0; row < buttonColumns[col].size(); row++) {
            //save 1 extra space for the cursor beside a button
            buttonColumns[col][row]->drawTV(3 + row, startCol + 1, startCol + 1 + 30);
        }
    }

    OSScreenPutFontEx(SCREEN_TV, (ScreenSizeData::tv_line_length / buttonColumns.size()) * curCol, 3 + curRow, ">");
}

void ProgressionPage::drawDRC() const {
    buttonColumns[curCol][curRow]->drawDRC();

    const std::vector<std::string>& descLines = wrap_string(getDesc(), ScreenSizeData::drc_line_length);
    const size_t startLine = ScreenSizeData::drc_num_lines - descLines.size();
    for(size_t i = 0; i < descLines.size(); i++) {
        OSScreenPutFontEx(SCREEN_DRC, 0, startLine + i, descLines[i].c_str());
    }
}



HintsPage::HintsPage() {
    using namespace std::literals::chrono_literals;

    buttonColumns[0][0] = std::make_unique<BasicButton>(Option::HoHoHints);
    buttonColumns[0][1] = std::make_unique<BasicButton>(Option::KorlHints);
    buttonColumns[0][2] = std::make_unique<BasicButton>(Option::ClearerHints);
    buttonColumns[0][3] = std::make_unique<BasicButton>(Option::UseAlwaysHints);

    buttonColumns[1][0] = std::make_unique<CounterButton>(Option::PathHints, 300ms, 300ms);
    buttonColumns[1][1] = std::make_unique<CounterButton>(Option::BarrenHints, 300ms, 300ms);
    buttonColumns[1][2] = std::make_unique<CounterButton>(Option::ItemHints, 300ms, 300ms);
    buttonColumns[1][3] = std::make_unique<CounterButton>(Option::LocationHints, 300ms, 300ms);
}

void HintsPage::open() {
    curCol = 0;
    curRow = 0;
}

bool HintsPage::update(const VPADStatus& stat) {
    bool moved = false;

    if(stat.trigger & VPAD_BUTTON_LEFT) {
        if(curCol <= 0) {
            curCol = buttonColumns.size() - 1; //wrap on leftmost row
        }
        else {
            curCol -= 1; //left one row
        }
        moved = true;
    }
    else if(stat.trigger & VPAD_BUTTON_RIGHT) {
        if(curCol >= buttonColumns.size() - 1) {
            curCol = 0; //wrap on rightmost row
        }
        else {
            curCol += 1; //right one row
        }
        moved = true;
    }

    if(stat.trigger & VPAD_BUTTON_UP) {
        if(curRow <= 0) {
            curRow = buttonColumns[curCol].size() - 1; //wrap on top
        }
        else {
            curRow -= 1; //up one row
        }
        moved = true;
    }
    else if(stat.trigger & VPAD_BUTTON_DOWN) {
        if(curRow >= buttonColumns[curCol].size() - 1) {
            curRow = 0; //wrap on buttom row
        }
        else {
            curRow += 1; //down one row
        }
        moved = true;
    }
    
    return moved || buttonColumns[curCol][curRow]->update(stat);
}

void HintsPage::drawTV() const {
    for(size_t col = 0; col < buttonColumns.size(); col++) {
        const size_t startCol = (ScreenSizeData::tv_line_length / buttonColumns.size()) * col;
        for(size_t row = 0; row < buttonColumns[col].size(); row++) {
            //save 1 extra space for the cursor beside a button
            buttonColumns[col][row]->drawTV(3 + row, startCol + 1, startCol + 1 + 34);
        }
    }

    OSScreenPutFontEx(SCREEN_TV, (ScreenSizeData::tv_line_length / buttonColumns.size()) * curCol, 3 + curRow, ">");
}

void HintsPage::drawDRC() const {
    buttonColumns[curCol][curRow]->drawDRC();

    const std::vector<std::string>& descLines = wrap_string(getDesc(), ScreenSizeData::drc_line_length);
    const size_t startLine = ScreenSizeData::drc_num_lines - descLines.size();
    for(size_t i = 0; i < descLines.size(); i++) {
        OSScreenPutFontEx(SCREEN_DRC, 0, startLine + i, descLines[i].c_str());
    }
}



EntrancePage::EntrancePage() {
    buttonColumns[0][0] = std::make_unique<BasicButton>(Option::RandomizeDungeonEntrances);
    buttonColumns[0][1] = std::make_unique<BasicButton>(Option::RandomizeBossEntrances);
    buttonColumns[0][2] = std::make_unique<BasicButton>(Option::RandomizeMinibossEntrances);
    buttonColumns[0][3] = std::make_unique<BasicButton>(Option::RandomizeCaveEntrances);
    buttonColumns[0][4] = std::make_unique<BasicButton>(Option::RandomizeDoorEntrances);
    buttonColumns[0][5] = std::make_unique<BasicButton>(Option::RandomizeMiscEntrances);
    buttonColumns[0][6] = std::make_unique<BasicButton>(Option::RandomStartIsland);

    buttonColumns[1][0] = std::make_unique<BasicButton>(Option::MixDungeons);
    buttonColumns[1][1] = std::make_unique<BasicButton>(Option::MixBosses);
    buttonColumns[1][2] = std::make_unique<BasicButton>(Option::MixMinibosses);
    buttonColumns[1][3] = std::make_unique<BasicButton>(Option::MixCaves);
    buttonColumns[1][4] = std::make_unique<BasicButton>(Option::MixDoors);
    buttonColumns[1][5] = std::make_unique<BasicButton>(Option::MixMisc);
    buttonColumns[1][6] = std::make_unique<BasicButton>(Option::DecoupleEntrances);
}

void EntrancePage::open() {
    curCol = 0;
    curRow = 0;
}

bool EntrancePage::update(const VPADStatus& stat) {
    bool moved = false;

    if(stat.trigger & VPAD_BUTTON_LEFT) {
        if(curCol <= 0) {
            curCol = buttonColumns.size() - 1; //wrap on leftmost row
        }
        else {
            curCol -= 1; //left one row
        }
        moved = true;
    }
    else if(stat.trigger & VPAD_BUTTON_RIGHT) {
        if(curCol >= buttonColumns.size() - 1) {
            curCol = 0; //wrap on rightmost row
        }
        else {
            curCol += 1; //right one row
        }
        moved = true;
    }

    if(stat.trigger & VPAD_BUTTON_UP) {
        if(curRow <= 0) {
            curRow = buttonColumns[curCol].size() - 1; //wrap on top
        }
        else {
            curRow -= 1; //up one row
        }
        moved = true;
    }
    else if(stat.trigger & VPAD_BUTTON_DOWN) {
        if(curRow >= buttonColumns[curCol].size() - 1) {
            curRow = 0; //wrap on buttom row
        }
        else {
            curRow += 1; //down one row
        }
        moved = true;
    }
    
    return moved || buttonColumns[curCol][curRow]->update(stat);
}

void EntrancePage::drawTV() const {
    for(size_t col = 0; col < buttonColumns.size(); col++) {
        const size_t startCol = (ScreenSizeData::tv_line_length / buttonColumns.size()) * col;
        for(size_t row = 0; row < buttonColumns[col].size(); row++) {
            //save 1 extra space for the cursor beside a button
            buttonColumns[col][row]->drawTV(3 + row, startCol + 1, startCol + 1 + 30);
        }
    }

    OSScreenPutFontEx(SCREEN_TV, (ScreenSizeData::tv_line_length / buttonColumns.size()) * curCol, 3 + curRow, ">");
}

void EntrancePage::drawDRC() const {
    buttonColumns[curCol][curRow]->drawDRC();

    const std::vector<std::string>& descLines = wrap_string(getDesc(), ScreenSizeData::drc_line_length);
    const size_t startLine = ScreenSizeData::drc_num_lines - descLines.size();
    for(size_t i = 0; i < descLines.size(); i++) {
        OSScreenPutFontEx(SCREEN_DRC, 0, startLine + i, descLines[i].c_str());
    }
}



ConveniencePage::ConveniencePage() {
    buttonColumns[0][0] = std::make_unique<BasicButton>(Option::InstantText);
    buttonColumns[0][1] = std::make_unique<BasicButton>(Option::FixRNG);
    buttonColumns[0][2] = std::make_unique<BasicButton>(Option::RevealSeaChart);
    buttonColumns[0][3] = std::make_unique<BasicButton>(Option::RemoveMusic);
    buttonColumns[0][4] = std::make_unique<BasicButton>(Option::Camera);
    buttonColumns[0][5] = std::make_unique<BasicButton>(Option::FirstPersonCamera);
    buttonColumns[0][6] = std::make_unique<BasicButton>(Option::CasualClothes);

    buttonColumns[1][0] = std::make_unique<BasicButton>(Option::AddShortcutWarps);
    buttonColumns[1][1] = std::make_unique<BasicButton>(Option::SkipRefights);
    buttonColumns[1][2] = std::make_unique<BasicButton>(Option::InvertCompass);
    buttonColumns[1][3] = std::make_unique<BasicButton>(Option::TargetType);
    buttonColumns[1][4] = std::make_unique<BasicButton>(Option::Gyroscope);
    buttonColumns[1][5] = std::make_unique<BasicButton>(Option::UIDisplay);
    buttonColumns[1][6] = std::make_unique<BasicButton>(Option::PigColor);
}

void ConveniencePage::open() {
    curCol = 0;
    curRow = 0;
}

bool ConveniencePage::update(const VPADStatus& stat) {
    bool moved = false;

    if(stat.trigger & VPAD_BUTTON_LEFT) {
        if(curCol <= 0) {
            curCol = buttonColumns.size() - 1; //wrap on leftmost row
        }
        else {
            curCol -= 1; //left one row
        }
        moved = true;
    }
    else if(stat.trigger & VPAD_BUTTON_RIGHT) {
        if(curCol >= buttonColumns.size() - 1) {
            curCol = 0; //wrap on rightmost row
        }
        else {
            curCol += 1; //right one row
        }
        moved = true;
    }

    if(stat.trigger & VPAD_BUTTON_UP) {
        if(curRow <= 0) {
            curRow = buttonColumns[curCol].size() - 1; //wrap on top
        }
        else {
            curRow -= 1; //up one row
        }
        moved = true;
    }
    else if(stat.trigger & VPAD_BUTTON_DOWN) {
        if(curRow >= buttonColumns[curCol].size() - 1) {
            curRow = 0; //wrap on buttom row
        }
        else {
            curRow += 1; //down one row
        }
        moved = true;
    }
    
    return moved || buttonColumns[curCol][curRow]->update(stat);
}

void ConveniencePage::drawTV() const {
    for(size_t col = 0; col < buttonColumns.size(); col++) {
        const size_t startCol = (ScreenSizeData::tv_line_length / buttonColumns.size()) * col;
        for(size_t row = 0; row < buttonColumns[col].size(); row++) {
            //save 1 extra space for the cursor beside a button
            buttonColumns[col][row]->drawTV(3 + row, startCol + 1, startCol + 1 + 30);
        }
    }

    OSScreenPutFontEx(SCREEN_TV, (ScreenSizeData::tv_line_length / buttonColumns.size()) * curCol, 3 + curRow, ">");
}

void ConveniencePage::drawDRC() const {
    buttonColumns[curCol][curRow]->drawDRC();

    const std::vector<std::string>& descLines = wrap_string(getDesc(), ScreenSizeData::drc_line_length);
    const size_t startLine = ScreenSizeData::drc_num_lines - descLines.size();
    for(size_t i = 0; i < descLines.size(); i++) {
        OSScreenPutFontEx(SCREEN_DRC, 0, startLine + i, descLines[i].c_str());
    }
}



AdvancedPage::AdvancedPage() {
    using namespace std::literals::chrono_literals;

    buttonColumns[0][0] = std::make_unique<BasicButton>(Option::RandomCharts);
    buttonColumns[0][1] = std::make_unique<BasicButton>(Option::NoSpoilerLog);
    buttonColumns[0][2] = std::make_unique<CounterButton>(Option::DamageMultiplier, 55ms, 250ms);

    buttonColumns[1][0] = std::make_unique<BasicButton>(Option::StartWithRandomItem);
    buttonColumns[1][1] = std::make_unique<BasicButton>(Option::CTMC);
    buttonColumns[1][2] = std::make_unique<BasicButton>(Option::Plandomizer);
}

void AdvancedPage::open() {
    curCol = 0;
    curRow = 0;
}

bool AdvancedPage::update(const VPADStatus& stat) {
    bool moved = false;

    if(stat.trigger & VPAD_BUTTON_LEFT) {
        if(curCol <= 0) {
            curCol = buttonColumns.size() - 1; //wrap on leftmost row
        }
        else {
            curCol -= 1; //left one row
        }
        moved = true;
    }
    else if(stat.trigger & VPAD_BUTTON_RIGHT) {
        if(curCol >= buttonColumns.size() - 1) {
            curCol = 0; //wrap on rightmost row
        }
        else {
            curCol += 1; //right one row
        }
        moved = true;
    }

    if(stat.trigger & VPAD_BUTTON_UP) {
        if(curRow <= 0) {
            curRow = buttonColumns[curCol].size() - 1; //wrap on top
        }
        else {
            curRow -= 1; //up one row
        }
        moved = true;
    }
    else if(stat.trigger & VPAD_BUTTON_DOWN) {
        if(curRow >= buttonColumns[curCol].size() - 1) {
            curRow = 0; //wrap on buttom row
        }
        else {
            curRow += 1; //down one row
        }
        moved = true;
    }
    
    return moved || buttonColumns[curCol][curRow]->update(stat);
}

void AdvancedPage::drawTV() const {
    for(size_t col = 0; col < buttonColumns.size(); col++) {
        const size_t startCol = (ScreenSizeData::tv_line_length / buttonColumns.size()) * col;
        for(size_t row = 0; row < buttonColumns[col].size(); row++) {
            //save 1 extra space for the cursor beside a button
            buttonColumns[col][row]->drawTV(3 + row, startCol + 1, startCol + 1 + 30);
        }
    }

    OSScreenPutFontEx(SCREEN_TV, (ScreenSizeData::tv_line_length / buttonColumns.size()) * curCol, 3 + curRow, ">");
}

void AdvancedPage::drawDRC() const {
    buttonColumns[curCol][curRow]->drawDRC();

    const std::vector<std::string>& descLines = wrap_string(getDesc(), ScreenSizeData::drc_line_length);
    const size_t startLine = ScreenSizeData::drc_num_lines - descLines.size();
    for(size_t i = 0; i < descLines.size(); i++) {
        OSScreenPutFontEx(SCREEN_DRC, 0, startLine + i, descLines[i].c_str());
    }
}



ItemsPage::ItemsPage() {
    using namespace std::literals::chrono_literals;

    listButtons = {
        GameItem::BaitBag,
        GameItem::BalladOfGales,
        GameItem::Bombs,
        GameItem::Boomerang,
        GameItem::CabanaDeed,
        GameItem::CommandMelody,
        GameItem::DekuLeaf,
        GameItem::DeliveryBag,
        GameItem::DinsPearl,
        GameItem::DRCBigKey,
        GameItem::DRCCompass,
        GameItem::DRCDungeonMap,
        {GameItem::DRCSmallKey, 1},
        {GameItem::DRCSmallKey, 2},
        {GameItem::DRCSmallKey, 3},
        {GameItem::DRCSmallKey, 4},
        GameItem::DragonTingleStatue,
        GameItem::EarthGodsLyric,
        GameItem::ETBigKey,
        GameItem::ETCompass,
        GameItem::ETDungeonMap,
        {GameItem::ETSmallKey, 1},
        {GameItem::ETSmallKey, 2},
        {GameItem::ETSmallKey, 3},
        GameItem::EarthTingleStatue,
        GameItem::EmptyBottle,
        GameItem::FaroresPearl,
        GameItem::ForbiddenTingleStatue,
        GameItem::FWBigKey,
        GameItem::FWCompass,
        GameItem::FWDungeonMap,
        GameItem::FWSmallKey,
        GameItem::FFCompass,
        GameItem::FFDungeonMap,
        GameItem::GhostShipChart,
        GameItem::GoddessTingleStatue,
        GameItem::GrapplingHook,
        GameItem::HerosCharm,
        GameItem::Hookshot,
        GameItem::HurricaneSpin,
        GameItem::IronBoots,
        GameItem::MaggiesLetter,
        GameItem::MagicArmor,
        GameItem::MoblinsLetter,
        GameItem::NayrusPearl,
        GameItem::NoteToMom,
        GameItem::PowerBracelets,
        {GameItem::ProgressiveBombBag, 1},
        {GameItem::ProgressiveBombBag, 2},
        {GameItem::ProgressiveBow, 1},
        {GameItem::ProgressiveBow, 2},
        {GameItem::ProgressiveBow, 3},
        {GameItem::ProgressiveMagicMeter, 1},
        {GameItem::ProgressiveMagicMeter, 2},
        {GameItem::ProgressivePictoBox, 1},
        {GameItem::ProgressivePictoBox, 2},
        {GameItem::ProgressiveQuiver, 1},
        {GameItem::ProgressiveQuiver, 2},
        {GameItem::ProgressiveSail, 1}, // technically doesn't need the num check right now, would want it if second sail could be shuffled though
        {GameItem::ProgressiveShield, 1},
        {GameItem::ProgressiveShield, 2},
        {GameItem::ProgressiveWallet, 3},
        {GameItem::ProgressiveWallet, 4},
        GameItem::SkullHammer,
        GameItem::SongOfPassing,
        GameItem::SpoilsBag,
        GameItem::Telescope,
        GameItem::TingleBottle,
        GameItem::TotGBigKey,
        GameItem::TotGCompass,
        GameItem::TotGDungeonMap,
        {GameItem::TotGSmallKey, 1},
        {GameItem::TotGSmallKey, 2},
        GameItem::WindGodsAria,
        GameItem::WTBigKey,
        GameItem::WTCompass,
        GameItem::WTDungeonMap,
        {GameItem::WTSmallKey, 1},
        {GameItem::WTSmallKey, 2},
        GameItem::WindTingleStatue
    };

    countButtons[0] = std::make_unique<CounterButton>(Option::StartingHP, 250ms, 300ms);
    countButtons[1] = std::make_unique<CounterButton>(Option::StartingHC, 100ms, 300ms);
    countButtons[2] = std::make_unique<CounterButton>(Option::StartingJoyPendants, 100ms, 300ms);
    countButtons[3] = std::make_unique<CounterButton>(Option::StartingSkullNecklaces, 150ms, 300ms);
    countButtons[4] = std::make_unique<CounterButton>(Option::StartingBokoBabaSeeds, 200ms, 300ms);
    countButtons[5] = std::make_unique<CounterButton>(Option::StartingGoldenFeathers, 120ms, 300ms);
    countButtons[6] = std::make_unique<CounterButton>(Option::StartingKnightsCrests, 200ms, 300ms);
    countButtons[7] = std::make_unique<CounterButton>(Option::StartingRedChuJellys, 130ms, 300ms);
    countButtons[8] = std::make_unique<CounterButton>(Option::StartingGreenChuJellys, 130ms, 300ms);
    countButtons[9] = std::make_unique<CounterButton>(Option::StartingBlueChuJellys, 130ms, 300ms);
}

void ItemsPage::open() {
    curCol = Column::LIST;
    curRow = 0;
    listScrollPos = 0;

    if(getValue(Option::SwordMode) == SwordModeToName(SwordMode::StartWithSword) && std::find(listButtons.begin(), listButtons.end(), GameItem::ProgressiveSword) == listButtons.end()) {
        const auto firstIt = std::find(listButtons.begin(), listButtons.end(), GameItem::ProgressiveWallet); // sword is before wallet in the alphabet
        if(firstIt != listButtons.end()) { // should always be true
            listButtons.insert(firstIt, 3, {GameItem::ProgressiveSword}); // add 3 swords
        }
    }
    else {
        // Swordless - no swords in the game
        // No starting sword - pointless to add swords since you would need at least 1 (which would just be start with sword)
        const auto firstIt = std::find(listButtons.begin(), listButtons.end(), GameItem::ProgressiveSword);
        if(firstIt != listButtons.end()) {
            listButtons.erase(firstIt, firstIt + 3); // would have 3 swords to remove
        }
    }

    // load the items currently set in the config
    for(ItemButton& button : listButtons) {
        button.loadState();
    }
}

bool ItemsPage::update(const VPADStatus& stat) {
    bool moved = false;

    if(stat.trigger & VPAD_BUTTON_LEFT || stat.trigger & VPAD_BUTTON_RIGHT) {
        switch(curCol) {
            case Column::LIST:
                curCol = Column::BUTTONS;
                curRow = std::clamp<size_t>(curRow, 0, countButtons.size() - 1);

                break;
            case Column::BUTTONS:
                curCol = Column::LIST;
                break;
        }

        moved = true;
    }

    if(stat.trigger & VPAD_BUTTON_UP) {
        switch(curCol) {
            case Column::LIST:
                if(curRow <= 0) {
                    if(listScrollPos <= 0) {
                        listScrollPos = listButtons.size() - LIST_HEIGHT - 1; // -1 because 0-indexed
                        curRow = LIST_HEIGHT - 1; // -1 because 0-indexed
                    }
                    else {
                        listScrollPos -= 1;
                    }
                }
                else {
                    curRow -= 1;
                }

                break;
            case Column::BUTTONS:
                if(curRow <= 0) {
                    curRow = countButtons.size() - 1; //wrap on top row
                }
                else {
                    curRow -= 1; //up one row
                }

                break;
        }

        moved = true;
    }
    else if(stat.trigger & VPAD_BUTTON_DOWN) {
        switch(curCol) {
            case Column::LIST:
                if(curRow >= LIST_HEIGHT - 1) {
                    if(listScrollPos >= listButtons.size() - LIST_HEIGHT - 1) { //-1 for 0-indexing
                        listScrollPos = 0;
                        curRow = 0;
                    }
                    else {
                        listScrollPos += 1;
                    }
                }
                else {
                    curRow += 1;
                }

                break;
            case Column::BUTTONS:
                if(curRow >= countButtons.size() - 1) {
                    curRow = 0; //wrap on bottom row
                }
                else {
                    curRow += 1; //down one row
                }

                break;
        }

        moved = true;
    }

    bool btnUpdate = false;
    switch(curCol) {
        case Column::LIST:
            if(listButtons[listScrollPos + curRow].update(stat)) {
                btnUpdate = true;

                //update gear list
                OptionCB::clearStartingItems();
                for(const auto& button : listButtons) {
                    if(button.isEnabled()) {
                        OptionCB::addStartingItem(button.getItem());
                    }
                }
            }

            break;
        case Column::BUTTONS:
            btnUpdate = countButtons[curRow]->update(stat);
            break;
    }
    
    return moved || btnUpdate;
}

void ItemsPage::drawTV() const {
    // draw visible part of the list
    const size_t listStartCol = 0;
    for(size_t row = 0; row < LIST_HEIGHT; row++) {
        listButtons[listScrollPos + row].drawTV(3 + row, 3, 1);
    }

    // draw second column of buttons
    const size_t countStartCol = ScreenSizeData::tv_line_length / 2;
    for(size_t row = 0; row < countButtons.size(); row++) {
        countButtons[row]->drawTV(3 + row, countStartCol + 1, countStartCol + 1 + 30);
    }
    
    // draw cursor
    switch(curCol) {
        case Column::LIST:
            OSScreenPutFontEx(SCREEN_TV, listStartCol, 3 + curRow, ">");

            break;
        case Column::BUTTONS:
            OSScreenPutFontEx(SCREEN_TV, countStartCol, 3 + curRow, ">");
            break;
    }
}

void ItemsPage::drawDRC() const {
    switch(curCol) {
        case Column::LIST:
            listButtons[listScrollPos + curRow].drawDRC();

            break;
        case Column::BUTTONS:
            countButtons[curRow]->drawDRC();
            break;
    }

    const std::vector<std::string>& descLines = wrap_string(getDesc(), ScreenSizeData::drc_line_length);
    const size_t startLine = ScreenSizeData::drc_num_lines - descLines.size();
    for(size_t i = 0; i < descLines.size(); i++) {
        OSScreenPutFontEx(SCREEN_DRC, 0, startLine + i, descLines[i].c_str());
    }
}
