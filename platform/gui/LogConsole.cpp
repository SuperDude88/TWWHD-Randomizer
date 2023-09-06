//based on https://github.com/devkitPro/wut/blob/095a397e7ecccf4742278592d653e59355b60e16/libraries/libwhb/src/console.c
#include "LogConsole.hpp"

#include <cstring>
#include <array>

#include <platform/gui/screen.hpp>

static constexpr size_t num_lines = 16;
static constexpr size_t line_length = 128;

using LogLine_t = std::array<char, line_length>;
static std::array<LogLine_t, num_lines> logLines;
static size_t curLine = 0;

static void AddLine(const LogLine_t& line) { //we expect LogLine_t is null-terminated
    if(curLine == num_lines) {
        for(size_t i = 0; i < logLines.size() - 1; i++) {
            logLines[i] = logLines[i + 1];
        }

        logLines.back() = line;
    }
    else {
        logLines[curLine] = line;
        curLine++;
    }
}


void LogConsoleWrite(const char* str) {
    LogLine_t line;

    const size_t len = std::min(std::strlen(str), line.size() - 1);

    std::memcpy(line.data(), str, len);
    line[len] = '\0';

    AddLine(line);
}

void LogConsoleDraw() {
    ScreenClear();

    for(size_t y = 0; y < curLine; y++) {
        OSScreenPutFontEx(SCREEN_TV, 0, y, logLines[y].data());
        OSScreenPutFontEx(SCREEN_DRC, 0, y, logLines[y].data());
    }

    ScreenDraw();
}
