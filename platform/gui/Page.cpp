#include "Page.hpp"

#include <utility/color.hpp>
#include <platform/gui/screen.hpp>
#include <platform/gui/TextWrap.hpp>

SeedPage::SeedPage() {
    resetTimer();
}

void SeedPage::open() {
    warnings = "";

    if(wasUpdated()) {
        warnings += "Config was made using a different randomizer version. Item placements may be different than expected.\n";
    }
    if(wasConverted()) {
        warnings += "Config was converted from an old or incorrect format. Some settings may be different than expected.\n";
    }

    resetTimer();
}

bool SeedPage::update(const VPADStatus& stat) {
    if (stat.trigger & VPAD_BUTTON_A) {
        OptionCB::changeSeed();
        return true;
    }

    if (!(stat.hold & VPAD_BUTTON_B)) {
        resetTimer();
    }
    else {
        if(Clock::now() >= resetTime) {
            OptionCB::resetInternal();
            resetTimer();
        }
    }

    return true;
}

void SeedPage::drawTV() const {
    using namespace std::literals::chrono_literals;

    OSScreenPutFontEx(SCREEN_TV, 0, 3, ("The current seed is \"" + getSeed() + "\". Hash: " + getSeedHash()).c_str());
    OSScreenPutFontEx(SCREEN_TV, 0, 4, "Press A to generate a new seed");
    // (total time - (final time - current time)) / (total duration / 10) = fraction of total in 10 increments
    const std::chrono::milliseconds remaining = 3s - std::chrono::duration_cast<std::chrono::milliseconds>(resetTime - Clock::now());
    const size_t count = remaining.count() / 300;
    std::string bar(10, ' ');
    bar.replace(0, count, count, '-');
    OSScreenPutFontEx(SCREEN_TV, 0, 6, ("Hold B to reset all settings to default [" + bar + "]").c_str());

    const std::vector<std::string>& warnLines = wrap_string(warnings, ScreenSizeData::tv_line_length);
    const size_t startLine = ScreenSizeData::tv_num_lines - 3 - warnLines.size();
    for(size_t i = 0; i < warnLines.size(); i++) {
        OSScreenPutFontEx(SCREEN_TV, 0, startLine + i, warnLines[i].c_str());
    }
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
    buttonColumns[0][1] = std::make_unique<CounterButton>(Option::NumRequiredDungeons, 250ms, 300ms);
    buttonColumns[0][2] = std::make_unique<BasicButton>(Option::ProgressDungeonSecrets);
    buttonColumns[0][3] = std::make_unique<BasicButton>(Option::ProgressTingleChests);
    buttonColumns[0][4] = std::make_unique<BasicButton>(Option::ProgressPuzzleCaves);
    buttonColumns[0][5] = std::make_unique<BasicButton>(Option::ProgressCombatCaves);
    buttonColumns[0][6] = std::make_unique<BasicButton>(Option::ProgressShortSidequests);
    buttonColumns[0][7] = std::make_unique<BasicButton>(Option::ProgressLongSidequests);
    buttonColumns[0][8] = std::make_unique<BasicButton>(Option::ProgressSpoilsTrading);
    buttonColumns[0][9] = std::make_unique<BasicButton>(Option::ProgressMinigames);
    buttonColumns[0][10] = std::make_unique<BasicButton>(Option::ProgressFreeGifts);
    buttonColumns[0][11] = std::make_unique<BasicButton>(Option::ProgressMail);
    buttonColumns[0][12] = std::make_unique<BasicButton>(Option::ProgressGreatFairies);
    buttonColumns[0][13] = std::make_unique<BasicButton>(Option::ProgressObscure);
    buttonColumns[0][14] = std::make_unique<BasicButton>(Option::RemoveSwords);

    buttonColumns[1][0] = std::make_unique<BasicButton>(Option::DungeonSmallKeys);
    buttonColumns[1][1] = std::make_unique<BasicButton>(Option::DungeonBigKeys);
    buttonColumns[1][2] = std::make_unique<BasicButton>(Option::DungeonMapsAndCompasses);
    buttonColumns[1][3] = std::make_unique<BasicButton>(Option::ProgressMisc);
    buttonColumns[1][4] = std::make_unique<BasicButton>(Option::ProgressExpPurchases);
    buttonColumns[1][5] = std::make_unique<BasicButton>(Option::ProgressBattlesquid);
    buttonColumns[1][6] = std::make_unique<BasicButton>(Option::ProgressSavageLabyrinth);
    buttonColumns[1][7] = std::make_unique<BasicButton>(Option::ProgressIslandPuzzles);
    buttonColumns[1][8] = std::make_unique<BasicButton>(Option::ProgressSubmarines);
    buttonColumns[1][9] = std::make_unique<BasicButton>(Option::ProgressPlatformsRafts);
    buttonColumns[1][10] = std::make_unique<BasicButton>(Option::ProgressEyeReefs);
    buttonColumns[1][11] = std::make_unique<BasicButton>(Option::ProgressOctosGunboats);
    buttonColumns[1][12] = std::make_unique<BasicButton>(Option::ProgressTreasureCharts);
    buttonColumns[1][13] = std::make_unique<BasicButton>(Option::ProgressTriforceCharts);
    buttonColumns[1][14] = std::make_unique<BasicButton>(Option::RandomCharts);
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
    buttonColumns[0][6] = std::make_unique<BasicButton>(Option::TargetType);

    buttonColumns[1][0] = std::make_unique<BasicButton>(Option::AddShortcutWarps);
    buttonColumns[1][1] = std::make_unique<BasicButton>(Option::SkipRefights);
    buttonColumns[1][2] = std::make_unique<BasicButton>(Option::InvertCompass);
    buttonColumns[1][3] = std::make_unique<BasicButton>(Option::Performance);
    buttonColumns[1][4] = std::make_unique<BasicButton>(Option::PigColor);
    buttonColumns[1][5] = std::make_unique<BasicButton>(Option::Gyroscope);
    buttonColumns[1][6] = std::make_unique<BasicButton>(Option::UIDisplay);
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

    buttonColumns[0][0] = std::make_unique<BasicButton>(Option::NoSpoilerLog);
    buttonColumns[0][1] = std::make_unique<CounterButton>(Option::DamageMultiplier, 55ms, 250ms);
    buttonColumns[0][2] = std::make_unique<BasicButton>(Option::ClassicMode);

    buttonColumns[1][0] = std::make_unique<BasicButton>(Option::CTMC);
    buttonColumns[1][1] = std::make_unique<BasicButton>(Option::Plandomizer);
    buttonColumns[1][2] = std::make_unique<BasicButton>(Option::RandomItemSlideItem);
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
        GameItem::TriforceShard1,
        GameItem::TriforceShard2,
        GameItem::TriforceShard3,
        GameItem::TriforceShard4,
        GameItem::TriforceShard5,
        GameItem::TriforceShard6,
        GameItem::TriforceShard7,
        GameItem::TriforceShard8,
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
    countButtons[10] = std::make_unique<BasicButton>(Option::StartWithRandomItem);
}

