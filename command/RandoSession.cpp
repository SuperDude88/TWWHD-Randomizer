#include "RandoSession.hpp"


#include <cstring>
#include <algorithm>
#include <functional>
#include <filesystem>
#include <memory>
#include <unordered_map>
#include <fstream>
#include <string>

#include <libs/BS_thread_pool.hpp>

#include <gui/desktop/update_dialog_header.hpp>
#include <utility/string.hpp>
#include <utility/platform.hpp>
#include <utility/file.hpp>
#include <utility/time.hpp>
#include <command/Log.hpp>

#include <filetypes/baseFiletype.hpp>
#include <filetypes/bdt.hpp>
#include <filetypes/bflim.hpp>
#include <filetypes/bflyt.hpp>
#include <filetypes/bfres.hpp>
#include <filetypes/charts.hpp>
#include <filetypes/dzx.hpp>
#include <filetypes/elf.hpp>
#include <filetypes/events.hpp>
#include <filetypes/jpc.hpp>
#include <filetypes/msbp.hpp>
#include <filetypes/msbt.hpp>
#include <filetypes/sarc.hpp>
#include <filetypes/wiiurpx.hpp>
#include <filetypes/yaz0.hpp>

#define CHECK_INITIALIZED(ret) if(!initialized) { ErrorLog::getInstance().log("Session is not initialized (encountered on line " TOSTRING(__LINE__) ")"); return ret; }

RandoSession g_session; //definition for extern stuff

#ifdef DEVKITPRO
static BS::thread_pool workerThreads(3);
#else
static BS::thread_pool workerThreads(std::thread::hardware_concurrency() ? std::thread::hardware_concurrency() : 4);
#endif

static std::atomic<size_t> total_num_tasks = 0;
static std::atomic<size_t> num_completed_tasks = 0;

static const std::unordered_map<std::string, RandoSession::CacheEntry::Format> str_to_format {
    {"BDT",    RandoSession::CacheEntry::Format::BDT},
    {"BFLIM",  RandoSession::CacheEntry::Format::BFLIM},
    {"BFLYT",  RandoSession::CacheEntry::Format::BFLYT},
    {"BFRES",  RandoSession::CacheEntry::Format::BFRES},
    {"CHARTS", RandoSession::CacheEntry::Format::CHARTS},
    {"DZX",    RandoSession::CacheEntry::Format::DZX},
    {"DZR",    RandoSession::CacheEntry::Format::DZX},
    {"DZS",    RandoSession::CacheEntry::Format::DZX},
    {"ELF",    RandoSession::CacheEntry::Format::ELF},
    {"EVENTS", RandoSession::CacheEntry::Format::EVENTS},
    {"JPC",    RandoSession::CacheEntry::Format::JPC},
    {"MSBP",   RandoSession::CacheEntry::Format::MSBP},
    {"MSBT",   RandoSession::CacheEntry::Format::MSBT},
    {"RPX",    RandoSession::CacheEntry::Format::RPX},
    {"SARC",   RandoSession::CacheEntry::Format::SARC},
    {"YAZ0",   RandoSession::CacheEntry::Format::YAZ0},
    {"STREAM", RandoSession::CacheEntry::Format::STREAM},
};

void RandoSession::CacheEntry::addAction(Action_t action) {
    actions.push_back(action);
}

void RandoSession::CacheEntry::addDependent(std::shared_ptr<CacheEntry> depends) {
    dependents.push_back(depends);
    depends->incrementPrereq();
}

const std::shared_ptr<RandoSession::CacheEntry> RandoSession::CacheEntry::getRoot() const {
    if(storedFormat == Format::ROOT) {
        if(!parent->children.contains(element.string())) {
            ErrorLog::getInstance().log("File cache did not contain element \"" + element.string() + "\"! This is usually because getRoot() was called after clearing the file cache.");

            return nullptr;
        }

        return parent->children.at(element.string());
    }

    std::shared_ptr<CacheEntry> top = this->parent;
    while(top->storedFormat != Format::ROOT) {
        top = top->parent;
    }

    return top;
}

bool RandoSession::CacheEntry::isSibling(const std::shared_ptr<RandoSession::CacheEntry> other) const {
    const CacheEntry* thisRoot = this;
    while(thisRoot->storedFormat != Format::ROOT) {
        thisRoot = thisRoot->parent.get();
    }

    const CacheEntry* otherRoot = other.get();
    while(otherRoot->storedFormat != Format::ROOT) {
        otherRoot = otherRoot->parent.get();
    }

    return thisRoot == otherRoot;
}


