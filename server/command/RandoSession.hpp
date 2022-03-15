#pragma once

#include "WWHDStructs.hpp"

#include <filesystem>
#include <string>
#include <vector>
#include <list>
#include <unordered_set>
#include <unordered_map>



class RandoSession
{
public:
    using fspath = std::filesystem::path;

    RandoSession(const fspath& gameBaseDir, const fspath& randoWorkingDir, const fspath& randoOutputDir);

    [[nodiscard]] fspath openGameFile(const fspath& relPath);
    [[nodiscard]] bool copyToGameFile(const fspath& source, const fspath& relPath);
    [[nodiscard]] bool repackCache();
private:
    void clearWorkingDir();
    fspath extractFile(const std::vector<std::string>& fileSpec);
    
    fspath relToGameAbsolute(const fspath& relPath);
    //fspath absToGameRelative(const fspath& absPath);

    fspath baseDir;
    fspath workingDir;
    fspath outputDir;
    std::unordered_set<std::string> fileCache;
    std::list<std::string> orderedCache;
};
