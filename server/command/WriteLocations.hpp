#pragma once

#include <string>
#include <vector>
#include <unordered_set>
#include <memory>
#include <utility>

#include "./RandoSession.hpp"
#include "../../logic/GameItem.hpp"
#include "../../libs/Yaml.hpp"
#include "../../logic/Dungeon.hpp"

extern RandoSession g_session;

enum struct [[nodiscard]] ModificationError {
    NONE = 0,
    MISSING_KEY = 0,
    INVALID_OFFSET,
    MISSING_VALUE,
    UNKNOWN_ACTOR_NAME,
    INVALID_MASK,
    INVALID_SYMBOL,
    RPX_ERROR,
    UNKNOWN,
    COUNT
};

void resetRPX();
void saveRPX();

class LocationModification
{
public:
    LocationModification() {}
    virtual ~LocationModification() {}
    LocationModification(const LocationModification& other) = default;
    LocationModification& operator=(const LocationModification& other) = default;
    LocationModification(LocationModification&& other) = default;
    LocationModification& operator=(LocationModification&& other) = default;

    virtual std::unique_ptr<LocationModification> duplicate() const { return std::make_unique<LocationModification>(*this); }
    virtual ModificationError parseArgs(Yaml::Node& locationObject) { return ModificationError::NONE; }
    virtual ModificationError writeLocation(const Item& item) { return ModificationError::NONE; }
};

class ModifyChest : public LocationModification {
private:
    inline static bool isCTMC = false;
    inline static bool raceMode = false;
    inline static std::unordered_map<std::string, Dungeon> dungeons = {};

    std::string filePath;
    std::vector<uint32_t> offsets;

    ModificationError setCTMCType(ACTR& chest, const Item& item);
public:

    ModifyChest() {}
    ~ModifyChest() override {}
    ModifyChest(const ModifyChest& other) = default;
    ModifyChest& operator=(const ModifyChest& other) = default;
    ModifyChest(ModifyChest&& other) = default;
    ModifyChest& operator=(ModifyChest&& other) = default;

    std::unique_ptr<LocationModification> duplicate() const override { return std::make_unique<ModifyChest>(*this); }
    ModificationError parseArgs(Yaml::Node& locationObject) override;
    ModificationError writeLocation(const Item& item) override;
    static void setCTMC(const bool& isCTMC_, const bool& raceMode_, const std::unordered_map<std::string, Dungeon>& dungeons_) { isCTMC = isCTMC_; raceMode = raceMode_; dungeons = dungeons_; }
};

class ModifyActor : public LocationModification {
private:
    std::string filePath;
    std::vector<uint32_t> offsets;

public:
    ModifyActor() {}
    ~ModifyActor() override {}
    ModifyActor(const ModifyActor& other) = default;
    ModifyActor& operator=(const ModifyActor& other) = default;
    ModifyActor(ModifyActor&& other) = default;
    ModifyActor& operator=(ModifyActor&& other) = default;

    std::unique_ptr<LocationModification> duplicate() const override { return std::make_unique<ModifyActor>(*this); }
    ModificationError parseArgs(Yaml::Node& locationObject) override;
    ModificationError writeLocation(const Item& item) override;
};

class ModifySCOB : public LocationModification {
private:
    std::string filePath;
    std::vector<uint32_t> offsets;

public:
    ModifySCOB() {}
    ~ModifySCOB() override {}
    ModifySCOB(const ModifySCOB& other) = default;
    ModifySCOB& operator=(const ModifySCOB& other) = default;
    ModifySCOB(ModifySCOB&& other) = default;
    ModifySCOB& operator=(ModifySCOB&& other) = default;

    std::unique_ptr<LocationModification> duplicate() const override { return std::make_unique<ModifySCOB>(*this); }
    ModificationError parseArgs(Yaml::Node& locationObject) override;
    ModificationError writeLocation(const Item& item) override;
};

class ModifyEvent : public LocationModification {
private:
    std::string filePath;
    uint32_t offset;
    uint32_t nameOffset;

public:
    ModifyEvent() {}
    ~ModifyEvent() override {}
    ModifyEvent(const ModifyEvent& other) = default;
    ModifyEvent& operator=(const ModifyEvent& other) = default;
    ModifyEvent(ModifyEvent&& other) = default;
    ModifyEvent& operator=(ModifyEvent&& other) = default;

    std::unique_ptr<LocationModification> duplicate() const override { return std::make_unique<ModifyEvent>(*this); }
    ModificationError parseArgs(Yaml::Node& locationObject) override;
    ModificationError writeLocation(const Item& item) override;
};

class ModifyRPX : public LocationModification {
private:
    std::vector<uint32_t> offsets;

public:
    ModifyRPX() {}
    ~ModifyRPX() override {}
    ModifyRPX(const ModifyRPX& other) = default;
    ModifyRPX& operator=(const ModifyRPX& other) = default;
    ModifyRPX(ModifyRPX&& other) = default;
    ModifyRPX& operator=(ModifyRPX&& other) = default;

    std::unique_ptr<LocationModification> duplicate() const override { return std::make_unique<ModifyRPX>(*this); }
    ModificationError parseArgs(Yaml::Node& locationObject) override;
    ModificationError writeLocation(const Item& item) override;
};

class ModifySymbol : public LocationModification {
private:
    std::vector<std::string> symbolNames;

public:
    ModifySymbol() {}
    ~ModifySymbol() override {}
    ModifySymbol(const ModifySymbol& other) = default;
    ModifySymbol& operator=(const ModifySymbol& other) = default;
    ModifySymbol(ModifySymbol&& other) = default;
    ModifySymbol& operator=(ModifySymbol&& other) = default;

    std::unique_ptr<LocationModification> duplicate() const override { return std::make_unique<ModifySymbol>(*this); }
    ModificationError parseArgs(Yaml::Node& locationObject) override;
    ModificationError writeLocation(const Item& item) override;
};

class ModifyBoss : public LocationModification {
private:
    std::vector<std::pair<std::string, uint32_t>> offsetsWithPath;

public:
    ModifyBoss() {}
    ~ModifyBoss() override {}
    ModifyBoss(const ModifyBoss& other) = default;
    ModifyBoss& operator=(const ModifyBoss& other) = default;
    ModifyBoss(ModifyBoss&& other) = default;
    ModifyBoss& operator=(ModifyBoss&& other) = default;

    std::unique_ptr<LocationModification> duplicate() const override { return std::make_unique<ModifyBoss>(*this); }
    ModificationError parseArgs(Yaml::Node& locationObject) override;
    ModificationError writeLocation(const Item& item) override;
};

const char* modErrorToName(ModificationError err);