RandoSession::RandoSession()
{

}

bool RandoSession::init(const fspath& gameBaseDir, const fspath& randoOutputDir) {
    baseDir = gameBaseDir;
    outputDir = randoOutputDir;

    if(baseDir.empty()) {
        ErrorLog::getInstance().log("No input path specified!");
        return false;
    }

    if(outputDir.empty()) {
        ErrorLog::getInstance().log("No output path specified!");
        return false;
    }

    if(baseDir == outputDir) {
        ErrorLog::getInstance().log("Output dir can not be the same as input dir!");
        return false;
    }

    if(!Utility::create_directories(outputDir)) { // handles directories that already exist
        ErrorLog::getInstance().log("Failed to create output folder " + Utility::toUtf8String(outputDir));
        return false;
    }

    clearCache();
    initialized = true;

    return true;
}

bool RandoSession::extractFile(std::shared_ptr<CacheEntry> current)
{
    RawFile* parentData = dynamic_cast<RawFile*>(current->parent->data.get());

    using Fmt = CacheEntry::Format;
    switch(current->storedFormat) {
        case Fmt::BDT:
        {
            if(parentData == nullptr) return false;
            current->data = std::make_unique<FileTypes::BDTFile>();
            dynamic_cast<FileTypes::BDTFile*>(current->data.get())->loadFromBinary(parentData->data);
        }
        break;
        case Fmt::BFLIM:
        {
            if(parentData == nullptr) return false;
            current->data = std::make_unique<FileTypes::FLIMFile>();
            dynamic_cast<FileTypes::FLIMFile*>(current->data.get())->loadFromBinary(parentData->data);
        }
        break;
        case Fmt::BFLYT:
        {
            if(parentData == nullptr) return false;
            current->data = std::make_unique<FileTypes::FLYTFile>();
            dynamic_cast<FileTypes::FLYTFile*>(current->data.get())->loadFromBinary(parentData->data);
        }
        break;
        case Fmt::BFRES:
        {
            if(parentData == nullptr) return false;
            current->data = std::make_unique<FileTypes::resFile>();
            dynamic_cast<FileTypes::resFile*>(current->data.get())->loadFromBinary(parentData->data);
        }
        break;
        case Fmt::CHARTS:
        {
            if(parentData == nullptr) return false;
            current->data = std::make_unique<FileTypes::ChartList>();
            dynamic_cast<FileTypes::ChartList*>(current->data.get())->loadFromBinary(parentData->data);
        }
        break;
        case Fmt::DZX:
        {
            if(parentData == nullptr) return false;
            current->data = std::make_unique<FileTypes::DZXFile>();
            dynamic_cast<FileTypes::DZXFile*>(current->data.get())->loadFromBinary(parentData->data.seekg(0, std::ios::beg));
        }
        break;
        case Fmt::ELF:
        {
            if(parentData == nullptr) return false;
            current->data = std::make_unique<FileTypes::ELF>();
            dynamic_cast<FileTypes::ELF*>(current->data.get())->loadFromBinary(parentData->data);
        }
        break;
        case Fmt::EVENTS:
        {
            if(parentData == nullptr) return false;
            current->data = std::make_unique<FileTypes::EventList>();
            dynamic_cast<FileTypes::EventList*>(current->data.get())->loadFromBinary(parentData->data);
        }
        break;
        case Fmt::JPC:
        {
            if(parentData == nullptr) return false;
            current->data = std::make_unique<FileTypes::JPC>();
            dynamic_cast<FileTypes::JPC*>(current->data.get())->loadFromBinary(parentData->data);
        }
        break;
        case Fmt::MSBP:
        {
            if(parentData == nullptr) return false;
            current->data = std::make_unique<FileTypes::MSBPFile>();
            dynamic_cast<FileTypes::MSBPFile*>(current->data.get())->loadFromBinary(parentData->data);
        }
        break;
        case Fmt::MSBT:
        {
            if(parentData == nullptr) return false;
            current->data = std::make_unique<FileTypes::MSBTFile>();
            dynamic_cast<FileTypes::MSBTFile*>(current->data.get())->loadFromBinary(parentData->data);
        }
        break;
        case Fmt::RPX:
        {
            if(parentData == nullptr) return false;
            current->data = std::make_unique<RawFile>();
            if (RPXError err = FileTypes::rpx_decompress(parentData->data, dynamic_cast<RawFile*>(current->data.get())->data); err != RPXError::NONE)
            {
                ErrorLog::getInstance().log(std::string("Encountered RPXError on line " TOSTRING(__LINE__) " of ") + __FILENAME__);
                return false;
            }
        }
        break;
        case Fmt::SARC:
        {
            if(parentData == nullptr) return false;
            current->data = std::make_unique<FileTypes::SARCFile>();
            dynamic_cast<FileTypes::SARCFile*>(current->data.get())->loadFromBinary(parentData->data);
        }
        break;
        case Fmt::YAZ0:
        {
            if(parentData == nullptr) return false;
            current->data = std::make_unique<RawFile>();
            if (YAZ0Error err = FileTypes::yaz0Decode(parentData->data, dynamic_cast<RawFile*>(current->data.get())->data); err != YAZ0Error::NONE)
            {
                ErrorLog::getInstance().log(std::string("Encountered YAZ0Error on line " TOSTRING(__LINE__) " of ") + __FILENAME__);
                return false;
            }
        }
        break;
        case Fmt::STREAM:
        {
            if (current->parent->storedFormat == Fmt::SARC) {
                FileTypes::SARCFile::File* file = dynamic_cast<FileTypes::SARCFile*>(current->parent->data.get())->getFile(current->element.string() + '\0');
                if(file == nullptr) {
                    ErrorLog::getInstance().log("Could not find " + current->element.string() + " in SARC");
                    return false;
                }

                current->data = std::make_unique<RawFile>(file->data);
            }
            else if (current->parent->storedFormat == Fmt::BFRES) {
                const auto& files = dynamic_cast<FileTypes::resFile*>(current->parent->data.get())->files;
                auto it = std::find_if(files.begin(), files.end(), [&](const FileTypes::resFile::FileSpec& spec) { return spec.fileName == current->element; });
                if(it == files.end()) {
                    ErrorLog::getInstance().log("Could not find " + current->element.string() + " in BFRES");
                    return false;
                }

                const std::string& resData = dynamic_cast<FileTypes::resFile*>(current->parent->data.get())->fileData;
                current->data = std::make_unique<RawFile>(resData.substr((*it).fileOffset - 0x6C, (*it).fileLength));
            }
            else {
                return false; //what
            }
        }
        break;
        case Fmt::ROOT:
        {
            current->data = std::make_unique<RawFile>();
            if(Utility::getFileContents((baseDir / current->element), dynamic_cast<RawFile*>(current->data.get())->data) != 0) return false;
        }
        break;
        case Fmt::EMPTY:
            [[fallthrough]];
        default:
            return false;
    }

    if(parentData != nullptr) parentData->data.str(std::string()); //clear parent sstream, don't need it
    return true;
}

