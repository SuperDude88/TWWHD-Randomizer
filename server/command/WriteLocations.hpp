#pragma once

#include <string>
#include <vector>
#include <memory>
#include <utility>

#include "./RandoSession.hpp"
#include "../../logic/GameItem.hpp"
#include "../../libs/ryml.hpp"

extern RandoSession g_session;

enum struct [[nodiscard]] ModificationError {
    NONE = 0,
    MISSING_KEY = 0,
    INVALID_OFFSET,
    MISSING_VALUE,
    UNKNOWN_ACTOR_NAME,
    INVALID_MASK,
    INVALID_SYMBOL,
    RPX_NOT_OPEN,
    UNKNOWN,
    COUNT
};

void saveRPX();

class LocationModification
{
public:
    virtual ~LocationModification() {}

    virtual std::unique_ptr<LocationModification> duplicate() const { return std::make_unique<LocationModification>(*this); }
    virtual ModificationError parseArgs(const ryml::NodeRef& locationObject) { return ModificationError::NONE; }
    virtual ModificationError writeLocation(const Item& item) { return ModificationError::NONE; }
};

class ModifyChest : public LocationModification {
private:
    std::string filePath;
    std::vector<uint32_t> offsets;

public:
    ~ModifyChest() override {}

    std::unique_ptr<LocationModification> duplicate() const override { return std::make_unique<ModifyChest>(*this); }
    ModificationError parseArgs(const ryml::NodeRef& locationObject) override;
    ModificationError writeLocation(const Item& item) override;
};

class ModifyActor : public LocationModification {
private:
    std::string filePath;
    std::vector<uint32_t> offsets;

public:
    ~ModifyActor() override {}

    std::unique_ptr<LocationModification> duplicate() const override { return std::make_unique<ModifyActor>(*this); }
    ModificationError parseArgs(const ryml::NodeRef& locationObject) override;
    ModificationError writeLocation(const Item& item) override;
};

class ModifySCOB : public LocationModification {
private:
    std::string filePath;
    std::vector<uint32_t> offsets;

public:
    ~ModifySCOB() override {}

    std::unique_ptr<LocationModification> duplicate() const override { return std::make_unique<ModifySCOB>(*this); }
    ModificationError parseArgs(const ryml::NodeRef& locationObject) override;
    ModificationError writeLocation(const Item& item) override;
};

class ModifyEvent : public LocationModification {
private:
    std::string filePath;
    uint32_t offset;
    uint32_t nameOffset;

public:
    ~ModifyEvent() override {}

    std::unique_ptr<LocationModification> duplicate() const override { return std::make_unique<ModifyEvent>(*this); }
    ModificationError parseArgs(const ryml::NodeRef& locationObject) override;
    ModificationError writeLocation(const Item& item) override;
};

class ModifyRPX : public LocationModification {
private:
    std::vector<uint32_t> offsets;

public:
    ~ModifyRPX() override {}

    std::unique_ptr<LocationModification> duplicate() const override { return std::make_unique<ModifyRPX>(*this); }
    ModificationError parseArgs(const ryml::NodeRef& locationObject) override;
    ModificationError writeLocation(const Item& item) override;
};

class ModifySymbol : public LocationModification {
private:
    std::string symbolName;

public:
    ~ModifySymbol() override {}

    std::unique_ptr<LocationModification> duplicate() const override { return std::make_unique<ModifySymbol>(*this); }
    ModificationError parseArgs(const ryml::NodeRef& locationObject) override;
    ModificationError writeLocation(const Item& item) override;
};

class ModifyBoss : public LocationModification {
private:
    std::vector<std::pair<std::string, uint32_t>> offsetsWithPath;

public:
    ~ModifyBoss() override {}

    std::unique_ptr<LocationModification> duplicate() const override { return std::make_unique<ModifyBoss>(*this); }
    ModificationError parseArgs(const ryml::NodeRef& locationObject) override;
    ModificationError writeLocation(const Item& item) override;
};

const char* modErrorToName(ModificationError err);
