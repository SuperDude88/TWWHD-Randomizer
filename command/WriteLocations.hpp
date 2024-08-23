#pragma once

#include <string>
#include <vector>
#include <memory>
#include <map>

#include <libs/yaml.hpp>

#include <command/RandoSession.hpp>
#include <command/WWHDStructs.hpp>
#include <logic/GameItem.hpp>
#include <logic/Dungeon.hpp>
#include <logic/WorldPool.hpp>

enum struct [[nodiscard]] ModificationError {
    NONE = 0,
    MISSING_KEY,
    INVALID_OFFSET,
    MISSING_VALUE,
    UNKNOWN_ACTOR_NAME,
    INVALID_MASK,
    INVALID_SYMBOL,
    RPX_ERROR,
    UNKNOWN,
    COUNT
};

class Location;
class LocationModification
{
public:
    LocationModification() = default;
    virtual ~LocationModification() = default;
    LocationModification(const LocationModification& other) = default;
    LocationModification& operator=(const LocationModification& other) = default;
    LocationModification(LocationModification&& other) = default;
    LocationModification& operator=(LocationModification&& other) = default;

    virtual std::unique_ptr<LocationModification> duplicate() const { return std::make_unique<LocationModification>(*this); }
    virtual ModificationError parseArgs(const YAML::Node& /* locationObject */) { return ModificationError::NONE; }
    virtual ModificationError writeLocation(const Item& /* item */) { return ModificationError::NONE; }
};

class ModifyChest final : public LocationModification {
private:
    inline static bool isCTMC = false;
    inline static bool raceMode = false;
    inline static std::map<std::string, Dungeon> dungeons = {};
    inline static std::list<Location*> playthroughLocations = {};

    fspath filePath;
    std::vector<uint32_t> offsets;

    ModificationError setCTMCType(ACTR& chest, const Item& item);
public:

    ModifyChest() = default;
    ~ModifyChest() override = default;
    ModifyChest(const ModifyChest& other) = default;
    ModifyChest& operator=(const ModifyChest& other) = default;
    ModifyChest(ModifyChest&& other) = default;
    ModifyChest& operator=(ModifyChest&& other) = default;

    std::unique_ptr<LocationModification> duplicate() const override { return std::make_unique<ModifyChest>(*this); }
    ModificationError parseArgs(const YAML::Node& locationObject) override;
    ModificationError writeLocation(const Item& item) override;
    static void setCTMC(const bool& isCTMC_, const bool& raceMode_, const std::map<std::string, Dungeon>& dungeons_, const std::list<Location*>& playthroughLocations_) { isCTMC = isCTMC_; raceMode = raceMode_; dungeons = dungeons_; playthroughLocations = playthroughLocations_;}
};

class ModifyActor final : public LocationModification {
private:
    fspath filePath;
    std::vector<uint32_t> offsets;

public:
    ModifyActor() = default;
    ~ModifyActor() override = default;
    ModifyActor(const ModifyActor& other) = default;
    ModifyActor& operator=(const ModifyActor& other) = default;
    ModifyActor(ModifyActor&& other) = default;
    ModifyActor& operator=(ModifyActor&& other) = default;

    std::unique_ptr<LocationModification> duplicate() const override { return std::make_unique<ModifyActor>(*this); }
    ModificationError parseArgs(const YAML::Node& locationObject) override;
    ModificationError writeLocation(const Item& item) override;
};

class ModifySCOB final : public LocationModification {
private:
    fspath filePath;
    std::vector<uint32_t> offsets;

public:
    ModifySCOB() = default;
    ~ModifySCOB() override = default;
    ModifySCOB(const ModifySCOB& other) = default;
    ModifySCOB& operator=(const ModifySCOB& other) = default;
    ModifySCOB(ModifySCOB&& other) = default;
    ModifySCOB& operator=(ModifySCOB&& other) = default;

    std::unique_ptr<LocationModification> duplicate() const override { return std::make_unique<ModifySCOB>(*this); }
    ModificationError parseArgs(const YAML::Node& locationObject) override;
    ModificationError writeLocation(const Item& item) override;
};

class ModifyEvent final : public LocationModification {
private:
    fspath filePath;
    uint32_t offset;
    uint32_t nameOffset;

public:
    ModifyEvent() = default;
    ~ModifyEvent() override = default;
    ModifyEvent(const ModifyEvent& other) = default;
    ModifyEvent& operator=(const ModifyEvent& other) = default;
    ModifyEvent(ModifyEvent&& other) = default;
    ModifyEvent& operator=(ModifyEvent&& other) = default;

    std::unique_ptr<LocationModification> duplicate() const override { return std::make_unique<ModifyEvent>(*this); }
    ModificationError parseArgs(const YAML::Node& locationObject) override;
    ModificationError writeLocation(const Item& item) override;
};

class ModifyRPX final : public LocationModification {
private:
    std::vector<uint32_t> offsets;

public:
    ModifyRPX() = default;
    ~ModifyRPX() override = default;
    ModifyRPX(const ModifyRPX& other) = default;
    ModifyRPX& operator=(const ModifyRPX& other) = default;
    ModifyRPX(ModifyRPX&& other) = default;
    ModifyRPX& operator=(ModifyRPX&& other) = default;

    std::unique_ptr<LocationModification> duplicate() const override { return std::make_unique<ModifyRPX>(*this); }
    ModificationError parseArgs(const YAML::Node& locationObject) override;
    ModificationError writeLocation(const Item& item) override;
};

class ModifySymbol final : public LocationModification {
private:
    std::vector<std::string> symbolNames;

public:
    ModifySymbol() = default;
    ~ModifySymbol() override = default;
    ModifySymbol(const ModifySymbol& other) = default;
    ModifySymbol& operator=(const ModifySymbol& other) = default;
    ModifySymbol(ModifySymbol&& other) = default;
    ModifySymbol& operator=(ModifySymbol&& other) = default;

    std::unique_ptr<LocationModification> duplicate() const override { return std::make_unique<ModifySymbol>(*this); }
    ModificationError parseArgs(const YAML::Node& locationObject) override;
    ModificationError writeLocation(const Item& item) override;
};

class ModifyBoss final : public LocationModification {
private:
    std::vector<std::pair<fspath, uint32_t>> offsetsWithPath;

public:
    ModifyBoss() = default;
    ~ModifyBoss() override = default;
    ModifyBoss(const ModifyBoss& other) = default;
    ModifyBoss& operator=(const ModifyBoss& other) = default;
    ModifyBoss(ModifyBoss&& other) = default;
    ModifyBoss& operator=(ModifyBoss&& other) = default;

    std::unique_ptr<LocationModification> duplicate() const override { return std::make_unique<ModifyBoss>(*this); }
    ModificationError parseArgs(const YAML::Node& locationObject) override;
    ModificationError writeLocation(const Item& item) override;
};

std::string modErrorToName(ModificationError err);

[[nodiscard]] bool writeLocations(WorldPool& worlds);
