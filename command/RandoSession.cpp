#include "RandoSession.hpp"

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <functional>
#include <memory>
#include <sstream>
#include <string>
#include <type_traits>
#include <queue>

#include <thread>
#include <condition_variable>
#include <mutex>

#include <gui/update_dialog_header.hpp>
#include <filetypes/wiiurpx.hpp>
#include <filetypes/yaz0.hpp>
#include <utility/string.hpp>
#include <utility/platform.hpp>
#include <utility/file.hpp>
#include <command/Log.hpp>

using namespace std::literals::chrono_literals;

#define CHECK_INITIALIZED(ret) if(!initialized) { ErrorLog::getInstance().log("Session is not initialized (encountered on line " TOSTRING(__LINE__) ")"); return ret; }

//partially based on https://github.com/bshoshany/thread-pool/blob/master/BS_thread_pool.hpp
class ThreadPool {
public:
    explicit ThreadPool() { 
        startThreads(); 
    }

    ~ThreadPool() {
        waitForAll();
        stopThreads();
    }

    template <typename F, typename... A>
    void push_task(const F& task, const A&... args)
    {
        {
            const std::scoped_lock<std::mutex> tasks_lock(tasks_mutex);
            if constexpr (sizeof...(args) == 0)
                tasks.push(std::function<RandoSession::RepackResult()>(task));
            else
                tasks.push(std::function<RandoSession::RepackResult()>([task, args...] { task(args...); }));
        }
        tasks_total++;
        new_tasks.notify_one();
    }

    size_t getTotalTasks() {
        return tasks_total;
    }

    void waitForAll() {
        UPDATE_DIALOG_LABEL("Repacking Files...");
        waiting = true;
        auto max_tasks = getTotalTasks();
        std::unique_lock<std::mutex> tasks_lock(tasks_mutex); //Don't really need this, just using it for the condition_variable
        task_finished.wait(tasks_lock, [this, max_tasks] {
            UPDATE_DIALOG_VALUE(int(100.0f - ((float(tasks_total)/float(max_tasks)) * 50.0f)))
            return tasks_total == 0;
        });
        // Update the label one last time so that it doesn't potentially get stuck at 99%
        UPDATE_DIALOG_VALUE(100);
        waiting = false;
    }
private:
    void startThreads() {
        running = true;
        threads.resize(num_threads);
        for(uint32_t i = 0; i < num_threads; i++) {
            threads.at(i) = std::thread(&ThreadPool::doWork, this);
        }
    }
    
    void stopThreads() {
        running = false;
        new_tasks.notify_all();
        for(std::thread& thread : threads) {
            thread.join();
        }
    }

    void doWork() {
        while(running) {
            std::function<RandoSession::RepackResult()> task;
            std::unique_lock<std::mutex> tasks_lock(tasks_mutex);
            new_tasks.wait(tasks_lock, [&] { return !tasks.empty() || !running; });
            if(running) {
                task = tasks.front();
                tasks.pop();
                tasks_lock.unlock();
                RandoSession::RepackResult result = task();
                if(result == RandoSession::RepackResult::DELAY) {
                    tasks_lock.lock();
                    tasks.push(task);
                    tasks_lock.unlock();
                }
                else if(result == RandoSession::RepackResult::SUCCESS) {
                    --tasks_total;
                    if(waiting) {
                        task_finished.notify_one();
                        if((tasks_total % 50) == 0) {
                            Utility::platformLog("Repacking file cache: %u entries remaining...\n", tasks_total.load());
                            BasicLog::getInstance().log("Repacking file cache: " + std::to_string(tasks_total) + " entries remaining...");
                        }
                    }
                }
                else {
                    ErrorLog::getInstance().log("Repack operation returned fail\n");
                }
            }
        }
    }

    #ifdef DEVKITPRO
    static constexpr uint32_t num_threads = 1;
    #else
    static constexpr uint32_t num_threads = 12;
    #endif

