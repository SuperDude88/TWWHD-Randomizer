#pragma once

#include <filesystem>
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>

#include <filetypes/baseFiletype.hpp>



#define CAST_ENTRY_TO_FILETYPE(var, type, data)   \
    {const auto temp = dynamic_cast<type*>(data); if(temp == nullptr) return 0;}  \
    type& var = *dynamic_cast<type*>(data);

class RandoSession
{
public:
    using fspath = std::filesystem::path;

    class CacheEntry {
    public:
        enum class Format {
            BDT = 0,
            BFLIM,
            BFLYT,
            BFRES,
            CHARTS,
            DZX,
            ELF,
            EVENTS,
            JPC,
            MSBP,
            MSBT,
            RPX,
            SARC,
            YAZ0,
            STREAM,
            ROOT, //root of chain, open file from disk
            EMPTY, //fileCache, no data
        };
        
        CacheEntry(std::shared_ptr<CacheEntry> parent_, const fspath& elem_, const Format& format_) :
            parent(parent_),
            element(elem_),
            storedFormat(format_)
        {}

        using Action_t = std::function<int(RandoSession*, FileType*)>;

        void addAction(Action_t action);
        void delayUntil(const fspath& req); //wait for prereq that adds mods, prevent repack-mod-repack

    private:
        const std::shared_ptr<CacheEntry> parent;
        std::unordered_map<std::string, std::shared_ptr<CacheEntry>> children; //can't use CacheEntry directly, unordered_map needs complete type per the standard
        std::vector<fspath> prereqs;

        const fspath element;
        const Format storedFormat;
        std::unique_ptr<FileType> data;
        std::vector<Action_t> actions; //store actions as lambdas to execute in order
    
        friend class RandoSession;
    };

    RandoSession();

    void init(const fspath& gameBaseDir, const fspath& randoOutputDir);
    [[nodiscard]] CacheEntry& openGameFile(const fspath& relPath);
    [[nodiscard]] std::ifstream openBaseFile(const fspath& relPath);
    [[nodiscard]] bool isCached(const fspath& relPath);
    [[nodiscard]] bool copyToGameFile(const fspath& source, const fspath& relPath);
    [[nodiscard]] bool restoreGameFile(const fspath& relPath);
    [[nodiscard]] bool modFiles();

    const fspath& getBaseDir() { return baseDir; }
    const fspath& getOutputDir() { return outputDir; }
private:
    CacheEntry& getEntry(const std::vector<std::string>& fileSpec);
    bool extractFile(std::shared_ptr<CacheEntry> current);
    bool repackFile(std::shared_ptr<CacheEntry> current);
    bool handleChildren(const fspath& filename, std::shared_ptr<CacheEntry> current);
    void clearCache();

    bool initialized = false;
    fspath baseDir;
    fspath outputDir;
    
    std::shared_ptr<CacheEntry> fileCache = std::make_shared<CacheEntry>(nullptr, "", CacheEntry::Format::EMPTY);
};
