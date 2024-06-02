#pragma once

#include <memory>
#include <unordered_map>
#include <chrono>
#include <functional>

#include <platform/input.hpp>
#include <platform/gui/OptionActions.hpp>

class BasicButton {
protected:
    const Option option;
    const std::string name;
    const std::string description;
    const TriggerCallback cb;

    using Duration_t = InputManager::Duration_t;
    InputManager::Duration_t delay;
    InputManager::Duration_t interval;
    
    BasicButton(const Option& option_, const std::string& name_, const std::string& desc_, const TriggerCallback& cb_, const Duration_t& delay_, const Duration_t& interval_) :
        option(option_),
        name(name_),
        description(desc_),
        cb(cb_),
        delay(delay_),
        interval(interval_)
    {}

public:
    BasicButton(const Option& option_, const Duration_t& delay_ = Duration_t(-1), const Duration_t& interval_ = Duration_t(-1)) :
        BasicButton(option_, getNameDesc(option_).first, getNameDesc(option_).second, getCallback(option_), delay_, interval_)
    {}
    virtual ~BasicButton() = default;

    virtual void hovered();
    virtual void unhovered();
    virtual bool update();
    virtual void drawTV(const size_t row, const size_t nameCol, const size_t valCol) const;
    virtual void drawDRC() const;
};

class ItemButton final : public BasicButton {
protected:
    const GameItem item;
    const size_t num; // how many of this item is needed before the button is enabled
    bool enabled;

public:
    ItemButton(const GameItem& item_, const size_t& num_ = 1) :
        BasicButton(Option::StartingGear),
        item(item_),
        num(num_)
    {}
    ~ItemButton() override = default;
    
    // Only need to define this here since none of the others get put in vectors
    // Need this since const members implicity delete assignment operators
    // Even though C++20 gave us these functions to get around it
    ItemButton& operator=(const ItemButton& rhs) {
        if(this != &rhs) {
            std::destroy_at(this);
            std::construct_at(this, rhs);
        }

        return *this;
    }

    bool operator==(const ItemButton& rhs) const;
    bool operator==(const GameItem& rhs) const;
    bool operator<(const ItemButton& rhs) const;

    // checks the current state in the config
    // we can't use this to update buttons because you could enable the second instance before the first, and the first would appear selected instead since the count is only 1
    // so we only use it while loading the page, and have the buttons track themselves after that
    inline void loadState() { enabled = OptionCB::hasStartingItem(item, num); }
    inline bool isEnabled() const { return enabled; }
    inline GameItem getItem() const { return item; }

    bool update() override;
    void drawTV(const size_t row, const size_t nameCol, const size_t valCol) const override;
    void drawDRC() const override;
};

// runs a callback when triggered, can show a value
class ActionButton final : public BasicButton {
private:
    const TriggerCallback valueCB;
public:
    ActionButton(const std::string& name_, const std::string& desc_, const TriggerCallback& triggerCB_, const TriggerCallback& valueCB_ = &OptionCB::invalidCB) :
        BasicButton(Option::INVALID, name_, desc_, triggerCB_, Duration_t(-1), Duration_t(-1)),
        valueCB(valueCB_)
    {}
    ~ActionButton() = default;

    bool update() override;
    void drawTV(const size_t row, const size_t nameCol, const size_t valCol) const override;
    void drawDRC() const override;
};

// modifies a color
class ColorButton final : public BasicButton {
private:
    const std::function<void(const std::string&)> colorCB;
public:
    ColorButton(const std::string& name_, const std::string& desc_, const std::function<void(const std::string&)>& colorCB_) :
        BasicButton(Option::INVALID, name_, desc_, OptionCB::invalidCB, Duration_t(-1), Duration_t(-1)),
        colorCB(colorCB_)
    {}
    ~ColorButton() = default;

    bool update(const std::string& colorName);
    void drawTV(const size_t row, const size_t nameCol, const size_t valCol) const override;
    void drawDRC() const override;
};

class FunctionButton final : public BasicButton {
private:
    const std::function<void(void)> triggerCB;
public:
    FunctionButton(const std::string& name_, const std::string& desc_, const std::function<void(void)>& triggerCB_) :
        BasicButton(Option::INVALID, name_, desc_, OptionCB::invalidCB, Duration_t(-1), Duration_t(-1)),
        triggerCB(triggerCB_)
    {}
    ~FunctionButton() = default;

    bool update() override;
    void drawTV(const size_t row, const size_t nameCol, const size_t valCol) const override;
    void drawDRC() const override;
};
