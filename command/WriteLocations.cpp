#include "WriteLocations.hpp"

#include <limits>
#include <type_traits>
#include <unordered_map>
#include <algorithm>

#include <utility/file.hpp>
#include <utility/platform.hpp>
#include <filetypes/util/elfUtil.hpp>
#include <command/RandoSession.hpp>
#include <command/WWHDStructs.hpp>
#include <command/Log.hpp>
#include <logic/Dungeon.hpp>
#include <logic/Location.hpp>
#include <logic/World.hpp>

#include <gui/desktop/update_dialog_header.hpp>

using namespace std::literals::string_literals;

static const std::unordered_map<std::string, uint32_t> item_id_mask_by_actor_name = {
    {"Bitem\0\0\0"s, 0x0000FF00},
    {"Ritem\0\0\0"s, 0x000000FF},
    {"Salvage\0"s, 0x00000FF0},
    {"SwSlvg\0\0"s, 0x00000FF0},
    {"Salvag2\0"s, 0x00000FF0},
    {"SalvagN\0"s, 0x00000FF0},
    {"SalvagE\0"s, 0x00000FF0},
    {"SalvFM\0\0"s, 0x00000FF0},
    {"TagKb\0\0\0"s, 0x000000FF},
    {"item\0\0\0\0"s, 0x000000FF},
    {"itemFLY\0"s, 0x000000FF}
};

static const std::unordered_set<std::string> pot_actor_names = {
    "kotubo\0\0"s,
    "ootubo1\0"s,
    "Kmtub\0\0\0"s,
    "Ktaru\0\0\0"s,
    "Ostool\0\0"s,
    "Odokuro\0"s,
    "Okioke\0\0"s,
    "Kmi02\0\0\0"s,
    "Ptubo\0\0\0"s,
    "KkibaB\0\0"s,
    "Kmi00\0\0\0"s,
    "Hbox2S\0\0"s
};

static const std::unordered_set<std::string> stone_head_actor_names = {
    "Homen1\0\0"s,
    "Homen2\0\0"s
};



namespace {
    static std::unordered_map<std::string, uint32_t> custom_symbols;

    void Load_Custom_Symbols(const fspath& file_path) {
        YAML::Node symbols;
        if(!LoadYAML(symbols, file_path, true)) {
            ErrorLog::getInstance().log("ERROR: Failed to load custom symbols when saving items");
            return;
        }

        for (const auto& symbol : symbols) {
            custom_symbols[symbol.first.as<std::string>()] = symbol.second.as<uint32_t>();
        }

        return;
    }
}


constexpr uint8_t getLowestSetBit(uint32_t mask) {
    uint8_t lowestSetIndex = 0xFF;
    for (uint8_t bitIndex = 0; bitIndex < 32; bitIndex++) {
        if (mask & (1 << bitIndex)) {
            lowestSetIndex = bitIndex;
            break;
        }
    }

    return lowestSetIndex;
}

template<typename T> requires std::is_arithmetic_v<T>
ModificationError setParam(ACTR& actor, const uint32_t& mask, T value) {
    const uint8_t shiftAmount = getLowestSetBit(mask);
    if (shiftAmount == 0xFF) LOG_ERR_AND_RETURN(ModificationError::INVALID_MASK)

    actor.params = (actor.params & (~mask)) | (static_cast<uint32_t>(value << shiftAmount) & mask);
    return ModificationError::NONE;
}



ModificationError ModifyChest::parseArgs(const YAML::Node& locationObject) {
    if(!locationObject["Path"]) {
        LOG_ERR_AND_RETURN(ModificationError::MISSING_KEY);
    }
    filePath = locationObject["Path"].as<std::string>();

    if(!locationObject["Offsets"].IsSequence()) {
        LOG_ERR_AND_RETURN(ModificationError::MISSING_KEY);
    }
    for (const auto& offset : locationObject["Offsets"])
    {
        offsets.push_back(offset.as<uint32_t>());
    }

    return ModificationError::NONE;
}

