#pragma once

#include <string>
#include <set>

#include <proc_ui/procui.h>

void ProcInit();
void ProcExit();
bool ProcIsRunning(ProcUIStatus* outStat = nullptr);
bool ProcIsForeground();

struct CombinedCB {
    const size_t priority;
    const ProcUICallback acquired;
    const ProcUICallback released;
    void* const arg;

    CombinedCB(const size_t& prio_, ProcUICallback acq_, ProcUICallback rel_, void* arg_ = nullptr) :
        priority(prio_),
        acquired(acq_),
        released(rel_),
        arg(arg_)
    {}

    bool operator<(const CombinedCB& rhs) const {
        return priority < rhs.priority;
    } 
};

class ScopedCallback {
public:
    ScopedCallback(const CombinedCB& cb);
    ~ScopedCallback();

private:
    std::set<CombinedCB>::iterator it;
};

void addCallbacks(const CombinedCB& cb);