    std::atomic<bool> running = false;
    std::atomic<bool> waiting = false;
    std::condition_variable new_tasks;
    std::condition_variable task_finished;
    std::queue<std::function<RandoSession::RepackResult()>> tasks;
    std::mutex tasks_mutex;
    std::atomic<size_t> tasks_total = 0;
    std::vector<std::thread> threads;
};

static ThreadPool workerThreads;



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

std::stringstream* RandoSession::extractFile(const std::vector<std::string>& fileSpec)
{
    // ["content/Common/Stage/example.szs", "YAZ0", "SARC", "data.bfres"]
    // first part is an extant game file
    std::string cacheKey{""};
    std::string resultKey{""};
    std::shared_ptr<CacheEntry> parentEntry = fileCache;
    std::shared_ptr<CacheEntry> nextEntry = nullptr;

    bool fromBaseDir = false; //whether to unpack from the cache or directly from the base directory
    
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

        parentEntry->children[resultKey] = std::make_shared<CacheEntry>();
        nextEntry = parentEntry->children[resultKey];
        nextEntry->parent = parentEntry;

        if(i == 1) {
            nextEntry->toOutput = true; //last level that would be repacked
        }
        else {
            nextEntry->toOutput = false;
        }
        
        if (element.compare("RPX") == 0)
        {
            if(fromBaseDir) {
                std::ifstream inputFile(baseDir / cacheKey, std::ios::binary);
                if (!inputFile.is_open()) {
                    ErrorLog::getInstance().log("Failed to open input file " + (baseDir / cacheKey).string());
                    return nullptr;
                }
                fromBaseDir = false;

                if (RPXError err = FileTypes::rpx_decompress(inputFile, nextEntry->data.emplace<std::stringstream>()); err != RPXError::NONE)
                {
                    ErrorLog::getInstance().log(std::string("Encountered RPXError on line " TOSTRING(__LINE__) " of ") + __FILENAME__);
                    return nullptr;
                }
            }
            else {
                if (RPXError err = FileTypes::rpx_decompress(std::get<std::stringstream>(parentEntry->data), nextEntry->data.emplace<std::stringstream>()); err != RPXError::NONE)
                {
                    ErrorLog::getInstance().log(std::string("Encountered RPXError on line " TOSTRING(__LINE__) " of ") + __FILENAME__);
                    return nullptr;
                }
                parentEntry->data = std::monostate{}; //don't need to keep data around
            }
        }
        else if (element.compare("YAZ0") == 0)
        {
            nextEntry->fullCompress = false;
            //nextEntry->toOutput = false; //sarc will go to output instead

            if(fromBaseDir) {
                std::ifstream inputFile(baseDir / cacheKey, std::ios::binary);
                if (!inputFile.is_open()) {
                    ErrorLog::getInstance().log("Failed to open input file " + (baseDir / cacheKey).string());
                    return nullptr;
                }

                fromBaseDir = false;

                if(YAZ0Error err = FileTypes::yaz0Decode(inputFile, nextEntry->data.emplace<std::stringstream>()); err != YAZ0Error::NONE)
                {
                    ErrorLog::getInstance().log(std::string("Encountered YAZ0Error on line " TOSTRING(__LINE__) " of ") + __FILENAME__);
                    return nullptr;
                }
            }
            else {
                if(YAZ0Error err = FileTypes::yaz0Decode(std::get<std::stringstream>(parentEntry->data), nextEntry->data.emplace<std::stringstream>()); err != YAZ0Error::NONE)
                {
                    ErrorLog::getInstance().log(std::string("Encountered YAZ0Error on line " TOSTRING(__LINE__) " of ") + __FILENAME__);
                    return nullptr;
                }
                parentEntry->data = std::monostate{}; //don't need to keep data around
            }
        }
        else if(element.compare("SARC") == 0)
        {
            if(parentEntry->fullCompress == false && parentEntry->toOutput == true) { //if yaz0 doesn't save to output, sarc does instead
                parentEntry->toOutput = false;
                nextEntry->toOutput = true;
            }

            FileTypes::SARCFile& sarc = nextEntry->data.emplace<FileTypes::SARCFile>();
            SARCError err = SARCError::NONE;
            if(fromBaseDir) {
                err = sarc.loadFromFile((baseDir / cacheKey).string());
                fromBaseDir = false;
            }
            else {
                err = sarc.loadFromBinary(std::get<std::stringstream>(parentEntry->data));
                parentEntry->data = std::monostate{}; //don't need to keep data around
            }
            if (err != SARCError::NONE) {
                ErrorLog::getInstance().log(std::string("Encountered SARCError on line " TOSTRING(__LINE__) " of ") + __FILENAME__);
                return nullptr;
            }
        }
        else if (element.compare("BFRES") == 0)
        {
            FileTypes::resFile& fres = nextEntry->data.emplace<FileTypes::resFile>();
            FRESError err = FRESError::NONE;
            if(fromBaseDir) {
                err = fres.loadFromFile((baseDir / cacheKey).string());
                fromBaseDir = false;
            }
            else {
                err = fres.loadFromBinary(std::get<std::stringstream>(parentEntry->data));
                parentEntry->data = std::monostate{}; //don't need to keep data around
            }
            if (err != FRESError::NONE) {
                ErrorLog::getInstance().log(std::string("Encountered FRESError on line " TOSTRING(__LINE__) " of ") + __FILENAME__);
                return nullptr;
            }
        }
        else
        {
            if(i == 0) {
                if(fileSpec.size() > 1) {
                    fromBaseDir = true;
                }
                else {
                    // file has no unpacking, just load
                    const fspath gameFilePath = baseDir / resultKey;
                    std::ifstream inFile(gameFilePath, std::ios::binary);
                    if (!inFile.is_open()) {
                        ErrorLog::getInstance().log("Failed to open input file " + (baseDir / cacheKey).string());
                        return nullptr;
                    }

                    nextEntry->data.emplace<std::stringstream>() << inFile.rdbuf();
                    nextEntry->toOutput = true;
                }
            }
            else {
                if(fileSpec[i - 1] == "SARC") {
                    FileTypes::SARCFile::File* file = std::get<FileTypes::SARCFile>(parentEntry->data).getFile(element + '\0');
                    if(file == nullptr) {
                        ErrorLog::getInstance().log("Could not find " + element + " in SARC");
                        return nullptr;
                    }
                    
                    nextEntry->data.emplace<std::stringstream>(std::move(file->data));
                }
                
                if(fileSpec[i - 1] == "BFRES") {
                    const auto& files = std::get<FileTypes::resFile>(parentEntry->data).files;
                    auto it = std::find_if(files.begin(), files.end(), [&](const FileTypes::resFile::FileSpec& spec) { return spec.fileName == element; });
                    if(it == files.end()) {
                        ErrorLog::getInstance().log("Could not find " + element + " in BFRES");
                        return nullptr;
                    }

                    nextEntry->data.emplace<std::stringstream>(std::get<FileTypes::resFile>(parentEntry->data).fileData.substr((*it).fileOffset - 0x6C, (*it).fileLength));
                }
            }
        }

        cacheKey = resultKey;
        parentEntry = nextEntry;
    }

    //reset cursors
    std::stringstream* ptr = &std::get<std::stringstream>(parentEntry->data);
    ptr->seekg(0, std::ios::beg);
    ptr->seekp(0, std::ios::beg);
    return ptr;
}

