#include "RandoSession.hpp"


#include <algorithm>
#include <functional>
#include <filesystem>
#include <memory>
#include <unordered_map>
#include <fstream>
#include <string>

#include <libs/BS_thread_pool.hpp>

#include <gui/update_dialog_header.hpp>
#include <utility/string.hpp>
#include <utility/platform.hpp>
#include <utility/file.hpp>
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

#ifdef DEVKITPRO
static BS::thread_pool workerThreads(3);
#else
static BS::thread_pool workerThreads(12);
#endif

static const std::unordered_map<std::string, RandoSession::CacheEntry::Format> str_to_format {
    {"BDT",    RandoSession::CacheEntry::Format::BDT},
    {"BFLIM",  RandoSession::CacheEntry::Format::BFLIM},
    {"BFLYT",  RandoSession::CacheEntry::Format::BFLYT},
    {"BFRES",  RandoSession::CacheEntry::Format::BFRES},
    {"CHARTS", RandoSession::CacheEntry::Format::CHARTS},
    {"DZX",    RandoSession::CacheEntry::Format::DZX},
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
        return parent->children.at(element.string());
    }

    std::shared_ptr<CacheEntry> top = this->parent;
    while(top->storedFormat != Format::ROOT) {
        top = top->parent;
    }

    return top;
}

RandoSession::RandoSession()
{

}

void RandoSession::init(const fspath& gameBaseDir, const fspath& randoOutputDir) { //might have more init stuff later
    baseDir = gameBaseDir;
    outputDir = randoOutputDir;

    clearCache();
    initialized = true;
    return;
}

