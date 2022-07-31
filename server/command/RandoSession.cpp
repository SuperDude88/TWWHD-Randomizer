#include "RandoSession.hpp"

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <functional>
#include <memory>
#include <string>
#include <type_traits>
#include <queue>

#include <thread>
#include <condition_variable>
#include <mutex>

#include "../filetypes/wiiurpx.hpp"
#include "../filetypes/yaz0.hpp"
#include "../filetypes/sarc.hpp"
#include "../filetypes/bfres.hpp"
#include "../utility/stringUtil.hpp"
#include "../utility/platform.hpp"
#include "../utility/file.hpp"
#include "./Log.hpp"

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
        waiting = true;
        std::unique_lock<std::mutex> tasks_lock(tasks_mutex); //Don't really need this, just using it for the condition_variable
        task_finished.wait(tasks_lock, [this] { return tasks_total == 0; });
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
    static constexpr uint32_t num_threads = 4;
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

void RandoSession::init(const fspath& gameBaseDir, const fspath& randoWorkingDir, const fspath& randoOutputDir) { //might have more init stuff later
    baseDir = gameBaseDir;
    workingDir = randoWorkingDir;
    outputDir = randoOutputDir;

    //clearWorkingDir();
    initialized = true;
    return;
}

void RandoSession::clearWorkingDir() const {
    if (std::filesystem::is_directory(workingDir)) {
        Utility::remove_all(workingDir);
    }
    Utility::create_directories(workingDir);

    return;
}



