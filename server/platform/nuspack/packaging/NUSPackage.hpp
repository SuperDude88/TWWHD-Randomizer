#pragma once

#include "NUSPackageConfig.hpp"
#include "tmd.hpp"
#include "fst.hpp"
#include "ticket.hpp"



enum class [[nodiscard]] NUSError {
    NONE = 0,
    UNKNOWN,
    COUNT
};

class NUSPackage {
public:
    Ticket ticket;
    FileTypes::TMDFile tmd;
    FileTypes::FSTFile fst;
    std::string outputDir;

    static NUSPackage createNew(const PackageConfig& config);

    NUSPackage() = default;
    NUSPackage(const Ticket& ticket_) :
        ticket(ticket_)
    {}

    void PackContents(const std::filesystem::path& out);
};
