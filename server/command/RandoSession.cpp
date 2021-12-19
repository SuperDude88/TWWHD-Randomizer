#include "RandoSession.hpp"
#include <fstream>
#include "../filetypes/yaz0.hpp"
#include "../filetypes/sarc.hpp"
#include "../filetypes/bfres.hpp"

#ifdef _WIN32
constexpr char PATHSEP = '\\';
#else
constexpr char PATHSEP = '/';
#endif

RandoSession::RandoSession(const fspath& gameBaseDir, 
             const fspath& randoWorkingDir, 
             const fspath& outputDir) : 
    gameBaseDirectory(gameBaseDir), workingDir(randoWorkingDir), outputDir(outputDir)
{
    
}

std::string splitPath(std::string& tail, char delim)
{
    std::string head;
    auto sepIndex = tail.find_first_of(delim);
    if (sepIndex == std::string::npos)
    {
        return "";
    }
    head = tail.substr(0, sepIndex);
    tail = tail.substr(sepIndex + 1);
    return head;
}

std::string splitPathRL(std::string& tail, char delim)
{
    std::string head;
    auto sepIndex = tail.find_last_of(delim);
    if (sepIndex == std::string::npos)
    {
        head = tail;
        tail = "";
        return head;
    }
    head = tail.substr(0, sepIndex);
    tail = tail.substr(sepIndex + 1);
    return head;
}

std::vector<std::string> sepPath(std::string& relPath, char delim) {
    std::vector<std::string> path;
    std::string tail = relPath;
    std::string segment;
    auto sepIndex = tail.find_first_of(delim);
    while ((sepIndex = tail.find_first_of(delim)), sepIndex != std::string::npos) {
        segment = tail.substr(0, sepIndex);
        tail = tail.substr(sepIndex + 1);
        path.push_back(segment);
    }
    return path;
}

std::string RandoSession::relToExtract(std::string relPath, char delim) {
    std::string outPath{};
    std::string absPath = (relToGameAbsolute(relPath)).string();
    std::vector<std::string> path = sepPath(absPath, delim);
    for (const auto& element : path) {
        if (element.empty()) continue;

        if (element.compare("YAZ0") == 0)
        {
            outPath = outPath + ".dec";
        }
        else if (element.compare("SARC") == 0)
        {
            // TODO: do we need to make / here platform agnostic?
            outPath = outPath + ".unpack/";
        }
        else if (element.compare("BFRES") == 0)
        {
            // TODO: do we need to make / here platform agnostic?
            outPath = outPath + ".res/";
        }
        else
        {
            outPath = outPath + element;
        }
    }
    return outPath;
}

RandoSession::fspath RandoSession::relToGameAbsolute(const RandoSession::fspath& relPath)
{
    return gameBaseDirectory / relPath;
}

RandoSession::fspath RandoSession::absToGameRelative(const RandoSession::fspath& absPath)
{
    return std::filesystem::relative(absPath, gameBaseDirectory);
}

