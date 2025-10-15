#include "Page.hpp"

#include <utility/color.hpp>
#include <utility/file.hpp>
#include <utility/platform.hpp>
#include <platform/input.hpp>
#include <gui/wiiu/screen.hpp>
#include <gui/wiiu/TextWrap.hpp>
#include <command/Log.hpp>
#include <options.hpp>


#ifdef USE_LIBRPXLOADER
    #include <rpxloader/rpxloader.h>
#endif

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

void SeedPage::close() {}

bool SeedPage::update() {
    if(typing_seed || typing_perma) {
        bool update = board.update();
        if(board.isClosed()) {
            if(const std::optional<std::string>& input = board.getInput(); input.has_value()) {
                const std::string str = input.value();
                if(typing_seed) {
                    OptionCB::setSeed(str);
                }
                else if(typing_perma) {
                    OptionCB::loadPermalink(str);
                }
            }

            typing_seed = false;
            typing_perma = false;

            return true;
        }
        
        return update;
    }

    if(InputManager::getInstance().pressed(ButtonInfo::A)) {
        OptionCB::changeSeed();
        return true;
    }
    else if(InputManager::getInstance().pressed(ButtonInfo::X)) {
        typing_seed = true;
        board.open("Seed", "", getSeed(), std::nullopt);

        return true;
    }
    else if(InputManager::getInstance().pressed(ButtonInfo::Y)) {
        typing_perma = true;
        board.open("Permalink", "", "", std::nullopt); // would be annoying to backspace a ton of permalink

        return true;
    }

    if(!InputManager::getInstance().held(ButtonInfo::B)) {
        resetTimer();

        return true;
    }
    else {
        if(Clock::now() >= resetTime) {
            OptionCB::resetInternal();
            resetTimer();
        }

        return true;
    }

    return false;
}

void SeedPage::drawTV() const {
    using namespace std::literals::chrono_literals;
    
    if(typing_seed || typing_perma) {
        board.drawTV(PAGE_FIRST_ROW, 0);
    }
    else {
        OSScreenPutFontEx(SCREEN_TV, 0, PAGE_FIRST_ROW, ("The current seed is \"" + getSeed() + "\". Hash: " + getSeedHash()).c_str());
        OSScreenPutFontEx(SCREEN_TV, 0, PAGE_FIRST_ROW + 1, "Press A to generate a new seed, press X to enter manually");

        const std::vector<std::string>& permaLines = wrap_string("Permalink: \"" + getPermalink() + "\".", ScreenSizeData::tv_line_length);
        for(size_t i = 0; i < permaLines.size(); i++) {
            OSScreenPutFontEx(SCREEN_TV, 0, 6 + i, permaLines[i].c_str());
        }
        OSScreenPutFontEx(SCREEN_TV, 0, 6 + permaLines.size(),  "Press Y to enter a new permalink");

        // (total time - (final time - current time)) / (total duration / 10) = fraction of total in 10 increments
        const std::chrono::milliseconds remaining = 3s - std::chrono::duration_cast<std::chrono::milliseconds>(resetTime - Clock::now());
        const size_t count = remaining.count() / 300;
        std::string bar(10, ' ');
        bar.replace(0, count, count, '-');
        OSScreenPutFontEx(SCREEN_TV, 0, 6 + permaLines.size() + 2, ("Hold B to reset all settings to default [" + bar + "]").c_str());

        const std::vector<std::string>& warnLines = wrap_string(warnings, ScreenSizeData::tv_line_length);
        const size_t startLine = ScreenSizeData::tv_num_lines - 3 - warnLines.size();
        for(size_t i = 0; i < warnLines.size(); i++) {
            OSScreenPutFontEx(SCREEN_TV, 0, startLine + i, warnLines[i].c_str());
        }
    }
}

void SeedPage::drawDRC() const {
    if(typing_seed || typing_perma) {
        board.drawDRC();
    }
    else {
        const std::vector<std::string>& descLines = wrap_string(getDesc(), ScreenSizeData::drc_line_length);

        const size_t startLine = ScreenSizeData::drc_num_lines - descLines.size();
        for(size_t i = 0; i < descLines.size(); i++) {
            OSScreenPutFontEx(SCREEN_DRC, 0, startLine + i, descLines[i].c_str());
        }
    }
}



