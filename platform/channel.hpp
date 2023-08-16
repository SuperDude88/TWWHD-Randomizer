#pragma once

#include <filesystem>

#include <coreinit/mcp.h>

MCPError getTitlePath(const uint64_t& titleID, std::filesystem::path& outPath);

[[nodiscard]] bool createOutputChannel(const std::filesystem::path& baseDir, const MCPInstallTarget& loc);
