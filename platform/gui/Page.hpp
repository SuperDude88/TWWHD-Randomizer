#pragma once

#include <platform/gui/Button.hpp>
#include <platform/gui/Keyboard.hpp>

class EmptyPage {
public:
    virtual ~EmptyPage() = default;

    virtual std::string getName() const = 0;
    virtual std::string getDesc() const = 0;

    virtual void open() = 0;
    virtual void close() = 0;
    virtual bool update() = 0;
    virtual void drawTV() const = 0;
    virtual void drawDRC() const = 0;
};

class SeedPage final : public EmptyPage {
private:
    bool typing_seed = false;
    bool typing_perma = false;
    USKeyboard board;

    using Clock = std::chrono::high_resolution_clock;
    Clock::time_point resetTime;

    void resetTimer() { resetTime = Clock::now() + std::chrono::seconds(3); }

    std::string warnings;
public:
    SeedPage();
    
    std::string getName() const override { return "Seed"; }
    std::string getDesc() const override { return "Control the seed used to randomize."; }

    void open() override;
    void close() override;
    bool update() override;
    void drawTV() const override;
    void drawDRC() const override;
};

class ProgressionPage final : public EmptyPage {
private:
    size_t curCol = 0;
    size_t curRow = 0;
    std::array<std::array<std::unique_ptr<BasicButton>, 15>, 2> buttonColumns;

public:
    ProgressionPage();
    
    std::string getName() const override { return "Progression"; }
    std::string getDesc() const override { return "These settings control where progress items can appear.\nDisabled locations will still be randomized, but can only contain optional items you don't need to beat the game."; }

    void open() override;
    void close() override;
    bool update() override;
    void drawTV() const override;
    void drawDRC() const override;
};

class HintsPage final : public EmptyPage {
private:
    size_t curCol = 0;
    size_t curRow = 0;
    std::array<std::vector<std::unique_ptr<BasicButton>>, 2> buttonColumns;

public:
    HintsPage();
    
    std::string getName() const override { return "Hints"; }
    std::string getDesc() const override { return "These settings control the generation and placement of hints.\nEach type of hint will be split evenly across the selected placement options."; }

    void open() override;
    void close() override;
    bool update() override;
    void drawTV() const override;
    void drawDRC() const override;
};

class EntrancePage final : public EmptyPage {
private:
    size_t curCol = 0;
    size_t curRow = 0;
    std::array<std::array<std::unique_ptr<BasicButton>, 7>, 2> buttonColumns;

public:
    EntrancePage();
    
    std::string getName() const override { return "Entrances"; }
    std::string getDesc() const override { return "These settings control entrance randomization."; }

    void open() override;
    void close() override;
    bool update() override;
    void drawTV() const override;
    void drawDRC() const override;
};

class ConveniencePage final : public EmptyPage {
private:
    size_t curCol = 0;
    size_t curRow = 0;
    std::array<std::array<std::unique_ptr<BasicButton>, 7>, 2> buttonColumns;

public:
    ConveniencePage();
    
    std::string getName() const override { return "Convenience"; }
    std::string getDesc() const override { return "These settings control convenience tweaks and other customization."; }

    void open() override;
    void close() override;
    bool update() override;
    void drawTV() const override;
    void drawDRC() const override;
};

class AdvancedPage final : public EmptyPage {
private:
    size_t curCol = 0;
    size_t curRow = 0;
    std::array<std::array<std::unique_ptr<BasicButton>, 3>, 2> buttonColumns;

public:
    AdvancedPage();
    
    std::string getName() const override { return "Advanced"; }
    std::string getDesc() const override { return "Various options that don't fit into the other categories."; }

    void open() override;
    void close() override;
    bool update() override;
    void drawTV() const override;
    void drawDRC() const override;
};

class ItemsPage final : public EmptyPage {
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
    
    std::string getName() const override { return "Starting Items"; }
    std::string getDesc() const override { return "Selected items will be given to Link at the start of the game and won't be placed in the world."; }

    void open() override;
    void close() override;
    bool update() override;
    void drawTV() const override;
    void drawDRC() const override;
};

class ColorPage final : public EmptyPage {
private:
    enum struct Subpage {
        PRESETS = 0,
        COLOR_PICKER = 1
    };

    class PresetsSubpage final : public EmptyPage {
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
    
        std::string getName() const override { return "Presets"; }
        std::string getDesc() const override { return "Change presets and model options."; }

        void open() override;
        void close() override;
        bool update() override;
        void drawTV() const override;
        void drawDRC() const override;
    };

    class ColorPickerSubpage final : public EmptyPage {
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
    
        std::string getName() const override { return "Color List"; }
        std::string getDesc() const override { return "Individually set custom colors."; }

        void open() override;
        void close() override;
        bool update() override;
        void drawTV() const override;
        void drawDRC() const override;

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
    
    std::string getName() const override { return "Colors"; }
    std::string getDesc() const override { return "Controls custom colors that get applied to the player model. Custom models are not yet supported."; }

    void open() override;
    void close() override;
    bool update() override;
    void drawTV() const override;
    void drawDRC() const override;
};

class MetaPage final : public EmptyPage {
private:
    // these are too new for the Wii U browser :(
    //static inline std::string GITHUB_URL = "https://github.com/SuperDude88/TWWHD-Randomizer";
    //static inline std::string DISCORD_URL = "https://discord.gg/wPvdQ2Krrm";
    bool rpxLoaderInit = false;
    std::string savePath = "sd:/wiiu/apps/save/ ... (TWWHD Randomizer)";

public:
    MetaPage();
    ~MetaPage();
    
    std::string getName() const override { return "About"; }
    std::string getDesc() const override { return "Patcher info and settings, unrelated to randomization."; }

    void open() override;
    void close() override;
    bool update() override;
    void drawTV() const override;
    void drawDRC() const override;
};
