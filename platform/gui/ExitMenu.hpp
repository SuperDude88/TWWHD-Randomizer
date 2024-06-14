#pragma once

enum class ExitMode {
    PLATFORM_ERROR,
    RANDOMIZATION_COMPLETE,
    RANDOMIZATION_ERROR
};

void waitForExitConfirm(const ExitMode& mode);
