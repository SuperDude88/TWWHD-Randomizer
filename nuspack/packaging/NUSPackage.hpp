#pragma once

#include <nuspack/packaging/NUSPackageConfig.hpp>
#include <nuspack/packaging/tmd.hpp>
#include <nuspack/packaging/fst.hpp>
#include <nuspack/packaging/ticket.hpp>



enum class [[nodiscard]] NUSError {
    NONE = 0,
    UNKNOWN,
    COUNT
};

class NUSPackage {
public:
    Ticket ticket;
    Contents contents;

    FileTypes::FSTFile fst;
    FileTypes::TMDFile tmd;
    std::string outputDir;

    NUSPackage() = delete;
    NUSPackage(const PackageConfig& config); //do all the things in a constructor to force no copy/move
    ~NUSPackage() = default;

    //don't copy or move, references/pointers break
    NUSPackage(const NUSPackage&) = delete;
    NUSPackage(NUSPackage&&) = delete;
    NUSPackage& operator=(const NUSPackage& other) = delete;
    NUSPackage& operator=(NUSPackage&& other) = delete;

    void PackContents(const std::filesystem::path& out);
};