std::stringstream* RandoSession::openGameFile(const RandoSession::fspath& relPath)
{
    CHECK_INITIALIZED(nullptr);
    return extractFile(Utility::Str::split(relPath.string(), '@'));
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

    std::stringstream* data = extractFile(Utility::Str::split(relPath.string(), '@'));

    if(data == nullptr) {
        ErrorLog::getInstance().log("Failed to extract file " + relPath.string());
        return false;
    }
    
    std::ifstream src(source, std::ios::binary);
	if(!src.is_open()) {
		ErrorLog::getInstance().log("Failed to open " + source.string());
		return false;
	}

    *data << src.rdbuf();
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

RandoSession::RepackResult RandoSession::repackFile(const std::string& element, std::shared_ptr<CacheEntry> entry) {
    if(std::any_of(entry->children.begin(), entry->children.end(), [](const std::pair<std::string, std::shared_ptr<CacheEntry>>& child) { return child.second->isRepacked == false; })) {
        return RepackResult::DELAY;
    }

    //Repack file in the cache directory
    //Write directly to output if it is the last step
    Utility::platformLog("Repacking %s\n", element.c_str());
    std::string resultKey;
    if (element.ends_with(".elf"))
    {
        resultKey = element.substr(0, element.size() - 4);

        //Repack to output directory if file exists, otherwise stay in cache
        if(entry->toOutput) {
            std::ofstream outputFile;
            outputFile.open(outputDir / resultKey, std::ios::binary);
            if (!outputFile.is_open()) {
                ErrorLog::getInstance().log("Failed to open output file " + (outputDir / resultKey).string());
                return RepackResult::FAIL;
            }

            auto& strm = std::get<std::stringstream>(entry->data);
            strm.seekg(0, std::ios::beg);
            strm.seekp(0, std::ios::beg);
            if (RPXError err = FileTypes::rpx_compress(strm, outputFile); err != RPXError::NONE)
            {
                ErrorLog::getInstance().log(std::string("Encountered RPXError on line " TOSTRING(__LINE__) " of ") + __FILENAME__);
                return RepackResult::FAIL;
            }
            entry->data = std::monostate{};
        }
        else {
            auto& strm = std::get<std::stringstream>(entry->data);
            strm.seekg(0, std::ios::beg);
            strm.seekp(0, std::ios::beg);
            if (RPXError err = FileTypes::rpx_compress(strm, entry->parent->data.emplace<std::stringstream>()); err != RPXError::NONE)
            {
                ErrorLog::getInstance().log(std::string("Encountered RPXError on line " TOSTRING(__LINE__) " of ") + __FILENAME__);
                return RepackResult::FAIL;
            }
            entry->data = std::monostate{};
        }
    }
    else if (element.ends_with(".dec"))
    {
        if(entry->fullCompress) {
            uint32_t compressLevel = 9;
            resultKey = element.substr(0, element.size() - 4);

            //Repack to output directory if file exists, otherwise stay in working dir
            if(entry->toOutput) {
                std::ofstream outputFile;
                outputFile.open(outputDir / resultKey, std::ios::binary);
                if (!outputFile.is_open()) {
                    ErrorLog::getInstance().log("Failed to open output file " + (outputDir / resultKey).string());
                    return RepackResult::FAIL;
                }

                Utility::platformLog("Recompressing %s\n", element.c_str());
                auto& strm = std::get<std::stringstream>(entry->data);
                strm.seekg(0, std::ios::beg);
                strm.seekp(0, std::ios::beg);
                if (YAZ0Error err = FileTypes::yaz0Encode(strm, outputFile, compressLevel); err != YAZ0Error::NONE)
                {
                    ErrorLog::getInstance().log(std::string("Encountered YAZ0Error on line " TOSTRING(__LINE__) " of ") + __FILENAME__);
                    return RepackResult::FAIL;
                }
                entry->data = std::monostate{};
            }
            else {
                Utility::platformLog("Recompressing %s\n", element.c_str());
                auto& strm = std::get<std::stringstream>(entry->data);
                strm.seekg(0, std::ios::beg);
                strm.seekp(0, std::ios::beg);
                if (YAZ0Error err = FileTypes::yaz0Encode(strm, entry->parent->data.emplace<std::stringstream>(), compressLevel); err != YAZ0Error::NONE)
                {
                    ErrorLog::getInstance().log(std::string("Encountered YAZ0Error on line " TOSTRING(__LINE__) " of ") + __FILENAME__);
                    return RepackResult::FAIL;
                }
                entry->data = std::monostate{};
            }
        }
        else {
            if(entry->data.index() == 2){
                entry->parent->data.emplace<std::stringstream>().swap(std::get<std::stringstream>(entry->data));
            }
        }
    }
    else if (element.ends_with(".unpack/"))
    {
        resultKey = element.substr(0, element.size() - 8);

        FileTypes::SARCFile& sarc = std::get<FileTypes::SARCFile>(entry->data);
        SARCError err = SARCError::NONE;

        //update the changed files
        for(const auto& child : entry->children) {
            err = sarc.replaceFile(child.first.substr(element.size()) + '\0', std::get<std::stringstream>(child.second->data));
            if (err != SARCError::NONE)
            {
                ErrorLog::getInstance().log(std::string("Encountered SARCError on line " TOSTRING(__LINE__) " of ") + __FILENAME__);
                return RepackResult::FAIL;
            }
            child.second->data = std::monostate{};
        }

        //Repack to output directory if file exists, otherwise stay in cache
        if(entry->toOutput) {
            if(resultKey.ends_with(".dec")) { //remove extra extension if skipping compression
                resultKey = resultKey.substr(0, resultKey.size() - 4);
            }
            err = sarc.writeToFile((outputDir / resultKey).string());
            entry->data = std::monostate{};
        }
        else {
            err = sarc.writeToStream(entry->parent->data.emplace<std::stringstream>());
            entry->data = std::monostate{};
        }

        if (err != SARCError::NONE) 
        {
            ErrorLog::getInstance().log(std::string("Encountered SARCError on line " TOSTRING(__LINE__) " of ") + __FILENAME__);
            return RepackResult::FAIL;
        }
    }
    else if (element.ends_with(".res/"))
    {
        resultKey = element.substr(0, element.size() - 5);

        FileTypes::resFile& fres = std::get<FileTypes::resFile>(entry->data);
        FRESError err = FRESError::NONE;

        //rebuild the changed files
        for(const auto& child : entry->children) {
            err = fres.replaceEmbeddedFile(child.first.substr(element.size()), std::get<std::stringstream>(child.second->data));
            if (err != FRESError::NONE)
            {
                ErrorLog::getInstance().log(std::string("Encountered FRESError on line " TOSTRING(__LINE__) " of ") + __FILENAME__);
                return RepackResult::FAIL;
            }
            child.second->data = std::monostate{};
        }
        
        //Repack to output directory if file exists, otherwise stay in working dir
        if(entry->toOutput) {
            err = fres.writeToFile((outputDir / resultKey).string());
            entry->data = std::monostate{};
        }
        else {
            err = fres.writeToStream(entry->parent->data.emplace<std::stringstream>());
            entry->data = std::monostate{};
        }
        
        if (err != FRESError::NONE)
        {
            ErrorLog::getInstance().log(std::string("Encountered FRESError on line " TOSTRING(__LINE__) " of ") + __FILENAME__);
            return RepackResult::FAIL;
        }
    }
    else
    {
        //Copy to output if the file didn't need any extraction
        //Skip copying for files that get repacked, they get repacked directly to the output
        if (entry->toOutput) {
        	std::ofstream dst(outputDir / element, std::ios::binary);
			if(!dst.is_open()) {
				ErrorLog::getInstance().log("Failed to open " + (outputDir / element).string());
				return RepackResult::FAIL;
			}

            const std::string& dataStr = std::get<std::stringstream>(entry->data).str();
			dst.write(&dataStr[0], dataStr.size());
            entry->data = std::monostate{};
        }
    }

    entry->isRepacked = true;
    entry->children.clear(); // children no longer cached or needed
    return RepackResult::SUCCESS;
}

void RandoSession::queueChildren(std::shared_ptr<CacheEntry> entry) {
    //go down the tree and wait for lower parts to queue
    for(auto& child : entry->children) { //go down the tree
        queueChildren(child.second);
    }

    //queue this level's children
    for(const auto& child : entry->children) {
        workerThreads.push_task(std::bind(&RandoSession::repackFile, this, child.first, child.second));
    }

    return;
}

bool RandoSession::repackCache()
{
    CHECK_INITIALIZED(false);

    queueChildren(fileCache);

    workerThreads.waitForAll();
    Utility::platformLog("Finished repacking files\n");
    BasicLog::getInstance().log("Finished repacking files\n");
    
    return true;
}

void RandoSession::clearCache()
{
    fileCache->children.clear();
    fileCache->isRepacked = false;
    fileCache->fullCompress = true;
    fileCache->toOutput = false;
}