RandoSession::fspath RandoSession::extractFile(const std::vector<std::string>& fileSpec)
{
    // ["content/Common/Stage/example.szs", "YAZ0", "SARC", "data.bfres"]
    // first part is an extant game file
    std::string cacheKey{""};
    std::string resultKey{""};
    for (const auto& element : fileSpec)
    {
        if (element.empty()) continue;

        if (element.compare("YAZ0") == 0)
        {
            resultKey = cacheKey + ".dec";
        }
        else if(element.compare("SARC") == 0)
        {
            // TODO: do we need to make / here platform agnostic?
            resultKey = cacheKey + ".unpack/";
        }
        else if (element.compare("BFRES") == 0)
        {
            // TODO: do we need to make / here platform agnostic?
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
        
        if (element.compare("YAZ0") == 0)
        {
            std::ifstream inputFile(workingDir / cacheKey, std::ios::binary);
            if (!inputFile.is_open()) return "";
            std::ofstream outputFile(workingDir / resultKey, std::ios::binary);
            if (!inputFile.is_open()) return "";
            if(FileTypes::yaz0Decode(inputFile, outputFile) == 0)
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
        cacheKey = resultKey;
    }
    return workingDir / resultKey;
}

std::ifstream RandoSession::openGameFile(const std::vector<std::string>& gameFilePath, RandoSession::fspath& relPath)
{
    auto workingPath = extractFile(gameFilePath);
    relPath = std::filesystem::relative(workingPath, workingDir);
    return std::ifstream(workingPath, std::ios::binary);
}

bool RandoSession::repackGameFile(const std::vector<std::string>& gameFilePath, bool inOutputDir)
{
    std::string resultKey{""};
    std::string cacheKey{""};
    for (const auto& element : gameFilePath)
    {
        if (element.empty()) continue;

        if (element.compare("YAZ0") == 0)
        {
            resultKey = cacheKey + ".dec";
        }
        else if (element.compare("SARC") == 0)
        {
            // TODO: do we need to make / here platform agnostic?
            resultKey = cacheKey + ".unpack/";
        }
        else if (element.compare("BFRES") == 0)
        {
            // TODO: do we need to make / here platform agnostic?
            resultKey = cacheKey + ".res/";
        }
        else
        {
            resultKey = cacheKey + element;
        }
        cacheKey = resultKey;
    }
    for (int i = gameFilePath.size() - 1; i >= 0; i--) { //Repack
        const auto& element = gameFilePath[i];
        if (element.empty()) continue;

        if (element.compare("YAZ0") == 0)
        {
            resultKey = resultKey.substr(0, resultKey.find(".dec")); //remove this from the path
            std::ifstream inputFile((workingDir / cacheKey).string(), std::ios::binary);
            if (!inputFile.is_open()) return 0;
            std::ofstream outputFile;
            if (i == 1 && inOutputDir == true) { //if this is the last step for recompression, index 1 because filetype always comes after path
                if (std::filesystem::exists((outputDir / resultKey).string()) == false) {
                    std::filesystem::create_directories((outputDir / resultKey).parent_path());
                }
                outputFile.open((outputDir / resultKey).string(), std::ios::binary);
            }
            else {
                if (std::filesystem::exists((workingDir / resultKey).string()) == false) {
                    std::filesystem::create_directories((workingDir / resultKey).parent_path());
                }
                outputFile.open((workingDir / resultKey).string(), std::ios::binary);
            }
            if (FileTypes::yaz0Encode(inputFile, outputFile) == 0)
            {
                return 0;
            }
        }
        else if (element.compare("SARC") == 0)
        {
            // TODO: do we need to make / here platform agnostic?
            resultKey = resultKey.substr(0, resultKey.find(".unpack/"));
            FileTypes::SARCFile sarc;
            auto err = SARCError::NONE;
            err = sarc.loadFromFile((workingDir / resultKey).string());
            if (err != SARCError::NONE) return 0;
            std::vector<SARCFileSpec> fileSpecs = sarc.getFileList(); //probably a better way, but this is simple to get the original list of files
            auto lastDelimPos = resultKey.find_last_of(PATHSEP);
            std::string storedFilename;
            if (lastDelimPos == std::string::npos)
            {
                storedFilename = resultKey;
            }
            else
            {
                storedFilename = resultKey.substr(lastDelimPos + 1);
            }
            FileTypes::SARCFile outSARC = FileTypes::SARCFile::createNew(storedFilename);
            for (const auto& file : fileSpecs) {
                std::ifstream inFile((workingDir / cacheKey).string() + "/" + file.fileName, std::ios::binary);
                if (!inFile.is_open())
                {
                    return 0;
                }
                err = outSARC.addFile(file.fileName, inFile);
                if (err != SARCError::NONE) return 0;
            }
            if (i == 1 && inOutputDir == true) { //if this is the last step for recompression, index 1 because filetype always comes after path
                if (std::filesystem::exists((outputDir / resultKey).string()) == false) {
                    std::filesystem::create_directories((outputDir / resultKey).parent_path());
                }
                err = outSARC.writeToFile((outputDir / resultKey).string());
                if (err != SARCError::NONE) return 0;
            }
            else {
                if (std::filesystem::exists((workingDir / resultKey).string()) == false) {
                    std::filesystem::create_directories((workingDir / resultKey).parent_path());
                }
                err = outSARC.writeToFile((workingDir / resultKey).string());
                if (err != SARCError::NONE) return 0;
            }
        }
        else if (element.compare("BFRES") == 0)
        {
            // TODO: do we need to make / here platform agnostic?
            resultKey = resultKey.substr(0, resultKey.find(".res/"));
            FileTypes::resFile fres;
            auto err = fres.loadFromFile((workingDir / resultKey).string()); //load the original BFRES
            if (err != FRESError::NONE) return 0;
            err = fres.replaceFromDir((workingDir / cacheKey).string()); //Update embedded files
            if (err != FRESError::NONE) return 0;
            if (i == 1 && inOutputDir == true) { //if this is the last step for recompression, index 1 because filetype always comes after path
                if (std::filesystem::exists((outputDir / resultKey).string()) == false) {
                    std::filesystem::create_directories((outputDir / resultKey).parent_path());
                }
                err = fres.writeToFile((outputDir / resultKey).string());
            } else {
                if (std::filesystem::exists((workingDir / resultKey).string()) == false) {
                    std::filesystem::create_directories((workingDir / resultKey).parent_path());
                }
                err = fres.writeToFile((workingDir / resultKey).string());
            }
            if (err != FRESError::NONE) return 0;
        }
        else
        {
            resultKey = resultKey.substr(0, resultKey.find(element));
        }

        cacheKey = resultKey;
        //Uncache each step as compression completes maybe

    }
    return 1;
}
