#include "WriteLocations.hpp"

#include <unordered_map>
#include <fstream>

#include "WWHDStructs.hpp"
#include "../filetypes/elf.hpp"
#include "../filetypes/util/elfUtil.hpp"
#include "../../libs/ryml.hpp"

#define FIELD_CHECK(j, field, err) if(!j.contains(field)) {lastError << "Unable to retrieve field: \"" << field << '"'; return err;}
#define YAML_FIELD_CHECK(ref, key, err) if(!ref.has_child(key)) {lastError << "Unable to find key: \"" << key << '"'; return err;}
#define OBJECT_CHECK(j, msg) if(!j.is_object()) {lastError << msg << ": Not an Object."; return WorldLoadingError::EXPECTED_JSON_OBJECT;}
#define MAPPING_CHECK(str1, str2) if (str1 != str2) {lastError << "\"" << str1 << "\" does not equal" << std::endl << "\"" << str2 << "\""; return WorldLoadingError::MAPPING_MISMATCH;}
#define VALID_CHECK(e, invalid, msg, err) if(e == invalid) {lastError << "\t" << msg; return err;}



static const std::unordered_map<std::string, uint32_t> item_id_mask_by_actor_name = {
    {std::string("Bitem\0\0\0", 8), 0x0000FF00},
    {std::string("Ritem\0\0\0", 8), 0x000000FF},
    {std::string("STBox\0\0\0", 8), 0x00000FF0},
    {std::string("Salvage\0", 8), 0x00000FF0},
    {std::string("SwSlvg\0\0", 8), 0x00000FF0},
    {std::string("Salvag2\0", 8), 0x00000FF0},
    {std::string("SalvagN\0", 8), 0x00000FF0},
    {std::string("SalvagE\0", 8), 0x00000FF0},
    {std::string("SalvFM\0\0", 8), 0x00000FF0},
    {std::string("TagKb\0\0\0", 8), 0x000000FF},
    {std::string("item\0\0\0\0", 8), 0x000000FF},
    {std::string("itemFLY\0", 8), 0x000000FF}
};



namespace {
    bool rpxOpen = false;
    FileTypes::ELF gRPX;
	static std::unordered_map<std::string, uint32_t> custom_symbols;

    void Load_Custom_Symbols(const std::string& file_path) {
		std::ifstream fptr(file_path, std::ios::in);

		nlohmann::json symbols = nlohmann::json::parse(fptr);
		for (const auto& symbol : symbols.items()) {
			uint32_t address = std::stoi((std::string)symbol.value(), nullptr, 16);
			custom_symbols[symbol.key()] = address;
		}

		return;
	}
}


void loadRPX() {
    RandoSession::fspath filePath = g_session.openGameFile("code/cking.rpx@RPX");
    gRPX.loadFromFile(filePath.string());
    rpxOpen = true;
    
    return;
}

void saveRPX() {
    RandoSession::fspath filePath = g_session.openGameFile("code/cking.rpx@RPX");
    gRPX.writeToFile(filePath.string());
    rpxOpen = false;

    return;
}


uint8_t getLowestSetBit(uint32_t mask) {
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
    uint8_t shiftAmount = getLowestSetBit(mask);
    if (shiftAmount == 0xFF) return ModificationError::INVALID_MASK;

    actor.params = (actor.params & (~mask)) | ((value << shiftAmount) & mask);
    return ModificationError::NONE;
}



ModificationError ModifyChest::parseArgs(const ryml::NodeRef& locationObject) {
    YAML_FIELD_CHECK(locationObject, "Path", ModificationError::MISSING_KEY);
    const auto& path = locationObject["Path"].val();
    filePath = std::string(path.data(), path.size());

	YAML_FIELD_CHECK(locationObject, "Offsets", ModificationError::MISSING_KEY);
    for (const ryml::NodeRef& offset : locationObject["Offsets"].children())
    {
        const auto& offsetStr = std::string(offset.val().data(), offset.val().size());
        unsigned long offsetValue = std::strtoul(offsetStr.c_str(), nullptr, 0);
        if (offsetValue == 0 || offsetValue == ULONG_MAX)
        {
            lastError << "Encountered an invalid offset for location " << locationName << " in world " << std::to_string(worldId + 1);
            return ModificationError::INVALID_OFFSET;
        }
        newEntry.method.offsets.push_back(offsetValue);
    }

    return ModificationError::NONE;
}

ModificationError ModifyChest::writeLocation(const Item& item) {
    RandoSession::fspath filePath = g_session.openGameFile(this->filePath);
    std::fstream file(filePath, std::ios::in | std::ios::out | std::ios::binary);

    for (const uint32_t& offset : offsets) {
        file.seekg(offset, std::ios::beg);
        ACTR chest = WWHDStructs::readACTR(file);

        chest.aux_params_2 &= 0x00FF;
        chest.aux_params_2 |= static_cast<uint16_t>(item.getGameItemId()) << 8;

        file.seekp(offset, std::ios::beg);
        WWHDStructs::writeACTR(file, chest);
    }

    return ModificationError::NONE;
}