RandoSession::fspath RandoSession::extractFile(const std::vector<std::string>& fileSpec)
{
    // ["content/Common/Stage/example.szs", "YAZ0", "SARC", "data.bfres"]
    // first part is an extant game file
    std::string cacheKey{""};
    std::string resultKey{""};
    CacheEntry* curEntry = &fileCache;
    bool fullRecompress = true;
    bool fromBaseDir = false; //determines whether to unpack from the working directory or directly from the base directory
    bool toOutput = false;
    for (size_t i = 0; i < fileSpec.size(); i++)
    {
        const std::string& element = fileSpec[i];
        
        if(i == 1) {
            toOutput = true; //last level that would be repacked
        }
        else {
            toOutput = false;
        }
        
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
        if (curEntry->children.count(resultKey) > 0)
        {
            cacheKey = resultKey;
            curEntry = curEntry->children.at(cacheKey).get();
            continue;
        }
        
        if (element.compare("RPX") == 0)
        {
            std::ifstream inputFile;
            if(fromBaseDir) {
                inputFile.open(baseDir / cacheKey, std::ios::binary);
                fromBaseDir = false;
            }
            else {
                inputFile.open(workingDir / cacheKey, std::ios::binary);
            }
            if (!inputFile.is_open()) {
                ErrorLog::getInstance().log("Failed to open input file " + (workingDir / cacheKey).string());
                return "";
            }

            const fspath outputPath = workingDir / resultKey;
            if (!std::filesystem::is_directory(outputPath.parent_path()))
            {
                if(!Utility::create_directories(outputPath.parent_path())) {
                    ErrorLog::getInstance().log("Failed to create directories for path " + outputPath.string());
                    return "";
                }
            }
            std::ofstream outputFile(outputPath, std::ios::binary);
            if (!outputFile.is_open()) {
                ErrorLog::getInstance().log("Failed to open output file " + outputPath.string());
                return "";
            }

            if (RPXError err = FileTypes::rpx_decompress(inputFile, outputFile); err != RPXError::NONE)
            {
                ErrorLog::getInstance().log(std::string("Encountered RPXError on line " TOSTRING(__LINE__) " of ") + __FILENAME__);
                return "";
            }
        }
        else if (element.compare("YAZ0") == 0)
        {
            if(i == 1) { //szs files in stage directory don't need to be recompressed
                fullRecompress = false;
                toOutput = false; //sarc will go to output instead
            }

            std::ifstream inputFile;
            if(fromBaseDir) {
                inputFile.open(baseDir / cacheKey, std::ios::binary);
                fromBaseDir = false;
            }
            else {
                inputFile.open(workingDir / cacheKey, std::ios::binary);
            }
            if (!inputFile.is_open()) {
                ErrorLog::getInstance().log("Failed to open input file " + (workingDir / cacheKey).string());
                return "";
            }

            const fspath outputPath = workingDir / resultKey;
            if (!std::filesystem::is_directory(outputPath.parent_path()))
            {
                if(!Utility::create_directories(outputPath.parent_path())) {
                    ErrorLog::getInstance().log("Failed to create directories for path " + outputPath.string());
                    return "";
                }
            }
            std::ofstream outputFile(outputPath, std::ios::binary);
            if (!outputFile.is_open()) {
                ErrorLog::getInstance().log("Failed to open output file " + outputPath.string());
                return "";
            }

            if(YAZ0Error err = FileTypes::yaz0Decode(inputFile, outputFile); err != YAZ0Error::NONE)
            {
                ErrorLog::getInstance().log(std::string("Encountered YAZ0Error on line " TOSTRING(__LINE__) " of ") + __FILENAME__);
                return "";
            }
        }
        else if(element.compare("SARC") == 0)
        {
            if(curEntry->fullCompress == false && curEntry->toOutput == false) { //if yaz0 doesn't save to output, sarc does instead
                toOutput = true;
            }

            FileTypes::SARCFile sarc;
            SARCError err = SARCError::NONE;
            if(fromBaseDir) {
                err = sarc.loadFromFile((baseDir / cacheKey).string());
                fromBaseDir = false;
            }
            else {
                err = sarc.loadFromFile((workingDir / cacheKey).string());
            }
            if (err != SARCError::NONE) {
                ErrorLog::getInstance().log(std::string("Encountered SARCError on line " TOSTRING(__LINE__) " of ") + __FILENAME__);
                return "";
            }

            const fspath extractTo = workingDir / resultKey;
            if (!std::filesystem::is_directory(extractTo))
            {
                if(!Utility::create_directories(extractTo)) {
                    ErrorLog::getInstance().log("Failed to create directories " + extractTo.string());
                    return "";
                }
            }

            if((err = sarc.extractToDir(extractTo.string())) != SARCError::NONE) {
                ErrorLog::getInstance().log(std::string("Encountered SARCError on line " TOSTRING(__LINE__) " of ") + __FILENAME__);
                return "";
            }
        }
        else if (element.compare("BFRES") == 0)
        {
            FileTypes::resFile fres;
            FRESError err = FRESError::NONE;
            if(fromBaseDir) {
                err = fres.loadFromFile((baseDir / cacheKey).string());
                fromBaseDir = false;
            }
            else {
                err = fres.loadFromFile((workingDir / cacheKey).string());
            }
            if (err != FRESError::NONE) {
                ErrorLog::getInstance().log(std::string("Encountered FRESError on line " TOSTRING(__LINE__) " of ") + __FILENAME__);
                return "";
            }

            const fspath extractTo = workingDir / resultKey;
            if (!std::filesystem::is_directory(extractTo))
            {
                if(!Utility::create_directories(extractTo)) {
                    ErrorLog::getInstance().log("Failed to create directories " + extractTo.string());
                    return "";
                }
            }

            err = fres.extractToDir(extractTo.string());
            if (err != FRESError::NONE) {
                ErrorLog::getInstance().log(std::string("Encountered FRESError on line " TOSTRING(__LINE__) " of ") + __FILENAME__);
                return "";
            }
        }
        else
        {
            //If there is something to unpack, don't copy, just unpack to working dir
            //Otherwise copy to working dir
            if(i == 0) {
                if(fileSpec.size() > 1) {
                    fromBaseDir = true;
                }
                else {
                    // attempt to copy from game directory
                    const fspath gameFilePath = baseDir / resultKey;
                    if (!std::filesystem::is_regular_file(gameFilePath))
                    {
                        ErrorLog::getInstance().log("Missing game file with path " + gameFilePath.string());
                        return "";
                    }

                    const fspath outputPath = workingDir / resultKey;
                    if (!std::filesystem::is_directory(outputPath.parent_path()))
                    {
                        if(!Utility::create_directories(outputPath.parent_path())) {
                            ErrorLog::getInstance().log("Failed to create directories for path " + outputPath.string());
                            return "";
                        }
                    }

                    if (!Utility::copy_file(gameFilePath, outputPath))
                    {
                        ErrorLog::getInstance().log("Failed copying file " + gameFilePath.string() + " to working directory");
                        return "";
                    }

                    toOutput = true;
                }
            }
        }
        cacheKey = resultKey;
        curEntry->children[cacheKey] = std::make_shared<CacheEntry>();
        curEntry = curEntry->children[cacheKey].get();
        curEntry->fullCompress = fullRecompress;
        curEntry->toOutput = toOutput;
        fullRecompress = true;
    }

    return workingDir / resultKey;
}

RandoSession::fspath RandoSession::openGameFile(const RandoSession::fspath& relPath)
{
    CHECK_INITIALIZED("");
    return extractFile(Utility::Str::split(relPath.string(), '@')); //some cases only need the path, not stream
}

bool RandoSession::isCached(const RandoSession::fspath& relPath)
{
    CHECK_INITIALIZED(false);
    const auto& splitPath = Utility::Str::split(relPath.string(), '@');
    const CacheEntry* curEntry = &fileCache;
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
        curEntry = curEntry->children.at(key).get();
    }

    return true;
}

