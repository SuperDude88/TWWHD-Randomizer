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

    static NUSPackage createNew(const PackageConfig& config);

    NUSPackage() :
        ticket(),
        contents(),
        fst(contents),
        tmd(ticket, contents),
        outputDir()
    {}
    NUSPackage(const Ticket& ticket_) :
        ticket(ticket_),
        contents(),
        fst(contents),
        tmd(ticket, contents),
        outputDir()
    {}

    void PackContents(const std::filesystem::path& out);
};