ModificationError ModifyChest::writeLocation(const Item& item) {
    RandoSession::CacheEntry& file = g_session.openGameFile(filePath);
    file.addAction([this, item](RandoSession* session, FileType* data) -> int {
        for (const uint32_t& offset : this->offsets) {
            CAST_ENTRY_TO_FILETYPE(generic, RawFile, data)
            std::stringstream& stream = generic.data;

            stream.seekg(offset, std::ios::beg);
            ACTR chest = WWHDStructs::readACTR(stream);

            if(isCTMC) LOG_AND_RETURN_BOOL_IF_ERR(setCTMCType(chest, item))

            chest.z_rot &= 0x00FF;
            chest.z_rot |= static_cast<uint16_t>(item.getGameItemId()) << 8;

            stream.seekp(offset, std::ios::beg);
            WWHDStructs::writeACTR(stream, chest);
        }

        return true;
    });
    

    return ModificationError::NONE;
}

ModificationError ModifyChest::setCTMCType(ACTR& chest, const Item& item) {
    if(item.isSmallKey() || item.isBigKey()) {
        if(item.getWorld()->getSettings().progression_dungeons == ProgressionDungeons::RaceMode) {
            // In race mode, only put keys for required dungeons in dark wood chests (the other keys go in light wood chests).
            // In some extreme entrance rando situations, traversing through unrequired dungeons may still
            // be required, so put small/big keys that unlock progression locations in dark wood chests also
            // TODO: put all keys for an unrequired dungeon in dark wood chests if you need to traverse through it, instead of
            // only the required ones. The current setup should match the old behavior, but it might make more sense
            // if all the keys are in the same type of chest.
            if(std::ranges::any_of(item.getWorld()->dungeons, [&item](const auto& dungeon){ return dungeon.second.isRequiredDungeon && (dungeon.second.smallKey == item || dungeon.second.bigKey == item); }) ||
              std::ranges::any_of(item.getChainLocations(), [](const auto& location){ return location->progression; })){
                LOG_AND_RETURN_IF_ERR(setParam(chest, 0x00F00000, uint8_t(1))) // Dark wood chest for Small and Big Keys
                return ModificationError::NONE;
            }

            LOG_AND_RETURN_IF_ERR(setParam(chest, 0x00F00000, uint8_t(0))) // Light wood chests for unrequired keys for empty dungeons
            return ModificationError::NONE;
        }

        LOG_AND_RETURN_IF_ERR(setParam(chest, 0x00F00000, uint8_t(1))) // Dark wood chest for Small and Big Keys
        return ModificationError::NONE;
    }
    else if(std::ranges::any_of(item.getChainLocations(), [](const auto& location){ return location->progression; })) {
        // If any of this item's chain locations are progression, then put the item in a spiky chest
        LOG_AND_RETURN_IF_ERR(setParam(chest, 0x00F00000, uint8_t(2))) // Metal chests for progress items (excluding keys)
        return ModificationError::NONE;
    }

    LOG_AND_RETURN_IF_ERR(setParam(chest, 0x00F00000, uint8_t(0))) // Light wood chests for non-progress items and consumables
    return ModificationError::NONE;
}


ModificationError ModifyActor::parseArgs(const YAML::Node& locationObject) {
    if(!locationObject["Path"]) {
        LOG_ERR_AND_RETURN(ModificationError::MISSING_KEY);
    }
    filePath = locationObject["Path"].as<std::string>();

    if(!locationObject["Offsets"].IsSequence()) {
        LOG_ERR_AND_RETURN(ModificationError::MISSING_KEY);
    }
    for (const auto& offset : locationObject["Offsets"])
    {
        offsets.push_back(offset.as<uint32_t>());
    }

    return ModificationError::NONE;
}