bool RandoSession::copyToGameFile(const fspath& source, const fspath& relPath) {
    CHECK_INITIALIZED(false);

    const fspath destPath = extractFile(Utility::Str::split(relPath.string(), '@'));
    if(destPath.empty()) {
        ErrorLog::getInstance().log("Failed to extract file " + relPath.string());
        return false;
    }

    return Utility::copy_file(source, destPath);
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

    //Repack file in the working directory
    //Write directly to output if it is the last step
    Utility::platformLog("Repacking %s\n", element.c_str());
    std::string resultKey;
    if (Utility::Str::endsWith(element, ".elf"))
    {
        resultKey = element.substr(0, element.size() - 4);

        std::ifstream inputFile((workingDir / element), std::ios::binary);
        if (!inputFile.is_open()) {
            ErrorLog::getInstance().log("Failed to open input file " + (workingDir / element).string());
            return RepackResult::FAIL;
        }

        //Repack to output directory if file exists, otherwise stay in working dir
        std::ofstream outputFile;
        if(entry->toOutput) {
            outputFile.open(outputDir / resultKey, std::ios::binary);
        }
        else {
            if (Utility::exists((workingDir / resultKey)) == false) {
                if(!Utility::create_directories((workingDir / resultKey).parent_path())) {
                    ErrorLog::getInstance().log("Failed to create directories for path " + (workingDir / resultKey).string());
                    return RepackResult::FAIL;
                }
            }
            
            outputFile.open(workingDir / resultKey, std::ios::binary);
        }
        if (!outputFile.is_open()) {
                ErrorLog::getInstance().log("Failed to open output file " + (workingDir / resultKey).string());
                return RepackResult::FAIL;
        }

        if (RPXError err = FileTypes::rpx_compress(inputFile, outputFile); err != RPXError::NONE)
        {
            ErrorLog::getInstance().log(std::string("Encountered RPXError on line " TOSTRING(__LINE__) " of ") + __FILENAME__);
            return RepackResult::FAIL;
        }
    }
    else if (Utility::Str::endsWith(element, ".dec"))
    {
        if(entry->fullCompress) {
            resultKey = element.substr(0, element.size() - 4);

            std::ifstream inputFile((workingDir / element), std::ios::binary);
            if (!inputFile.is_open()) {
                ErrorLog::getInstance().log("Failed to open input file " + (workingDir / element).string());
                return RepackResult::FAIL;
            }

            //Repack to output directory if file exists, otherwise stay in working dir
            std::ofstream outputFile;
            if(entry->toOutput) {
                outputFile.open(outputDir / resultKey, std::ios::binary);
            }
            else {
                if (Utility::exists((workingDir / resultKey)) == false) {
                    if(!Utility::create_directories((workingDir / resultKey).parent_path())) {
                        ErrorLog::getInstance().log("Failed to create directories for path " + (workingDir / resultKey).string());
                        return RepackResult::FAIL;
                    }
                }

                outputFile.open(workingDir / resultKey, std::ios::binary);
            }
            if (!outputFile.is_open()) {
                ErrorLog::getInstance().log("Failed to open output file " + (workingDir / resultKey).string());
                return RepackResult::FAIL;
            }

            Utility::platformLog("Recompressing %s\n", element.c_str());
            if (YAZ0Error err = FileTypes::yaz0Encode(inputFile, outputFile, 9); err != YAZ0Error::NONE)
            {
                ErrorLog::getInstance().log(std::string("Encountered YAZ0Error on line " TOSTRING(__LINE__) " of ") + __FILENAME__);
                return RepackResult::FAIL;
            }
        }
    }
    else if (Utility::Str::endsWith(element, ".unpack/"))
    {
        resultKey = element.substr(0, element.size() - 8);

        FileTypes::SARCFile sarc;
        SARCError err = SARCError::NONE;
        //The randomizer skips copying the original file if it can be unpacked from the base directory (is slightly faster than copying)
        //If that was the case, we must also read the original SARC from the base directory
        if(Utility::exists((baseDir / resultKey))) {
            err = sarc.loadFromFile((baseDir / resultKey).string());
        }
        else {
            err = sarc.loadFromFile((workingDir / resultKey).string());
        }
        if (err != SARCError::NONE)
        {
            ErrorLog::getInstance().log(std::string("Encountered SARCError on line " TOSTRING(__LINE__) " of ") + __FILENAME__);
            return RepackResult::FAIL;
        }

        //only rebuild the changed files, saves time
        for(const auto& child : entry->children) {
            err = sarc.replaceFile(child.first.substr(element.size()) + '\0', (workingDir / child.first).string());
            if (err != SARCError::NONE)
            {
                ErrorLog::getInstance().log(std::string("Encountered SARCError on line " TOSTRING(__LINE__) " of ") + __FILENAME__);
                return RepackResult::FAIL;
            }
        }

        //Repack to output directory if file exists, otherwise stay in working dir
        if(entry->toOutput) {
            if(Utility::Str::endsWith(resultKey, ".dec")) { //remove extra extension if skipping compression
                resultKey = resultKey.substr(0, resultKey.size() - 4);
            }
            err = sarc.writeToFile((outputDir / resultKey).string());
        }
        else {
            if (Utility::exists((workingDir / resultKey)) == false) {
                if(!Utility::create_directories((workingDir / resultKey).parent_path())) {
                    ErrorLog::getInstance().log("Failed to create directories for path " + (workingDir / resultKey).string());
                    return RepackResult::FAIL;
                }
            }

            err = sarc.writeToFile((workingDir / resultKey).string());
        }

        if (err != SARCError::NONE) 
        {
            ErrorLog::getInstance().log(std::string("Encountered SARCError on line " TOSTRING(__LINE__) " of ") + __FILENAME__);
            return RepackResult::FAIL;
        }
    }
    else if (Utility::Str::endsWith(element, ".res/"))
    {
        resultKey = element.substr(0, element.size() - 5);

        FileTypes::resFile fres;
        FRESError err = FRESError::NONE;
        if(Utility::exists((baseDir / resultKey))) {
            err = fres.loadFromFile((baseDir / resultKey).string());
        }
        else {
            err = fres.loadFromFile((workingDir / resultKey).string());
        }
        if (err != FRESError::NONE)
        {
            ErrorLog::getInstance().log(std::string("Encountered FRESError on line " TOSTRING(__LINE__) " of ") + __FILENAME__);
            return RepackResult::FAIL;
        }

        //only rebuild the changed files, saves time
        for(const auto& child : entry->children) {
            err = fres.replaceEmbeddedFile(child.first.substr(element.size()), (workingDir / child.first).string());
            if (err != FRESError::NONE)
            {
                ErrorLog::getInstance().log(std::string("Encountered FRESError on line " TOSTRING(__LINE__) " of ") + __FILENAME__);
                return RepackResult::FAIL;
            }
        }
        
        //Repack to output directory if file exists, otherwise stay in working dir
        if(entry->toOutput) {
            err = fres.writeToFile((outputDir / resultKey).string());
        }
        else {
            if (Utility::exists((workingDir / resultKey)) == false) {
                if(!Utility::create_directories((workingDir / resultKey).parent_path())) {
                    ErrorLog::getInstance().log("Failed to create directories for path " + (workingDir / resultKey).string());
                    return RepackResult::FAIL;
                }
            }

            err = fres.writeToFile((workingDir / resultKey).string());
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
            if(!Utility::copy_file(workingDir / element, outputDir / element)) {
                ErrorLog::getInstance().log("Failed copying file " + (workingDir / element).string() + " to output directory");
                return RepackResult::FAIL;
            }
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

    //fileCache is stored as an entry, not a pointer to an entry, can't just use the queue function
    for(auto& child : fileCache.children) { //go down the tree
        queueChildren(child.second);
    }
    //queue this level's children
    for(const auto& child : fileCache.children) {
        workerThreads.push_task(std::bind(&RandoSession::repackFile, this, child.first, child.second));
    }

    workerThreads.waitForAll();
    Utility::platformLog("Finished repacking files\n");
    BasicLog::getInstance().log("Finished repacking files\n");
    
    return true;
}



std::vector<std::pair<std::string, std::shared_ptr<RandoSession::CacheEntry>>> generateList(RandoSession::CacheEntry& entry) {
    static std::vector<std::pair<std::string, std::shared_ptr<RandoSession::CacheEntry>>> items;

    //go down the tree and wait for lower parts to queue
    for(auto& child : entry.children) { //go down the tree
        generateList(*child.second.get());
    }

    //queue this level's children
    for(const auto& child : entry.children) {
        std::pair<std::string, std::shared_ptr<RandoSession::CacheEntry>> temp = std::make_pair(child.first, child.second);
        items.push_back(temp);
    }

    return items;
}

//single threaded things for debugging fun Wii U heap corruption
void printLoop() {
    size_t i = 0;
    while(true) {
        if(i > 10) break;
        Utility::platformLog("Didn't crash!%d\n", i);
        i++;
        std::this_thread::sleep_for(std::chrono::seconds(20));
    }
}

bool RandoSession::repackCache_singleThread() {
    const auto& items = generateList(fileCache);
    std::thread print(printLoop);
    for(auto& item : items) {
        Utility::platformLog("Repacking %s\n", item.first.c_str());
        repackFile(item.first, item.second);
    }
    Utility::platformLog("Finished repacking files\n");
    BasicLog::getInstance().log("Finished repacking files\n");
    return true;
}
