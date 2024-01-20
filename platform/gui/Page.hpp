#pragma once

#include <platform/gui/Button.hpp>
#include <platform/gui/Keyboard.hpp>

class EmptyPage {
public:
    virtual ~EmptyPage() {}

    virtual std::string getName() const = 0;
    virtual std::string getDesc() const = 0;

    virtual void open() = 0;
    virtual void close() = 0;
    virtual bool update() = 0;
    virtual void drawTV() const = 0;
    virtual void drawDRC() const = 0;
};

class SeedPage : public EmptyPage {
private:
    using Clock = std::chrono::high_resolution_clock;
    Clock::time_point resetTime;

    void resetTimer() { resetTime = Clock::now() + std::chrono::seconds(3); }

    std::string warnings;
public:
    SeedPage();
    
    std::string getName() const { return "Seed"; }
    std::string getDesc() const { return "Control the seed used to randomize."; }

    void open();
    void close();
    bool update();
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
    void close();
    bool update();
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
    void close();
    bool update();
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
    void close();
    bool update();
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
    void close();
    bool update();
    void drawTV() const;
    void drawDRC() const;
};

class AdvancedPage : public EmptyPage {
private:
    size_t curCol = 0;
    size_t curRow = 0;
    std::array<std::array<std::unique_ptr<BasicButton>, 3>, 2> buttonColumns;

public:
    AdvancedPage();
    
    std::string getName() const { return "Advanced"; }
    std::string getDesc() const { return "Various options that don't fit into the other categories."; }

    void open();
    void close();
    bool update();
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
    void close();
    bool update();
    void drawTV() const;
    void drawDRC() const;
};

class ColorPage : public EmptyPage {
private:
    enum struct Subpage {
        PRESETS = 0,
        COLOR_PICKER = 1
    };

    class PresetsSubpage : public EmptyPage {
    private:
        ColorPage& parent;

        static constexpr size_t LIST_HEIGHT = 20;

        enum struct Column {
            LIST = 0,
            BUTTONS = 1
        };

        Column curCol = Column::LIST;
        size_t curRow = 0;
        size_t listScrollPos = 0;
        size_t selectedListIdx = 0;

        std::array<std::unique_ptr<BasicButton>, 4> toggles;
        
    public:
        PresetsSubpage(ColorPage& parent_);
    
        std::string getName() const { return "Presets"; }
        std::string getDesc() const { return "Change presets and model options."; }

        void open();
    void close();
        bool update();
        void drawTV() const;
        void drawDRC() const;
    };

    class ColorPickerSubpage : public EmptyPage {
    private:
        ColorPage& parent;

        static constexpr size_t LIST_HEIGHT = 20;

        size_t curCol = 0;
        size_t curRow = 0;
        size_t listScrollPos = 0;
        size_t selectedListIdx = 0;

        HexKeyboard board;

        bool picking = false;
        void setPicking(const bool& enable_, const std::string& color_ = "") { picking = enable_; }

        std::array<ColorButton, 3> actions;
        
    public:
        ColorPickerSubpage(ColorPage& parent_);
    
        std::string getName() const { return "Color List"; }
        std::string getDesc() const { return "Individually set custom colors."; }

        void open();
        void close();
        bool update();
        void drawTV() const;
        void drawDRC() const;

        bool updateList();
        void drawListTV() const;
        void drawListDRC() const;

        bool updatePicker();
        void drawPickerTV() const;
        void drawPickerDRC() const;
    };

    void setSubpage(const Subpage& sub_) {
        curSubpage = sub_;

        switch(curSubpage) {
            case Subpage::PRESETS:
                picker.close();
                presets.open();
                break;
            case Subpage::COLOR_PICKER:
                presets.close();
                picker.open();
                break;
        }
    }

    Subpage curSubpage;

    PresetsSubpage presets;
    ColorPickerSubpage picker;

public:
    ColorPage();
    
    std::string getName() const { return "Colors"; }
    std::string getDesc() const { return "Controls custom colors that get applied to the player model. Custom models are not yet supported."; }

    void open();
    void close();
    bool update();
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
    void close();
    bool update();
    void drawTV() const;
    void drawDRC() const;
};
