#pragma once

#include <coreinit/screen.h>

namespace ScreenSizeData {
    constexpr size_t drc_num_lines = 18;
    constexpr size_t drc_line_length = 64;
    constexpr size_t tv_num_lines = 28;
    constexpr size_t tv_line_length = 100;
}

void ConsoleScreenInit();

void SetColor(const uint32_t& col);
void ScreenClear();
void ScreenDraw();