bool RandoSession::repackFile(std::shared_ptr<CacheEntry> current)
{
    RawFile* parentData = dynamic_cast<RawFile*>(current->parent->data.get());

    using Fmt = CacheEntry::Format;
    switch(current->storedFormat) {
        case Fmt::BDT:
        {
            return false; //TODO: how should file accesses be named here?
        }
        case Fmt::BFLIM:
        {
            if(parentData == nullptr) return false;
            dynamic_cast<FileTypes::FLIMFile*>(current->data.get())->writeToStream(parentData->data.seekp(0, std::ios::beg));
        }
        return true;
        case Fmt::BFLYT:
        {
            if(parentData == nullptr) return false;
            dynamic_cast<FileTypes::FLYTFile*>(current->data.get())->writeToStream(parentData->data.seekp(0, std::ios::beg));
        }
        return true;
        case Fmt::BFRES:
        {
            if(parentData == nullptr) return false;
            dynamic_cast<FileTypes::resFile*>(current->data.get())->writeToStream(parentData->data.seekp(0, std::ios::beg));
        }
        return true;
        case Fmt::CHARTS:
        {
            if(parentData == nullptr) return false;
            dynamic_cast<FileTypes::ChartList*>(current->data.get())->writeToStream(parentData->data.seekp(0, std::ios::beg));
        }
        return true;
        case Fmt::DZX:
        {
            if(parentData == nullptr) return false;
            dynamic_cast<FileTypes::DZXFile*>(current->data.get())->writeToStream(parentData->data.seekp(0, std::ios::beg));
        }
        return true;
        case Fmt::ELF:
        {
            if(parentData == nullptr) return false;
            dynamic_cast<FileTypes::ELF*>(current->data.get())->writeToStream(parentData->data.seekp(0, std::ios::beg));
        }
        return true;
        case Fmt::EVENTS:
        {
            if(parentData == nullptr) return false;
            dynamic_cast<FileTypes::EventList*>(current->data.get())->writeToStream(parentData->data.seekp(0, std::ios::beg));
        }
        return true;
        case Fmt::JPC:
        {
            if(parentData == nullptr) return false;
            dynamic_cast<FileTypes::JPC*>(current->data.get())->writeToStream(parentData->data.seekp(0, std::ios::beg));
        }
        return true;
        case Fmt::MSBP:
        {
            if(parentData == nullptr) return false;
            dynamic_cast<FileTypes::MSBPFile*>(current->data.get())->writeToStream(parentData->data.seekp(0, std::ios::beg));
        }
        return true;
        case Fmt::MSBT:
        {
            if(parentData == nullptr) return false;
            dynamic_cast<FileTypes::MSBTFile*>(current->data.get())->writeToStream(parentData->data.seekp(0, std::ios::beg));
        }
        return true;
        case Fmt::RPX:
        {
            if(parentData == nullptr) return false;
            if (RPXError err = FileTypes::rpx_compress(dynamic_cast<RawFile*>(current->data.get())->data.seekg(0, std::ios::beg), parentData->data.seekp(0, std::ios::beg)); err != RPXError::NONE)
            {
                ErrorLog::getInstance().log(std::string("Encountered RPXError on line " TOSTRING(__LINE__) " of ") + __FILENAME__);
                return false;
            }
        }
        return true;
        case Fmt::SARC:
        {
            if(parentData == nullptr) return false;
            dynamic_cast<FileTypes::SARCFile*>(current->data.get())->writeToStream(parentData->data.seekp(0, std::ios::beg));
        }
        return true;
        case Fmt::YAZ0:
        {
            if(parentData == nullptr) return false;
            //const uint32_t compressLevel = current->parent->storedFormat == Fmt::ROOT ? 1 : 9;
            if (YAZ0Error err = FileTypes::yaz0Encode(dynamic_cast<RawFile*>(current->data.get())->data, parentData->data.seekp(0, std::ios::beg), 7); err != YAZ0Error::NONE)
            {
                ErrorLog::getInstance().log(std::string("Encountered YAZ0Error on line " TOSTRING(__LINE__) " of ") + __FILENAME__);
                return false;
            }
        }
        return true;
        case Fmt::STREAM:
        {
            if (current->parent->storedFormat == Fmt::SARC) {
                FileTypes::SARCFile* file = dynamic_cast<FileTypes::SARCFile*>(current->parent->data.get());
                if(file == nullptr) {
                    ErrorLog::getInstance().log("Could not get parent SARC of " + current->element.string());
                    return false;
                }

                file->replaceFile(current->element.string() + '\0', dynamic_cast<RawFile*>(current->data.get())->data);
            }
            else if (current->parent->storedFormat == Fmt::BFRES) {
                FileTypes::resFile* file = dynamic_cast<FileTypes::resFile*>(current->parent->data.get());
                if(file == nullptr) {
                    ErrorLog::getInstance().log("Could not get parent BFRES of" + current->element.string());
                    return false;
                }

                file->replaceEmbeddedFile(current->element.string(), dynamic_cast<RawFile*>(current->data.get())->data);
            }
        }
        return true;
        case Fmt::ROOT:
        {
            std::ofstream output(outputDir / current->element, std::ios::binary);
            if(!output.is_open()) return false;
            const std::string& data = dynamic_cast<RawFile*>(current->data.get())->data.str();
            output.write(&data[0], data.size());
        }
        return true;
        case Fmt::EMPTY:
            [[fallthrough]];
        default:
            return false;
    }
}