ModificationError ModifyActor::parseArgs(const ryml::NodeRef& locationObject) {
    FIELD_CHECK(locationObject, "Path", ModificationError::MISSING_KEY);
    filePath = locationObject.at("Path").get<std::string>();
    FIELD_CHECK(locationObject, "Offsets", ModificationError::MISSING_KEY);
    const auto& offsets_j = locationObject.at("Offsets").get<std::vector<nlohmann::json>>();
    for (const auto& offset_j : offsets_j)
    {
        if (offset_j.is_number())
        {
            offsets.push_back(offset_j.get<uint32_t>());
        }
        else
        {
            uint32_t offsetValue = std::stoi(offset_j.get<std::string>().c_str(), nullptr, 16);
            if (offsetValue == 0 || offsetValue == UINT32_MAX)
            {
                return ModificationError::INVALID_OFFSET;
            }
            offsets.push_back(offsetValue);
        }
    }

    return ModificationError::NONE;
}

ModificationError ModifyActor::writeLocation(const Item& item) {
    RandoSession::fspath filePath = g_session.openGameFile(this->filePath);
    std::fstream file(filePath, std::ios::in | std::ios::out | std::ios::binary);

    for (const uint32_t& offset : offsets) {
        file.seekg(offset, std::ios::beg);
        ACTR actor = WWHDStructs::readACTR(file);

        if (item_id_mask_by_actor_name.count(actor.name) == 0) {
            continue;
        }
        if (ModificationError err = setParam(actor, item_id_mask_by_actor_name.at(actor.name), static_cast<uint8_t>(item.getGameItemId())); err != ModificationError::NONE) return err;

        file.seekp(offset, std::ios::beg);
        WWHDStructs::writeACTR(file, actor);
    }

    return ModificationError::NONE;
}


ModificationError ModifySCOB::parseArgs(const ryml::NodeRef& locationObject) {
    FIELD_CHECK(locationObject, "Path", ModificationError::MISSING_KEY);
    filePath = locationObject.at("Path").get<std::string>();
    FIELD_CHECK(locationObject, "Offsets", ModificationError::MISSING_KEY);
    const auto& offsets_j = locationObject.at("Offsets").get<std::vector<nlohmann::json>>();
    for (const auto& offset_j : offsets_j)
    {
        if (offset_j.is_number())
        {
            offsets.push_back(offset_j.get<uint32_t>());
        }
        else
        {
            uint32_t offsetValue = std::stoi(offset_j.get<std::string>().c_str(), nullptr, 16);
            if (offsetValue == 0 || offsetValue == UINT32_MAX)
            {
                return ModificationError::INVALID_OFFSET;
            }
            offsets.push_back(offsetValue);
        }
    }

    return ModificationError::NONE;
}

ModificationError ModifySCOB::writeLocation(const Item& item) {
    RandoSession::fspath filePath = g_session.openGameFile(this->filePath);
    std::fstream file(filePath, std::ios::in | std::ios::out | std::ios::binary);

    for (const uint32_t& offset : offsets) {
        file.seekg(offset, std::ios::beg);
        SCOB scob = WWHDStructs::readSCOB(file);

        if (item_id_mask_by_actor_name.count(scob.actr.name) == 0) {
            continue;
        }
        if (ModificationError err = setParam(scob.actr, item_id_mask_by_actor_name.at(scob.actr.name), static_cast<uint8_t>(item.getGameItemId())); err != ModificationError::NONE) return err;

        file.seekp(offset, std::ios::beg);
        WWHDStructs::writeSCOB(file, scob);
    }

    return ModificationError::NONE;
}


ModificationError ModifyEvent::parseArgs(const ryml::NodeRef& locationObject) {
    FIELD_CHECK(locationObject, "Path", ModificationError::MISSING_KEY);
    filePath = locationObject.at("Path").get<std::string>();
    FIELD_CHECK(locationObject, "Offset", ModificationError::MISSING_KEY);
    const std::string& offset_s = locationObject.at("Offset").get<std::string>();
    uint32_t offsetValue = std::stoi(offset_s.c_str(), nullptr, 16);
    if (offsetValue == 0 || offsetValue == UINT32_MAX)
    {
        return ModificationError::INVALID_OFFSET;
    }
    offset = offsetValue;

    FIELD_CHECK(locationObject, "NameOffset", ModificationError::MISSING_KEY);
    const std::string& nameOffset_s = locationObject.at("NameOffset").get<std::string>();
    offsetValue = std::stoi(nameOffset_s.c_str(), nullptr, 16);
    if (offsetValue == 0 || offsetValue == UINT32_MAX)
    {
        return ModificationError::INVALID_OFFSET;
    }
    nameOffset = offsetValue;

    return ModificationError::NONE;
}

