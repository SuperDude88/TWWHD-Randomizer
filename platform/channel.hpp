#pragma once

#include <utility/path.hpp>

#include <coreinit/mcp.h>

MCPError getTitlePath(const uint64_t& titleID, fspath& outPath);
bool checkEnoughFreeSpace(const MCPInstallTarget& device, const uint64_t& minSpace);

[[nodiscard]] bool createOutputChannel(const fspath& baseDir, const MCPInstallTarget& loc);