void ItemsPage::open() {
    curCol = Column::LIST;
    curRow = 0;
    listScrollPos = 0;

    if(getValue(Option::RemoveSwords) == "Disabled") { 
        // only add swords if they aren't already there
        if(std::find(listButtons.begin(), listButtons.end(), GameItem::ProgressiveSword) == listButtons.end()) {
            auto it = std::find(listButtons.begin(), listButtons.end(), GameItem::ProgressiveWallet); // sword is before wallet in the alphabet
            if(it != listButtons.end()) { // should always be true
                for(size_t count = 1; count <= 4; count++) {
                    it = listButtons.insert(it, {GameItem::ProgressiveSword, count}); // add 4 swords
                    it++;
                }
            }
        }
    }
    else {
        const auto firstIt = std::find(listButtons.begin(), listButtons.end(), GameItem::ProgressiveSword);
        if(firstIt != listButtons.end()) {
            listButtons.erase(firstIt, firstIt + 4); // would have 4 swords to remove
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
                        listScrollPos = listButtons.size() - LIST_HEIGHT;
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
                    if(listScrollPos >= listButtons.size() - LIST_HEIGHT) {
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



ColorPage::ColorPage() :
    presets(*this),
    picker(*this)
{}

void ColorPage::open() {
    curSubpage = Subpage::PRESETS;
}

bool ColorPage::update(const VPADStatus& stat) {
    switch(curSubpage) {
        case Subpage::PRESETS:
            return presets.update(stat);
        case Subpage::COLOR_PICKER:
            return picker.update(stat);
    }

    return false;
}

void ColorPage::drawTV() const {
    switch(curSubpage) {
        case Subpage::PRESETS:
            return presets.drawTV();
        case Subpage::COLOR_PICKER:
            return picker.drawTV();
    }
}

void ColorPage::drawDRC() const {
    switch(curSubpage) {
        case Subpage::PRESETS:
            return presets.drawDRC();
        case Subpage::COLOR_PICKER:
            return picker.drawDRC();
    }
}

ColorPage::PresetsSubpage::PresetsSubpage(ColorPage& parent_) : 
    parent(parent_)
{
    toggles[0] = std::make_unique<ActionButton>("Casual Clothes", "Enable this if you want to wear your casual clothes instead of the Hero's Clothes.", &OptionCB::toggleCasualClothes, &OptionCB::isCasual);
    toggles[1] = std::make_unique<ActionButton>("Randomize Colors Orderly", "", &OptionCB::randomizeColorsOrderly);
    toggles[2] = std::make_unique<ActionButton>("Randomize Colors Chaotically", "", &OptionCB::randomizeColorsChaotically);
    toggles[3] = std::make_unique<FunctionButton>("Select Colors Manually", "", std::bind(&ColorPage::setSubpage, &parent, Subpage::COLOR_PICKER));
}

void ColorPage::PresetsSubpage::open() {
    curCol = Column::LIST;
    curRow = 0;
    listScrollPos = 0;
}

bool ColorPage::PresetsSubpage::update(const VPADStatus& stat) {
    bool moved = false;

    if(stat.trigger & VPAD_BUTTON_LEFT || stat.trigger & VPAD_BUTTON_RIGHT) {
        switch(curCol) {
            case Column::LIST:
                curCol = Column::BUTTONS;
                curRow = std::clamp<size_t>(curRow, 0, toggles.size() - 1);

                break;
            case Column::BUTTONS:
                curCol = Column::LIST;
                curRow = std::clamp<size_t>(curRow, 0, std::min(LIST_HEIGHT, getModel().getPresets().size()) - 1);

                break;
        }

        moved = true;
    }

    if(stat.trigger & VPAD_BUTTON_UP) {
        switch(curCol) {
            case Column::LIST:
                if(curRow <= 0) {
                    if(listScrollPos <= 0) {
                        curRow = std::min(LIST_HEIGHT, getModel().getPresets().size()) - 1; // -1 because 0-indexed
                        if(getModel().getPresets().size() > LIST_HEIGHT) {
                            listScrollPos = getModel().getPresets().size() - LIST_HEIGHT;
                        }
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
                    curRow = toggles.size() - 1; //wrap on top row
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
                if(curRow >= std::min(LIST_HEIGHT, getModel().getPresets().size()) - 1) {
                    if(getModel().getPresets().size() > LIST_HEIGHT) {
                        if(listScrollPos >= getModel().getPresets().size() - LIST_HEIGHT) {
                            listScrollPos = 0;
                            curRow = 0;
                        }
                        else {
                            listScrollPos += 1;
                        }
                    }
                    else {
                        listScrollPos = 0;
                        curRow = 0;
                    }
                }
                else {
                    curRow += 1;
                }

                break;
            case Column::BUTTONS:
                if(curRow >= toggles.size() - 1) {
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
            if(stat.trigger & VPAD_BUTTON_A) {
                btnUpdate = true;

                getModel().loadPreset(listScrollPos + curRow);
            }

            break;
        case Column::BUTTONS:
            btnUpdate = toggles[curRow]->update(stat);
            break;
    }
    
    return moved || btnUpdate;
}

static void drawSquare(const uint32_t& color, const uint32_t& x, const uint32_t& y, const uint32_t& size) {
    for(uint32_t row = 0; row < size; row++) {
        for(uint32_t col = 0; col < size; col++) {
            OSScreenPutPixelEx(SCREEN_TV, x + col, y + row, color);
        }
    }
}

static const std::array<std::string, 11> heroTextures = {
    "Hair",
    "Skin",
    "Mouth",
    "Eyes",
    "Sclera",
    "Tunic",
    "Undershirt",
    "Pants",
    "Boots",
    "Belt",
    "Belt Buckle",
};
static const std::array<std::string, 11> casualTextures = {
    "Hair",
    "Skin",
    "Mouth",
    "Eyes",
    "Sclera",
    "Shirt",
    "Shirt Emblem",
    "Armbands",
    "Pants",
    "Shoes",
    "Shoe Soles",
};

void ColorPage::PresetsSubpage::drawTV() const {
    // draw visible part of the list
    for(size_t row = 0; row < std::min(LIST_HEIGHT, getModel().getPresets().size()); row++) {
        OSScreenPutFontEx(SCREEN_TV, 1, 3 + row, getModel().getPresets()[listScrollPos + row].name.c_str());
    }

    // draw second column of buttons
    const size_t countStartCol = ScreenSizeData::tv_line_length / 2;
    for(size_t row = 0; row < toggles.size(); row++) {
        toggles[row]->drawTV(3 + row, countStartCol + 1, countStartCol + 1 + 30);
    }
    
    // draw loaded colors
    static constexpr uint32_t starting_y_pos = 223;
    const std::array<std::string, 11>& textures = getModel().casual ? casualTextures : heroTextures;
    for(size_t i = 0; i < textures.size(); i++) {
        const size_t row = 8 + i;
        OSScreenPutFontEx(SCREEN_TV, countStartCol + 1, row, textures[i].c_str());

        std::string hexColor = getModel().getColor(textures[i]);
        for(auto& c : hexColor) {
            c = std::toupper(c); //capitalize for consistency
        }

        OSScreenPutFontEx(SCREEN_TV, countStartCol + 1 + 15, row, hexColor.c_str());
        drawSquare(std::stoi(hexColor, nullptr, 16) << 8, 13 * (countStartCol + 1 + 15 + 6), starting_y_pos + i * 24, 24);
    }
    
    // draw cursor
    switch(curCol) {
        case Column::LIST:
            OSScreenPutFontEx(SCREEN_TV, 0, 3 + curRow, ">");

            break;
        case Column::BUTTONS:
            OSScreenPutFontEx(SCREEN_TV, countStartCol, 3 + curRow, ">");
            break;
    }
}

void ColorPage::PresetsSubpage::drawDRC() const {
    switch(curCol) {
        case Column::LIST:
            break;
        case Column::BUTTONS:
            toggles[curRow]->drawDRC();
            break;
    }

    const std::vector<std::string>& descLines = wrap_string(getDesc(), ScreenSizeData::drc_line_length);
    const size_t startLine = ScreenSizeData::drc_num_lines - descLines.size();
    for(size_t i = 0; i < descLines.size(); i++) {
        OSScreenPutFontEx(SCREEN_DRC, 0, startLine + i, descLines[i].c_str());
    }
}

ColorPage::ColorPickerSubpage::ColorPickerSubpage(ColorPage& parent_) : 
    parent(parent_),
    actions{
        ColorButton("Random", "", &ColorCB::randomizeColor),
        ColorButton("Pick", "", std::bind(&ColorPage::ColorPickerSubpage::setPicking, this, true, std::placeholders::_1)), // this is a really stupid way to do this
        ColorButton("Reset", "", &ColorCB::resetColor)
    }
{}

void ColorPage::ColorPickerSubpage::open() {
    curCol = 0;
    curRow = 0;
    listScrollPos = 0;

    setPicking(false);
}

bool ColorPage::ColorPickerSubpage::update(const VPADStatus& stat) {
    if(picking) {
        return updatePicker(stat);
    }
    else {
        return updateList(stat);
    }
}

void ColorPage::ColorPickerSubpage::drawTV() const {
    if(picking) {
        return drawPickerTV();
    }
    else {
        return drawListTV();
    }
}

void ColorPage::ColorPickerSubpage::drawDRC() const {
    if(picking) {
        return drawPickerDRC();
    }
    else {
        return drawListDRC();
    }
}

bool ColorPage::ColorPickerSubpage::updateList(const VPADStatus& stat) {
    if (stat.trigger & VPAD_BUTTON_B) {
        parent.setSubpage(Subpage::PRESETS);
        return true;
    }

    bool moved = false;
    if(stat.trigger & VPAD_BUTTON_LEFT) {
        if(curCol <= 0) {
            curCol = actions.size() - 1; //wrap on leftmost row
        }
        else {
            curCol -= 1; //left one row
        }
        moved = true;
    }
    else if(stat.trigger & VPAD_BUTTON_RIGHT) {
        if(curCol >= actions.size() - 1) {
            curCol = 0; //wrap on rightmost row
        }
        else {
            curCol += 1; //right one row
        }
        moved = true;
    }

    if(stat.trigger & VPAD_BUTTON_UP) {
        // technically doesn't matter since they're the same number of textures but if some ever get split this will adjust itself
        if(getModel().casual) {
            if(curRow <= 0) {
                if(listScrollPos <= 0) {
                    curRow = std::min(LIST_HEIGHT, casualTextures.size()) - 1; // -1 because 0-indexed
                    if(casualTextures.size() > LIST_HEIGHT) {
                        listScrollPos = casualTextures.size() - LIST_HEIGHT;
                    }
                }
                else {
                    listScrollPos -= 1;
                }
            }
            else {
                curRow -= 1;
            }
        }
        else {
            if(curRow <= 0) {
                if(listScrollPos <= 0) {
                    curRow = std::min(LIST_HEIGHT, heroTextures.size()) - 1; // -1 because 0-indexed
                    if(heroTextures.size() > LIST_HEIGHT) {
                        listScrollPos = heroTextures.size() - LIST_HEIGHT;
                    }
                }
                else {
                    listScrollPos -= 1;
                }
            }
            else {
                curRow -= 1;
            }
        }

        moved = true;
    }
    else if(stat.trigger & VPAD_BUTTON_DOWN) {
        if(getModel().casual) {
            if(curRow >= std::min(LIST_HEIGHT, casualTextures.size()) - 1) {
                if(casualTextures.size() > LIST_HEIGHT) {
                    if(listScrollPos >= casualTextures.size() - LIST_HEIGHT) {
                        listScrollPos = 0;
                        curRow = 0;
                    }
                    else {
                        listScrollPos += 1;
                    }
                }
                else {
                    listScrollPos = 0;
                    curRow = 0;
                }
            }
            else {
                curRow += 1;
            }
        }
        else {
            if(curRow >= std::min(LIST_HEIGHT, heroTextures.size()) - 1) {
                if(heroTextures.size() > LIST_HEIGHT) {
                    if(listScrollPos >= heroTextures.size() - LIST_HEIGHT) {
                        listScrollPos = 0;
                        curRow = 0;
                    }
                    else {
                        listScrollPos += 1;
                    }
                }
                else {
                    listScrollPos = 0;
                    curRow = 0;
                }
            }
            else {
                curRow += 1;
            }
        }

        moved = true;
    }

    bool btnUpdate = false;
    if(getModel().casual) {
        btnUpdate = actions[curCol].update(stat, casualTextures[listScrollPos + curRow]);
    }
    else {
        btnUpdate = actions[curCol].update(stat, heroTextures[listScrollPos + curRow]);
    }
    
    return moved || btnUpdate;
}

void ColorPage::ColorPickerSubpage::drawListTV() const {
    static constexpr size_t startCol = 2;
    static constexpr uint32_t starting_y_pos = 104;
    const std::array<std::string, 11>& textures = getModel().casual ? casualTextures : heroTextures;
    for(size_t row = 0; row < std::min(LIST_HEIGHT, textures.size()); row++) {
        OSScreenPutFontEx(SCREEN_TV, startCol, row + 3, textures[listScrollPos + row].c_str());

        std::string hexColor = getModel().getColor(textures[listScrollPos + row]);
        for(auto& c : hexColor) {
            c = std::toupper(c); //capitalize for consistency
        }

        OSScreenPutFontEx(SCREEN_TV, startCol + 13, 3 + row, hexColor.c_str());
        drawSquare(std::stoi(hexColor, nullptr, 16) << 8, 14 * (startCol + 13 + 7), starting_y_pos + row * 24, 24);
        
        actions[0].drawTV(row + 3, startCol + 23, startCol + 23 + 1);
        actions[1].drawTV(row + 3, startCol + 31, startCol + 31 + 1);
        actions[2].drawTV(row + 3, startCol + 37, startCol + 37 + 1);
    }
    
    // draw cursor
    size_t cursorCol = 0;
    switch(curCol) {
        case 0:
            cursorCol = startCol + 23 - 1;
            break;
        case 1:
            cursorCol = startCol + 31 - 1;
            break;
        case 2:
            cursorCol = startCol + 37 - 1;
            break;
    }

    OSScreenPutFontEx(SCREEN_TV, cursorCol, 3 + curRow, ">");

    OSScreenPutFontEx(SCREEN_TV, ScreenSizeData::tv_line_length / 2, 3, "Press B to go back.");
}

void ColorPage::ColorPickerSubpage::drawListDRC() const {}

bool ColorPage::ColorPickerSubpage::updatePicker(const VPADStatus& stat) {
    if(picking && board.isClosed()) {
        const std::array<std::string, 11>& textures = getModel().casual ? casualTextures : heroTextures;

        const std::string& name = textures[listScrollPos + curRow];
        board.open(name, "", getModel().getColor(name), 6);

        return true;
    }

    bool update = board.update(stat);
    if(board.isClosed()) {
        if(const std::optional<std::string>& input = board.getInput(); input.has_value()) {
            std::string color = input.value();
            color.resize(6, '0');
            if(getModel().casual) {
                getModel().setColor(casualTextures[listScrollPos + curRow], color);
            }
            else {
                getModel().setColor(heroTextures[listScrollPos + curRow], color);
            }
        }

        setPicking(false);

        return true;
    }

    return update;
}

void ColorPage::ColorPickerSubpage::drawPickerTV() const {
    board.drawTV(3, 0);
}

void ColorPage::ColorPickerSubpage::drawPickerDRC() const {
    board.drawDRC();
}



MetaPage::MetaPage() {}

void MetaPage::open() {}

bool MetaPage::update(const VPADStatus& stat) {
    //if (stat.trigger & VPAD_BUTTON_A) {
    //    SysAppBrowserArgs args;
    //    args.stdArgs.argString = nullptr;
    //    args.stdArgs.size = 0;
    //    args.url = GITHUB_URL.data();
    //    args.urlSize = GITHUB_URL.size();
    //    SYSSwitchToBrowserForViewer(&args);

    //    return true;
    //}
    //if (stat.trigger & VPAD_BUTTON_B) {
    //    SysAppBrowserArgs args;
    //    args.stdArgs.argString = nullptr;
    //    args.stdArgs.size = 0;
    //    args.url = DISCORD_URL.data();
    //    args.urlSize = DISCORD_URL.size();
    //    SYSSwitchToBrowserForViewer(&args);

    //    return true;
    //}
    
    return false;
}

void MetaPage::drawTV() const {
    const std::vector<std::string>& lines = wrap_string("Written by csunday95, gymnast86, and SuperDude88.\n\nIf you get stuck, check the seed's spoiler log in sd:/wiiu/apps/save/ ... (TWWHD Randomizer). Report any issues in the Discord server or create a GitHub issue.", ScreenSizeData::drc_line_length);

    const size_t startLine = 3;
    for(size_t i = 0; i < lines.size(); i++) {
        OSScreenPutFontEx(SCREEN_TV, 0, startLine + i, lines[i].c_str());
    }
}

void MetaPage::drawDRC() const {
    const std::vector<std::string>& descLines = wrap_string(getDesc(), ScreenSizeData::drc_line_length);

    const size_t startLine = ScreenSizeData::drc_num_lines - descLines.size();
    for(size_t i = 0; i < descLines.size(); i++) {
        OSScreenPutFontEx(SCREEN_DRC, 0, startLine + i, descLines[i].c_str());
    }
}
