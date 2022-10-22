#pragma once

#include <filesystem>
#include <memory>
#include <string>
#include <sstream>
#include <vector>
#include <variant>
#include <unordered_map>
#include <atomic>

#include <filetypes/sarc.hpp>
#include <filetypes/bfres.hpp>
#include <utility/platform.hpp>
#include <command/WWHDStructs.hpp>



class RandoSession
{
public:
    using fspath = std::filesystem::path;
    class CacheEntry {
    public:
        //sarc, and fres can be optimized by storing their filetype instead of a sstream
        //other files just use a stream
        std::variant<FileTypes::SARCFile, FileTypes::resFile, std::stringstream, std::monostate> data = std::monostate{};

        std::shared_ptr<CacheEntry> parent;
        std::unordered_map<std::string, std::shared_ptr<CacheEntry>> children; //can't use CacheEntry directly, unordered_map needs complete type per the standard

        std::atomic<bool> isRepacked = false; //for this entry
        bool fullCompress = true; //whether to recompress the yaz0 or not, .pack files need to be recompressed currently because they crash otherwise
        bool toOutput = false; //whether to save to output, allows it to save without checking a file exists
    };

    enum class RepackResult {
        FAIL = 0,
        SUCCESS = 1,
        DELAY = 2
    };

    RandoSession();

    void init(const fspath& gameBaseDir, const fspath& randoOutputDir);
    [[nodiscard]] std::stringstream* openGameFile(const fspath& relPath);
    [[nodiscard]] bool isCached(const fspath& relPath);
    [[nodiscard]] bool copyToGameFile(const fspath& source, const fspath& relPath);
    [[nodiscard]] bool restoreGameFile(const fspath& relPath);
    [[nodiscard]] bool repackCache();
    [[nodiscard]] bool repackCache_singleThread();

    const fspath& getBaseDir() { return baseDir; }
    const fspath& getOutputDir() { return outputDir; }
private:
    std::stringstream* extractFile(const std::vector<std::string>& fileSpec);
    RepackResult repackFile(const std::string& element, std::shared_ptr<CacheEntry> entry);
    void queueChildren(std::shared_ptr<CacheEntry> entry);
    void clearCache();

    bool initialized = false;
    fspath baseDir;
    fspath outputDir;
    
    std::shared_ptr<CacheEntry> fileCache = std::make_shared<CacheEntry>();
};