ModificationError ModifyActor::writeLocation(const Item& item) {
    RandoSession::CacheEntry& file = g_session.openGameFile(filePath);
    file.addAction([this, item](RandoSession* session, FileType* data) -> int {
        CAST_ENTRY_TO_FILETYPE(generic, RawFile, data)
        std::stringstream& stream = generic.data;

        for (const uint32_t& offset : this->offsets) {
            stream.seekg(offset, std::ios::beg);
            ACTR actor = WWHDStructs::readACTR(stream);

            if(pot_actor_names.contains(actor.name)) { // special case for pots since they have a custom param
                if(item.getGameItemId() == GameItem::HeartDrop) {
                    LOG_AND_RETURN_BOOL_IF_ERR(setParam(actor, 0x0000003F, 0x00));
                }
                else {
                    LOG_AND_RETURN_BOOL_IF_ERR(setParam(actor, 0x0000003F, 0x3F));
                }
                actor.z_rot = (actor.z_rot & ~0xFF00) | (static_cast<uint8_t>(item.getGameItemId()) << 8);
            }
            else if(stone_head_actor_names.contains(actor.name)) {
                if(item.getGameItemId() == GameItem::HeartDrop) {
                    LOG_AND_RETURN_BOOL_IF_ERR(setParam(actor, 0x0003F000, 0x00));
                }
                else {
                    LOG_AND_RETURN_BOOL_IF_ERR(setParam(actor, 0x0003F000, 0x3F));
                }
                actor.x_rot = (actor.x_rot & ~0x00FF) | static_cast<uint8_t>(item.getGameItemId());
            }
            else if (item_id_mask_by_actor_name.contains(actor.name)) {
                LOG_AND_RETURN_BOOL_IF_ERR(setParam(actor, item_id_mask_by_actor_name.at(actor.name), static_cast<uint8_t>(item.getGameItemId())))

                if((actor.name == "item\0\0\0\0"s || actor.name == "itemFLY\0"s) && (actor.params & 0x03000000) == 0) { // uses d_a_item actor, has behavior type 0
                    LOG_AND_RETURN_BOOL_IF_ERR(setParam(actor, 0x03000000, 3)); // set behavior type to 3 (doesn't fade out)
                }
            }
            else {
                LOG_ERR_AND_RETURN_BOOL(ModificationError::UNKNOWN_ACTOR_NAME)
            }

            stream.seekp(offset, std::ios::beg);
            WWHDStructs::writeACTR(stream, actor);
        }

        return true;
    });

    return ModificationError::NONE;
}


ModificationError ModifySCOB::parseArgs(const YAML::Node& locationObject) {
    if(!locationObject["Path"]) {
        LOG_ERR_AND_RETURN(ModificationError::MISSING_KEY);
    }
    filePath = locationObject["Path"].as<std::string>();

    if(!locationObject["Offsets"].IsSequence()) {
        LOG_ERR_AND_RETURN(ModificationError::MISSING_KEY);
    }
    for (const auto& offset : locationObject["Offsets"])
    {
        offsets.push_back(offset.as<uint32_t>());
    }

    return ModificationError::NONE;
}

ModificationError ModifySCOB::writeLocation(const Item& item) {
    RandoSession::CacheEntry& file = g_session.openGameFile(filePath);
    file.addAction([this, item](RandoSession* session, FileType* data) -> int {
        CAST_ENTRY_TO_FILETYPE(generic, RawFile, data)
        std::stringstream& stream = generic.data;

        for (const uint32_t& offset : this->offsets) {
            stream.seekg(offset, std::ios::beg);
            SCOB scob = WWHDStructs::readSCOB(stream);

            if (item_id_mask_by_actor_name.count(scob.actr.name) == 0) {
                LOG_ERR_AND_RETURN_BOOL(ModificationError::UNKNOWN_ACTOR_NAME)
            }
            LOG_AND_RETURN_BOOL_IF_ERR(setParam(scob.actr, item_id_mask_by_actor_name.at(scob.actr.name), static_cast<uint8_t>(item.getGameItemId())))

            stream.seekp(offset, std::ios::beg);
            WWHDStructs::writeSCOB(stream, scob);
        }

        return true;
    });

    return ModificationError::NONE;
}


