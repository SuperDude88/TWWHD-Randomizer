#include "RandoSession.hpp"

#include <fstream>

#include "../filetypes/wiiurpx.hpp"
#include "../filetypes/yaz0.hpp"
#include "../filetypes/sarc.hpp"
#include "../filetypes/bfres.hpp"



#ifdef _WIN32
constexpr char PATHSEP = '\\';
#else
constexpr char PATHSEP = '/';
#endif



bool endsWith(const std::string& a, const std::string& b) {
    if (b.size() > a.size()) return false;
    return std::equal(a.end() - b.size(), a.end(), b.begin());
}

std::vector<std::string> splitPath(const std::string& relPath, char delim) {
    std::vector<std::string> path;
    std::string tail = relPath;
    std::string segment;
    auto sepIndex = tail.find_first_of(delim);
    while ((sepIndex = tail.find_first_of(delim)), sepIndex != std::string::npos) {
        segment = tail.substr(0, sepIndex);
        tail = tail.substr(sepIndex + 1);
        path.push_back(segment);
    }
    path.push_back(tail); //add anything past the last delim to the list
    return path;
}



RandoSession::RandoSession(const fspath& gameBaseDir, const fspath& randoWorkingDir, const fspath& outputDir) : 
    gameBaseDirectory(gameBaseDir),
    workingDir(randoWorkingDir),
    outputDir(outputDir)
{
    clearWorkingDir(); //should be cleared at the start of each session
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
    return gameBaseDirectory / relPath;
}

/*RandoSession::fspath RandoSession::absToGameRelative(const RandoSession::fspath& absPath)
{
    return std::filesystem::relative(absPath, gameBaseDirectory);
}*/


RandoSession::fspath RandoSession::extractFile(const std::vector<std::string>& fileSpec)
{
    // ["content/Common/Stage/example.szs", "YAZ0", "SARC", "data.bfres"]
    // first part is an extant game file
    std::string cacheKey{""};
    std::string resultKey{""};
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
        if (fileCache.count(resultKey) > 0)
        {
            cacheKey = resultKey;
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
        fileCache.emplace(resultKey);
        orderedCache.push_back(resultKey);
        cacheKey = resultKey;
    }
    return workingDir / resultKey;
}

RandoSession::fspath RandoSession::openGameFile(const RandoSession::fspath& relPath)
{
    auto workingPath = extractFile(splitPath(relPath.string(), '@'));
    return workingPath; //some cases we dont want to open the file, any others it can be opened afterwards
}

bool RandoSession::copyToGameFile(const fspath& source, const fspath& relPath) {
    const fspath destPath = extractFile(splitPath(relPath.string(), '@'));
    return std::filesystem::copy_file(source, destPath, std::filesystem::copy_options::overwrite_existing);
}

bool RandoSession::repackCache()
{
    std::string resultKey;
    while (orderedCache.size() > 0) { //Repack
        auto it = orderedCache.end() - 1;
        const std::string& element = *it;
        if (element.empty()) continue;

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
            resultKey = resultKey.substr(0, element.size() - 5);
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
                if(!std::filesystem::copy_file(workingDir / element, outputDir / element, std::filesystem::copy_options::overwrite_existing)) return 0;
            }
        }
        
        fileCache.erase(element);
        orderedCache.pop_back();
    }

    return 1;
}
