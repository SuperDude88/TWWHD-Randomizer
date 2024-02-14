#pragma once

#include <filesystem>
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include <atomic>

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
        enum struct Format {
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
        void addDependent(std::shared_ptr<CacheEntry> depends); //add entry to tree after this one is completed, prevent repack-mod-repack

        size_t incrementPrereq() { return ++numPrereqs; }
        size_t decrementPrereq() { return --numPrereqs; }
        size_t getNumPrereqs() const { return numPrereqs; }
        void setFinished() { finished = true; }
        bool isFinished() const { return finished; }
        const std::shared_ptr<CacheEntry> getParent() const { return parent; }
        const std::shared_ptr<CacheEntry> getRoot() const; // NOTE: this only works before fileCache is cleared
        bool isSibling(const std::shared_ptr<CacheEntry> other) const;

    private:
        const std::shared_ptr<CacheEntry> parent = nullptr;
        std::unordered_map<std::string, std::shared_ptr<CacheEntry>> children = {}; //can't use CacheEntry directly, unordered_map needs complete type per the standard
        std::vector<std::shared_ptr<CacheEntry>> dependents = {};

        const fspath element = "";
        const Format storedFormat = Format::EMPTY;
        std::unique_ptr<FileType> data = nullptr;
        std::vector<Action_t> actions = {}; //store actions as lambdas to execute in order
        std::atomic<size_t> numPrereqs = 0;
        std::atomic<bool> finished = false;
    
        friend class RandoSession;
    };

    RandoSession();

    void setFirstTimeSetup(const bool& doSetup) { firstTimeSetup = doSetup; }
    bool init(const fspath& gameBaseDir, const fspath& randoOutputDir);
    [[nodiscard]] CacheEntry& openGameFile(const fspath& relPath);
    [[nodiscard]] bool copyToGameFile(const fspath& source, const fspath& relPath, const bool& resourceFile = false);
    [[nodiscard]] bool restoreGameFile(const fspath& relPath);
    [[nodiscard]] bool modFiles();

    const fspath& getBaseDir() const { return baseDir; }
    const fspath& getOutputDir() const { return outputDir; }
private:
    std::shared_ptr<CacheEntry> getEntry(const std::vector<std::string>& fileSpec);
    bool extractFile(std::shared_ptr<CacheEntry> current);
    bool repackFile(std::shared_ptr<CacheEntry> current);
    bool handleChildren(const fspath filename, std::shared_ptr<CacheEntry> current);
    void clearCache();
    bool runFirstTimeSetup();

    bool firstTimeSetup = false;
    bool initialized = false;
    fspath baseDir;
    fspath outputDir;
    
    std::shared_ptr<CacheEntry> fileCache = std::make_shared<CacheEntry>(nullptr, "", CacheEntry::Format::EMPTY);
};

extern RandoSession g_session; //defined in RandoSession.cpp, shared between a couple files, set up in randomizer.cpp