ModificationError ModifyEvent::writeLocation(const Item& item) {
    RandoSession::fspath filePath = g_session.openGameFile(this->filePath);
    std::ofstream file(filePath, std::ios::out | std::ios::binary);

    uint8_t itemID = static_cast<uint8_t>(item.getGameItemId());

    std::string name = "011get_item";
    if (0x6D <= itemID && itemID <= 0x72) {
        name = "059get_dance";
        itemID -= 0x6D;
    }
    name.resize(0x20);

    file.seekp(nameOffset, std::ios::beg);
    file.write(&name[0], 0x20);

    file.seekp(offset, std::ios::beg);
    file.write(reinterpret_cast<char*>(&itemID), 1);

    return ModificationError::NONE;
}


ModificationError ModifyRPX::parseArgs(const ryml::NodeRef& locationObject) {
    FIELD_CHECK(locationObject, "Offsets", ModificationError::MISSING_KEY);
    const auto& offsets_j = locationObject.at("Offsets").get<std::vector<nlohmann::json>>();
    for (const auto& offset_j : offsets_j)
    {
        if (offset_j.is_number())
        {
            offsets.push_back(offset_j.get<uint32_t>());
        }
        else
        {
            uint32_t offsetValue = std::stoi(offset_j.get<std::string>().c_str(), nullptr, 16);
            if (offsetValue == 0 || offsetValue == UINT32_MAX)
            {
                return ModificationError::INVALID_OFFSET;
            }
            offsets.push_back(offsetValue);
        }
    }

    return ModificationError::NONE;
}

ModificationError ModifyRPX::writeLocation(const Item& item) {
    if (rpxOpen == false) loadRPX();

    uint8_t itemID = static_cast<uint8_t>(item.getGameItemId());
    for (const uint32_t& address : offsets) {
        elfUtil::write_u8(gRPX, elfUtil::AddressToOffset(gRPX, address), itemID);
    }

    return ModificationError::NONE;
}


ModificationError ModifySymbol::parseArgs(const ryml::NodeRef& locationObject) {
    FIELD_CHECK(locationObject, "SymbolName", ModificationError::MISSING_KEY);
    symbolName = locationObject.at("SymbolName").get<std::string>();

    return ModificationError::NONE;
}

ModificationError ModifySymbol::writeLocation(const Item& item) {
    if (rpxOpen == false) loadRPX();
    if (custom_symbols.size() == 0) Load_Custom_Symbols("./asm/custom_symbols.txt");
    if (custom_symbols.count(symbolName) == 0) return ModificationError::INVALID_SYMBOL;

    uint32_t address = custom_symbols.at(symbolName);
    uint8_t itemID = static_cast<uint8_t>(item.getGameItemId());

    elfUtil::write_u8(gRPX, elfUtil::AddressToOffset(gRPX, address), itemID);
    return ModificationError::NONE;
}


ModificationError ModifyBoss::parseArgs(const ryml::NodeRef& locationObject) {
    FIELD_CHECK(locationObject, "Paths", ModificationError::MISSING_KEY);
    FIELD_CHECK(locationObject, "Offsets", ModificationError::MISSING_KEY);
    const auto& paths_j = locationObject.at("Paths").get<std::vector<nlohmann::json>>();
    const auto& offsets_j = locationObject.at("Offsets").get<std::vector<nlohmann::json>>();
    if (paths_j.size() != offsets_j.size()) return ModificationError::MISSING_VALUE;

    offsetsWithPath.reserve(paths_j.size());
    for (size_t i = 0; i < paths_j.size(); i++)
    {
        std::pair<std::string, uint32_t> offset_with_path;
        if (offsets_j[i].is_number())
        {
            offset_with_path.second = offsets_j[i].get<uint32_t>();
        }
        else
        {
            uint32_t offsetValue = std::stoi(offsets_j[i].get<std::string>().c_str(), nullptr, 16);
            if (offsetValue == 0 || offsetValue == UINT32_MAX)
            {
                return ModificationError::INVALID_OFFSET;
            }
            offset_with_path.second = offsetValue;
        }

        offset_with_path.first = paths_j[i].get<std::string>();
        offsetsWithPath.push_back(offset_with_path);
    }

    return ModificationError::NONE;
}

ModificationError ModifyBoss::writeLocation(const Item& item) {
    if (rpxOpen == false) loadRPX();

    for (const auto& [path, offset] : offsetsWithPath) {
        RandoSession::fspath filePath = g_session.openGameFile(path);
        std::fstream file(filePath, std::ios::in | std::ios::out | std::ios::binary);

        uint8_t itemID = static_cast<uint8_t>(item.getGameItemId());

        if (path == "code/cking.rpx@RPX") {
            elfUtil::write_u8(gRPX, elfUtil::AddressToOffset(gRPX, offset), itemID);
            continue;
        }

        file.seekg(offset, std::ios::beg);
        ACTR actor = WWHDStructs::readACTR(file);

        if(ModificationError err = setParam(actor, item_id_mask_by_actor_name.at(actor.name), itemID); err != ModificationError::NONE) return err;

        file.seekp(offset, std::ios::beg);
        WWHDStructs::writeACTR(file, actor);
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
        case ModificationError::RPX_NOT_OPEN:
            return "RPX_NOT_OPEN";
        default:
            return "UNKNOWN";
    }
}