ProgressionPage::ProgressionPage() {
    using namespace std::literals::chrono_literals;

    buttonColumns[0][0] = std::make_unique<BasicButton>(Option::ProgressDungeons);
    buttonColumns[0][1] = std::make_unique<BasicButton>(Option::NumRequiredDungeons, 300ms, 250ms);
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

void ProgressionPage::close() {
    buttonColumns[curCol][curRow]->unhovered();
}

bool ProgressionPage::update() {
    bool moved = false;

    if(InputManager::getInstance().pressed(ButtonInfo::LEFT)) {
        buttonColumns[curCol][curRow]->unhovered();

        if(curCol <= 0) {
            curCol = buttonColumns.size() - 1; //wrap on leftmost row
        }
        else {
            curCol -= 1; //left one row
        }
        moved = true;

        buttonColumns[curCol][curRow]->hovered();
    }
    else if(InputManager::getInstance().pressed(ButtonInfo::RIGHT)) {
        buttonColumns[curCol][curRow]->unhovered();

        if(curCol >= buttonColumns.size() - 1) {
            curCol = 0; //wrap on rightmost row
        }
        else {
            curCol += 1; //right one row
        }
        moved = true;

        buttonColumns[curCol][curRow]->hovered();
    }

    if(InputManager::getInstance().pressed(ButtonInfo::UP)) {
        buttonColumns[curCol][curRow]->unhovered();

        if(curRow <= 0) {
            curRow = buttonColumns[curCol].size() - 1; //wrap on top
        }
        else {
            curRow -= 1; //up one row
        }
        moved = true;

        buttonColumns[curCol][curRow]->hovered();
    }
    else if(InputManager::getInstance().pressed(ButtonInfo::DOWN)) {
        buttonColumns[curCol][curRow]->unhovered();

        if(curRow >= buttonColumns[curCol].size() - 1) {
            curRow = 0; //wrap on buttom row
        }
        else {
            curRow += 1; //down one row
        }
        moved = true;

        buttonColumns[curCol][curRow]->hovered();
    }
    
    return moved || buttonColumns[curCol][curRow]->update();
}

void ProgressionPage::drawTV() const {
    for(size_t col = 0; col < buttonColumns.size(); col++) {
        const size_t startCol = (ScreenSizeData::tv_line_length / buttonColumns.size()) * col;
        for(size_t row = 0; row < buttonColumns[col].size(); row++) {
            //save 1 extra space for the cursor beside a button
            buttonColumns[col][row]->drawTV(PAGE_FIRST_ROW + row, startCol + 1, startCol + 1 + 30);
        }
    }

    OSScreenPutFontEx(SCREEN_TV, (ScreenSizeData::tv_line_length / buttonColumns.size()) * curCol, PAGE_FIRST_ROW + curRow, ">");
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

    // we can't use an initializer list for a vector<unique_ptr> because of copy/move issues
    buttonColumns[0].emplace_back(std::make_unique<BasicButton>(Option::HoHoHints));
    buttonColumns[0].emplace_back(std::make_unique<BasicButton>(Option::KorlHints));
    buttonColumns[0].emplace_back(std::make_unique<BasicButton>(Option::ClearerHints));
    buttonColumns[0].emplace_back(std::make_unique<BasicButton>(Option::UseAlwaysHints));
    buttonColumns[0].emplace_back(std::make_unique<BasicButton>(Option::HintImportance));

    buttonColumns[1].emplace_back(std::make_unique<BasicButton>(Option::PathHints, 300ms, 300ms));
    buttonColumns[1].emplace_back(std::make_unique<BasicButton>(Option::BarrenHints, 300ms, 300ms));
    buttonColumns[1].emplace_back(std::make_unique<BasicButton>(Option::ItemHints, 300ms, 300ms));
    buttonColumns[1].emplace_back(std::make_unique<BasicButton>(Option::LocationHints, 300ms, 300ms));

}

void HintsPage::open() {
    curCol = 0;
    curRow = 0;
}

void HintsPage::close() {
    buttonColumns[curCol][curRow]->unhovered();
}

bool HintsPage::update() {
    bool moved = false;

    if(InputManager::getInstance().pressed(ButtonInfo::LEFT)) {
        buttonColumns[curCol][curRow]->unhovered();
        if(curCol <= 0) {
            curCol = buttonColumns.size() - 1; //wrap on leftmost row
        }
        else {
            curCol -= 1; //left one row
        }
        moved = true;
    }
    else if(InputManager::getInstance().pressed(ButtonInfo::RIGHT)) {
        buttonColumns[curCol][curRow]->unhovered();
        if(curCol >= buttonColumns.size() - 1) {
            curCol = 0; //wrap on rightmost row
        }
        else {
            curCol += 1; //right one row
        }
        moved = true;
    }

    if(moved) {
        curCol = std::clamp<size_t>(curCol, 0, buttonColumns.size() - 1);
        curRow = std::clamp<size_t>(curRow, 0, buttonColumns[curCol].size() - 1);

        buttonColumns[curCol][curRow]->hovered();
    }

    if(InputManager::getInstance().pressed(ButtonInfo::UP)) {
        buttonColumns[curCol][curRow]->unhovered();
        if(curRow <= 0) {
            curRow = buttonColumns[curCol].size() - 1; //wrap on top
        }
        else {
            curRow -= 1; //up one row
        }
        moved = true;
    }
    else if(InputManager::getInstance().pressed(ButtonInfo::DOWN)) {
        buttonColumns[curCol][curRow]->unhovered();

        if(curRow >= buttonColumns[curCol].size() - 1) {
            curRow = 0; //wrap on buttom row
        }
        else {
            curRow += 1; //down one row
        }
        moved = true;
    }

    if(moved) {
        curCol = std::clamp<size_t>(curCol, 0, buttonColumns.size() - 1);
        curRow = std::clamp<size_t>(curRow, 0, buttonColumns[curCol].size() - 1);

        buttonColumns[curCol][curRow]->hovered();
    }

    return moved || buttonColumns[curCol][curRow]->update();
}

void HintsPage::drawTV() const {
    for(size_t col = 0; col < buttonColumns.size(); col++) {
        const size_t startCol = (ScreenSizeData::tv_line_length / buttonColumns.size()) * col;
        for(size_t row = 0; row < buttonColumns[col].size(); row++) {
            //save 1 extra space for the cursor beside a button
            buttonColumns[col][row]->drawTV(PAGE_FIRST_ROW + row, startCol + 1, startCol + 1 + 34);
        }
    }

    OSScreenPutFontEx(SCREEN_TV, (ScreenSizeData::tv_line_length / buttonColumns.size()) * curCol, PAGE_FIRST_ROW + curRow, ">");
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

void EntrancePage::close() {
    buttonColumns[curCol][curRow]->unhovered();
}

bool EntrancePage::update() {
    bool moved = false;

    if(InputManager::getInstance().pressed(ButtonInfo::LEFT)) {
        buttonColumns[curCol][curRow]->unhovered();

        if(curCol <= 0) {
            curCol = buttonColumns.size() - 1; //wrap on leftmost row
        }
        else {
            curCol -= 1; //left one row
        }
        moved = true;

        buttonColumns[curCol][curRow]->hovered();
    }
    else if(InputManager::getInstance().pressed(ButtonInfo::RIGHT)) {
        buttonColumns[curCol][curRow]->unhovered();

        if(curCol >= buttonColumns.size() - 1) {
            curCol = 0; //wrap on rightmost row
        }
        else {
            curCol += 1; //right one row
        }
        moved = true;

        buttonColumns[curCol][curRow]->hovered();
    }

    if(InputManager::getInstance().pressed(ButtonInfo::UP)) {
        buttonColumns[curCol][curRow]->unhovered();

        if(curRow <= 0) {
            curRow = buttonColumns[curCol].size() - 1; //wrap on top
        }
        else {
            curRow -= 1; //up one row
        }
        moved = true;

        buttonColumns[curCol][curRow]->hovered();
    }
    else if(InputManager::getInstance().pressed(ButtonInfo::DOWN)) {
        buttonColumns[curCol][curRow]->unhovered();

        if(curRow >= buttonColumns[curCol].size() - 1) {
            curRow = 0; //wrap on buttom row
        }
        else {
            curRow += 1; //down one row
        }
        moved = true;

        buttonColumns[curCol][curRow]->hovered();
    }
    
    return moved || buttonColumns[curCol][curRow]->update();
}

void EntrancePage::drawTV() const {
    for(size_t col = 0; col < buttonColumns.size(); col++) {
        const size_t startCol = (ScreenSizeData::tv_line_length / buttonColumns.size()) * col;
        for(size_t row = 0; row < buttonColumns[col].size(); row++) {
            //save 1 extra space for the cursor beside a button
            buttonColumns[col][row]->drawTV(PAGE_FIRST_ROW + row, startCol + 1, startCol + 1 + 30);
        }
    }

    OSScreenPutFontEx(SCREEN_TV, (ScreenSizeData::tv_line_length / buttonColumns.size()) * curCol, PAGE_FIRST_ROW + curRow, ">");
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
    buttonColumns[0].emplace_back(std::make_unique<BasicButton>(Option::InstantText));
    buttonColumns[0].emplace_back(std::make_unique<BasicButton>(Option::QuietSwiftSail));
    buttonColumns[0].emplace_back(std::make_unique<BasicButton>(Option::FixRNG));
    buttonColumns[0].emplace_back(std::make_unique<BasicButton>(Option::RevealSeaChart));
    buttonColumns[0].emplace_back(std::make_unique<BasicButton>(Option::RemoveMusic));
    buttonColumns[0].emplace_back(std::make_unique<BasicButton>(Option::Camera));
    buttonColumns[0].emplace_back(std::make_unique<BasicButton>(Option::FirstPersonCamera));
    buttonColumns[0].emplace_back(std::make_unique<BasicButton>(Option::TargetType));

    buttonColumns[1].emplace_back(std::make_unique<BasicButton>(Option::AddShortcutWarps));
    buttonColumns[1].emplace_back(std::make_unique<BasicButton>(Option::SkipRefights));
    buttonColumns[1].emplace_back(std::make_unique<BasicButton>(Option::InvertCompass));
    buttonColumns[1].emplace_back(std::make_unique<BasicButton>(Option::Performance));
    buttonColumns[1].emplace_back(std::make_unique<BasicButton>(Option::PigColor));
    buttonColumns[1].emplace_back(std::make_unique<BasicButton>(Option::Gyroscope));
    buttonColumns[1].emplace_back(std::make_unique<BasicButton>(Option::UIDisplay));
}

void ConveniencePage::open() {
    curCol = 0;
    curRow = 0;
}

void ConveniencePage::close() {
    buttonColumns[curCol][curRow]->unhovered();
}

bool ConveniencePage::update() {
    bool moved = false;

    if(InputManager::getInstance().pressed(ButtonInfo::LEFT)) {
        buttonColumns[curCol][curRow]->unhovered();
        if(curCol <= 0) {
            curCol = buttonColumns.size() - 1; //wrap on leftmost row
        }
        else {
            curCol -= 1; //left one row
        }
        moved = true;
    }
    else if(InputManager::getInstance().pressed(ButtonInfo::RIGHT)) {
        buttonColumns[curCol][curRow]->unhovered();
        if(curCol >= buttonColumns.size() - 1) {
            curCol = 0; //wrap on rightmost row
        }
        else {
            curCol += 1; //right one row
        }
        moved = true;
    }

    if(moved) {
        curCol = std::clamp<size_t>(curCol, 0, buttonColumns.size() - 1);
        curRow = std::clamp<size_t>(curRow, 0, buttonColumns[curCol].size() - 1);

        buttonColumns[curCol][curRow]->hovered();
    }

    if(InputManager::getInstance().pressed(ButtonInfo::UP)) {
        buttonColumns[curCol][curRow]->unhovered();
        if(curRow <= 0) {
            curRow = buttonColumns[curCol].size() - 1; //wrap on top
        }
        else {
            curRow -= 1; //up one row
        }
        moved = true;
    }
    else if(InputManager::getInstance().pressed(ButtonInfo::DOWN)) {
        buttonColumns[curCol][curRow]->unhovered();

        if(curRow >= buttonColumns[curCol].size() - 1) {
            curRow = 0; //wrap on buttom row
        }
        else {
            curRow += 1; //down one row
        }
        moved = true;
    }

    if(moved) {
        curCol = std::clamp<size_t>(curCol, 0, buttonColumns.size() - 1);
        curRow = std::clamp<size_t>(curRow, 0, buttonColumns[curCol].size() - 1);

        buttonColumns[curCol][curRow]->hovered();
    }

    return moved || buttonColumns[curCol][curRow]->update();
}

void ConveniencePage::drawTV() const {
    for(size_t col = 0; col < buttonColumns.size(); col++) {
        const size_t startCol = (ScreenSizeData::tv_line_length / buttonColumns.size()) * col;
        for(size_t row = 0; row < buttonColumns[col].size(); row++) {
            //save 1 extra space for the cursor beside a button
            buttonColumns[col][row]->drawTV(PAGE_FIRST_ROW + row, startCol + 1, startCol + 1 + 30);
        }
    }

    OSScreenPutFontEx(SCREEN_TV, (ScreenSizeData::tv_line_length / buttonColumns.size()) * curCol, PAGE_FIRST_ROW + curRow, ">");
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
    buttonColumns[0][1] = std::make_unique<BasicButton>(Option::DamageMultiplier, 250ms, 55ms);
    buttonColumns[0][2] = std::make_unique<BasicButton>(Option::ClassicMode);

    buttonColumns[1][0] = std::make_unique<BasicButton>(Option::CTMC);
    buttonColumns[1][1] = std::make_unique<BasicButton>(Option::Plandomizer);
    buttonColumns[1][2] = std::make_unique<BasicButton>(Option::RandomItemSlideItem);
}

void AdvancedPage::open() {
    curCol = 0;
    curRow = 0;
}

void AdvancedPage::close() {
    buttonColumns[curCol][curRow]->unhovered();
}

bool AdvancedPage::update() {
    bool moved = false;

    if(InputManager::getInstance().pressed(ButtonInfo::LEFT)) {
        buttonColumns[curCol][curRow]->unhovered();

        if(curCol <= 0) {
            curCol = buttonColumns.size() - 1; //wrap on leftmost row
        }
        else {
            curCol -= 1; //left one row
        }
        moved = true;

        buttonColumns[curCol][curRow]->hovered();
    }
    else if(InputManager::getInstance().pressed(ButtonInfo::RIGHT)) {
        buttonColumns[curCol][curRow]->unhovered();

        if(curCol >= buttonColumns.size() - 1) {
            curCol = 0; //wrap on rightmost row
        }
        else {
            curCol += 1; //right one row
        }
        moved = true;

        buttonColumns[curCol][curRow]->hovered();
    }

    if(InputManager::getInstance().pressed(ButtonInfo::UP)) {
        buttonColumns[curCol][curRow]->unhovered();

        if(curRow <= 0) {
            curRow = buttonColumns[curCol].size() - 1; //wrap on top
        }
        else {
            curRow -= 1; //up one row
        }
        moved = true;

        buttonColumns[curCol][curRow]->hovered();
    }
    else if(InputManager::getInstance().pressed(ButtonInfo::DOWN)) {
        buttonColumns[curCol][curRow]->unhovered();

        if(curRow >= buttonColumns[curCol].size() - 1) {
            curRow = 0; //wrap on buttom row
        }
        else {
            curRow += 1; //down one row
        }
        moved = true;

        buttonColumns[curCol][curRow]->hovered();
    }
    
    return moved || buttonColumns[curCol][curRow]->update();
}

void AdvancedPage::drawTV() const {
    for(size_t col = 0; col < buttonColumns.size(); col++) {
        const size_t startCol = (ScreenSizeData::tv_line_length / buttonColumns.size()) * col;
        for(size_t row = 0; row < buttonColumns[col].size(); row++) {
            //save 1 extra space for the cursor beside a button
            buttonColumns[col][row]->drawTV(PAGE_FIRST_ROW + row, startCol + 1, startCol + 1 + 30);
        }
    }

    OSScreenPutFontEx(SCREEN_TV, (ScreenSizeData::tv_line_length / buttonColumns.size()) * curCol, PAGE_FIRST_ROW + curRow, ">");
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

    countButtons[0] = std::make_unique<BasicButton>(Option::StartingHP, 300ms, 100ms);
    countButtons[1] = std::make_unique<BasicButton>(Option::StartingHC, 300ms, 250ms);
    countButtons[2] = std::make_unique<BasicButton>(Option::StartingJoyPendants, 300ms, 100ms);
    countButtons[3] = std::make_unique<BasicButton>(Option::StartingSkullNecklaces, 300ms, 150ms);
    countButtons[4] = std::make_unique<BasicButton>(Option::StartingBokoBabaSeeds, 300ms, 200ms);
    countButtons[5] = std::make_unique<BasicButton>(Option::StartingGoldenFeathers, 300ms, 120ms);
    countButtons[6] = std::make_unique<BasicButton>(Option::StartingKnightsCrests, 300ms, 200ms);
    countButtons[7] = std::make_unique<BasicButton>(Option::StartingRedChuJellys, 300ms, 130ms);
    countButtons[8] = std::make_unique<BasicButton>(Option::StartingGreenChuJellys, 300ms, 130ms);
    countButtons[9] = std::make_unique<BasicButton>(Option::StartingBlueChuJellys, 300ms, 130ms);
    countButtons[10] = std::make_unique<BasicButton>(Option::StartWithRandomItem);
}

void ItemsPage::open() {
    curCol = Column::LIST;
    curRow = 0;
    listScrollPos = 0;

    if(getValue(Option::RemoveSwords) == "Disabled") { 
        // only add swords if they aren't already there
        if(std::find(listButtons.begin(), listButtons.end(), GameItem::ProgressiveSword) == listButtons.end()) {
            for(size_t count = 1; count <= 4; count++) {
                listButtons.emplace_back(GameItem::ProgressiveSword, count); // add 4 swords
            }
        }
        // do the same for hurricane spin
        if(std::find(listButtons.begin(), listButtons.end(), GameItem::HurricaneSpin) == listButtons.end()) {
            listButtons.emplace_back(GameItem::HurricaneSpin); // add hurricane spin
        }
    }
    else {
        std::erase(listButtons, GameItem::ProgressiveSword); // erase all swords
        std::erase(listButtons, GameItem::HurricaneSpin); // erase hurricane spin
    }
    
    // make sure our buttons are in order
    std::sort(listButtons.begin(), listButtons.end());

    // load the items currently set in the config
    for(ItemButton& button : listButtons) {
        button.loadState();
    }
}

void ItemsPage::close() {
    switch(curCol) {
        case Column::LIST:
            listButtons[listScrollPos + curRow].unhovered();
            break;
        case Column::BUTTONS:
            countButtons[curRow]->unhovered();
            break;
    }
}

bool ItemsPage::update() {
    bool moved = false;

    if(InputManager::getInstance().pressed(ButtonInfo::LEFT) || InputManager::getInstance().pressed(ButtonInfo::RIGHT)) {
        switch(curCol) {
            case Column::LIST:
                listButtons[listScrollPos + curRow].unhovered();
                curCol = Column::BUTTONS;
                curRow = std::clamp<size_t>(curRow, 0, countButtons.size() - 1);
                countButtons[curRow]->hovered();

                break;
            case Column::BUTTONS:
                countButtons[curRow]->unhovered();
                curCol = Column::LIST;
                listButtons[listScrollPos + curRow].hovered();

                break;
        }

        moved = true;
    }

    if(InputManager::getInstance().pressed(ButtonInfo::UP)) {
        switch(curCol) {
            case Column::LIST:
                listButtons[listScrollPos + curRow].unhovered();

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

                listButtons[listScrollPos + curRow].hovered();

                break;
            case Column::BUTTONS:
                countButtons[curRow]->unhovered();

                if(curRow <= 0) {
                    curRow = countButtons.size() - 1; //wrap on top row
                }
                else {
                    curRow -= 1; //up one row
                }
                
                countButtons[curRow]->hovered();

                break;
        }

        moved = true;
    }
    else if(InputManager::getInstance().pressed(ButtonInfo::DOWN)) {
        switch(curCol) {
            case Column::LIST:
                listButtons[listScrollPos + curRow].unhovered();

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
                
                listButtons[listScrollPos + curRow].hovered();

                break;
            case Column::BUTTONS:
                countButtons[curRow]->unhovered();

                if(curRow >= countButtons.size() - 1) {
                    curRow = 0; //wrap on bottom row
                }
                else {
                    curRow += 1; //down one row
                }
                
                countButtons[curRow]->hovered();

                break;
        }

        moved = true;
    }

    bool btnUpdate = false;
    switch(curCol) {
        case Column::LIST:
            if(listButtons[listScrollPos + curRow].update()) {
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
            btnUpdate = countButtons[curRow]->update();
            break;
    }
    
    return moved || btnUpdate;
}

void ItemsPage::drawTV() const {
    // draw visible part of the list
    const size_t listStartCol = 0;
    for(size_t row = 0; row < LIST_HEIGHT; row++) {
        listButtons[listScrollPos + row].drawTV(PAGE_FIRST_ROW + row, 3, 1);
    }

    // draw second column of buttons
    const size_t countStartCol = ScreenSizeData::tv_line_length / 2;
    for(size_t row = 0; row < countButtons.size(); row++) {
        countButtons[row]->drawTV(PAGE_FIRST_ROW + row, countStartCol + 1, countStartCol + 1 + 30);
    }
    
    // draw cursor
    switch(curCol) {
        case Column::LIST:
            OSScreenPutFontEx(SCREEN_TV, listStartCol, PAGE_FIRST_ROW + curRow, ">");

            break;
        case Column::BUTTONS:
            OSScreenPutFontEx(SCREEN_TV, countStartCol, PAGE_FIRST_ROW + curRow, ">");
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

LocationsPage::LocationsPage() {
    // Read location_data.yaml and map each location to the categories
    // it has
    YAML::Node locationDataTree;
    if(!LoadYAML(locationDataTree, Utility::get_data_path() / "logic/location_data.yaml", true)) {
        ErrorLog::getInstance().log("Failed to load location_data.yaml");
        return;
    }

    for (const auto& locationObject : locationDataTree)
    {
        auto locName = locationObject["Names"]["English"].as<std::string>();
        locationCategories[locName] = {};
        for (const auto& category : locationObject["Category"])
        {
            const auto& cat = nameToLocationCategory(category.as<std::string>());
            // Ignore invalid categories. Those should've been caught in testing
            // and erroring here won't help anything
            if (cat == LocationCategory::INVALID)
            {
                continue;
            }
            locationCategories[locName].insert(cat);
        }
    }
}

void LocationsPage::open() {
    // Clear and reset the list whenever we open the page
    curRow = 0;
    listScrollPos = 0;

    listButtons.clear();
    // Names are already sorted alphabetically due to being
    // in a std::map
    for (const auto& [locName, categories] : locationCategories) {
        if (OptionCB::hasAllCategories(categories) && locName != "Ganon's Tower - Defeat Ganondorf")
        {
            listButtons.emplace_back(locName, categories);
            listButtons.back().loadState();
        }
    }
}

void LocationsPage::close() {
    listButtons[listScrollPos + curRow].unhovered();
}

bool LocationsPage::update() {
    bool moved = false;

    if(InputManager::getInstance().pressed(ButtonInfo::UP)) {
        listButtons[listScrollPos + curRow].unhovered();

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

        listButtons[listScrollPos + curRow].hovered();

        moved = true;
    }
    else if(InputManager::getInstance().pressed(ButtonInfo::DOWN)) {

        listButtons[listScrollPos + curRow].unhovered();

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
        
        listButtons[listScrollPos + curRow].hovered();

        moved = true;
    }

    bool btnUpdate = false;
    if(listButtons[listScrollPos + curRow].update()) {
        btnUpdate = true;

        //update excluded locations
        OptionCB::clearExcludedLocations();
        for(const auto& button : listButtons) {
            if(button.isEnabled()) {
                OptionCB::addExcludedLocation(button.getLocationName());
            }
        }
    }
    
    return moved || btnUpdate;
}

void LocationsPage::drawTV() const {
    // draw visible part of the list
    const size_t listStartCol = 0;
    for(size_t row = 0; row < LIST_HEIGHT; row++) {
        listButtons[listScrollPos + row].drawTV(PAGE_FIRST_ROW + row, 3, 1);
    }
    
    // draw cursor
    OSScreenPutFontEx(SCREEN_TV, listStartCol, PAGE_FIRST_ROW + curRow, ">");
}

void LocationsPage::drawDRC() const {
    listButtons[listScrollPos + curRow].drawDRC();

    const std::vector<std::string>& descLines = wrap_string(getDesc(), ScreenSizeData::drc_line_length);
    const size_t startLine = ScreenSizeData::drc_num_lines - descLines.size();
    for(size_t i = 0; i < descLines.size(); i++) {
        OSScreenPutFontEx(SCREEN_DRC, 0, startLine + i, descLines[i].c_str());
    }
}

ColorPage::ColorPage() :
    presets(*this),
    color(*this),
    model(*this)
{}

void ColorPage::open() {
    curSubpage = Subpage::PRESETS;
}

void ColorPage::close() {}

bool ColorPage::update() {
    switch(curSubpage) {
        case Subpage::PRESETS:
            return presets.update();
        case Subpage::COLOR_PICKER:
            return color.update();
        case Subpage::MODEL_PICKER:
            return model.update();
    }

    return false;
}

void ColorPage::drawTV() const {
    switch(curSubpage) {
        case Subpage::PRESETS:
            return presets.drawTV();
        case Subpage::COLOR_PICKER:
            return color.drawTV();
        case Subpage::MODEL_PICKER:
            return model.drawTV();
    }
}

void ColorPage::drawDRC() const {
    switch(curSubpage) {
        case Subpage::PRESETS:
            return presets.drawDRC();
        case Subpage::COLOR_PICKER:
            return color.drawDRC();
        case Subpage::MODEL_PICKER:
            return model.drawDRC();
    }
}

ColorPage::PresetsSubpage::PresetsSubpage(ColorPage& parent_) : 
    parent(parent_)
{
    toggles[0] = std::make_unique<FunctionButton>("Select Link Model", "Select a model for Link. To install more, read https://github.com/SuperDude88/TWWHD-Randomizer/tree/main/customizer/INSTALL.md", std::bind(&ColorPage::setSubpage, &parent, Subpage::MODEL_PICKER));
    toggles[1] = std::make_unique<ActionButton>("Casual Clothes", "Enable this if you want to wear your casual clothes instead of the Hero's Clothes. Custom models will have their own variant of casual clothes", &OptionCB::toggleCasualClothes, &OptionCB::isCasual);
    toggles[2] = std::make_unique<ActionButton>("Randomize Colors Orderly", "", &OptionCB::randomizeColorsOrderly);
    toggles[3] = std::make_unique<ActionButton>("Randomize Colors Chaotically", "", &OptionCB::randomizeColorsChaotically);
    toggles[4] = std::make_unique<FunctionButton>("Select Colors Manually", "", std::bind(&ColorPage::setSubpage, &parent, Subpage::COLOR_PICKER));
}

void ColorPage::PresetsSubpage::open() {
    
    listScrollPos = 0;
    curRow = 0;
    curCol = Column::BUTTONS;
}

void ColorPage::PresetsSubpage::close() {}

bool ColorPage::PresetsSubpage::update() {
    bool moved = false;
    if (getModel().user_provided) {
        curCol = Column::BUTTONS;
    }

    if(!getModel().user_provided && (InputManager::getInstance().pressed(ButtonInfo::LEFT) || InputManager::getInstance().pressed(ButtonInfo::RIGHT))) {
        switch(curCol) {
            case Column::LIST:
                curCol = Column::BUTTONS;
                curRow = std::clamp<size_t>(curRow, 0, toggles.size() - 1); 

                break;
            case Column::BUTTONS:
                curCol = Column::LIST;
                curRow = std::clamp<size_t>(curRow, 1, std::min(LIST_HEIGHT, getModel().getPresets().size()) - 1); //cant select extra text

                break;
        }

        moved = true;
    }

    if(InputManager::getInstance().pressed(ButtonInfo::UP)) {
        switch(curCol) {
            case Column::LIST:
                if(curRow <= 1) {
                    if(listScrollPos <= 0) {
                        curRow = std::min(LIST_HEIGHT, getModel().getPresets().size()); // no -1 because you account for extra text
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
                    curRow = (getModel().user_provided ? 2 : toggles.size()) - 1; //wrap on top row
                }
                else {
                    curRow -= 1; //up one row
                }

                break;
        }

        moved = true;
    }
    else if(InputManager::getInstance().pressed(ButtonInfo::DOWN)) {
        switch(curCol) {
            case Column::LIST:
                //account for additional text
                if(curRow >= std::min(LIST_HEIGHT, getModel().getPresets().size())) {
                    if(getModel().getPresets().size() > LIST_HEIGHT) {
                        if(listScrollPos >= getModel().getPresets().size() - LIST_HEIGHT) {
                            listScrollPos = 0;
                            curRow = 1;
                        }
                        else {
                            listScrollPos += 1;
                        }
                    }
                    else {
                        listScrollPos = 0;
                        curRow = 1;
                    }
                }
                else {
                    curRow += 1;
                }

                break;
            case Column::BUTTONS:
                if (curRow >= (getModel().user_provided ? 2 : toggles.size()) - 1) {
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
            if(InputManager::getInstance().pressed(ButtonInfo::A) && curRow != 0) {
                btnUpdate = true;

                getModel().loadPreset(listScrollPos + curRow - 1);
            }

            break;
        case Column::BUTTONS:
            btnUpdate = toggles[curRow]->update();
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
    // draw current link model name
    std::string model = getModel().modelName;

    if (getModel().modelName.empty()) {
        model = "Link";
    }
    else if (getModel().modelName == "?") {
        model = "Random Model";
    }
    OSScreenPutFontEx(SCREEN_TV, 1, PAGE_FIRST_ROW, (std::string("Selected Model : ") + model).c_str());
    const size_t countStartCol = ScreenSizeData::tv_line_length / 2;
    //show the color stuff only if the selected model isnt custom
    if (!getModel().user_provided) {
        // draw visible part of the list
        for (size_t row = 0; row < std::min(LIST_HEIGHT, getModel().getPresets().size()); row++) {
            OSScreenPutFontEx(SCREEN_TV, 1, PAGE_FIRST_ROW + row + 1, getModel().getPresets()[listScrollPos + row].name.c_str());
        }

        // draw second column of buttons
        
        for (size_t row = 0; row < toggles.size(); row++) {
            toggles[row]->drawTV(PAGE_FIRST_ROW + row, countStartCol + 1, countStartCol + 1 + 30);
        }

        // draw loaded colors
        static constexpr uint32_t starting_y_pos = 271;
        const std::array<std::string, 11>& textures = getModel().casual ? casualTextures : heroTextures;
        for (size_t i = 0; i < textures.size(); i++) {
            const size_t row = PAGE_FIRST_ROW + 6 + i;
            OSScreenPutFontEx(SCREEN_TV, countStartCol + 1, row, textures[i].c_str());

            std::string hexColor = getModel().getColor(textures[i]);
            for (auto& c : hexColor) {
                c = std::toupper(c); //capitalize for consistency
            }

            OSScreenPutFontEx(SCREEN_TV, countStartCol + 1 + 15, row, hexColor.c_str());
            drawSquare(std::stoi(hexColor, nullptr, 16) << 8, 13 * (countStartCol + 1 + 15 + 6), starting_y_pos + i * 24, 24);
        }
    }
    else {
        //still show the model page button and the casual clothes toggle
        for (size_t row = 0; row < 2; row++) {
            toggles[row]->drawTV(PAGE_FIRST_ROW + row, countStartCol + 1, countStartCol + 1 + 30);
        }
        //add some text to not make the user confused
        const std::vector<std::string>& messageLines = wrap_string("Color options are not available when using a custom model. Select the default model (Link) to access the color customizer.", ScreenSizeData::tv_line_length / 3);
        for (size_t i = 0; i < messageLines.size(); i++) {
            OSScreenPutFontEx(SCREEN_TV, 1, PAGE_FIRST_ROW + i + 1, messageLines[i].c_str());
        }
    }


   
    
    // draw cursor
    switch(curCol) {
        case Column::LIST:
            OSScreenPutFontEx(SCREEN_TV, 0, PAGE_FIRST_ROW + curRow, ">");
            break;
        case Column::BUTTONS:
            OSScreenPutFontEx(SCREEN_TV, countStartCol, PAGE_FIRST_ROW + curRow, ">");
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

void ColorPage::ColorPickerSubpage::close() {}

bool ColorPage::ColorPickerSubpage::update() {
    if(picking) {
        return updateColorPicker();
    }
    else {
        return updateList();
    }
}

void ColorPage::ColorPickerSubpage::drawTV() const {
    if(picking) {
        return drawColorPickerTV();
    }
    else {
        return drawListTV();
    }
}

void ColorPage::ColorPickerSubpage::drawDRC() const {
    if(picking) {
        return drawColorPickerDRC();
    }
    else {
        return drawListDRC();
    }
}

bool ColorPage::ColorPickerSubpage::updateList() {
    if (InputManager::getInstance().pressed(ButtonInfo::B)) {
        parent.setSubpage(Subpage::PRESETS);
        return true;
    }

    bool moved = false;
    if(InputManager::getInstance().pressed(ButtonInfo::LEFT)) {
        if(curCol <= 0) {
            curCol = actions.size() - 1; //wrap on leftmost row
        }
        else {
            curCol -= 1; //left one row
        }
        moved = true;
    }
    else if(InputManager::getInstance().pressed(ButtonInfo::RIGHT)) {
        if(curCol >= actions.size() - 1) {
            curCol = 0; //wrap on rightmost row
        }
        else {
            curCol += 1; //right one row
        }
        moved = true;
    }

    if(InputManager::getInstance().pressed(ButtonInfo::UP)) {
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
    else if(InputManager::getInstance().pressed(ButtonInfo::DOWN)) {
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
        btnUpdate = actions[curCol].update(casualTextures[listScrollPos + curRow]);
    }
    else {
        btnUpdate = actions[curCol].update(heroTextures[listScrollPos + curRow]);
    }
    
    return moved || btnUpdate;
}

void ColorPage::ColorPickerSubpage::drawListTV() const {
    static constexpr size_t startCol = 2;
    static constexpr uint32_t starting_y_pos = 128;
    const std::array<std::string, 11>& textures = getModel().casual ? casualTextures : heroTextures;
    for(size_t row = 0; row < std::min(LIST_HEIGHT, textures.size()); row++) {
        OSScreenPutFontEx(SCREEN_TV, startCol, row + PAGE_FIRST_ROW, textures[listScrollPos + row].c_str());

        std::string hexColor = getModel().getColor(textures[listScrollPos + row]);
        for(auto& c : hexColor) {
            c = std::toupper(c); //capitalize for consistency
        }

        OSScreenPutFontEx(SCREEN_TV, startCol + 13, PAGE_FIRST_ROW + row, hexColor.c_str());
        drawSquare(std::stoi(hexColor, nullptr, 16) << 8, 14 * (startCol + 13 + 7), starting_y_pos + row * 24, 24);
        
        actions[0].drawTV(row + PAGE_FIRST_ROW, startCol + 23, startCol + 23 + 1);
        actions[1].drawTV(row + PAGE_FIRST_ROW, startCol + 31, startCol + 31 + 1);
        actions[2].drawTV(row + PAGE_FIRST_ROW, startCol + 37, startCol + 37 + 1);
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

    OSScreenPutFontEx(SCREEN_TV, cursorCol, PAGE_FIRST_ROW + curRow, ">");

    OSScreenPutFontEx(SCREEN_TV, ScreenSizeData::tv_line_length / 2, PAGE_FIRST_ROW, "Press B to go back.");
}

void ColorPage::ColorPickerSubpage::drawListDRC() const {}

bool ColorPage::ColorPickerSubpage::updateColorPicker() {
    if(picking && board.isClosed()) {
        const std::array<std::string, 11>& textures = getModel().casual ? casualTextures : heroTextures;

        const std::string& name = textures[listScrollPos + curRow];
        board.open(name, "", getModel().getColor(name), 6);

        return true;
    }

    bool update = board.update();
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

void ColorPage::ColorPickerSubpage::drawColorPickerTV() const {
    board.drawTV(PAGE_FIRST_ROW, 0);
}

void ColorPage::ColorPickerSubpage::drawColorPickerDRC() const {
    board.drawDRC();
}

ColorPage::ModelPickerSubpage::ModelPickerSubpage(ColorPage& parent_) :
    parent(parent_)
{}

void ColorPage::ModelPickerSubpage::open() {
    curCol = 0;
    curRow = 0;
    listScrollPos = 0;

    listButtons.clear();

    listButtons.emplace_back("Link");
    listButtons.emplace_back("Random Model");
    for (size_t i = 0; i < listButtons.size(); i++) {
        listButtons[i].setEnabled(false);
    }

    if (!getModel().user_provided || !std::filesystem::is_directory(Utility::get_models_dir() / getModel().modelName)) {
        listButtons[0].setEnabled(true);
    }

    if (getModel().modelName == "?") {
        listButtons[1].setEnabled(true);
    }

    //one button for each model, select the one that corresponds to the preferences file
    for (auto entry : std::filesystem::directory_iterator(Utility::get_models_dir())) {
        if (!entry.is_directory()) continue;
        listButtons.emplace_back(entry.path().filename());
        if (entry.path().filename() == getModel().modelName) { 
            listButtons[listButtons.size() - 1].setEnabled(true);
        }
    }
    //if theres no custom model, remove the "random model" button
    if (listButtons.size() <= 2) {
        listButtons.pop_back();
    }
    

}

void ColorPage::ModelPickerSubpage::close() {}

bool ColorPage::ModelPickerSubpage::update() {
    if (InputManager::getInstance().pressed(ButtonInfo::B)) {
        parent.setSubpage(Subpage::PRESETS);
        return true;
    }
    bool moved = false;
    if (InputManager::getInstance().pressed(ButtonInfo::UP)) {
        listButtons[listScrollPos + curRow].unhovered();
        if (curRow <= 0) {
            if (listScrollPos <= 0) {
                if (listButtons.size() <= LIST_HEIGHT) {
                    listScrollPos = 0;
                    curRow = listButtons.size() - 1;
                }
                else {
                    curRow = LIST_HEIGHT - 1; // -1 because 0-indexed
                    listScrollPos = listButtons.size() - LIST_HEIGHT;
                }
            }
            else {
                listScrollPos -= 1;
            }
        }
        else {
            curRow -= 1;
        }

        listButtons[listScrollPos + curRow].hovered();
        moved = true;
    }

    else if (InputManager::getInstance().pressed(ButtonInfo::DOWN)) {
        listButtons[listScrollPos + curRow].unhovered();
        if (curRow >= std::min(LIST_HEIGHT, listButtons.size()) - 1) {
            if (listButtons.size() > LIST_HEIGHT) {
                if (listScrollPos >= listButtons.size() - LIST_HEIGHT) {
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

        listButtons[listScrollPos + curRow].hovered();
        moved = true;
    }

    bool btnUpdate = false;
    //only one custom model can be selected, disable every other
    if (listButtons[listScrollPos + curRow].update()) {
        for (size_t i = 0; i < listButtons.size(); i++) {
            listButtons[i].setEnabled(false);
        }
        listButtons[listScrollPos + curRow].setEnabled(true);
        btnUpdate = true;
    }
    return moved || btnUpdate;
}

void ColorPage::ModelPickerSubpage::drawTV() const {
    // draw visible part of the list
    const size_t listStartCol = 0;
    if (listButtons.size() > LIST_HEIGHT) {
        for (size_t row = 0; row < LIST_HEIGHT; row++) {
            listButtons[listScrollPos + row].drawTV(PAGE_FIRST_ROW + row, 3, 1);
        }
    }
    else {
        for (size_t row = 0; row < listButtons.size(); row++) {
            listButtons[listScrollPos + row].drawTV(PAGE_FIRST_ROW + row, 3, 1);
        }
    }
    OSScreenPutFontEx(SCREEN_TV, ScreenSizeData::tv_line_length / 2, PAGE_FIRST_ROW, "Select a custom model from the list.");
    OSScreenPutFontEx(SCREEN_TV, ScreenSizeData::tv_line_length / 2, PAGE_FIRST_ROW + 1, "Press B to go back.");
    OSScreenPutFontEx(SCREEN_TV, listStartCol, PAGE_FIRST_ROW + curRow, ">");
}

void ColorPage::ModelPickerSubpage::drawDRC() const {
    
}


MetaPage::MetaPage() {
    using namespace std::literals::string_literals;

    #ifdef USE_LIBRPXLOADER
        uint32_t version;
        if(const RPXLoaderStatus err = RPXLoader_GetVersion(&version); err != RPX_LOADER_RESULT_SUCCESS) {
            LOG_TO_DEBUG("Displaying fallback save path, RPXLoader_GetVersion encountered error " + std::to_string(err));
            return;
        }
        
        if(version < 3) {
            LOG_TO_DEBUG("Displaying fallback save path, librpxloader is outdated.");
            return;
        }

        if(const RPXLoaderStatus err = RPXLoader_InitLibrary(); err != RPX_LOADER_RESULT_SUCCESS) {
            LOG_TO_DEBUG("Displaying fallback save path, RPXLoader_InitLibrary encountered error " + std::to_string(err));
            return;
        }
        rpxLoaderInit = true;

        std::array<char, 128> pathBuf{}; // probably bigger than necessary
        if(const RPXLoaderStatus err = RPXLoader_GetPathOfSaveRedirection(pathBuf.data(), pathBuf.size()); err != RPX_LOADER_RESULT_SUCCESS) {
            LOG_TO_DEBUG("Displaying fallback save path, RPXLoader_GetPathOfSaveRedirection encountered error " + std::to_string(err));
            return;
        }

        savePath = "sd:/"s + pathBuf.data();
    #endif
}

MetaPage::~MetaPage() {
    #ifdef USE_LIBRPXLOADER
        if(rpxLoaderInit) {
            if(const RPXLoaderStatus err = RPXLoader_DeInitLibrary(); err != RPX_LOADER_RESULT_SUCCESS) {
                LOG_TO_DEBUG("RPXLoader_DeInitLibrary encountered error " + std::to_string(err));
                return;
            }

            rpxLoaderInit = false;
        }
    #endif
}

void MetaPage::open() {}

void MetaPage::close() {}

bool MetaPage::update() {
    //if (InputManager::getInstance().pressed(ButtonInfo::A)) {
    //    SysAppBrowserArgs args;
    //    args.stdArgs.argString = nullptr;
    //    args.stdArgs.size = 0;
    //    args.url = GITHUB_URL.data();
    //    args.urlSize = GITHUB_URL.size();
    //    SYSSwitchToBrowserForViewer(&args);

    //    return true;
    //}
    //if (InputManager::getInstance().pressed(ButtonInfo::B)) {
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
    const std::vector<std::string>& lines = wrap_string("Written by csunday95, gymnast86, and SuperDude88.\n\nIf you get stuck, check the seed's spoiler log in " + savePath + ".\nReport any issues in the Discord server or create a GitHub issue.", ScreenSizeData::tv_line_length);

    for(size_t i = 0; i < lines.size(); i++) {
        OSScreenPutFontEx(SCREEN_TV, 0, PAGE_FIRST_ROW + i, lines[i].c_str());
    }
}

void MetaPage::drawDRC() const {
    const std::vector<std::string>& descLines = wrap_string(getDesc(), ScreenSizeData::drc_line_length);

    const size_t startLine = ScreenSizeData::drc_num_lines - descLines.size();
    for(size_t i = 0; i < descLines.size(); i++) {
        OSScreenPutFontEx(SCREEN_DRC, 0, startLine + i, descLines[i].c_str());
    }
}
