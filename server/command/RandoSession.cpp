#include "RandoSession.hpp"

#include <fstream>
#include <mutex>
#include <syncstream>
#include <unordered_map>
#include <memory>
#include <mutex>
#include <future>

#include "../filetypes/wiiurpx.hpp"
#include "../filetypes/yaz0.hpp"
#include "../filetypes/sarc.hpp"
#include "../filetypes/bfres.hpp"
#include "../utility/stringUtil.hpp"
#include "./Log.hpp"

using namespace Utility::Str;
using namespace std::literals::chrono_literals;



RandoSession::RandoSession(const fspath& gameBaseDir, const fspath& randoWorkingDir, const fspath& randoOutputDir) : 
    baseDir(gameBaseDir),
    workingDir(randoWorkingDir),
    outputDir(randoOutputDir)
{
    //need to mount filesystem before clearing, global construction might cause issues with that on console
    //still need to clear the directory first though
    //clearWorkingDir();
}

void RandoSession::init() { //might have more init stuff later
    clearWorkingDir();
    initialized = true;
}

void RandoSession::clearWorkingDir() {
    if (std::filesystem::is_directory(workingDir)) {
        std::filesystem::remove_all(workingDir);
    }
    std::filesystem::create_directories(workingDir);
    return;
}

RandoSession::fspath RandoSession::relToGameAbsolute(const RandoSession::fspath& relPath)
{
    return baseDir / relPath;
}



RandoSession::fspath RandoSession::extractFile(const std::vector<std::string>& fileSpec)
{
    // ["content/Common/Stage/example.szs", "YAZ0", "SARC", "data.bfres"]
    // first part is an extant game file
    std::string cacheKey{""};
    std::string resultKey{""};
    CacheEntry* curEntry = &fileCache;
    for (const auto& element : fileSpec)
    {
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
            std::ifstream inputFile(workingDir / cacheKey, std::ios::binary);
            if (!inputFile.is_open()) return "";
            std::ofstream outputFile(workingDir / resultKey, std::ios::binary);
            if (!inputFile.is_open()) return "";
            if (RPXError err = FileTypes::rpx_decompress(inputFile, outputFile); err != RPXError::NONE)
            {
                return "";
            }
        }
        else if (element.compare("YAZ0") == 0)
        {
            std::ifstream inputFile(workingDir / cacheKey, std::ios::binary);
            if (!inputFile.is_open()) return "";
            std::ofstream outputFile(workingDir / resultKey, std::ios::binary);
            if (!inputFile.is_open()) return "";
            if(YAZ0Error err = FileTypes::yaz0Decode(inputFile, outputFile); err != YAZ0Error::NONE)
            {
                return "";
            }
        }
        else if(element.compare("SARC") == 0)
        {
            FileTypes::SARCFile sarc;
            auto err = sarc.loadFromFile((workingDir / cacheKey).string());
            if (err != SARCError::NONE) return "";
            fspath extractTo = workingDir / resultKey;
            if (!std::filesystem::is_directory(extractTo))
            {
                std::filesystem::create_directory(extractTo);
            }
            if((err = sarc.extractToDir(extractTo.string())) != SARCError::NONE) return "";
        }
        else if (element.compare("BFRES") == 0)
        {
            FileTypes::resFile fres;
            auto err = fres.loadFromFile((workingDir / cacheKey).string());
            if (err != FRESError::NONE) return "";
            fspath extractTo = workingDir / resultKey;
            if (!std::filesystem::is_directory(extractTo))
            {
                std::filesystem::create_directory(extractTo);
            }
            if ((err = fres.extractToDir(extractTo.string())) != FRESError::NONE) return "";
        }
        else
        {
            if (!std::filesystem::is_regular_file(workingDir / resultKey))
            {
                // attempt to copy from game directory
                fspath gameFilePath = relToGameAbsolute(resultKey);
                if (!std::filesystem::is_regular_file(gameFilePath))
                {
                    return "";
                }
                fspath outputPath = workingDir / resultKey;
                if (!std::filesystem::is_directory(outputPath.parent_path()))
                {
                    if(!std::filesystem::create_directories(outputPath.parent_path())) return "";
                }
                auto copyOptions = std::filesystem::copy_options::overwrite_existing;
                if (!std::filesystem::copy_file(gameFilePath, outputPath, copyOptions))
                {
                    return "";
                }
            }
        }
        cacheKey = resultKey;
        curEntry->children[cacheKey] = std::make_unique<CacheEntry>();
        curEntry = curEntry->children[cacheKey].get();
    }

    return workingDir / resultKey;
}

