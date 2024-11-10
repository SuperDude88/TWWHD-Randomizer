#pragma once

enum class ExitMode {
    IMMEDIATE,
    PLATFORM_ERROR,
    GUI_ERROR,
    RANDOMIZATION_COMPLETE,
    RANDOMIZATION_ERROR,
};

void waitForExitConfirm(const ExitMode& mode);