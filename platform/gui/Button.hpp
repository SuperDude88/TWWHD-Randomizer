#pragma once

#include <memory>
#include <unordered_map>
#include <chrono>

#include <vpad/input.h>

#include <platform/gui/OptionActions.hpp>

class BasicButton {
protected:
    const Option option;
    const std::string name;
    const std::string description;
    const TriggerCallback cb;
    
    BasicButton(const Option& option_, const std::string& name_, const std::string& desc_, const TriggerCallback& cb_) :
        option(option_),
        name(name_),
        description(desc_),
        cb(cb_)
    {}

public:
    BasicButton(const Option& option_) :
        BasicButton(option_, getNameDesc(option_).first, getNameDesc(option_).second, getCallback(option_))
    {}
    virtual ~BasicButton() {}

    virtual bool update(const VPADStatus& stat);
    virtual void drawTV(const size_t row, const size_t nameCol, const size_t valCol) const;
    virtual void drawDRC() const;
};

class CounterButton : public BasicButton {
protected:
    using DeltaT_t = std::chrono::milliseconds;
    DeltaT_t cycleFreq; // time between increments
    DeltaT_t minHold; // wait period so holding isn't immediate

public:
    CounterButton(const Option& option_, const DeltaT_t& freq_ = std::chrono::seconds(1), const DeltaT_t& min_ = std::chrono::seconds(0)) :
        BasicButton(option_),
        cycleFreq(freq_),
        minHold(min_)
    {}
    virtual ~CounterButton() {}

    virtual bool update(const VPADStatus& stat);
    virtual void drawTV(const size_t row, const size_t nameCol, const size_t valCol) const;
    virtual void drawDRC() const;
};

class ItemButton : public BasicButton {
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
    virtual ~ItemButton() {}
    
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

    // checks the current state in the config
    // we can't use this to update buttons because you could enable the second instance before the first, and the first would appear selected instead since the count is only 1
    // so we only use it while loading the page, and have the buttons track themselves after that
    inline void loadState() { enabled = OptionCB::hasStartingItem(item, num); }
    inline bool isEnabled() const { return enabled; }
    inline GameItem getItem() const { return item; }

    virtual bool update(const VPADStatus& stat);
    virtual void drawTV(const size_t row, const size_t nameCol, const size_t valCol) const;
    virtual void drawDRC() const;
};

// runs a callback when triggered, can show a value
class ActionButton : public BasicButton {
private:
    const TriggerCallback valueCB;
public:
    ActionButton(const std::string& name_, const std::string& desc_, const TriggerCallback& triggerCB_, const TriggerCallback& valueCB_ = &OptionCB::invalidCB) :
        BasicButton(Option::INVALID, name_, desc_, triggerCB_),
        valueCB(valueCB_)
    {}
    virtual ~ActionButton() {}

    virtual bool update(const VPADStatus& stat);
    virtual void drawTV(const size_t row, const size_t nameCol, const size_t valCol) const;
    virtual void drawDRC() const;
};
