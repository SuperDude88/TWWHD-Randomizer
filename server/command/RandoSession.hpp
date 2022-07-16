#pragma once

#include "WWHDStructs.hpp"

#include <filesystem>
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include <atomic>



class RandoSession
{
public:
    using fspath = std::filesystem::path;
    class CacheEntry {
    public:
        std::unordered_map<std::string, std::shared_ptr<CacheEntry>> children; //can't use CacheEntry directly, unordered_map needs complete type per the standard
        std::atomic<bool> isRepacked = false; //for this entry
    };

    enum class RepackResult {
        FAIL = 0,
        SUCCESS = 1,
        DELAY = 2
    };

    RandoSession();

    void init(const fspath& gameBaseDir, const fspath& randoWorkingDir, const fspath& randoOutputDir);
    [[nodiscard]] fspath openGameFile(const fspath& relPath);
    [[nodiscard]] bool copyToGameFile(const fspath& source, const fspath& relPath);
    [[nodiscard]] bool restoreGameFile(const fspath& relPath);
    [[nodiscard]] bool repackCache();
    [[nodiscard]] bool repackCache_singleThread();

    const fspath& getBaseDir() { return baseDir; }
    const fspath& getWorkingDir() { return workingDir; }
    const fspath& getOutputDir() { return outputDir; }
private:
    void clearWorkingDir() const;
    fspath extractFile(const std::vector<std::string>& fileSpec);
    RepackResult repackFile(const std::string& element, std::shared_ptr<CacheEntry> entry);
    void queueChildren(std::shared_ptr<CacheEntry> entry);

    bool initialized = false;
    fspath baseDir;
    fspath workingDir;
    fspath outputDir;
    CacheEntry fileCache;
};
