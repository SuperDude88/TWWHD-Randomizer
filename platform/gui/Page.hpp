#pragma once

#include <platform/gui/Button.hpp>

class EmptyPage {
public:
    virtual ~EmptyPage() {}

    virtual std::string getName() const = 0;
    virtual std::string getDesc() const = 0;

    virtual void open() = 0;
    virtual bool update(const VPADStatus& stat) = 0;
    virtual void drawTV() const = 0;
    virtual void drawDRC() const = 0;
};

class SeedPage : public EmptyPage {
private:
    using Clock = std::chrono::high_resolution_clock;
    Clock::time_point resetTime;

    void resetTimer() { resetTime = Clock::now() + std::chrono::seconds(3); }

public:
    SeedPage();
    
    std::string getName() const { return "Seed"; }
    std::string getDesc() const { return "Control the seed used to randomize."; }

    void open();
    bool update(const VPADStatus& stat);
    void drawTV() const;
    void drawDRC() const;
};

class ProgressionPage : public EmptyPage {
private:
    size_t curCol = 0;
    size_t curRow = 0;
    std::array<std::array<std::unique_ptr<BasicButton>, 15>, 2> buttonColumns;

public:
    ProgressionPage();
    
    std::string getName() const { return "Progression"; }
    std::string getDesc() const { return "These settings control where progress items can appear.\nDisabled locations will still be randomized, but can only contain optional items you don't need to beat the game."; }

    void open();
    bool update(const VPADStatus& stat);
    void drawTV() const;
    void drawDRC() const;
};

class HintsPage : public EmptyPage {
private:
    size_t curCol = 0;
    size_t curRow = 0;
    std::array<std::array<std::unique_ptr<BasicButton>, 4>, 2> buttonColumns;

public:
    HintsPage();
    
    std::string getName() const { return "Hints"; }
    std::string getDesc() const { return "These settings control the generation and placement of hints.\nEach type of hint will be split evenly across the selected placement options."; }

    void open();
    bool update(const VPADStatus& stat);
    void drawTV() const;
    void drawDRC() const;
};

class EntrancePage : public EmptyPage {
private:
    size_t curCol = 0;
    size_t curRow = 0;
    std::array<std::array<std::unique_ptr<BasicButton>, 7>, 2> buttonColumns;

public:
    EntrancePage();
    
    std::string getName() const { return "Entrances"; }
    std::string getDesc() const { return "These settings control entrance randomization."; }

    void open();
    bool update(const VPADStatus& stat);
    void drawTV() const;
    void drawDRC() const;
};

class ConveniencePage : public EmptyPage {
private:
    size_t curCol = 0;
    size_t curRow = 0;
    std::array<std::array<std::unique_ptr<BasicButton>, 7>, 2> buttonColumns;

public:
    ConveniencePage();
    
    std::string getName() const { return "Convenience"; }
    std::string getDesc() const { return "These settings control convenience tweaks and other customization."; }

    void open();
    bool update(const VPADStatus& stat);
    void drawTV() const;
    void drawDRC() const;
};

class AdvancedPage : public EmptyPage {
private:
    size_t curCol = 0;
    size_t curRow = 0;
    std::array<std::array<std::unique_ptr<BasicButton>, 2>, 2> buttonColumns;

public:
    AdvancedPage();
    
    std::string getName() const { return "Advanced"; }
    std::string getDesc() const { return "Various options that don't fit into the other categories."; }

    void open();
    bool update(const VPADStatus& stat);
    void drawTV() const;
    void drawDRC() const;
};

class ItemsPage : public EmptyPage {
private:
    static constexpr size_t LIST_HEIGHT = 20;

    enum struct Column {
        LIST = 0,
        BUTTONS = 1
    };

    Column curCol = Column::LIST;
    size_t curRow = 0;
    size_t listScrollPos = 0;

    std::vector<ItemButton> listButtons; //sorted alphabetically, sometimes has items added/removed
    std::array<std::unique_ptr<BasicButton>, 11> countButtons;

public:
    ItemsPage();
    
    std::string getName() const { return "Starting Items"; }
    std::string getDesc() const { return "Selected items will be given to Link at the start of the game and won't be placed in the world."; }

    void open();
    bool update(const VPADStatus& stat);
    void drawTV() const;
    void drawDRC() const;
};

class ColorPage : public EmptyPage {
private:
    static constexpr size_t LIST_HEIGHT = 20;

    enum struct Column {
        LIST = 0,
        BUTTONS = 1
    };

    Column curCol = Column::LIST;
    size_t curRow = 0;
    size_t listScrollPos = 0;
    size_t selectedListIdx = 0;

    std::array<std::unique_ptr<BasicButton>, 3> toggles;

public:
    ColorPage();
    
    std::string getName() const { return "Colors"; }
    std::string getDesc() const { return "Controls custom colors that get applied to the player model. Custom models are not yet supported."; }

    void open();
    bool update(const VPADStatus& stat);
    void drawTV() const;
    void drawDRC() const;
};

class MetaPage : public EmptyPage {
private:
    // these are too new for the Wii U browser :(
    //static inline std::string GITHUB_URL = "https://github.com/SuperDude88/TWWHD-Randomizer";
    //static inline std::string DISCORD_URL = "https://discord.gg/uvGqsmHNc";

public:
    MetaPage();
    
    std::string getName() const { return "About"; }
    std::string getDesc() const { return "Patcher info and settings, unrelated to randomization."; }

    void open();
    bool update(const VPADStatus& stat);
    void drawTV() const;
    void drawDRC() const;
};
