#include "WriteLocations.hpp"

#include <limits>
#include <unordered_map>
#include <fstream>
#include <filesystem>
#include <algorithm>

#include <libs/json.hpp>

#include <logic/Dungeon.hpp>
#include <command/RandoSession.hpp>
#include <command/WWHDStructs.hpp>
#include <command/Log.hpp>
#include <filetypes/util/elfUtil.hpp>

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



namespace {
	static std::unordered_map<std::string, uint32_t> custom_symbols;

    void Load_Custom_Symbols(const std::string& file_path) {
		std::ifstream fptr(file_path, std::ios::in);

		nlohmann::json symbols = nlohmann::json::parse(fptr);
		for (const auto& symbol : symbols.items()) {
			uint32_t address = std::stoul(symbol.value().get<std::string>(), nullptr, 16);
			custom_symbols[symbol.key()] = address;
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

template<typename T>
ModificationError setParam(ACTR& actor, const uint32_t& mask, T value) {
    const uint8_t shiftAmount = getLowestSetBit(mask);
    if (shiftAmount == 0xFF) LOG_ERR_AND_RETURN(ModificationError::INVALID_MASK)

    actor.params = (actor.params & (~mask)) | (static_cast<uint32_t>(value << shiftAmount) & mask);
    return ModificationError::NONE;
}



ModificationError ModifyChest::parseArgs(Yaml::Node& locationObject) {
    const auto& path = locationObject["Path"].As<std::string>();
    filePath = std::string(path.data(), path.size());

    for (auto offsetIt = locationObject["Offsets"].Begin(); offsetIt != locationObject["Offsets"].End(); offsetIt++)
    {
        Yaml::Node offset = (*offsetIt).second;
        const auto& offsetStr = offset.As<std::string>();
        unsigned long offsetValue = std::strtoul(offsetStr.c_str(), nullptr, 0);
        if (offsetValue == 0 || offsetValue == std::numeric_limits<unsigned long>::max())
        {
            LOG_ERR_AND_RETURN(ModificationError::INVALID_OFFSET)
        }
        offsets.push_back(offsetValue);
    }

    return ModificationError::NONE;
}

ModificationError ModifyChest::writeLocation(const Item& item) {
    RandoSession::CacheEntry& file = g_session.openGameFile(filePath);
    file.addAction([this, item](RandoSession* session, FileType* data) -> int {
        for (const uint32_t& offset : this->offsets) {
            CAST_ENTRY_TO_FILETYPE(generic, GenericFile, data)
            std::stringstream& stream = generic.data;

            stream.seekg(offset, std::ios::beg);
            ACTR chest = WWHDStructs::readACTR(stream);

            if(isCTMC) LOG_AND_RETURN_BOOL_IF_ERR(setCTMCType(chest, item))

            chest.aux_params_2 &= 0x00FF;
            chest.aux_params_2 |= static_cast<uint16_t>(item.getGameItemId()) << 8;

            stream.seekp(offset, std::ios::beg);
            WWHDStructs::writeACTR(stream, chest);
        }

        return true;
    });
    

    return ModificationError::NONE;
}

ModificationError ModifyChest::setCTMCType(ACTR& chest, const Item& item) {
    if(item.isMajorItem()) {
        LOG_AND_RETURN_IF_ERR(setParam(chest, 0x00F00000, uint8_t(2))) // Metal chests for progress items (excluding keys)
        return ModificationError::NONE;
    }

    if(gameItemToName(item.getGameItemId()).ends_with("Key")) {
        // In race mode, only put the dungeon keys for required dungeons in dark wood chests.
        // The other keys go into light wood chests.
        if(raceMode) {
            if(std::any_of(dungeons.begin(), dungeons.end(), [&item](auto& dungeon){return dungeon.second.isRaceModeDungeon && (dungeon.second.smallKey == item.getName() || dungeon.second.bigKey == item.getName());})){
                LOG_AND_RETURN_IF_ERR(setParam(chest, 0x00F00000, uint8_t(1))) // Dark wood chest for Small and Big Keys
                return ModificationError::NONE;
            }
            else {
                LOG_AND_RETURN_IF_ERR(setParam(chest, 0x00F00000, uint8_t(0))) // Light wood chests for keys for empty dungeons (in race mode)
                return ModificationError::NONE;
            }
        }

        LOG_AND_RETURN_IF_ERR(setParam(chest, 0x00F00000, uint8_t(1))) // Dark wood chest for Small and Big Keys
        return ModificationError::NONE;
    }

    LOG_AND_RETURN_IF_ERR(setParam(chest, 0x00F00000, uint8_t(0))) // Light wood chests for non-progress items and consumables
    return ModificationError::NONE;
}


ModificationError ModifyActor::parseArgs(Yaml::Node& locationObject) {
    const auto& path = locationObject["Path"].As<std::string>();
    filePath = std::string(path.data(), path.size());

    for (auto offsetIt = locationObject["Offsets"].Begin(); offsetIt != locationObject["Offsets"].End(); offsetIt++)
    {
        Yaml::Node offset = (*offsetIt).second;
        const auto& offsetStr = offset.As<std::string>();
        unsigned long offsetValue = std::strtoul(offsetStr.c_str(), nullptr, 0);
        if (offsetValue == 0 || offsetValue == std::numeric_limits<unsigned long>::max()) {
          LOG_ERR_AND_RETURN(ModificationError::INVALID_OFFSET)
        }
        offsets.push_back(offsetValue);
    }

    return ModificationError::NONE;
}

ModificationError ModifyActor::writeLocation(const Item& item) {
    RandoSession::CacheEntry& file = g_session.openGameFile(filePath);
    file.addAction([this, item](RandoSession* session, FileType* data) -> int {
        CAST_ENTRY_TO_FILETYPE(generic, GenericFile, data)
        std::stringstream& stream = generic.data;

        for (const uint32_t& offset : this->offsets) {
            stream.seekg(offset, std::ios::beg);
            ACTR actor = WWHDStructs::readACTR(stream);

            if (item_id_mask_by_actor_name.count(actor.name) == 0) {
                LOG_ERR_AND_RETURN_BOOL(ModificationError::UNKNOWN_ACTOR_NAME)
            }
            LOG_AND_RETURN_BOOL_IF_ERR(setParam(actor, item_id_mask_by_actor_name.at(actor.name), static_cast<uint8_t>(item.getGameItemId())))

            stream.seekp(offset, std::ios::beg);
            WWHDStructs::writeACTR(stream, actor);
        }

        return true;
    });

    return ModificationError::NONE;
}


ModificationError ModifySCOB::parseArgs(Yaml::Node& locationObject) {
    const auto& path = locationObject["Path"].As<std::string>();
    filePath = std::string(path.data(), path.size());

    for (auto offsetIt = locationObject["Offsets"].Begin(); offsetIt != locationObject["Offsets"].End(); offsetIt++)
    {
        Yaml::Node offset = (*offsetIt).second;
        const auto& offsetStr = offset.As<std::string>();
        unsigned long offsetValue = std::strtoul(offsetStr.c_str(), nullptr, 0);
        if (offsetValue == 0 || offsetValue == std::numeric_limits<unsigned long>::max())
        {
            LOG_ERR_AND_RETURN(ModificationError::INVALID_OFFSET)
        }
        offsets.push_back(offsetValue);
    }

    return ModificationError::NONE;
}

ModificationError ModifySCOB::writeLocation(const Item& item) {
    RandoSession::CacheEntry& file = g_session.openGameFile(filePath);
    file.addAction([this, item](RandoSession* session, FileType* data) -> int {
        CAST_ENTRY_TO_FILETYPE(generic, GenericFile, data)
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


ModificationError ModifyEvent::parseArgs(Yaml::Node& locationObject) {
    const auto& path = locationObject["Path"].As<std::string>();
    filePath = std::string(path.data(), path.size());

    std::string offsetStr(locationObject["Offset"].As<std::string>());
    unsigned long offsetValue = std::strtoul(offsetStr.c_str(), nullptr, 0);
    if (offsetValue == 0 || offsetValue == std::numeric_limits<unsigned long>::max())
    {
        LOG_ERR_AND_RETURN(ModificationError::INVALID_OFFSET)
    }
    this->offset = offsetValue;

    offsetStr = locationObject["NameOffset"].As<std::string>();
    offsetValue = std::strtoul(offsetStr.c_str(), nullptr, 0);
    if (offsetValue == 0 || offsetValue == std::numeric_limits<unsigned long>::max())
    {
        LOG_ERR_AND_RETURN(ModificationError::INVALID_OFFSET)
    }
    this->nameOffset = offsetValue;

    return ModificationError::NONE;
}

ModificationError ModifyEvent::writeLocation(const Item& item) {
    RandoSession::CacheEntry& file = g_session.openGameFile(filePath);
    file.addAction([this, item](RandoSession* session, FileType* data) -> int {
        CAST_ENTRY_TO_FILETYPE(generic, GenericFile, data)
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


ModificationError ModifyRPX::parseArgs(Yaml::Node& locationObject) {
    for (auto offsetIt = locationObject["Offsets"].Begin(); offsetIt != locationObject["Offsets"].End(); offsetIt++)
    {
        Yaml::Node offset = (*offsetIt).second;
        const auto& offsetStr = offset.As<std::string>();
        unsigned long offsetValue = std::strtoul(offsetStr.c_str(), nullptr, 0);
        if (offsetValue == 0 || offsetValue == std::numeric_limits<unsigned long>::max())
        {
            LOG_ERR_AND_RETURN(ModificationError::INVALID_OFFSET)
        }
        offsets.push_back(offsetValue);
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


ModificationError ModifySymbol::parseArgs(Yaml::Node& locationObject) {
    for (auto it = locationObject["SymbolNames"].Begin(); it != locationObject["SymbolNames"].End(); it++)
    {
        Yaml::Node symbol = (*it).second;
        symbolNames.push_back(symbol.As<std::string>());
    }

    return ModificationError::NONE;
}

ModificationError ModifySymbol::writeLocation(const Item& item) {
    if (custom_symbols.size() == 0) Load_Custom_Symbols(DATA_PATH "asm/custom_symbols.json");

    for(const auto& symbol : symbolNames) {
        uint32_t address;
        uint8_t itemID = static_cast<uint8_t>(item.getGameItemId());

        if (symbol[0] == '@') { //support hardcoding addresses for checks like zunari (where a symbol and address are needed)
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


ModificationError ModifyBoss::parseArgs(Yaml::Node& locationObject) {
    if(locationObject["Paths"].Size() != locationObject["Offsets"].Size()) LOG_ERR_AND_RETURN(ModificationError::MISSING_VALUE)

    offsetsWithPath.reserve(locationObject["Paths"].Size());

    for (size_t i = 0; i < locationObject["Paths"].Size(); i++)
    {
        std::pair<std::string, uint32_t> offset_with_path;

        std::string offsetStr(locationObject["Offsets"][i].As<std::string>());
        unsigned long offsetValue = std::strtoul(offsetStr.c_str(), nullptr, 0);
        if (offsetValue == 0 || offsetValue == std::numeric_limits<unsigned long>::max())
        {
            LOG_ERR_AND_RETURN(ModificationError::INVALID_OFFSET)
        }
        offset_with_path.first = locationObject["Paths"][i].As<std::string>();
        offset_with_path.second = offsetValue;

        offsetsWithPath.push_back(offset_with_path);
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
                CAST_ENTRY_TO_FILETYPE(generic, GenericFile, data)
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



const char* modErrorToName(ModificationError err) {
    switch (err) {
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
