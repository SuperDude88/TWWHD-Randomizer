#pragma once

#include <proc_ui/procui.h>

void ProcInit();
void ProcExit();
bool ProcIsRunning();

void ProcSetCallback(ProcUICallbackType type, ProcUICallback cb, void* arg = nullptr, uint32_t priority = 100);