std::shared_ptr<RandoSession::CacheEntry> RandoSession::getEntry(const std::vector<std::string>& fileSpec) {
    // ["content/Common/Stage/example.szs", "YAZ0", "SARC", "data.bfres"]
    // first part is an extant game file
    std::string cacheKey{""};
    std::string resultKey{""};
    std::shared_ptr<CacheEntry> parentEntry = fileCache;
    std::shared_ptr<CacheEntry> nextEntry = nullptr;

    for (size_t i = 0; i < fileSpec.size(); i++)
    {
        const std::string& element = fileSpec[i];

        if (element.empty()) continue;

        if (element.compare("RPX") == 0)
        {
            resultKey = cacheKey + ".elf";
        }
        else if (element.compare("YAZ0") == 0)
        {
            resultKey = cacheKey + ".dec";
        }
        else if(element.compare("SARC") == 0)
        {
            resultKey = cacheKey + ".unpack/";
        }
        else if (element.compare("BFRES") == 0)
        {
            resultKey = cacheKey + ".res/";
        }
        else
        {
            resultKey = cacheKey + element;
        }
        // if we've already cached this
        if (parentEntry->children.count(resultKey) > 0)
        {
            cacheKey = resultKey;
            parentEntry = parentEntry->children.at(cacheKey);
            continue;
        }

        CacheEntry::Format fmt = CacheEntry::Format::EMPTY;
        if(str_to_format.contains(element)) {
            fmt = str_to_format.at(element);
        }
        else {
            if(i == 0) {
                fmt = CacheEntry::Format::ROOT;
            }
            else {
                fmt = CacheEntry::Format::STREAM;
            }
        }

        parentEntry->children[resultKey] = std::make_shared<CacheEntry>(parentEntry, element, fmt);
        nextEntry = parentEntry->children[resultKey];
        cacheKey = resultKey;
        parentEntry = nextEntry;
    }

    return parentEntry;
}