bool RandoSession::extractFile(std::shared_ptr<CacheEntry> current)
{
    GenericFile* parentData = dynamic_cast<GenericFile*>(current->parent->data.get());

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
            current->data = std::make_unique<GenericFile>();
            if (RPXError err = FileTypes::rpx_decompress(parentData->data, dynamic_cast<GenericFile*>(current->data.get())->data); err != RPXError::NONE)
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
            current->data = std::make_unique<GenericFile>();
            if (YAZ0Error err = FileTypes::yaz0Decode(parentData->data, dynamic_cast<GenericFile*>(current->data.get())->data); err != YAZ0Error::NONE)
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

                current->data = std::make_unique<GenericFile>(file->data);
            }
            else if (current->parent->storedFormat == Fmt::BFRES) {
                const auto& files = dynamic_cast<FileTypes::resFile*>(current->parent->data.get())->files;
                auto it = std::find_if(files.begin(), files.end(), [&](const FileTypes::resFile::FileSpec& spec) { return spec.fileName == current->element; });
                if(it == files.end()) {
                    ErrorLog::getInstance().log("Could not find " + current->element.string() + " in BFRES");
                    return false;
                }

                const std::string& resData = dynamic_cast<FileTypes::resFile*>(current->parent->data.get())->fileData;
                current->data = std::make_unique<GenericFile>(resData.substr((*it).fileOffset - 0x6C, (*it).fileLength));
            }
            else {
                return false; //what
            }
        }
        break;
        case Fmt::ROOT:
        {
            std::ifstream input(baseDir / current->element, std::ios::binary);
            if(!input.is_open()) return false;
            current->data = std::make_unique<GenericFile>();
            dynamic_cast<GenericFile*>(current->data.get())->data << input.rdbuf();
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
    GenericFile* parentData = dynamic_cast<GenericFile*>(current->parent->data.get());
    
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
            if (RPXError err = FileTypes::rpx_compress(dynamic_cast<GenericFile*>(current->data.get())->data.seekg(0, std::ios::beg), parentData->data.seekp(0, std::ios::beg)); err != RPXError::NONE)
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
            const uint32_t compressLevel = current->parent->storedFormat == Fmt::ROOT ? 1 : 9;
            if (YAZ0Error err = FileTypes::yaz0Encode(dynamic_cast<GenericFile*>(current->data.get())->data, parentData->data.seekp(0, std::ios::beg), compressLevel); err != YAZ0Error::NONE)
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

                file->replaceFile(current->element.string() + '\0', dynamic_cast<GenericFile*>(current->data.get())->data);
            }
            else if (current->parent->storedFormat == Fmt::BFRES) {
                FileTypes::resFile* file = dynamic_cast<FileTypes::resFile*>(current->parent->data.get());
                if(file == nullptr) {
                    ErrorLog::getInstance().log("Could not get parent BFRES of" + current->element.string());
                    return false;
                }

                file->replaceEmbeddedFile(current->element.string(), dynamic_cast<GenericFile*>(current->data.get())->data);
            }
        }
        return true;
        case Fmt::ROOT:
        {
            std::ofstream output(outputDir / current->element, std::ios::binary);
            if(!output.is_open()) return false;
            const std::string& data = dynamic_cast<GenericFile*>(current->data.get())->data.str();
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

RandoSession::CacheEntry& RandoSession::openGameFile(const RandoSession::fspath& relPath)
{
    //CHECK_INITIALIZED(nullptr);
    return *getEntry(Utility::Str::split(relPath.string(), '@'));
}

std::ifstream RandoSession::openBaseFile(const fspath &relPath) {
    CHECK_INITIALIZED(std::ifstream());
    
    //const fspath file = outputDir / relPath;
    //if(!Utility::copy_file(baseDir / relPath, file)) {
    //    ErrorLog::getInstance().log("Could not open original data for " + relPath.string());
    //    return std::ifstream();
    //}
    //if(!std::filesystem::is_regular_file(file)) {
    //    ErrorLog::getInstance().log("Could not open " + relPath.string() + " after copy");
    //    return std::ifstream();
    //}
    const fspath file = baseDir / relPath;
    if(!std::filesystem::is_regular_file(file)) {
        ErrorLog::getInstance().log("Could not open original data for " + relPath.string());
        return std::ifstream();
    }

    return std::ifstream(file, std::ios::binary);
}

bool RandoSession::isCached(const RandoSession::fspath& relPath)
{
    CHECK_INITIALIZED(false);
    const auto& splitPath = Utility::Str::split(relPath.string(), '@');
    std::shared_ptr<CacheEntry> curEntry = fileCache;

    std::string key;
    for(const std::string& element : splitPath) {
        if (element.compare("RPX") == 0)
        {
            key += ".elf";
        }
        else if (element.compare("YAZ0") == 0)
        {
            key += ".dec";
        }
        else if(element.compare("SARC") == 0)
        {
            key += ".unpack/";
        }
        else if (element.compare("BFRES") == 0)
        {
            key += ".res/";
        }
        else
        {
            key += element;
        }

        if(curEntry->children.count(key) == 0) return false;
        curEntry = curEntry->children.at(key);
    }

    return true;
}

bool RandoSession::copyToGameFile(const fspath& source, const fspath& relPath) {
    CHECK_INITIALIZED(false);

    RandoSession::CacheEntry& entry = openGameFile(relPath);
    entry.addAction([source](RandoSession* session, FileType* data) -> int {
        std::ifstream src(source, std::ios::binary);
	    if(!src.is_open()) {
	    	ErrorLog::getInstance().log("Failed to open " + source.string());
	    	return false;
	    }

        GenericFile* dst = dynamic_cast<GenericFile*>(data);
        if(dst == nullptr) return false;
        dst->data.str(std::string()); //clear data so we overwrite it
        dst->data << src.rdbuf();
		
        return true;
    });

    return true;
}

bool RandoSession::restoreGameFile(const fspath& relPath) { //Restores a file from the base directory (without extracting any data)
    CHECK_INITIALIZED(false);
    
    const fspath src = baseDir / relPath;
    const fspath dst = outputDir / relPath;
    if(!std::filesystem::is_regular_file(src)) {
        ErrorLog::getInstance().log("Could not restore data, " + relPath.string() + " is not a regular file");
        return false;
    }
    return Utility::copy_file(src, dst);
}

bool RandoSession::handleChildren(const fspath& filename, std::shared_ptr<CacheEntry> current) {
    if(current->parent->parent == nullptr) { //only print start of chain to avoid spam
        Utility::platformLog(std::string("Working on ") + filename.string() + "\n");
    }
    //extract this level, move down tree
    if(!extractFile(current)) return false;

    //has mods to stream (item location edits), handle these before filetype stuff
    if(current->children.size() == 1 && current->actions.size() != 0 && dynamic_cast<GenericFile*>(current->data.get()) != nullptr) {
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
        if(dependent->getRoot() == current->getRoot()) { //IMPROVEMENT: more precise sibling checks, filename stuff
            RandoSession::handleChildren(filename / dependent->element, dependent);
        }
        else { //IMPROVEMENT: check entry is root, handle other edge cases
            workerThreads.push_task(&RandoSession::handleChildren, this, dependent->element, dependent); //add root of this other chain
        }
    }

    //clear children once done
    current->children.clear();

    //delete ourselves if we are the chain root, fileCache won't
    if(current->storedFormat == CacheEntry::Format::ROOT) {
        current->parent->children.erase(filename.string());
    }

    return true;
}

bool RandoSession::modFiles()
{
    CHECK_INITIALIZED(false);
    
    for(auto& [filename, child] : fileCache->children) {
        //has dependency, it will add it when necessary
        if(child->getNumPrereqs() > 0) {
            continue;
        }

        workerThreads.push_task(&RandoSession::handleChildren, this, filename, child);
        workerThreads.total_task_size++;
    }
    
    UPDATE_DIALOG_LABEL("Repacking Files...");
    workerThreads.total_tasks_completed = 0;

    workerThreads.wait_for_tasks();

    Utility::platformLog("Finished repacking files\n");
    BasicLog::getInstance().log("Finished repacking files");
    
    return true;
}

void RandoSession::clearCache()
{
    fileCache->children.clear();
    fileCache->dependents.clear();
    fileCache->data = nullptr;
    fileCache->actions.clear();
    fileCache->numPrereqs = 0;
}
