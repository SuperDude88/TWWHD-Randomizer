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

public:
    BasicButton(const Option& option_) :
        option(option_),
        name(getNameDesc(option_).first),
        description(getNameDesc(option_).second),
        cb(getCallback(option_))
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