RandoSession::CacheEntry& RandoSession::openGameFile(const fspath& relPath)
{
    //CHECK_INITIALIZED(nullptr);
    return *getEntry(Utility::Str::split(relPath.string(), '@'));
}

bool RandoSession::copyToGameFile(const fspath& source, const fspath& relPath, const bool& resourceFile /* = false*/) {
    CHECK_INITIALIZED(false);

    RandoSession::CacheEntry& entry = openGameFile(relPath);
    entry.addAction([source, resourceFile](RandoSession* session, FileType* data) -> int {
        RawFile* dst = dynamic_cast<RawFile*>(data);
        if(dst == nullptr) return false;
        dst->data.str(std::string()); //clear data so we overwrite it
        std::string fileData = "";
        if(Utility::getFileContents(source, fileData, resourceFile) != 0) return false; //TODO: proper time against rdbuf
        dst->data.str(fileData); 

        return true;
    });

    return true;
}

bool RandoSession::restoreGameFile(const fspath& relPath) { //Restores a file from the base directory (without extracting any data)
    CHECK_INITIALIZED(false);

    CacheEntry& entry = openGameFile(relPath); //doesn't need actions, open loads from base and resaves to output
    return true;
}

bool RandoSession::handleChildren(const fspath filename, std::shared_ptr<CacheEntry> current) {
    if(current->parent->parent == nullptr) { //only print start of chain to avoid spam
        Utility::platformLog("Working on " + filename.string());
    }
    //extract this level, move down tree
    if(!extractFile(current)) return false;

    //has mods to stream (item location edits), handle these before filetype stuff
    if(current->children.size() == 1 && current->actions.size() != 0 && dynamic_cast<RawFile*>(current->data.get()) != nullptr) {
        for(auto& action : current->actions) {
            action(this, current->data.get());
        }
    }

    //bottom of branch, run mods
    if(current->children.size() == 0) {
        for(auto& action : current->actions) {
            action(this, current->data.get());
        }
    }
    else { //modify, repack children
        for(auto& [filename, child] : current->children) {
            if(child->getNumPrereqs() > 0 || child->isFinished() == true) continue; //skip this child, prereq did/will do it

            RandoSession::handleChildren(filename, child);
        }
    }

    //repack this level
    repackFile(current);

    current->setFinished();

    //handle dependents
    for(auto& dependent : current->dependents) {
        //check if this is the last dependency
        if(dependent->decrementPrereq() > 0) continue; //decrement returns new value

        //handle the data
        if(current->isSibling(dependent)) { //IMPROVEMENT: more precise sibling checks, filename stuff
            RandoSession::handleChildren(filename / dependent->element, dependent);
        }
        else { //IMPROVEMENT: check entry is root, handle other edge cases
            workerThreads.push_task(&RandoSession::handleChildren, this, dependent->element, dependent); //add root of this other chain
        }
    }

    //clear children once done
    current->children.clear();

    //update progress if this is the root of the chain
    if(current->storedFormat == CacheEntry::Format::ROOT) {
        num_completed_tasks++;
        UPDATE_DIALOG_VALUE(int(50.0f + 49.0f * (float(num_completed_tasks)/float(total_num_tasks)))); //also update progress bar
    }

    return true;
}

