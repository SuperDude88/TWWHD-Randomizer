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