RandoSession::fspath RandoSession::openGameFile(const RandoSession::fspath& relPath)
{
    return extractFile(split(relPath.string(), '@')); //some cases only need the path, not stream
}

bool RandoSession::copyToGameFile(const fspath& source, const fspath& relPath) {
    if(!initialized) return false;
    const fspath destPath = extractFile(split(relPath.string(), '@'));
    return std::filesystem::copy_file(source, destPath, std::filesystem::copy_options::overwrite_existing);
}

bool RandoSession::repackFile(const std::string& element) {
    std::string resultKey;
    if (endsWith(element, ".elf"))
    {
        resultKey = element.substr(0, element.size() - 4);
        std::ifstream inputFile((workingDir / element), std::ios::binary);
        if (!inputFile.is_open()) return 0;
        std::ofstream outputFile;
        if (std::filesystem::exists((workingDir / resultKey)) == false) {
            std::filesystem::create_directories((workingDir / resultKey).parent_path());
        }
        outputFile.open((workingDir / resultKey), std::ios::binary);
        if (RPXError err = FileTypes::rpx_compress(inputFile, outputFile); err != RPXError::NONE)
        {
            return 0;
        }
    }
    else if (endsWith(element, ".dec"))
    {
        resultKey = element.substr(0, element.size() - 4);
        std::ifstream inputFile((workingDir / element), std::ios::binary);
        if (!inputFile.is_open()) return 0;
        std::ofstream outputFile;
        if (std::filesystem::exists((workingDir / resultKey)) == false) {
            std::filesystem::create_directories((workingDir / resultKey).parent_path());
        }
        outputFile.open((workingDir / resultKey), std::ios::binary);
        if (YAZ0Error err = FileTypes::yaz0Encode(inputFile, outputFile); err != YAZ0Error::NONE)
        {
            return 0;
        }
    }
    else if (endsWith(element, ".unpack/"))
    {
        resultKey = element.substr(0, element.size() - 8);
        FileTypes::SARCFile sarc;
        auto err = sarc.loadFromFile((workingDir / resultKey).string());
        if (err != SARCError::NONE) return 0;
        err = sarc.rebuildFromDir((workingDir / element).string());
        if (err != SARCError::NONE) return 0;
        if (std::filesystem::exists((workingDir / resultKey)) == false) {
            std::filesystem::create_directories((workingDir / resultKey).parent_path());
        }
        err = sarc.writeToFile((workingDir / resultKey).string());
        if (err != SARCError::NONE) return 0;
    }
    else if (endsWith(element, ".res/"))
    {
        resultKey = element.substr(0, element.size() - 5);
        FileTypes::resFile fres;
        auto err = fres.loadFromFile((workingDir / resultKey).string()); //load the original BFRES
        if (err != FRESError::NONE) return 0;
        err = fres.replaceFromDir((workingDir / element).string()); //Update embedded files
        if (err != FRESError::NONE) return 0;
        
        if (std::filesystem::exists((workingDir / resultKey)) == false) {
            std::filesystem::create_directories((workingDir / resultKey).parent_path());
        }
        err = fres.writeToFile((workingDir / resultKey).string());
        if (err != FRESError::NONE) return 0;
    }
    else
    {
        if (std::filesystem::exists((outputDir / element))) { //if it exists in the output directory (the game files on console storage), copy to that file
            if(!std::filesystem::copy_file(workingDir / element, outputDir / element, std::filesystem::copy_options::overwrite_existing)) 
                return 0;
        }
    }

    return 1;
}

bool RandoSession::repackChildren(CacheEntry& entry, const std::string& temp) {
    std::vector<std::future<bool>> futures;

    for(auto& child : entry.children) { //go down the tree
        futures.push_back(std::async(std::launch::async, &RandoSession::repackChildren, this, std::ref(child.second), child.first));
    }

    while(futures.size() != 0) { //wait for lower parts of tree to finish
        futures[0].wait();
        if(futures[0].get() == false) return false;
        futures.erase(futures.begin());
    }

    std::vector<std::future<bool>> futures2;
    for(const auto& child : entry.children)  { //repack this level once children are done
        futures2.push_back(std::async(std::launch::async, &RandoSession::repackFile, this, child.first));
    }

    while(futures2.size() != 0) { //wait for children to finish
        futures2[0].wait();
        if(futures2[0].get() == false) return false;
        futures2.erase(futures2.begin());
    }
    entry.children.clear(); //children no longer cached

    return true;
}

bool RandoSession::repackCache()
{
    //traverse tree, give each child a thread, when no children repack, wait until all children finish and pack back up tree
    return repackChildren(fileCache, "root");
}