#ifdef DEVKITPRO
//based on https://github.com/emiyl/dumpling/blob/12935ede46e9720fdec915cdb430d10eb7df54a7/source/app/dumping.cpp#L208
static bool iterate_directory_recursive(RandoSession& session, const fspath cur) {
    DIR* dirHandle = nullptr;
    if (dirHandle = opendir(cur.string().c_str()); dirHandle == nullptr) {
        ErrorLog::getInstance().log("Couldn't open directory: " + cur.string());
        return false;
    }

    // Loop over directory contents
    struct dirent* dirEntry;
    while ((dirEntry = readdir(dirHandle)) != nullptr) {
        const fspath entryPath = cur / dirEntry->d_name;
        const fspath relPath = entryPath.string().substr(session.getBaseDir().string().length() + 1);

        // Use lstat since readdir returns DT_REG for symlinks
        struct stat fileStat;
        if (lstat(entryPath.string().c_str(), &fileStat) != 0) {
            ErrorLog::getInstance().log("Couldn't check what type this file/folder was: " + entryPath.string());
            return false;
        }

        if (S_ISLNK(fileStat.st_mode)) {
            continue;
        }
        else if (S_ISREG(fileStat.st_mode)) {
            // Add file to fileCache
            RandoSession::CacheEntry& entry = session.openGameFile(relPath);
        }
        else if (S_ISDIR(fileStat.st_mode)) {
            // Ignore root and parent folder entries
            if (std::strncmp(dirEntry->d_name, ".", 1) == 0 || std::strncmp(dirEntry->d_name, "..", 2) == 0) continue;

            // Create corresponding folder in the output path
            if(!Utility::create_directories(session.getOutputDir() / relPath)) {
                ErrorLog::getInstance().log("Failed to create dir: " + (session.getOutputDir() / relPath).string());
                closedir(dirHandle);
                return false;
            }

            // Iterate over all the files in this subdirectory
            if (!iterate_directory_recursive(session, entryPath)) {
                ErrorLog::getInstance().log("Failed to iterate dir: " + entryPath.string());
                closedir(dirHandle);
                return false;
            }
        }
    }

    closedir(dirHandle);
    return true;
}
#endif

bool RandoSession::runFirstTimeSetup() {
    #ifdef DEVKITPRO
        // create folders and add all game files to RandoSession (it will thread the copies and skip anything that we will modify and overwrite)
        Utility::platformLog("Running first time setup (this will increase repack time)");
        if(!iterate_directory_recursive(*this, baseDir / "content")) { //code and meta folders are handled by channel install
            return false; 
        }
    #else
        #ifdef ENABLE_TIMING
            ScopedTimer<"Copying dump took "> copyTimer;
        #endif
 
        Utility::platformLog("Copying dump to output...");
        UPDATE_DIALOG_LABEL("Copying dump to output...");
        std::filesystem::copy(baseDir / "code", outputDir / "code", std::filesystem::copy_options::recursive);
        std::filesystem::copy(baseDir / "content", outputDir / "content", std::filesystem::copy_options::recursive);
        std::filesystem::copy(baseDir / "meta", outputDir / "meta", std::filesystem::copy_options::recursive);
    #endif

    setFirstTimeSetup(false);

    return true;
}

bool RandoSession::modFiles()
{
    #ifdef ENABLE_TIMING
        ScopedTimer<"Repacking took "> timer;
    #endif

    CHECK_INITIALIZED(false);

    if(firstTimeSetup) {
        if(!runFirstTimeSetup()) {
            ErrorLog::getInstance().log("Failed to complete first time setup!");
            return false;
        }
    }

    total_num_tasks = fileCache->children.size();
    for(auto& [filename, child] : fileCache->children) {
        //has dependency, it will add it when necessary
        if(child->getNumPrereqs() > 0) {
            continue;
        }

        workerThreads.push_task(&RandoSession::handleChildren, this, filename, child);  
    }
    
    // uncache everything
    // do this now so the shared_ptrs free themselves as the tasks finish
    // this prevents file data from hanging around in RAM (which softlocks on console, probably gets too full)
    fileCache->children.clear();

    UPDATE_DIALOG_LABEL("Repacking Files...");

    workerThreads.wait_for_tasks();

    Utility::platformLog("Finished repacking files");
    LOG_TO_DEBUG("Finished repacking files");

    return true;
}

void RandoSession::clearCache()
{
    fileCache->children.clear();
    fileCache->dependents.clear();
    fileCache->data = nullptr;
    fileCache->actions.clear();
    fileCache->numPrereqs = 0;
    
    total_num_tasks = 0;
    num_completed_tasks = 0;
}
