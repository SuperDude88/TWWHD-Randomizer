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
    static constexpr const char* PageName = "Seed";
    static constexpr const char* PageDesc = "Control the seed used to randomize.";
    using Clock = std::chrono::high_resolution_clock;
    Clock::time_point resetTime;

    void resetTimer() { resetTime = Clock::now() + std::chrono::seconds(3); }

public:
    SeedPage();
    
    std::string getName() const { return PageName; }
    std::string getDesc() const { return PageDesc; }

    void open();
    bool update(const VPADStatus& stat);
    void drawTV() const;
    void drawDRC() const;
};

class ProgressionPage : public EmptyPage {
private:
    static constexpr const char* PageName = "Progression";
    static constexpr const char* PageDesc = "These settings control where progress items can appear.\nDisabled locations will still be randomized, but can only contain optional items you don't need to beat the game.";

    size_t curCol = 0;
    size_t curRow = 0;
    std::array<std::array<std::unique_ptr<BasicButton>, 15>, 2> buttonColumns;

public:
    ProgressionPage();
    
    std::string getName() const { return PageName; }
    std::string getDesc() const { return PageDesc; }

    void open();
    bool update(const VPADStatus& stat);
    void drawTV() const;
    void drawDRC() const;
};

class HintsPage : public EmptyPage {
private:
    static constexpr const char* PageName = "Hints";
    static constexpr const char* PageDesc = "These settings control the generation and placement of hints.\nEach type of hint will be split evenly across the selected placement options.";

    size_t curCol = 0;
    size_t curRow = 0;
    std::array<std::array<std::unique_ptr<BasicButton>, 4>, 2> buttonColumns;

public:
    HintsPage();
    
    std::string getName() const { return PageName; }
    std::string getDesc() const { return PageDesc; }

    void open();
    bool update(const VPADStatus& stat);
    void drawTV() const;
    void drawDRC() const;
};

class EntrancePage : public EmptyPage {
private:
    static constexpr const char* PageName = "Entrances";
    static constexpr const char* PageDesc = "These settings control entrance randomization.";

    size_t curCol = 0;
    size_t curRow = 0;
    std::array<std::array<std::unique_ptr<BasicButton>, 7>, 2> buttonColumns;

public:
    EntrancePage();
    
    std::string getName() const { return PageName; }
    std::string getDesc() const { return PageDesc; }

    void open();
    bool update(const VPADStatus& stat);
    void drawTV() const;
    void drawDRC() const;
};

class ConveniencePage : public EmptyPage {
private:
    static constexpr const char* PageName = "Convenience";
    static constexpr const char* PageDesc = "These settings control convenience tweaks and other customization.";

    size_t curCol = 0;
    size_t curRow = 0;
    std::array<std::array<std::unique_ptr<BasicButton>, 7>, 2> buttonColumns;

public:
    ConveniencePage();
    
    std::string getName() const { return PageName; }
    std::string getDesc() const { return PageDesc; }

    void open();
    bool update(const VPADStatus& stat);
    void drawTV() const;
    void drawDRC() const;
};

class AdvancedPage : public EmptyPage {
private:
    static constexpr const char* PageName = "Advanced";
    static constexpr const char* PageDesc = "Various options that don't fit into the other categories.";

    size_t curCol = 0;
    size_t curRow = 0;
    std::array<std::array<std::unique_ptr<BasicButton>, 3>, 2> buttonColumns;

public:
    AdvancedPage();
    
    std::string getName() const { return PageName; }
    std::string getDesc() const { return PageDesc; }

    void open();
    bool update(const VPADStatus& stat);
    void drawTV() const;
    void drawDRC() const;
};

class ItemsPage : public EmptyPage {
private:
    static constexpr const char* PageName = "Starting Items";
    static constexpr const char* PageDesc = "Selected items will be given to Link at the start of the game and won't be placed in the world.";
    static constexpr size_t LIST_HEIGHT = 20;

    enum class Column {
        LIST = 0,
        BUTTONS = 1
    };

    Column curCol = Column::LIST;
    size_t curRow = 0;
    size_t listScrollPos = 0;

    std::vector<ItemButton> listButtons; //sorted alphabetically, sometimes has items added/removed
    std::array<std::unique_ptr<BasicButton>, 10> countButtons;

public:
    ItemsPage();
    
    std::string getName() const { return PageName; }
    std::string getDesc() const { return PageDesc; }

    void open();
    bool update(const VPADStatus& stat);
    void drawTV() const;
    void drawDRC() const;
};