ModificationError ModifyEvent::parseArgs(const YAML::Node& locationObject) {
    if(!locationObject["Path"]) {
        LOG_ERR_AND_RETURN(ModificationError::MISSING_KEY);
    }
    filePath = locationObject["Path"].as<std::string>();

    if(!locationObject["Offset"]) {
        LOG_ERR_AND_RETURN(ModificationError::MISSING_KEY);
    }
    this->offset = locationObject["Offset"].as<uint32_t>();
    
    if(!locationObject["NameOffset"]) {
        LOG_ERR_AND_RETURN(ModificationError::MISSING_KEY);
    }
    this->nameOffset = locationObject["NameOffset"].as<uint32_t>();

    return ModificationError::NONE;
}

ModificationError ModifyEvent::writeLocation(const Item& item) {
    RandoSession::CacheEntry& file = g_session.openGameFile(filePath);
    file.addAction([this, item](RandoSession* session, FileType* data) -> int {
        CAST_ENTRY_TO_FILETYPE(generic, RawFile, data)
        std::stringstream& stream = generic.data;
        
        uint8_t itemID = static_cast<uint8_t>(item.getGameItemId());
    
        std::string name = "011get_item";
        if (0x6D <= itemID && itemID <= 0x72) {
            name = "059get_dance";
            itemID -= 0x6D;
        }
        name.resize(0x20);

        stream.seekp(nameOffset, std::ios::beg);
        stream.write(&name[0], 0x20);

        stream.seekp(offset, std::ios::beg);
        stream.write(reinterpret_cast<const char*>(&itemID), 1);

        return true;
    });

    return ModificationError::NONE;
}


ModificationError ModifyRPX::parseArgs(const YAML::Node& locationObject) {
    if(!locationObject["Offsets"].IsSequence()) {
        LOG_ERR_AND_RETURN(ModificationError::MISSING_KEY);
    }
    for (const auto& offset : locationObject["Offsets"])
    {
        offsets.push_back(offset.as<uint32_t>());
    }

    return ModificationError::NONE;
}

ModificationError ModifyRPX::writeLocation(const Item& item) {
    uint8_t itemID = static_cast<uint8_t>(item.getGameItemId());

    RandoSession::CacheEntry& file = g_session.openGameFile("code/cking.rpx@RPX@ELF");
    file.addAction([this, itemID](RandoSession* session, FileType* data) -> int {
        CAST_ENTRY_TO_FILETYPE(elf, FileTypes::ELF, data);
        
        for (const uint32_t& address : this->offsets) {
            if(const auto& err = elfUtil::write_u8(elf, elfUtil::AddressToOffset(elf, address), itemID); err != ELFError::NONE) {
                LOG_ERR_AND_RETURN_BOOL(ModificationError::RPX_ERROR);
            }
        }

        return true;
    });

    return ModificationError::NONE;
}


ModificationError ModifySymbol::parseArgs(const YAML::Node& locationObject) {
    if(!locationObject["SymbolNames"].IsSequence()) {
        LOG_ERR_AND_RETURN(ModificationError::MISSING_KEY);
    }
    for (const auto& symbol : locationObject["SymbolNames"])
    {
        symbolNames.push_back(symbol.as<std::string>());
    }

    return ModificationError::NONE;
}

ModificationError ModifySymbol::writeLocation(const Item& item) {
    if (custom_symbols.size() == 0) Load_Custom_Symbols(Utility::get_data_path() / "asm/custom_symbols.yaml");

    for(const auto& symbol : symbolNames) {
        uint32_t address;
        uint8_t itemID = static_cast<uint8_t>(item.getGameItemId());

        if (symbol[0] == '@') { // support hardcoding addresses for checks like zunari (where a symbol and address are needed)
            const std::string offsetStr = symbol.substr(1);
            address = std::strtoul(offsetStr.c_str(), nullptr, 0);
            if (address == 0 || address == std::numeric_limits<decltype(address)>::max())
            {
                LOG_ERR_AND_RETURN(ModificationError::INVALID_OFFSET)
            }
        }
        else {
            if (custom_symbols.count(symbol) == 0) LOG_ERR_AND_RETURN(ModificationError::INVALID_SYMBOL);
            address = custom_symbols.at(symbol);
        }

        RandoSession::CacheEntry& file = g_session.openGameFile("code/cking.rpx@RPX@ELF");
        file.addAction([address, itemID](RandoSession* session, FileType* data) -> int {
            CAST_ENTRY_TO_FILETYPE(elf, FileTypes::ELF, data);

            if(const auto& err = elfUtil::write_u8(elf, elfUtil::AddressToOffset(elf, address), itemID); err != ELFError::NONE) {
                LOG_ERR_AND_RETURN_BOOL(ModificationError::RPX_ERROR);
            }

            return true;
        });
    }
    return ModificationError::NONE;
}


