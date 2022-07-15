#pragma once

#include "WWHDStructs.hpp"

#include <filesystem>
#include <string>
#include <vector>
#include <list>
#include <unordered_set>
#include <unordered_map>



class CacheEntry {
public:
    std::unordered_map<std::string, CacheEntry> children;
};

class RandoSession
{
public:
    using fspath = std::filesystem::path;
    class CacheEntry {
    public:
        std::unordered_map<std::string, std::unique_ptr<CacheEntry>> children; //can't use CacheEntry directly, unordered_map needs complete type per the standard
    };

    RandoSession(const fspath& gameBaseDir, const fspath& randoWorkingDir, const fspath& randoOutputDir);

    void init();
    [[nodiscard]] fspath openGameFile(const fspath& relPath);
    [[nodiscard]] bool copyToGameFile(const fspath& source, const fspath& relPath);
    [[nodiscard]] bool repackCache();
private:
    void clearWorkingDir();
    fspath extractFile(const std::vector<std::string>& fileSpec);
    bool repackFile(const std::string& element);
    bool repackChildren(CacheEntry& entry);
    
    fspath relToGameAbsolute(const fspath& relPath);

    bool initialized = false;
    fspath baseDir;
    fspath workingDir;
    fspath outputDir;
    CacheEntry fileCache;
};