ModificationError ModifyBoss::parseArgs(const YAML::Node& locationObject) {
    if(!locationObject["Paths"].IsMap()) {
        LOG_ERR_AND_RETURN(ModificationError::MISSING_KEY);
    }

    offsetsWithPath.reserve(locationObject["Paths"].size());
    for (auto path_pair : locationObject["Paths"])
    {
        offsetsWithPath.emplace_back(path_pair.first.as<std::string>(), path_pair.second.as<uint32_t>());
    }

    return ModificationError::NONE;
}

ModificationError ModifyBoss::writeLocation(const Item& item) {
    for (const auto& [path, offset] : offsetsWithPath) {
        uint8_t itemID = static_cast<uint8_t>(item.getGameItemId());

        if (path == "code/cking.rpx@RPX@ELF") {
            RandoSession::CacheEntry& rpx = g_session.openGameFile("code/cking.rpx@RPX@ELF");
            rpx.addAction([offset = offset, itemID](RandoSession* session, FileType* data) -> int {
                CAST_ENTRY_TO_FILETYPE(elf, FileTypes::ELF, data)

                if(const auto& err = elfUtil::write_u8(elf, elfUtil::AddressToOffset(elf, offset), itemID); err != ELFError::NONE) {
                    LOG_ERR_AND_RETURN_BOOL(ModificationError::RPX_ERROR);
                }

                return true;
            });

            continue;
        }
        else {
            RandoSession::CacheEntry& file = g_session.openGameFile(path);
            file.addAction([offset = offset, itemID](RandoSession* session, FileType* data) -> int {
                CAST_ENTRY_TO_FILETYPE(generic, RawFile, data)
                std::stringstream& stream = generic.data;
                
                stream.seekg(offset, std::ios::beg);
                ACTR actor = WWHDStructs::readACTR(stream);

                LOG_AND_RETURN_BOOL_IF_ERR(setParam(actor, item_id_mask_by_actor_name.at(actor.name), itemID))

                stream.seekp(offset, std::ios::beg);
                WWHDStructs::writeACTR(stream, actor);

                return true;
            });
        }
    }

    return ModificationError::NONE;
}

std::string modErrorToName(ModificationError err) {
    switch (err) {
        case ModificationError::NONE:
            return "NONE";
        case ModificationError::MISSING_KEY:
            return "MISSING_KEY";
        case ModificationError::INVALID_OFFSET:
            return "INVALID_OFFSET";
        case ModificationError::MISSING_VALUE:
            return "MISSING_VALUE";
        case ModificationError::UNKNOWN_ACTOR_NAME:
            return "UNKNOWN_ACTOR_NAME";
        case ModificationError::INVALID_MASK:
            return "INVALID_MASK";
        case ModificationError::INVALID_SYMBOL:
            return "INVALID_SYMBOL";
        case ModificationError::RPX_ERROR:
            return "RPX_ERROR";
        default:
            return "UNKNOWN";
    }
}

bool writeLocations(WorldPool& worlds) {
    Utility::platformLog("Saving items...");
    UPDATE_DIALOG_VALUE(40);
    UPDATE_DIALOG_LABEL("Saving items...");

    ModifyChest::setCTMC(worlds[0].getSettings().chest_type_matches_contents);

    for (auto& [name, location] : worlds[0].locationTable) {
        if (const ModificationError err = location->method->writeLocation(location->currentItem); err != ModificationError::NONE) {
            ErrorLog::getInstance().log("Encountered ModificationError::" + modErrorToName(err) + " while saving location " + location->getName());
            return false;
        }
    }

    return true;
}
