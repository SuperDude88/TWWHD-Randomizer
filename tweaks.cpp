#include "tweaks.hpp"

#include <typeinfo>
#include <memory>
#include <algorithm>
#include <sstream>
#include <numbers>

#include <version.hpp>
#include <text_replacements.hpp>
#include <libs/tinyxml2.hpp>
#include <libs/yaml.hpp>
#include <asm/patches/asm_constants.hpp>
#include <filetypes/bflim.hpp>
#include <filetypes/bflyt.hpp>
#include <filetypes/bfres.hpp>
#include <filetypes/charts.hpp>
#include <filetypes/dzx.hpp>
#include <filetypes/elf.hpp>
#include <filetypes/util/elfUtil.hpp>
#include <filetypes/events.hpp>
#include <filetypes/jpc.hpp>
#include <filetypes/msbt.hpp>
#include <filetypes/util/msbtMacros.hpp>
#include <utility/color.hpp>
#include <utility/string.hpp>
#include <utility/file.hpp>
#include <command/Log.hpp>
#include <command/GamePath.hpp>
#include <command/RandoSession.hpp>

#define FILETYPE_ERROR_CHECK(func) {  \
    if(const auto error = func; error != decltype(error)::NONE) {\
        ErrorLog::getInstance().log(std::string("Encountered ") + &(typeid(error).name()[5]) + " on line " TOSTRING(__LINE__)); \
        return (int)TweakError::FILETYPE_ERROR;  \
    } \
}

#define RPX_ERROR_CHECK(func) { \
    if(const ELFError error = func; error != ELFError::NONE) {\
        ErrorLog::getInstance().log(std::string("Failed rpx operation on line ") + std::to_string(__LINE__)); \
        return (int)TweakError::RPX_OPERATION_FAILED;  \
    } \
}

#define TWEAK_ERR_CHECK(func) { \
    if(const TweakError error = func; error != TweakError::NONE) { \
        ErrorLog::getInstance().log(std::string("Encountered error in tweak ") + #func); \
        return error;  \
    } \
}

using eType = Utility::Endian::Type;


static std::unordered_map<std::string, uint32_t> custom_symbols;

static TweakError Load_Custom_Symbols(const fspath& file_path) {
    std::string file_data;
    if(Utility::getFileContents(file_path, file_data, true)) LOG_ERR_AND_RETURN(TweakError::DATA_FILE_MISSING);

    YAML::Node symbols = YAML::Load(file_data);
    for (const auto& symbol : symbols) {
        custom_symbols[symbol.first.as<std::string>()] = symbol.second.as<uint32_t>();
    }

    return TweakError::NONE;
}

static TweakError Apply_Patch(const fspath& file_path) {
    std::string file_data;
    if (Utility::getFileContents(file_path, file_data, true)) LOG_ERR_AND_RETURN(TweakError::DATA_FILE_MISSING);

    const YAML::Node patches = YAML::Load(file_data);
    RandoSession::CacheEntry& entry = g_session.openGameFile("code/cking.rpx@RPX@ELF");

    if(!patches["Data"] && !patches["Relocations"]) return TweakError::PATCH_MISSING_KEY;

    if(patches["Data"]) {
        entry.addAction([patches](RandoSession* session, FileType* data) -> int {
            CAST_ENTRY_TO_FILETYPE(elf, FileTypes::ELF, data)
            for (const auto& patch : patches["Data"]) {
                const uint32_t offset = patch.first.as<uint32_t>();
                offset_t sectionOffset = elfUtil::AddressToOffset(elf, offset);
                if (!sectionOffset) { //address not in section
                    std::string data;
                    for (const uint8_t& byte : patch.second.as<std::vector<uint8_t>>()) {
                        data += byte;
                    }
                    RPX_ERROR_CHECK(elf.extend_section(2, offset, data)); //add data at the specified offset
                }
                else {
                    for (const uint8_t& byte : patch.second.as<std::vector<uint8_t>>()) {
                        RPX_ERROR_CHECK(elfUtil::write_u8(elf, sectionOffset, byte));
                        sectionOffset.offset++; //Cycles through the bytes individually, need to increase the offset by one each time
                    }
                }
            }

            return true;
        });
    }
    
    if(patches["Relocations"]) {
        for (const auto& relocation : patches["Relocations"]) {
            if(!relocation["r_offset"].IsScalar() || !relocation["r_info"].IsScalar() || !relocation["r_addend"].IsScalar()) {
                LOG_ERR_AND_RETURN(TweakError::RELOCATION_MISSING_KEY);
            }

            Elf32_Rela reloc;
            reloc.r_offset = relocation["r_offset"].as<uint32_t>();
            reloc.r_info = relocation["r_info"].as<uint32_t>();
            reloc.r_addend = relocation["r_addend"].as<uint32_t>();

            entry.addAction([reloc](RandoSession* session, FileType* data) -> int {
                CAST_ENTRY_TO_FILETYPE(elf, FileTypes::ELF, data)

                if(reloc.r_offset >= 0x10000000) {
                    if(reloc.r_offset >= 0x1018C0C0) {   
                        RPX_ERROR_CHECK(elfUtil::addRelocation(elf, 9, reloc)); //in the .data section, go in .rela.data
                    }
                    else {
                        RPX_ERROR_CHECK(elfUtil::addRelocation(elf, 8, reloc)); //in the .rodata section, go in .rela.rodata
                    }
                }
                else {
                    RPX_ERROR_CHECK(elfUtil::addRelocation(elf, 7, reloc)); //in the .text section, go in .rela.text
                }

                return true;
            });
        }
    }

    return TweakError::NONE;
}



TweakError set_new_game_starting_location(const uint8_t spawn_id, const uint8_t room_index) {
    RandoSession::CacheEntry& entry = g_session.openGameFile("code/cking.rpx@RPX@ELF");
    entry.addAction([spawn_id, room_index](RandoSession* session, FileType* data) -> int {
        CAST_ENTRY_TO_FILETYPE(elf, FileTypes::ELF, data)
        
        RPX_ERROR_CHECK(elfUtil::write_u8(elf, elfUtil::AddressToOffset(elf, 0x025B508F), room_index));
        RPX_ERROR_CHECK(elfUtil::write_u8(elf, elfUtil::AddressToOffset(elf, 0x025B50CB), room_index));
        RPX_ERROR_CHECK(elfUtil::write_u8(elf, elfUtil::AddressToOffset(elf, 0x025B5093), spawn_id));
        RPX_ERROR_CHECK(elfUtil::write_u8(elf, elfUtil::AddressToOffset(elf, 0x025B50CF), spawn_id));

        return true;
    });
    return TweakError::NONE;
}

TweakError change_ship_starting_island(const uint8_t room_num) {
    const fspath path = getRoomDzrPath("sea", room_num);

    RandoSession::CacheEntry& room = g_session.openGameFile(path);
    RandoSession::CacheEntry& stage = g_session.openGameFile("content/Common/Pack/first_szs_permanent.pack@SARC@sea_Stage.szs@YAZ0@SARC@Stage.bfres@BFRES@stage.dzs@DZX");
    room.addAction([&stage](RandoSession* session, FileType* data) -> int {
        CAST_ENTRY_TO_FILETYPE(room_dzr, FileTypes::DZXFile, data)
        
        std::vector<ChunkEntry*> ship_spawns = room_dzr.entries_by_type("SHIP");
        ChunkEntry* ship_spawn_0 = nullptr;
        for (ChunkEntry* spawn : ship_spawns) { //Find spawn with ID 0
            if (*reinterpret_cast<uint8_t*>(&spawn->data[0xE]) == 0) ship_spawn_0 = spawn;
        }
        if(ship_spawn_0 == nullptr) LOG_ERR_AND_RETURN_BOOL(TweakError::MISSING_ENTITY);

        stage.addAction([ship_spawn_0 = *ship_spawn_0](RandoSession* session, FileType* data) -> int {
            CAST_ENTRY_TO_FILETYPE(stage_dzs, FileTypes::DZXFile, data)

            std::vector<ChunkEntry*> actors = stage_dzs.entries_by_type("ACTR");
            for (ChunkEntry* actor : actors) {
                if (std::strncmp(&actor->data[0], "Ship", 4) == 0) {
                    actor->data.replace(0xC, 0xC, ship_spawn_0.data, 0x0, 0xC);
                    actor->data.replace(0x1A, 0x2, ship_spawn_0.data, 0xC, 0x2);
                    actor->data.replace(0x10, 0x4, "\xC8\xF4\x24\x00"s, 0x0, 0x4); //prevent softlock on fire mountain (may be wrong offset)
                    break;
                }
            }
            
            return true;
        });

        return true;
    });
    room.addDependent(stage.getRoot());

    return TweakError::NONE;
}

TweakError make_all_text_instant() {
    for (const auto& language : Text::supported_languages) {
        const fspath paths[4] = {
        "content/Common/Pack/permanent_2d_Us" + language + ".pack@SARC@message_msbt.szs@YAZ0@SARC@message.msbt@MSBT",
        "content/Common/Pack/permanent_2d_Us" + language + ".pack@SARC@message2_msbt.szs@YAZ0@SARC@message2.msbt@MSBT",
        "content/Common/Pack/permanent_2d_Us" + language + ".pack@SARC@message3_msbt.szs@YAZ0@SARC@message3.msbt@MSBT",
        "content/Common/Pack/permanent_2d_Us" + language + ".pack@SARC@message4_msbt.szs@YAZ0@SARC@message4.msbt@MSBT"
        };

        for (const auto& path : paths) {
            RandoSession::CacheEntry& entry = g_session.openGameFile(path);
            entry.addAction([](RandoSession* session, FileType* data) -> int {
                CAST_ENTRY_TO_FILETYPE(msbt, FileTypes::MSBTFile, data);

                for (auto& [label, message] : msbt.messages_by_label) {
                    std::u16string& String = message.text.message;

                    message.attributes.drawType = 1; //draw instant

                    std::u16string::size_type wait = String.find(u"\x0e\x01\x06\x02"s); //dont use macro because duration shouldnt matter
                    while (wait != std::u16string::npos) {
                        String.erase(wait, 5);
                        wait = String.find(u"\x0e\x01\x06\x02"s);
                    }

                    // Most of these waits are in cutscenes like the credits
                    // Removing them forces you to advance the text yourself which is undesirable
                    // So only apply it to a few instances
                    static const std::unordered_set<std::string> wait_dismiss_to_remove = {
                        // Healing Grandma text
                        "02034",
                        "02039",
                        "02040",
                        // Tingle's text
                        "03522",
                        "04316",
                        // Beedle's text
                        "03956",
                        // Hollo's text
                        "05162",
                        // Auction text
                        "07416",
                        // Killer Bees' text
                        "09935",
                        // Dampa's text
                        "12920",
                        // Flight Control Platform text
                        "10930",
                    };

                    if(wait_dismiss_to_remove.contains(label)) {
                        std::u16string::size_type wait_dismiss = String.find(u"\x0e\x01\x03\x02"s); //dont use macro because duration shouldnt matter
                        while (wait_dismiss != std::u16string::npos) {
                            String.erase(wait_dismiss, 5);
                            wait_dismiss = String.find(u"\x0e\x01\x03\x02"s);
                        }
                    }

                    std::u16string::size_type wait_dismiss_prompt = String.find(u"\x0e\x01\x02\x02"s); //dont use macro because duration shouldnt matter
                    while (wait_dismiss_prompt != std::u16string::npos) {
                        String.erase(wait_dismiss_prompt, 5);
                        wait_dismiss_prompt = String.find(u"\x0e\x01\x02\x02"s);
                    }
                }
            
                return true;
            });
        }
    }

    return TweakError::NONE;
}

TweakError fix_deku_leaf_model() {
    RandoSession::CacheEntry& entry = g_session.openGameFile("content/Common/Stage/Omori_Room0.szs@YAZ0@SARC@Room0.bfres@BFRES@room.dzr");
    entry.addAction([](RandoSession* session, FileType* data) -> int 
    {
        CAST_ENTRY_TO_FILETYPE(generic, RawFile, data) //do this on the stream so it happens before location mod

        FileTypes::DZXFile dzr;
        LOG_AND_RETURN_BOOL_IF_ERR(dzr.loadFromBinary(generic.data));

        const std::vector<ChunkEntry*> actors = dzr.entries_by_type("ACTR");
        for (ChunkEntry* actor : actors) {
            if (std::strncmp(&actor->data[0], "itemDek\x00", 8) == 0) actor->data = "item\x00\x00\x00\x00\x01\xFF\x02\x34\xc4\x08\x7d\x81\x45\x9d\x59\xec\xc3\xf5\x8e\xd9\x00\x00\x00\x00\x00\xff\xff\xff"s;
        }

        LOG_AND_RETURN_BOOL_IF_ERR(dzr.writeToStream(generic.data));

        return true;
    });

    return TweakError::NONE;
}

TweakError allow_all_items_to_be_field_items() {

    static constexpr uint32_t item_resources_list_start = 0x101E4674;
    static constexpr uint32_t field_item_resources_list_start = 0x101E6A74;

    const std::array<uint8_t, 172> Item_Ids_Without_Field_Model = {
    0x1a, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f, 0x30, 0x31, 0x32, 0x33, 0x35, 0x36, 0x39, 0x3a, 0x3c, 0x3e, 0x3f, 0x42, 0x43, 0x4c, 0x4d, 0x4e, 0x50, 0x51, 0x52,
    0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f, 0x70, 0x71, 0x72, 0x77, 0x78, 0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0x7f, 0x80, 0x98,
    0x99, 0x9a, 0x9b, 0x9c, 0x9d, 0x9e, 0x9f, 0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf, 0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xbb, 0xbc, 0xbd,
    0xbe, 0xbf, 0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf, 0xd0, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda, 0xdb, 0xdc, 0xde, 0xdd, 0xdf, 0xe0, 0xe1, 0xe2,
    0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea, 0xeb, 0xec, 0xed, 0xee, 0xef, 0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe
    };

    const std::unordered_map<uint8_t, uint32_t> szs_name_pointers{
        {0x1a, 0x1004e5d8}, {0x20, 0x1004e414}, {0x21, 0x1004ec54}, {0x22, 0x1004e578}, {0x23, 0x1004e4a8}, {0x24, 0x1004e4d0}, {0x25, 0x1004e548}, {0x26, 0x1004e658}, {0x27, 0x1004e730}, {0x28, 0x1004e4f0}, {0x29, 0x1004e498}, {0x2a, 0x1004e550}, {0x2b, 0x1004e4a0}, {0x2c, 0x1004e4d8}, {0x2d, 0x1004e6b0}, {0x2e, 0x1004e5c0}, {0x2f, 0x1004e4e8}, {0x30, 0x1004e4c8}, {0x31, 0x1004e41c}, {0x32, 0x1004e5c0},
        {0x33, 0x1004e510}, {0x35, 0x1004e580}, {0x36, 0x1004e590}, {0x38, 0x1004e558}, {0x3c, 0x1004e560}, {0x3f, 0x1004e440}, {0x42, 0x1004e518}, {0x43, 0x1004e520}, {0x4c, 0x1004e4b8}, {0x4d, 0x1004e4b0}, {0x4e, 0x1004e698}, {0x50, 0x1004e430}, {0x51, 0x1004e538}, {0x52, 0x1004e530}, {0x53, 0x1004e528}, {0x54, 0x1004e5b0}, {0x55, 0x1004e5b0}, {0x56, 0x1004e5b8}, {0x57, 0x1004e5a0}, {0x58, 0x1004e5a8},
        {0x59, 0x1004e598}, {0x61, 0x1004e570}, {0x62, 0x1004e600}, {0x63, 0x1004e608}, {0x64, 0x1004e610}, {0x65, 0x1004e618}, {0x66, 0x1004e620}, {0x67, 0x1004e628}, {0x68, 0x1004e630}, {0x69, 0x1004ec24}, {0x6a, 0x1004ec3c}, {0x6b, 0x1004ec48}, {0x6c, 0x1004e518}, {0x6d, 0x1004e518}, {0x6e, 0x1004e518}, {0x6f, 0x1004e518}, {0x70, 0x1004e518}, {0x71, 0x1004e518}, {0x72, 0x1004e518}, {0x77, 0x1004e434},
        {0x78, 0x1004e434}, {0x79, 0x1004e638}, {0x7a, 0x1004e638}, {0x7b, 0x1004e638}, {0x7c, 0x1004e638}, {0x7d, 0x1004e638}, {0x7e, 0x1004e638}, {0x7f, 0x1004e638}, {0x80, 0x1004e638}, {0x98, 0x1004e5e0}, {0x99, 0x1004e5e8}, {0x9a, 0x1004e5f0}, {0x9b, 0x1004e5f8}, {0x9c, 0x1004e688}, {0x9d, 0x1004e500}, {0x9e, 0x1004e4f8}, {0x9f, 0x1004e658}, {0xa0, 0x1004e518}, {0xa1, 0x1004e518}, {0xa2, 0x1004e518},
        {0xa3, 0x1004e660}, {0xa4, 0x1004e668}, {0xa5, 0x1004e670}, {0xa6, 0x1004e678}, {0xa7, 0x1004e680}, {0xab, 0x1004e470}, {0xac, 0x1004e478}, {0xad, 0x1004e490}, {0xae, 0x1004e4a0}, {0xaf, 0x1004e480}, {0xb0, 0x1004e488}, {0xb3, 0x1004e5d8}, {0xb4, 0x1004e5d8}, {0xb5, 0x1004e5d8}, {0xb6, 0x1004e5d8}, {0xb7, 0x1004e5d8}, {0xb8, 0x1004e5d8}, {0xb9, 0x1004e5d8}, {0xba, 0x1004e5d8}, {0xbb, 0x1004e5d8},
        {0xbc, 0x1004e5d8}, {0xbd, 0x1004e5d8}, {0xbe, 0x1004e5d8}, {0xbf, 0x1004e5d8}, {0xc0, 0x1004e5d8}, {0xc1, 0x1004e5d8}, {0xc2, 0x1004e588}, {0xc3, 0x1004e588}, {0xc4, 0x1004e588}, {0xc5, 0x1004e588}, {0xc6, 0x1004e588}, {0xc7, 0x1004e588}, {0xc8, 0x1004e588}, {0xc9, 0x1004e588}, {0xca, 0x1004e588}, {0xcb, 0x1004e468}, {0xcc, 0x1004e640}, {0xcd, 0x1004e640}, {0xce, 0x1004e640}, {0xcf, 0x1004e640},
        {0xd0, 0x1004e640}, {0xd1, 0x1004e640}, {0xd2, 0x1004e640}, {0xd3, 0x1004e640}, {0xd4, 0x1004e640}, {0xd5, 0x1004e640}, {0xd6, 0x1004e640}, {0xd7, 0x1004e640}, {0xd8, 0x1004e640}, {0xd9, 0x1004e640}, {0xda, 0x1004e640}, {0xdb, 0x1004e650}, {0xdc, 0x1004e468}, {0xdd, 0x1004e640}, {0xde, 0x1004e640}, {0xdf, 0x1004e640}, {0xe0, 0x1004e640}, {0xe1, 0x1004e640}, {0xe2, 0x1004e640}, {0xe3, 0x1004e640},
        {0xe4, 0x1004e640}, {0xe5, 0x1004e640}, {0xe6, 0x1004e640}, {0xe7, 0x1004e640}, {0xe8, 0x1004e640}, {0xe9, 0x1004e640}, {0xea, 0x1004e640}, {0xeb, 0x1004e640}, {0xec, 0x1004e640}, {0xed, 0x1004e648}, {0xee, 0x1004e648}, {0xef, 0x1004e648}, {0xf0, 0x1004e648}, {0xf1, 0x1004e648}, {0xf2, 0x1004e648}, {0xf3, 0x1004e648}, {0xf4, 0x1004e648}, {0xf5, 0x1004e648}, {0xf6, 0x1004e648}, {0xf7, 0x1004e648},
        {0xf8, 0x1004e648}, {0xf9, 0x1004e648}, {0xfa, 0x1004e638}, {0xfb, 0x1004e648}, {0xfc, 0x1004e638}, {0xfd, 0x1004e648}, {0xfe, 0x1004e638}
    };

    RandoSession::CacheEntry& rpx = g_session.openGameFile("code/cking.rpx@RPX@ELF");

    for (const uint8_t& item_id : Item_Ids_Without_Field_Model) {
        uint32_t item_resources_addr_to_fix = 0x0;
        uint8_t item_id_to_copy_from = 0x0;
        if (item_id == 0x39 || item_id == 0x3a || item_id == 0x3e) {
            item_id_to_copy_from = 0x38;
            item_resources_addr_to_fix = item_resources_list_start + item_id * 0x24;
        }
        else if (item_id >= 0x6d && item_id <= 0x72) {
            item_id_to_copy_from = 0x22;
            item_resources_addr_to_fix = item_resources_list_start + item_id * 0x24;
        }
        else if (item_id == 0xb1 || item_id == 0xb2) {
            item_id_to_copy_from = 0x52;
            item_resources_addr_to_fix = item_resources_list_start + item_id * 0x24;
        }
        else {
            item_id_to_copy_from = item_id;
        }

        const uint32_t item_resources_addr_to_copy_from = item_resources_list_start + item_id_to_copy_from * 0x24;
        const uint32_t field_item_resources_addr = field_item_resources_list_start + item_id * 0x1c;
        uint32_t szs_name_pointer = 0;
        uint32_t section_start = 0x10000000;

        if (item_id == 0xAA) {
            //szs_name_pointer = custom_symbols.at("hurricane_spin_item_resource_arc_name");
            szs_name_pointer = szs_name_pointers.at(0x38); //issues with custom .szs currently, use sword model instead
            item_resources_addr_to_fix = item_resources_list_start + item_id * 0x24;


            //section_start = 0x02000000; //custom stuff only gets put in .text
            //not needed because we use the sword model (would be for a custom szs)
        }
        else {
            szs_name_pointer = szs_name_pointers.at(item_id_to_copy_from);
        }

        rpx.addAction([=](RandoSession* session, FileType* data) -> int {
            CAST_ENTRY_TO_FILETYPE(elf, FileTypes::ELF, data)
            RPX_ERROR_CHECK(elfUtil::write_u32(elf, elfUtil::AddressToOffset(elf, field_item_resources_addr), szs_name_pointer));
        
            return true;
        });
        if (item_resources_addr_to_fix) {
            rpx.addAction([=](RandoSession* session, FileType* data) -> int {
                CAST_ENTRY_TO_FILETYPE(elf, FileTypes::ELF, data)
                RPX_ERROR_CHECK(elfUtil::write_u32(elf, elfUtil::AddressToOffset(elf, item_resources_addr_to_fix), szs_name_pointer));
            
                return true;
            });
        }

        //need relocation entries so pointers work on console
        Elf32_Rela relocation;
        relocation.r_offset = field_item_resources_addr;
        if (section_start == 0x02000000) {
            relocation.r_info = 0x00000101; //need different .symtab index for .text pointer
        }
        else {
            relocation.r_info = 0x00000201;
        }
        relocation.r_addend = szs_name_pointer - section_start; //needs offset into the .rodata section (.text for vscroll), subtract start address from data location

        rpx.addAction([=](RandoSession* session, FileType* data) -> int {
            CAST_ENTRY_TO_FILETYPE(elf, FileTypes::ELF, data)
            RPX_ERROR_CHECK(elfUtil::addRelocation(elf, 9, relocation));

            return true;
        });

        if (item_resources_addr_to_fix) {
            Elf32_Rela relocation2;
            relocation2.r_offset = item_resources_addr_to_fix;
            relocation2.r_info = relocation.r_info; //same as first entry
            relocation2.r_addend = relocation.r_addend; //same as first entry

            rpx.addAction([=](RandoSession* session, FileType* data) -> int {
                CAST_ENTRY_TO_FILETYPE(elf, FileTypes::ELF, data)
                RPX_ERROR_CHECK(elfUtil::addRelocation(elf, 9, relocation2));

                return true;
            });
        }

        rpx.addAction([=](RandoSession* session, FileType* data) -> int {
            CAST_ENTRY_TO_FILETYPE(elf, FileTypes::ELF, data)
            
            const std::vector<uint8_t> data1 = elfUtil::read_bytes(elf, elfUtil::AddressToOffset(elf, item_resources_addr_to_copy_from + 8), 0xD);
            const std::vector<uint8_t> data2 = elfUtil::read_bytes(elf, elfUtil::AddressToOffset(elf, item_resources_addr_to_copy_from + 0x1C), 4);
            RPX_ERROR_CHECK(elfUtil::write_bytes(elf, elfUtil::AddressToOffset(elf, field_item_resources_addr + 4), data1));
            RPX_ERROR_CHECK(elfUtil::write_bytes(elf, elfUtil::AddressToOffset(elf, field_item_resources_addr + 0x14), data2));
            if (item_resources_addr_to_fix) {
                RPX_ERROR_CHECK(elfUtil::write_bytes(elf, elfUtil::AddressToOffset(elf, item_resources_addr_to_fix + 8), data1));
                RPX_ERROR_CHECK(elfUtil::write_bytes(elf, elfUtil::AddressToOffset(elf, item_resources_addr_to_fix + 0x1C) , data2));
            }
        
            return true;
        });
    }

    rpx.addAction([=](RandoSession* session, FileType* data) -> int {
        CAST_ENTRY_TO_FILETYPE(elf, FileTypes::ELF, data)
            
        for (const uint32_t& address : { 0x0255220CU, 0x02552214U, 0x0255221CU, 0x02552224U, 0x0255222CU, 0x02552234U, 0x02552450U }) { //unsigned to make compiler happy
            RPX_ERROR_CHECK(elfUtil::write_u32(elf, elfUtil::AddressToOffset(elf, address), 0x60000000));
        }

        return true;
    });

    //execItemGet, mode_wait, and getYOffset had their switch cases optimized out, so their patches are a little more involved in HD
    LOG_AND_RETURN_IF_ERR(Apply_Patch(Utility::get_data_path() / "asm/patch_diffs/field_items_diff.yaml")); 
    
    const uint32_t item_info_list_start = 0x101E8674;
    for (unsigned int item_id = 0x00; item_id < 0xFF + 1; item_id++) {
        const uint32_t item_info_entry_addr = item_info_list_start + 4 * item_id;

        rpx.addAction([=](RandoSession* session, FileType* data) -> int {
            CAST_ENTRY_TO_FILETYPE(elf, FileTypes::ELF, data)

            const uint8_t original_y_offset = elfUtil::read_u8(elf, elfUtil::AddressToOffset(elf, item_info_entry_addr + 1));
            if (original_y_offset == 0) {
                RPX_ERROR_CHECK(elfUtil::write_u8(elf, elfUtil::AddressToOffset(elf, item_info_entry_addr + 1), 0x28));
            }
            uint8_t original_radius = elfUtil::read_u8(elf, elfUtil::AddressToOffset(elf, item_info_entry_addr + 2));
            if (original_radius == 0) {
                RPX_ERROR_CHECK(elfUtil::write_u8(elf, elfUtil::AddressToOffset(elf, item_info_entry_addr + 2), 0x28));
            }

            return true;
        });
    }
    //IMPROVEMENT: make vscroll not crash + add it to code
    //WWHD changes make this much more complex ^

    return TweakError::NONE;
}

TweakError remove_shop_item_forced_uniqueness_bit() {
    const uint32_t shop_item_data_list_start = 0x101eaea4;
    RandoSession::CacheEntry& rpx = g_session.openGameFile("code/cking.rpx@RPX@ELF");

    for (const uint8_t shop_item_index : { 0x0, 0xB, 0xC, 0xD }) {
        const uint32_t shop_item_data_addr = shop_item_data_list_start + shop_item_index * 0x10;
        
        rpx.addAction([=](RandoSession* session, FileType* data) -> int {
            CAST_ENTRY_TO_FILETYPE(elf, FileTypes::ELF, data)

            uint8_t buy_requirements_bitfield = elfUtil::read_u8(elf, elfUtil::AddressToOffset(elf, shop_item_data_addr + 0xC));
            buy_requirements_bitfield = (buy_requirements_bitfield & (~0x2));
            RPX_ERROR_CHECK(elfUtil::write_u8(elf, elfUtil::AddressToOffset(elf, shop_item_data_addr + 0xC), buy_requirements_bitfield));
        
            return true;
        });
    }

    return TweakError::NONE;
}

TweakError remove_ff2_cutscenes(const bool& randomize_boss_entrances) {
    RandoSession::CacheEntry& entry = g_session.openGameFile("content/Common/Stage/M2tower_Room0.szs@YAZ0@SARC@Room0.bfres@BFRES@room.dzr@DZX");

    entry.addAction([&](RandoSession* session, FileType* data) -> int {
        CAST_ENTRY_TO_FILETYPE(dzr, FileTypes::DZXFile, data)
        
        std::vector<ChunkEntry*> spawns = dzr.entries_by_type("PLYR");
        for (ChunkEntry* spawn : spawns) {
            if (spawn->data[29] == '\x10') {
                spawn->data[8] = '\xFF';
                break;
            }
        }

        if (!randomize_boss_entrances) {
            std::vector<ChunkEntry*> exits = dzr.entries_by_type("SCLS");
            for (ChunkEntry* exit : exits) {
                if (std::strncmp(&exit->data[0], "M2ganon\x00", 8) == 0) {
                    exit->data = "sea\x00\x00\x00\x00\x00\x00\x01\x00\xFF"s;
                    break;
                }
            }
        }

        return true;
    });

    return TweakError::NONE;
}

TweakError make_items_progressive() {
    LOG_AND_RETURN_IF_ERR(Apply_Patch(Utility::get_data_path() / "asm/patch_diffs/make_items_progressive_diff.yaml"));

    const uint32_t item_get_func_pointer = 0x0001DA54; //First relevant relocation entry in .rela.data (overwrites .data section when loaded)

    RandoSession::CacheEntry& rpx = g_session.openGameFile("code/cking.rpx@RPX@ELF");
    rpx.addAction([](RandoSession* session, FileType* data) -> int {
        CAST_ENTRY_TO_FILETYPE(elf, FileTypes::ELF, data);

        for (const uint8_t sword_id : {0x38, 0x39, 0x3A, 0x3D, 0x3E}) {
            const uint32_t item_get_func_addr = item_get_func_pointer + (sword_id * 0xC) + 8;
            RPX_ERROR_CHECK(elfUtil::write_u32(elf, elfUtil::AddressToOffset(elf, item_get_func_addr, 9), custom_symbols.at("progressive_sword_item_func") - 0x02000000));
        }
        for (const uint8_t shield_id : {0x3B, 0x3C}) {
            const uint32_t item_get_func_addr = item_get_func_pointer + (shield_id * 0xC) + 8;
            RPX_ERROR_CHECK(elfUtil::write_u32(elf, elfUtil::AddressToOffset(elf, item_get_func_addr, 9), custom_symbols.at("progressive_shield_item_func") - 0x02000000));
        }
        for (const uint8_t bow_id : {0x27, 0x35, 0x36}) {
            const uint32_t item_get_func_addr = item_get_func_pointer + (bow_id * 0xC) + 8;
            RPX_ERROR_CHECK(elfUtil::write_u32(elf, elfUtil::AddressToOffset(elf, item_get_func_addr, 9), custom_symbols.at("progressive_bow_item_func") - 0x02000000));
        }
        for (const uint8_t wallet_id : {0xAB, 0xAC}) {
            const uint32_t item_get_func_addr = item_get_func_pointer + (wallet_id * 0xC) + 8;
            RPX_ERROR_CHECK(elfUtil::write_u32(elf, elfUtil::AddressToOffset(elf, item_get_func_addr, 9), custom_symbols.at("progressive_wallet_item_func") - 0x02000000));
        }
        for (const uint8_t bomb_bag_id : {0xAD, 0xAE}) {
            const uint32_t item_get_func_addr = item_get_func_pointer + (bomb_bag_id * 0xC) + 8;
            RPX_ERROR_CHECK(elfUtil::write_u32(elf, elfUtil::AddressToOffset(elf, item_get_func_addr, 9), custom_symbols.at("progressive_bomb_bag_item_func") - 0x02000000));
        }
        for (const uint8_t quiver_id : {0xAF, 0xB0}) {
            const uint32_t item_get_func_addr = item_get_func_pointer + (quiver_id * 0xC) + 8;
            RPX_ERROR_CHECK(elfUtil::write_u32(elf, elfUtil::AddressToOffset(elf, item_get_func_addr, 9), custom_symbols.at("progressive_quiver_item_func") - 0x02000000));
        }
        for (const uint8_t picto_id : {0x23, 0x26}) {
            const uint32_t item_get_func_addr = item_get_func_pointer + (picto_id * 0xC) + 8;
            RPX_ERROR_CHECK(elfUtil::write_u32(elf, elfUtil::AddressToOffset(elf, item_get_func_addr, 9), custom_symbols.at("progressive_picto_box_item_func") - 0x02000000));
        }
        for (const uint8_t sail_id : {0x77, 0x78}) {
            const uint32_t item_get_func_addr = item_get_func_pointer + (sail_id * 0xC) + 8;
            RPX_ERROR_CHECK(elfUtil::write_u32(elf, elfUtil::AddressToOffset(elf, item_get_func_addr, 9), custom_symbols.at("progressive_sail_item_func") - 0x02000000));
        }
        for (const uint8_t magic_id : {0xB1, 0xB2}) {
            const uint32_t item_get_func_addr = item_get_func_pointer + (magic_id * 0xC) + 8;
            RPX_ERROR_CHECK(elfUtil::write_u32(elf, elfUtil::AddressToOffset(elf, item_get_func_addr, 9), custom_symbols.at("progressive_magic_meter_item_func") - 0x02000000));
        }

        //nop some code that sets max bombs and arrows to 30
        //This avoids downgrading bomb bags or quivers
        RPX_ERROR_CHECK(elfUtil::write_u32(elf, elfUtil::AddressToOffset(elf, 0x0254e8c4), 0x60000000));
        RPX_ERROR_CHECK(elfUtil::write_u32(elf, elfUtil::AddressToOffset(elf, 0x0254e8cc), 0x60000000));
        RPX_ERROR_CHECK(elfUtil::write_u32(elf, elfUtil::AddressToOffset(elf, 0x0254e66c), 0x60000000));
        RPX_ERROR_CHECK(elfUtil::write_u32(elf, elfUtil::AddressToOffset(elf, 0x0254e674), 0x60000000));

        //Modify the deku leaf to skip giving you magic
        //Instead we make magic its own item that the player starts with by default
        //Allows other items to use magic before getting leaf
        RPX_ERROR_CHECK(elfUtil::write_u32(elf, elfUtil::AddressToOffset(elf, 0x0254e96c), 0x60000000));
        RPX_ERROR_CHECK(elfUtil::write_u32(elf, elfUtil::AddressToOffset(elf, 0x0254e97c), 0x60000000));

        return true;
    });
    
    // Add an item get message for the normal magic meter since it didn't have one in vanilla
    std::unordered_map<std::string, std::u16string> messages = {
        {"English", DRAW_INSTANT + u"You got " + TEXT_COLOR_RED + u"magic power" + TEXT_COLOR_DEFAULT + u"!\nNow you can use magic items!\0"s},
        {"Spanish", DRAW_INSTANT + u"¡Has obtenido el " + TEXT_COLOR_RED + u"Poder Mágico" + TEXT_COLOR_DEFAULT + u"!\n¡Ahora podrás utilizar objetos mágicos!\0"s},
        {"French", DRAW_INSTANT + u"Vous obtenez l'" + TEXT_COLOR_RED + u"Energie Magique" + TEXT_COLOR_DEFAULT + u"!\n" + Text::word_wrap_string(u"Vous pouvez maintenant utiliser les objets magiques!\0"s, 43)},
    };

    for (const auto& language : Text::supported_languages) {
        RandoSession::CacheEntry& entry = g_session.openGameFile("content/Common/Pack/permanent_2d_Us" + language + ".pack@SARC@message_msbt.szs@YAZ0@SARC@message.msbt@MSBT");
    
        entry.addAction([messages, language](RandoSession* session, FileType* data) -> int {
            CAST_ENTRY_TO_FILETYPE(msbt, FileTypes::MSBTFile, data)

            const Message& to_copy = msbt.messages_by_label["00" + std::to_string(101 + 0xB2)];
            const std::u16string message = messages.at(language);
            msbt.addMessage("00" + std::to_string(101 + 0xB1), to_copy.attributes, to_copy.style, message);

            return true;
        });
    
    }

    return TweakError::NONE;
}

TweakError add_ganons_tower_warp_to_ff2() {
    RandoSession::CacheEntry& entry = g_session.openGameFile("content/Common/Pack/szs_permanent1.pack@SARC@sea_Room1.szs@YAZ0@SARC@Room1.bfres@BFRES@room.dzr@DZX");
    
    entry.addAction([](RandoSession* session, FileType* data) -> int {
        CAST_ENTRY_TO_FILETYPE(dzr, FileTypes::DZXFile, data)

        ChunkEntry& warp = dzr.add_entity("ACTR", 1);
        warp.data = "Warpmj\x00\x00\x00\x00\x00\x11\xc8\x93\x0f\xd9\x00\x00\x00\x00\xc8\x91\xf7\xfa\x00\x00\x00\x00\x00\x00\xff\xff"s;
        
        return 1;
    });

    return TweakError::NONE;
}

TweakError add_chest_in_place_medli_gift() {
    RandoSession::CacheEntry& stage = g_session.openGameFile("content/Common/Stage/M_Dra09_Stage.szs@YAZ0@SARC@Stage.bfres@BFRES@stage.dzs");
    stage.addAction([](RandoSession* session, FileType* data) -> int {
        CAST_ENTRY_TO_FILETYPE(generic, RawFile, data) //do this on the stream so it happens before location mod

        FileTypes::DZXFile dzs;
        LOG_AND_RETURN_BOOL_IF_ERR(dzs.loadFromBinary(generic.data));

        ChunkEntry& chest = dzs.add_entity("TRES");
        chest.data = "takara3\x00\xFF\x20\x08\x80\xc4\xca\x99\xec\x46\x54\x80\x00\x43\x83\x84\x5a\x00\x09\xcc\x16\x0f\xff\xff\xff"s;

        LOG_AND_RETURN_BOOL_IF_ERR(dzs.writeToStream(generic.data));
    
        return true;
    });

    RandoSession::CacheEntry& dungeon = g_session.openGameFile("content/Common/Stage/M_NewD2_Stage.szs@YAZ0@SARC@Stage.bfres@BFRES@stage.dzs@DZX");
    dungeon.addAction([](RandoSession* session, FileType* data) -> int {
        CAST_ENTRY_TO_FILETYPE(dzs, FileTypes::DZXFile, data)

        ChunkEntry& dummyChest = dzs.add_entity("TRES");
        dummyChest.data = "takara3\x00\xFF\x20\x08\x80\xc4\xca\x99\xec\x46\x54\x80\x00\x43\x83\x84\x5a\x00\x09\xcc\x16\x0f\xff\xff\xff"s;

        return true;
    });

    return TweakError::NONE;
}

TweakError add_chest_in_place_queen_fairy_cutscene() {
    RandoSession::CacheEntry& room = g_session.openGameFile("content/Common/Pack/szs_permanent2.pack@SARC@sea_Room9.szs@YAZ0@SARC@Room9.bfres@BFRES@room.dzr");
    room.addAction([](RandoSession* session, FileType* data) -> int {
        CAST_ENTRY_TO_FILETYPE(generic, RawFile, data) //do this on the stream so it happens before location mod

        FileTypes::DZXFile dzr;
        LOG_AND_RETURN_BOOL_IF_ERR(dzr.loadFromBinary(generic.data));

        ChunkEntry& chest = dzr.add_entity("TRES");
        chest.data = "takara3\x00\xFF\x20\x0e\x00\xc8\x2f\xcf\xc0\x44\x34\xc0\x00\xc8\x43\x4e\xc0\x00\x09\x10\x00\xa5\xff\xff\xff"s;

        LOG_AND_RETURN_BOOL_IF_ERR(dzr.writeToStream(generic.data));

        return true;
    });

    return TweakError::NONE;
}

TweakError add_more_magic_jars() {
    //DRC doesn't have any magic since you wouldn't normally have Deku Leaf there
    //But it can be required in the randomizer, so we make some skulls drop magic
    //Same thing for arrows
    {
        RandoSession::CacheEntry& entry = g_session.openGameFile("content/Common/Stage/M_NewD2_Room2.szs@YAZ0@SARC@Room2.bfres@BFRES@room.dzr@DZX");
        entry.addAction([](RandoSession* session, FileType* data) -> int {
            CAST_ENTRY_TO_FILETYPE(drc_hub, FileTypes::DZXFile, data)

            std::vector<ChunkEntry*> actors = drc_hub.entries_by_type("ACTR");
            std::vector<ChunkEntry*> skulls;
            for (ChunkEntry* actor : actors) {
                if (std::strncmp(&actor->data[0], "Odokuro\x00", 8) == 0) skulls.push_back(actor);
            }

            if(skulls.size() < 6) LOG_ERR_AND_RETURN_BOOL(TweakError::MISSING_ENTITY);

            skulls[0]->data.replace(0x8, 0x4, "\x75\x7f\xff\x10", 0, 4); //arrows in case logic expects you to use them for BK chest
            skulls[2]->data.replace(0x8, 0x4, "\x75\x7f\xff\x09", 0, 4); //small magic
            skulls[5]->data.replace(0x8, 0x4, "\x75\x7f\xff\x0A", 0, 4); //large magic

            return true;
        });
    }

    {
        RandoSession::CacheEntry& entry = g_session.openGameFile("content/Common/Stage/M_NewD2_Room10.szs@YAZ0@SARC@Room10.bfres@BFRES@room.dzr@DZX");
        entry.addAction([](RandoSession* session, FileType* data) -> int {
            CAST_ENTRY_TO_FILETYPE(drc_before_boss, FileTypes::DZXFile, data)

            std::vector<ChunkEntry*> actors = drc_before_boss.entries_by_type("ACTR");
            std::vector<ChunkEntry*> skulls;
            for (ChunkEntry* actor : actors) {
                if (std::strncmp(&actor->data[0], "Odokuro\x00", 8) == 0) skulls.push_back(actor);
            }

            if(skulls.size() < 11) LOG_ERR_AND_RETURN_BOOL(TweakError::MISSING_ENTITY);

            skulls[0]->data.replace(0x8, 0x4, "\x75\x7f\xff\x0A", 0, 4); //large magic
            skulls[9]->data.replace(0x8, 0x4, "\x75\x7f\xff\x0A", 0, 4); //large magic

            return true;
        });
    }

    //The grass on the small islands behind DRI do not have guaranteed magic
    //Add grass that will always drop some on each of the islands
    {
        RandoSession::CacheEntry& entry = g_session.openGameFile("content/Common/Pack/szs_permanent1.pack@SARC@sea_Room13.szs@YAZ0@SARC@Room13.bfres@BFRES@room.dzr@DZX");
        entry.addAction([](RandoSession* session, FileType* data) -> int {
            CAST_ENTRY_TO_FILETYPE(dri, FileTypes::DZXFile, data)

            ChunkEntry& grass1 = dri.add_entity("ACTR");
            grass1.data = "kusax1\x00\x00\x00\x00\x0E\x00\x48\x4C\xC7\x80\x44\xED\x80\x00\xC8\x45\xB7\xC0\x00\x00\x00\x00\x00\x00\xFF\xFF"s; //62.50% chance of small magic, 37.50% chance of large magic
            ChunkEntry& grass2 = dri.add_entity("ACTR");
            grass2.data = "kusax1\x00\x00\x00\x00\x0E\x00\x48\x4C\x6D\x40\x44\xA2\x80\x00\xC8\x4D\x38\x40\x00\x00\x00\x00\x00\x00\xFF\xFF"s; //62.50% chance of small magic, 37.50% chance of large magic

            return true;
        });
    }

    //Add magic to one of the pots outside the TotG miniboss
    {
        RandoSession::CacheEntry& entry = g_session.openGameFile("content/Common/Stage/Siren_Room14.szs@YAZ0@SARC@Room14.bfres@BFRES@room.dzr@DZX");
        entry.addAction([](RandoSession* session, FileType* data) -> int {
            CAST_ENTRY_TO_FILETYPE(totg, FileTypes::DZXFile, data)

            std::vector<ChunkEntry*> actors = totg.entries_by_type("ACTR");
            std::vector<ChunkEntry*> pots;
            for (ChunkEntry* actor : actors) {
                if (std::strncmp(&actor->data[0], "kotubo\x00\x00", 8) == 0) pots.push_back(actor);
            }

            if(pots.size() < 2) LOG_ERR_AND_RETURN_BOOL(TweakError::MISSING_ENTITY);

            pots[1]->data = "\x6B\x6F\x74\x75\x62\x6F\x00\x00\x70\x7F\xFF\x0A\xC5\x6E\x20\x00\x43\x66\x00\x05\xC5\xDF\xC0\x00\x00\x00\x00\x00\x00\xFF\xFF\xFF"s;

            return true;
        });
    }

    return TweakError::NONE;
}

TweakError modify_title_screen() {
    using namespace NintendoWare::Layout;

    RandoSession::CacheEntry& lytEntry = g_session.openGameFile("content/Common/Layout/Title_00.szs@YAZ0@SARC@blyt/Title_00.bflyt@BFLYT");
    lytEntry.addAction([](RandoSession* session, FileType* data) -> int {
        CAST_ENTRY_TO_FILETYPE(layout, FileTypes::FLYTFile, data)
        
        //add version number
        Pane& newPane = layout.rootPane.children[0].children[1].children[3].duplicateChildPane(1); //unused version number text
        newPane.pane->name = "T_Version";
        newPane.pane->name.resize(0x18);
        dynamic_cast<txt1*>(newPane.pane.get())->text = u"Ver " + Utility::Str::toUTF16(RANDOMIZER_VERSION) + u'\0'; //~20-21 characters per line
        dynamic_cast<txt1*>(newPane.pane.get())->fontIndex = 0;
        dynamic_cast<txt1*>(newPane.pane.get())->restrictedLen = dynamic_cast<txt1*>(newPane.pane.get())->text.length() * 2; //includes null
        dynamic_cast<txt1*>(newPane.pane.get())->lineAlignment = txt1::LineAlignment::CENTER;

        //update "HD" image position
        layout.rootPane.children[0].children[1].children[1].children[0].pane->translation.X = -165.0f;
        layout.rootPane.children[0].children[1].children[1].children[0].pane->translation.Y = 12.0f;

        //update subtitle size/position
        layout.rootPane.children[0].children[1].children[1].children[1].children[0].pane->translation.Y = -30.0f;
        layout.rootPane.children[0].children[1].children[1].children[1].children[0].pane->height = 120.0f;

        //update subtitle mask size/position
        layout.rootPane.children[0].children[1].children[1].children[1].children[1].pane->translation.Y = -30.0f;
        layout.rootPane.children[0].children[1].children[1].children[1].children[1].pane->height = 120.0f;

        return true;
    });

    //update "The Legend of Zelda" texture
    RandoSession::CacheEntry& tEntry = g_session.openGameFile("content/Common/Layout/Title_00.szs@YAZ0@SARC@timg/TitleLogoZelda_00^l.bflim@BFLIM");
    tEntry.addAction([](RandoSession* session, FileType* data) -> int {
        CAST_ENTRY_TO_FILETYPE(title, FileTypes::FLIMFile, data)
        
        FILETYPE_ERROR_CHECK(title.replaceWithDDS(Utility::get_data_path() / "assets/Title.dds", GX2TileMode::GX2_TILE_MODE_DEFAULT, 0, true));

        return true;
    });

    //update "The Wind Waker" texture
    RandoSession::CacheEntry& sEntry = g_session.openGameFile("content/Common/Layout/Title_00.szs@YAZ0@SARC@timg/TitleLogoWindwaker_00^l.bflim@BFLIM");
    sEntry.addAction([](RandoSession* session, FileType* data) -> int {
        CAST_ENTRY_TO_FILETYPE(subtitle, FileTypes::FLIMFile, data)
        
        FILETYPE_ERROR_CHECK(subtitle.replaceWithDDS(Utility::get_data_path() / "assets/Subtitle.dds", GX2TileMode::GX2_TILE_MODE_DEFAULT, 0, true));

        return true;
    });

    //update mask for "The Wind Waker" texture
    RandoSession::CacheEntry& mEntry = g_session.openGameFile("content/Common/Layout/Title_00.szs@YAZ0@SARC@timg/TitleLogoWindwakerMask_00^s.bflim@BFLIM");
    mEntry.addAction([](RandoSession* session, FileType* data) -> int {
        CAST_ENTRY_TO_FILETYPE(mask, FileTypes::FLIMFile, data)
        
        FILETYPE_ERROR_CHECK(mask.replaceWithDDS(Utility::get_data_path() / "assets/SubtitleMask.dds", GX2TileMode::GX2_TILE_MODE_DEFAULT, 0, false));

        return true;
    });

    //update sparkle size/position
    RandoSession::CacheEntry& rpx = g_session.openGameFile("code/cking.rpx@RPX@ELF");
    rpx.addAction([](RandoSession* session, FileType* data) -> int {
        CAST_ENTRY_TO_FILETYPE(elf, FileTypes::ELF, data)

        RPX_ERROR_CHECK(elfUtil::write_float(elf, elfUtil::AddressToOffset(elf, 0x101F7048), 1.4f)); //scale
        RPX_ERROR_CHECK(elfUtil::write_float(elf, elfUtil::AddressToOffset(elf, 0x101F7044), 2.25f)); //possibly particle size, JP changes it for its larger title text
        RPX_ERROR_CHECK(elfUtil::write_float(elf, elfUtil::AddressToOffset(elf, 0x10108280), -38.0f)); //vertical position

        return true;
    });

    return TweakError::NONE;
}

TweakError update_name_and_icon() {
    if(!g_session.copyToGameFile(Utility::get_data_path() / "assets/iconTex.tga", "meta/iconTex.tga", /*resourceFile = */ true)) LOG_ERR_AND_RETURN(TweakError::FILE_COPY_FAILED);

    RandoSession::CacheEntry& metaEntry = g_session.openGameFile("meta/meta.xml");
    metaEntry.addAction([](RandoSession* session, FileType* data) -> int {
        CAST_ENTRY_TO_FILETYPE(generic, RawFile, data)\
        std::stringstream& metaStream = generic.data;

        tinyxml2::XMLDocument meta;
        meta.Parse(metaStream.str().c_str(), metaStream.str().size());

        tinyxml2::XMLElement* metaRoot = meta.RootElement();
        metaRoot->FirstChildElement("longname_en")->SetText("THE LEGEND OF ZELDA\nThe Wind Waker HD Randomizer");
        metaRoot->FirstChildElement("longname_fr")->SetText("THE LEGEND OF ZELDA\nThe Wind Waker HD Randomizer");
        metaRoot->FirstChildElement("longname_es")->SetText("THE LEGEND OF ZELDA\nThe Wind Waker HD Randomizer");
        metaRoot->FirstChildElement("longname_pt")->SetText("THE LEGEND OF ZELDA\nThe Wind Waker HD Randomizer");

        metaRoot->FirstChildElement("shortname_en")->SetText("The Wind Waker HD Randomizer");
        metaRoot->FirstChildElement("shortname_fr")->SetText("The Wind Waker HD Randomizer");
        metaRoot->FirstChildElement("shortname_es")->SetText("The Wind Waker HD Randomizer");
        metaRoot->FirstChildElement("shortname_pt")->SetText("The Wind Waker HD Randomizer");

        //change the title ID so it gets its own channel when repacked
        metaRoot->FirstChildElement("title_id")->SetText("0005000010143599");
        
        tinyxml2::XMLPrinter printer;
        meta.Print(&printer);
        metaStream.str(printer.CStr());

        return true;
    });
    
    RandoSession::CacheEntry& appEntry = g_session.openGameFile("code/app.xml");
    appEntry.addAction([](RandoSession* session, FileType* data) -> int {
        CAST_ENTRY_TO_FILETYPE(generic, RawFile, data)\
        std::stringstream& appStream = generic.data;

        tinyxml2::XMLDocument app;
        app.Parse(appStream.str().c_str(), appStream.str().size());
        tinyxml2::XMLElement* appRoot = app.RootElement();
        appRoot->FirstChildElement("title_id")->SetText("0005000010143599");
        
        tinyxml2::XMLPrinter printer;
        app.Print(&printer);
        appStream.str(printer.CStr());

        return true;
    });

    return TweakError::NONE;
}

TweakError allow_dungeon_items_to_appear_anywhere(World& world) {
    struct dungeon_item_info {
        const std::string short_name;
        const std::string base_item_name;
        const GameItem item_value;
    };

    const uint32_t item_get_func_pointer = 0x0001DA54; //First relevant relocation entry in .rela.data (overwrites .data section when loaded)
    const uint32_t item_resources_list_start = 0x101E4674;
    const uint32_t field_item_resources_list_start = 0x101E6A74;
    const uint32_t item_info_list_start = 0x101E8674;

    const std::unordered_map<std::string, std::string> dungeon_names = {
        {"DRC", "Dragon Roost Cavern"},
        {"FW", "Forbidden Woods"},
        {"TotG", "Tower of the Gods"},
        {"FF", "Forsaken Fortress"},
        {"ET", "Earth Temple"},
        {"WT", "Wind Temple"}
    };

    const std::unordered_map<std::string, uint8_t> item_name_to_id = {
        {"Small Key", 0x15},
        {"Dungeon Map", 0x4C},
        {"Compass", 0x4D},
        {"Big Key", 0x4E}
    };

    const std::array<dungeon_item_info, 22> dungeon_items = {{
        {"DRC", "Small Key", GameItem::DRCSmallKey},
        {"DRC", "Big Key", GameItem::DRCBigKey},
        {"DRC", "Dungeon Map", GameItem::DRCDungeonMap},
        {"DRC", "Compass", GameItem::DRCCompass},
        {"FW", "Small Key", GameItem::FWSmallKey},
        {"FW", "Big Key", GameItem::FWBigKey},
        {"FW", "Dungeon Map", GameItem::FWDungeonMap},
        {"FW", "Compass", GameItem::FWCompass},
        {"TotG", "Small Key", GameItem::TotGSmallKey},
        {"TotG", "Big Key", GameItem::TotGBigKey},
        {"TotG", "Dungeon Map", GameItem::TotGDungeonMap},
        {"TotG", "Compass", GameItem::TotGCompass},
        {"FF", "Dungeon Map", GameItem::FFDungeonMap},
        {"FF", "Compass", GameItem::FFCompass},
        {"ET", "Small Key", GameItem::ETSmallKey},
        {"ET", "Big Key", GameItem::ETBigKey},
        {"ET", "Dungeon Map", GameItem::ETDungeonMap},
        {"ET", "Compass", GameItem::ETCompass},
        {"WT", "Small Key", GameItem::WTSmallKey},
        {"WT", "Big Key", GameItem::WTBigKey},
        {"WT", "Dungeon Map", GameItem::WTDungeonMap},
        {"WT", "Compass", GameItem::WTCompass}
    }};

    const std::unordered_map<GameItem, std::string> itemToFunc = {
        {GameItem::DRCSmallKey, "drc_small_key_item_get_func"},
        {GameItem::DRCBigKey, "drc_big_key_item_get_func"},
        {GameItem::DRCDungeonMap, "drc_dungeon_map_item_get_func"},
        {GameItem::DRCCompass, "drc_compass_item_get_func"},
        {GameItem::FWSmallKey, "fw_small_key_item_get_func"},
        {GameItem::FWBigKey, "fw_big_key_item_get_func"},
        {GameItem::FWDungeonMap, "fw_dungeon_map_item_get_func"},
        {GameItem::FWCompass, "fw_compass_item_get_func"},
        {GameItem::TotGSmallKey, "totg_small_key_item_get_func"},
        {GameItem::TotGBigKey, "totg_big_key_item_get_func"},
        {GameItem::TotGDungeonMap, "totg_dungeon_map_item_get_func"},
        {GameItem::TotGCompass, "totg_compass_item_get_func"},
        {GameItem::FFDungeonMap, "ff_dungeon_map_item_get_func"},
        {GameItem::FFCompass, "ff_compass_item_get_func"},
        {GameItem::ETSmallKey, "et_small_key_item_get_func"},
        {GameItem::ETBigKey, "et_big_key_item_get_func"},
        {GameItem::ETDungeonMap, "et_dungeon_map_item_get_func"},
        {GameItem::ETCompass, "et_compass_item_get_func"},
        {GameItem::WTSmallKey, "wt_small_key_item_get_func"},
        {GameItem::WTBigKey, "wt_big_key_item_get_func"},
        {GameItem::WTDungeonMap, "wt_dungeon_map_item_get_func"},
        {GameItem::WTCompass, "wt_compass_item_get_func"},
    };

    const std::unordered_map<uint8_t, uint32_t> szs_name_pointers = {
        {0x15, 0x1004E448},
        {0x4C, 0x1004E4b8},
        {0x4D, 0x1004E4b0},
        {0x4E, 0x1004E698}
    };

    const std::unordered_map<std::string, std::u16string> messageBegin = {
      {"English", u"You got "},
      {"Spanish", u"¡Has conseguido "},
      {"French", u"Vous obtenez "},
    };

    RandoSession::CacheEntry& rpx = g_session.openGameFile("code/cking.rpx@RPX@ELF");
    for (const dungeon_item_info& item_data : dungeon_items) {
        const std::string item_name = item_data.short_name + " " + item_data.base_item_name;
        const uint8_t item_id = static_cast<uint8_t>(item_data.item_value);
        const uint8_t base_item_id = item_name_to_id.at(item_data.base_item_name);
        const std::string dungeon_name = dungeon_names.at(item_data.short_name);

        rpx.addAction([=](RandoSession* session, FileType* data) -> int {
            CAST_ENTRY_TO_FILETYPE(elf, FileTypes::ELF, data)

            RPX_ERROR_CHECK(elfUtil::write_u32(elf, elfUtil::AddressToOffset(elf, item_get_func_pointer + (0xC * item_id) + 0x8, 9), custom_symbols.at(itemToFunc.at(item_data.item_value)) - 0x02000000)); //write to the relocation entries

            return true;
        });
        
        // Get the item out here, since trying to get it in the action causing a seg fault sometimes
        auto dungeon_item = world.getItem(dungeon_name + " " + item_data.base_item_name);

        for (const auto& language : Text::supported_languages) {
            RandoSession::CacheEntry& entry = g_session.openGameFile("content/Common/Pack/permanent_2d_Us" + language + ".pack@SARC@message_msbt.szs@YAZ0@SARC@message.msbt@MSBT");
            entry.addAction([=](RandoSession* session, FileType* data) -> int {
                CAST_ENTRY_TO_FILETYPE(msbt, FileTypes::MSBTFile, data)

                const uint32_t message_id = 101 + item_id;
                const Message& to_copy = msbt.messages_by_label["00" + std::to_string(101 + base_item_id)];
                std::u16string message = messageBegin.at(language) + dungeon_item.getUTF16Name(language, Text::Type::PRETTY) + u"!"s + TEXT_END;

                message = Text::word_wrap_string(message, 39);
                msbt.addMessage("00" + std::to_string(message_id), to_copy.attributes, to_copy.style, message);

                return true;
            });
        }

        const uint32_t item_resources_addr_to_copy_from = item_resources_list_start + base_item_id * 0x24;
        const uint32_t field_item_resources_addr_to_copy_from = field_item_resources_list_start + base_item_id * 0x1C;

        const uint32_t item_resources_addr = item_resources_list_start + item_id * 0x24;
        const uint32_t field_item_resources_addr = field_item_resources_list_start + item_id * 0x1C;

        const uint32_t szs_name_pointer = szs_name_pointers.at(base_item_id);

        rpx.addAction([=](RandoSession* session, FileType* data) -> int {
            CAST_ENTRY_TO_FILETYPE(elf, FileTypes::ELF, data)

            RPX_ERROR_CHECK(elfUtil::write_u32(elf, elfUtil::AddressToOffset(elf, field_item_resources_addr), szs_name_pointer));
            RPX_ERROR_CHECK(elfUtil::write_u32(elf, elfUtil::AddressToOffset(elf, item_resources_addr), szs_name_pointer));

            return true;
        });

        // Need relocation entries so pointers work on console
        Elf32_Rela relocation;
        relocation.r_offset = field_item_resources_addr;
        relocation.r_info = 0x00000201;
        relocation.r_addend = szs_name_pointer - 0x10000000; //needs offset into .rodata section, subtract start address from data location

        Elf32_Rela relocation2;
        relocation2.r_offset = item_resources_addr;
        relocation2.r_info = relocation.r_info; //same as first entry
        relocation2.r_addend = relocation.r_addend; //same as first entry

        // Also update the item info for custom dungeon items to match the vanilla ones
        // Includes the flag that stops the item from fading out over time
        uint32_t item_info_value = 0x00282800;
        if(item_data.base_item_name == "Small Key") {
            item_info_value = 0x14281E05;
        }
        else if(item_data.base_item_name == "Big Key") {
            item_info_value = 0x00282805;
        }

        const uint32_t item_info_entry_addr = item_info_list_start + item_id * 4;

        rpx.addAction([=](RandoSession* session, FileType* data) -> int {
            CAST_ENTRY_TO_FILETYPE(elf, FileTypes::ELF, data)

            RPX_ERROR_CHECK(elfUtil::addRelocation(elf, 9, relocation));
            RPX_ERROR_CHECK(elfUtil::addRelocation(elf, 9, relocation2));

            const std::vector<uint8_t> data1 = elfUtil::read_bytes(elf, elfUtil::AddressToOffset(elf, item_resources_addr_to_copy_from + 8), 0xD);
            RPX_ERROR_CHECK(elfUtil::write_bytes(elf, elfUtil::AddressToOffset(elf, item_resources_addr + 8), data1));
            RPX_ERROR_CHECK(elfUtil::write_bytes(elf, elfUtil::AddressToOffset(elf, field_item_resources_addr + 4), data1));

            const std::vector<uint8_t> data2 = elfUtil::read_bytes(elf, elfUtil::AddressToOffset(elf, item_resources_addr_to_copy_from + 0x1C), 4);
            RPX_ERROR_CHECK(elfUtil::write_bytes(elf, elfUtil::AddressToOffset(elf, item_resources_addr + 0x1C), data2));
            RPX_ERROR_CHECK(elfUtil::write_bytes(elf, elfUtil::AddressToOffset(elf, field_item_resources_addr + 0x14), data2));

            const std::vector<uint8_t> data3 = elfUtil::read_bytes(elf, elfUtil::AddressToOffset(elf, item_resources_addr_to_copy_from + 0x15), 0x7);
            RPX_ERROR_CHECK(elfUtil::write_bytes(elf, elfUtil::AddressToOffset(elf, item_resources_addr + 0x15), data3));

            const std::vector<uint8_t> data4 = elfUtil::read_bytes(elf, elfUtil::AddressToOffset(elf, item_resources_addr_to_copy_from + 0x20), 0x4);
            RPX_ERROR_CHECK(elfUtil::write_bytes(elf, elfUtil::AddressToOffset(elf, item_resources_addr + 0x20), data4));

            const std::vector<uint8_t> data5 = elfUtil::read_bytes(elf, elfUtil::AddressToOffset(elf, field_item_resources_addr_to_copy_from + 0x11), 0x3);
            RPX_ERROR_CHECK(elfUtil::write_bytes(elf, elfUtil::AddressToOffset(elf, field_item_resources_addr + 0x11), data5));

            const std::vector<uint8_t> data6 = elfUtil::read_bytes(elf, elfUtil::AddressToOffset(elf, field_item_resources_addr_to_copy_from + 0x18), 0x4);
            RPX_ERROR_CHECK(elfUtil::write_bytes(elf, elfUtil::AddressToOffset(elf, field_item_resources_addr + 0x18), data6));


            RPX_ERROR_CHECK(elfUtil::write_u32(elf, elfUtil::AddressToOffset(elf, item_info_entry_addr), item_info_value));

            return true;
        });
    }

    return TweakError::NONE;
}

TweakError remove_bog_warp_in_cs() {
    for (uint8_t i = 1; i < 49+1; i++) {
        const fspath path = getRoomDzrPath("sea", i);
        RandoSession::CacheEntry& entry = g_session.openGameFile(path);
        entry.addAction([](RandoSession* session, FileType* data) -> int {
            CAST_ENTRY_TO_FILETYPE(room_dzr, FileTypes::DZXFile, data)

            for (ChunkEntry* spawn : room_dzr.entries_by_type("PLYR")) {
                uint8_t spawn_type = ((*reinterpret_cast<uint8_t*>(&spawn->data[0xB]) & 0xF0) >> 4);
                if (spawn_type == 0x09) {
                    spawn->data[0xB] = (spawn->data[0xB] & 0x0F) | 0x20;
                }
            }

            return true;
        });
    }

    return TweakError::NONE;
}

TweakError fix_shop_item_y_offsets() {
    const uint32_t shop_item_display_data_list_start = 0x1003A930;
    const std::unordered_set<uint8_t> ArrowID = { 0x10, 0x11, 0x12 };
    RandoSession::CacheEntry& rpx = g_session.openGameFile("code/cking.rpx@RPX@ELF");

    for (unsigned int id = 0; id < 0xFF + 1; id++) {
        const uint32_t display_data_addr = shop_item_display_data_list_start + id * 0x20;
        rpx.addAction([=](RandoSession* session, FileType* data) -> int {
            CAST_ENTRY_TO_FILETYPE(elf, FileTypes::ELF, data)
            const float y_offset = elfUtil::read_float(elf, elfUtil::AddressToOffset(elf, display_data_addr + 0x10));

            if (y_offset == 0.0f && ArrowID.count(id) == 0) {
                // If the item didn't originally have a Y offset we need to give it one so it's not sunken into the pedestal.
                // Only exception are for items 10 11 and 12 - arrow refill pickups. Those have no Y offset but look fine already.
                static const float new_y_offset = 20.0f;
                RPX_ERROR_CHECK(elfUtil::write_float(elf, elfUtil::AddressToOffset(elf, display_data_addr + 0x10), new_y_offset));
            }

            return true;
        });
    }

    return TweakError::NONE;
}

TweakError update_text_replacements(World& world) {
    auto textReplacements = generate_text_replacements(world);

    for (auto& [messageLabel, languages] : textReplacements) {
        for (const auto& language : Text::supported_languages) {
            // Don't do anything if there's no text yet
            if (languages[language].empty()) continue;

            // Calculate which msbt we need to open
            auto labelIdx = std::stoi(messageLabel);
            std::string messageNum = ""; // default is message_msbt
            if (labelIdx > 3235 && labelIdx < 6566) { // message2_msbt
                messageNum = "2";
            } else if (labelIdx >= 6566 && labelIdx <= 10761) { // message3_msbt
                messageNum = "3";
            } else if (labelIdx > 10761) { // message4_msbt
                messageNum = "4";
            }

            fspath filePath = std::string("content/Common/Pack/permanent_2d_Us") + language + ".pack@SARC@message" + messageNum + "_msbt.szs@YAZ0@SARC@message" + messageNum + ".msbt@MSBT";
            RandoSession::CacheEntry& entry = g_session.openGameFile(filePath);
            entry.addAction([language, messageLabel = messageLabel, languages = languages](RandoSession* session, FileType* data) -> int {
                CAST_ENTRY_TO_FILETYPE(msbt, FileTypes::MSBTFile, data)

                msbt.messages_by_label[messageLabel].text.message = languages.at(language);

                return true;
            });
        }
    }

    return TweakError::NONE;
}

TweakError shorten_zephos_event() {
    RandoSession::CacheEntry& list = g_session.openGameFile("content/Common/Pack/first_szs_permanent.pack@SARC@sea_Stage.szs@YAZ0@SARC@Stage.bfres@BFRES@event_list.dat@EVENTS");

    list.addAction([](RandoSession* session, FileType* data) -> int {
        CAST_ENTRY_TO_FILETYPE(event_list, FileTypes::EventList, data)
        
        if(event_list.Events_By_Name.count("TACT_HT") == 0) LOG_ERR_AND_RETURN_BOOL(TweakError::MISSING_EVENT);
        std::shared_ptr<Event> wind_shrine_event = event_list.Events_By_Name.at("TACT_HT");

        std::shared_ptr<Actor> zephos = wind_shrine_event->get_actor("Hr");
        if (zephos == nullptr) {
            LOG_ERR_AND_RETURN_BOOL(TweakError::MISSING_EVENT);
        }

        std::shared_ptr<Actor> link = wind_shrine_event->get_actor("Link");
        if (link == nullptr) {
            LOG_ERR_AND_RETURN_BOOL(TweakError::MISSING_EVENT);
        }

        std::shared_ptr<Actor> camera = wind_shrine_event->get_actor("CAMERA");
        if (camera == nullptr) {
            LOG_ERR_AND_RETURN_BOOL(TweakError::MISSING_EVENT);
        }

        zephos->actions.erase(zephos->actions.begin() + 7, zephos->actions.end());
        link->actions.erase(link->actions.begin() + 7, link->actions.end());
        camera->actions.erase(camera->actions.begin() + 5, camera->actions.end());
        wind_shrine_event->ending_flags = {
            zephos->actions.back()->flag_id_to_set,
            link->actions.back()->flag_id_to_set,
            camera->actions.back()->flag_id_to_set,
        };

        return true;
    });

    return TweakError::NONE;
}

// Korl Hints
TweakError update_korl_dialog(World& world) {

    if (!world.korlHints.empty()) {
        for (const auto& language: Text::supported_languages) {
            RandoSession::CacheEntry& entry = g_session.openGameFile("content/Common/Pack/permanent_2d_Us" + language + ".pack@SARC@message2_msbt.szs@YAZ0@SARC@message2.msbt@MSBT");

            std::vector<std::u16string> hintMessages = {u""};
            size_t curLine = 0;
            for (auto location : world.korlHints) {
                std::u16string hint = location->hint.text[language];
                hint = Text::word_wrap_string(hint, 42); // wrap shorter lines, some edge cases are still too wide with 43
                hint = Text::pad_str_4_lines(hint);
                const size_t numLines = std::count(hint.begin(), hint.end(), u'\n'); // 4 for most hints, 8+ for long hints with multiple textboxes

                // if we would have >10 textboxes
                if(curLine + numLines > 40) {
                    hintMessages.back().back() = u'\0'; // add null terminator
                    hintMessages.emplace_back(); // start a new hint message
                    curLine = 0; // restart line counter
                }

                hintMessages.back() += hint; // add hint to back of current message
                curLine += numLines; // add hint lines to count
            }

            if(hintMessages.back().back() != u'\0') {
                hintMessages.back().back() = u'\0';
            }

            if(custom_symbols.count("last_korl_hint_message_number") == 0) LOG_ERR_AND_RETURN(TweakError::MISSING_SYMBOL);
            const uint32_t num_messages_address = custom_symbols.at("last_korl_hint_message_number");
            g_session.openGameFile("code/cking.rpx@RPX@ELF").addAction([num_messages_address, numExtra = hintMessages.size() - 1](RandoSession* session, FileType* data) -> int {
                CAST_ENTRY_TO_FILETYPE(elf, FileTypes::ELF, data)

                RPX_ERROR_CHECK(elfUtil::write_u32(elf, elfUtil::AddressToOffset(elf, num_messages_address), 3443 + numExtra));

                return true;
            });

            for (uint32_t x = 0; x < hintMessages.size(); x++) {
                const std::u16string msg = hintMessages[x];
                const std::string label = "0"s + std::to_string(3443 + x);
                entry.addAction([label, msg](RandoSession* session, FileType* data) -> int {
                    CAST_ENTRY_TO_FILETYPE(msbt, FileTypes::MSBTFile, data)

                    msbt.messages_by_label[label].text.message = msg;

                    return true;
                });
            }
        }
    }
    return TweakError::NONE;
}

// Ho Ho Hints
TweakError update_ho_ho_dialog(World& world) {

    // If no ho ho hints, don't do anything
    if (world.hohoHints.empty()) {
        return TweakError::NONE;
    }

    for (const auto& language : Text::supported_languages) {
        RandoSession::CacheEntry& entry = g_session.openGameFile("content/Common/Pack/permanent_2d_Us" + language + ".pack@SARC@message4_msbt.szs@YAZ0@SARC@message4.msbt@MSBT");
        
        entry.addAction([=, &world](RandoSession* session, FileType* data) -> int {
            for (auto& [hohoLocation, hintLocations] : world.hohoHints) {
                std::u16string hintLines = u"";
                size_t i = 0; // counter to know when to add null terminator
                for (auto location : hintLocations) {
                    std::u16string hint = u"";
                    if (i == 0) {
                        hint += SOUND(0x0103) u"Ho ho! "s;
                    }
                    i++;
                    hint += location->hint.text[language];
                    hint = Text::word_wrap_string(hint, 43);
                    if (i == hintLocations.size()) {
                        hint += u'\0'; // add null terminator on last hint before padding
                    }
                    hint = Text::pad_str_4_lines(hint);
                    hintLines += hint;
                }
                CAST_ENTRY_TO_FILETYPE(msbt, FileTypes::MSBTFile, data)

                msbt.messages_by_label[hohoLocation->messageLabel].text.message = hintLines;
            }
            return true;
        });
    }
    return TweakError::NONE;
}

TweakError rotate_ho_ho_to_face_hints(World& world) {

    // If no ho ho hints, don't do anything
    if (world.hohoHints.empty()) {
        return TweakError::NONE;
    }

    for (auto& [hohoLocation, hintLocations] : world.hohoHints) {
        std::string island = "";
        for (auto location : hintLocations) {
            for (auto region : location->hintRegions) {
                // If this region is a dungeon, use the dungeon's island instead
                if (world.dungeons.contains(region)) {
                    region = world.dungeons[region].islands.front();
                }
        
                if (islandNameToRoomNum(region) != 0) {
                    island = region;
                }
            }
        
            if (island != "") {
                break;
            }
        }

        if (island != "") {
            LOG_TO_DEBUG("Rotating " + hohoLocation->getName() + " to face " + island);
            auto islandNumToFace = islandNameToRoomNum(island);
            auto hohoIslandNum = islandNameToRoomNum(hohoLocation->hintRegions.front());

            const fspath filepath = getRoomDzrPath("sea", hohoIslandNum);
            RandoSession::CacheEntry& room = g_session.openGameFile(filepath);

            room.addAction([=](RandoSession* session, FileType* data) -> int {

                CAST_ENTRY_TO_FILETYPE(islandDzr, FileTypes::DZXFile, data)
                auto actors = islandDzr.entries_by_type("ACTR");

                // For each ho ho actor (actor name "Ah"), rotate him to face the destSectorMult
                for (auto actor : actors) {
                    if (std::strncmp(&actor->data[0], "Ah\x00\x00\x00\x00\x00\x00", 8) == 0) {
                        float hohoX = Utility::Endian::toPlatform(eType::Big, *(float*)&actor->data[0x0C]);
                        float hohoZ = Utility::Endian::toPlatform(eType::Big, *(float*)&actor->data[0x14]);
                        
                        // Calculate coordinates of the island to face
                        auto island = islandNumToFace - 1;

                        // Get X and Z
                        int islandX = island % 7;
                        int islandZ = island / 7;

                        // Shift X and Z since the origin is the center of the map, not the top left corner
                        islandX -= 3;
                        islandZ -= 3;

                        // Multiply by 100k to get actual coordinate values
                        islandX *= 100000;
                        islandZ *= 100000;

                        auto angleRad = atan2(islandX - hohoX, islandZ - hohoZ);
                        uint16_t angle = int(angleRad * (0x8000 / std::numbers::pi)) % 0x10000;

                        Utility::Endian::toPlatform_inplace(eType::Big, angle);
                        actor->data.replace(0x1A, 2, reinterpret_cast<const char*>(&angle), 2);
                    }
                }
                return true;
            });
        } 
    }
    return TweakError::NONE;
}

TweakError set_starting_health(const uint16_t heartPieces, const uint16_t heartContainers) {
    if(custom_symbols.count("starting_quarter_hearts") == 0) LOG_ERR_AND_RETURN(TweakError::MISSING_SYMBOL);
    const uint16_t starting_health = (heartContainers * 4) + heartPieces;
    const uint32_t starting_quarter_hearts_address = custom_symbols.at("starting_quarter_hearts");

    g_session.openGameFile("code/cking.rpx@RPX@ELF").addAction([starting_quarter_hearts_address, starting_health](RandoSession* session, FileType* data) -> int {
        CAST_ENTRY_TO_FILETYPE(elf, FileTypes::ELF, data)

        RPX_ERROR_CHECK(elfUtil::write_u16(elf, elfUtil::AddressToOffset(elf, starting_quarter_hearts_address), starting_health));

        return true;
    });

    if(starting_health < 8) {
        LOG_AND_RETURN_IF_ERR(Apply_Patch(Utility::get_data_path() / "asm/patch_diffs/remove_low_health_effects_diff.yaml")); 
    }

    return TweakError::NONE;
}

TweakError set_starting_magic(const uint8_t& startingMagic) {
    if(custom_symbols.count("starting_magic") == 0) LOG_ERR_AND_RETURN(TweakError::MISSING_SYMBOL);
    const uint32_t starting_magic_address = custom_symbols.at("starting_magic");
    g_session.openGameFile("code/cking.rpx@RPX@ELF").addAction([starting_magic_address, startingMagic](RandoSession* session, FileType* data) -> int {
        CAST_ENTRY_TO_FILETYPE(elf, FileTypes::ELF, data)

        RPX_ERROR_CHECK(elfUtil::write_u8(elf, elfUtil::AddressToOffset(elf, starting_magic_address), startingMagic));

        return true;
    });
    
    return TweakError::NONE;
}

TweakError set_damage_multiplier(const float& multiplier) {
    if(custom_symbols.count("custom_damage_multiplier") == 0) LOG_ERR_AND_RETURN(TweakError::MISSING_SYMBOL);
    const uint32_t damage_multiplier_address = custom_symbols.at("custom_damage_multiplier");
    g_session.openGameFile("code/cking.rpx@RPX@ELF").addAction([damage_multiplier_address, multiplier](RandoSession* session, FileType* data) -> int {
        CAST_ENTRY_TO_FILETYPE(elf, FileTypes::ELF, data)

        RPX_ERROR_CHECK(elfUtil::write_float(elf, elfUtil::AddressToOffset(elf, damage_multiplier_address), multiplier));
        
        return true;
    });

    // Update the hero mode description if we changed the multiplier
    if(multiplier != 2.0f) {
        for (const auto& language : Text::supported_languages) {
            RandoSession::CacheEntry& entry = g_session.openGameFile("content/Common/Pack/permanent_2d_Us" + language + ".pack@SARC@SequenceWindow_00_msbt.szs@YAZ0@SARC@SequenceWindow_00.msbt@MSBT");
            entry.addAction([=](RandoSession* session, FileType* data) -> int {
                CAST_ENTRY_TO_FILETYPE(msbt, FileTypes::MSBTFile, data)

                static const std::unordered_map<std::string, std::u16string> word_to_replace = {
                    {"English", u"double"s},
                    {"Spanish", u"el doble"s},
                    {"French", u"doublés"s},
                };

                std::u16string& message = msbt.messages_by_label["T_Msg_00_hardmode00"].text.message;
                const std::u16string& replace = word_to_replace.at(language);
                const std::u16string& replacement = Utility::Str::toUTF16(std::to_string(static_cast<uint8_t>(multiplier)) + "x");
                message.replace(message.find(replace), replace.size(), replacement);

                return true;
            });
        }
    }

    return TweakError::NONE;
}

TweakError set_pig_color(const PigColor& color) {
    if(custom_symbols.count("outset_pig_color") == 0) LOG_ERR_AND_RETURN(TweakError::MISSING_SYMBOL);
    const uint32_t pig_color_address = custom_symbols.at("outset_pig_color");
    g_session.openGameFile("code/cking.rpx@RPX@ELF").addAction([pig_color_address, color](RandoSession* session, FileType* data) -> int {
        CAST_ENTRY_TO_FILETYPE(elf, FileTypes::ELF, data)

        RPX_ERROR_CHECK(elfUtil::write_u8(elf, elfUtil::AddressToOffset(elf, pig_color_address), static_cast<uint8_t>(color)));

        return true;
    });
    
    return TweakError::NONE;
}

TweakError add_pirate_ship_to_windfall() {
    RandoSession::CacheEntry& windfall = g_session.openGameFile("content/Common/Pack/szs_permanent1.pack@SARC@sea_Room11.szs@YAZ0@SARC@Room11.bfres@BFRES@room.dzr@DZX");
    windfall.addAction([](RandoSession* session, FileType* data) -> int {
        CAST_ENTRY_TO_FILETYPE(windfallDzr, FileTypes::DZXFile, data)

        std::vector<ChunkEntry*> wf_layer_2_actors = windfallDzr.entries_by_type_and_layer("ACTR", 2);
        std::string layer_2_ship_data; //copy actor data, add_entity reallocates vector and invalidates pointer
        for (ChunkEntry* actor : wf_layer_2_actors) {
            if (std::strncmp(&actor->data[0], "Pirates\x00", 8) == 0) layer_2_ship_data = actor->data;
        }
        if(layer_2_ship_data.empty()) LOG_ERR_AND_RETURN_BOOL(TweakError::MISSING_ENTITY);

        ChunkEntry& default_layer_ship = windfallDzr.add_entity("ACTR");
        default_layer_ship.data = layer_2_ship_data;
        default_layer_ship.data[0xA] = '\x00';

        return true;
    });

    RandoSession::CacheEntry& shipRoom = g_session.openGameFile("content/Common/Stage/Asoko_Room0.szs@YAZ0@SARC@Room0.bfres@BFRES@room.dzr@DZX");
    shipRoom.addAction([](RandoSession* session, FileType* data) -> int {
        CAST_ENTRY_TO_FILETYPE(shipDzr, FileTypes::DZXFile, data)

        for (const int layer_num : {2, 3}) {
            std::vector<ChunkEntry*> actors = shipDzr.entries_by_type_and_layer("ACTR", layer_num);
            for (ChunkEntry* actor : actors) {
                if (std::strncmp(&actor->data[0], "P2b\x00\x00\x00\x00\x00", 8) == 0) shipDzr.remove_entity(actor);
            }
        }

        ChunkEntry& aryll = shipDzr.add_entity("ACTR");
        aryll.data = "Ls1\x00\x00\x00\x00\x00\x00\x00\x00\x00\x44\x16\x00\x00\xC4\x09\x80\x00\xC3\x48\x00\x00\x00\x00\xC0\x00\x00\x00\xFF\xFF"s;

        //const uint8_t countdown_happening_switch = 0xC0;
        //const uint8_t aryll_opened_door_switch = 0xC1;
        //const uint8_t countdown_not_happening_switch = 0xC2;
        //const uint8_t inside_chest_room_switch = 0xC3;
        const uint8_t door_should_be_open_switch = 0xC4;

        ChunkEntry& swc00 = shipDzr.add_entity("SCOB");
        swc00.data = "SW_C00\x00\x00\x00\x00\xFF\xC3\x00\x00\x00\x00\xC4\x09\x80\x00\xC5\x73\xC0\x00\x00\x00\x00\x00\x00\x00\xFF\xFF\x40\x24\x40\xFF"s;

        ChunkEntry& swop = shipDzr.add_entity("ACTR");
        swop.data = "SwOp\x00\x00\x00\x00\x01\xC0\xC2\x13\x00\x00\x00\x00\x00\x00\x00\x00\xC5\x89\x80\x00\x00\xFF\x00\x00\x00\x00\xFF\xFF"s;

        //for starting the event (hardcode the index because it avoids the file dependency, not great but whatever)
        ChunkEntry& swop2 = shipDzr.add_entity("ACTR");
        swop2.data = "SwOp\x00\x00\x00\x00\x02\xC2\xC1\x10\x00\x00\x00\x00\x00\x00\x00\x00\xC5\x73\xC0\x00\x00\x02\x00\x00\x00\x96\xFF\xFF"s;
        //opening the door
        ChunkEntry& swop3 = shipDzr.add_entity("ACTR");
        swop3.data = "SwOp\x00\x00\x00\x00\x02\xC0\xC4\x12\x00\x00\x00\x00\x00\x00\x00\x00\xC5\x54\x80\x00\x00\xFF\x00\x00\x00\x00\xFF\xFF"s;
        
        for (const int layer_num : {2, 3}) {
            std::vector<ChunkEntry*> actors = shipDzr.entries_by_type_and_layer("ACTR", layer_num);
            for (ChunkEntry* actor : actors) {
                if (std::strncmp(&actor->data[0], "Ashut\x00\x00\x00", 8) == 0) actor->data[0xB] = door_should_be_open_switch;
            }
        }

        return true;
    });
    
    for (const auto& language : Text::supported_languages) {
        RandoSession::CacheEntry& entry = g_session.openGameFile("content/Common/Pack/permanent_2d_Us" + language + ".pack@SARC@message_msbt.szs@YAZ0@SARC@message.msbt@MSBT");
        entry.addAction([](RandoSession* session, FileType* data) -> int {
            CAST_ENTRY_TO_FILETYPE(msbt, FileTypes::MSBTFile, data)
            msbt.messages_by_label["03008"].attributes.soundEffect = 106;

            Attributes attributes;
            attributes.character = 0x3; //Aryll
            attributes.boxStyle = 0x0;
            attributes.drawType = 0x1;
            attributes.screenPos = 0x3;
            attributes.lineAlignment = 1; //left alignment
            TSY1Entry tsy;
            tsy.styleIndex = 0x12A;
            msbt.addMessage("00849", attributes, tsy, u""s); // Text set in text_replacements.cpp
            msbt.addMessage("00850", attributes, tsy, u""s);

            return true;
        });
    }

    // Add a custom event where Aryll notices if the player got trapped in the chest room after the timer ran out and opens the door for them.
    RandoSession::CacheEntry& shipEventList = g_session.openGameFile("content/Common/Stage/Asoko_Stage.szs@YAZ0@SARC@Stage.bfres@BFRES@event_list.dat@EVENTS");
    shipEventList.addAction([](RandoSession* session, FileType* data) -> int {
        CAST_ENTRY_TO_FILETYPE(event_list, FileTypes::EventList, data)

        Event& event = event_list.add_event("AryllOpensDoor");
        
        std::shared_ptr<Actor> camera = event.add_actor(event_list, "CAMERA");
        camera->staff_type = 2;
        std::shared_ptr<Actor> aryll_actor = event.add_actor(event_list, "Ls1");
        aryll_actor->staff_type = 0;
        std::shared_ptr<Actor> link = event.add_actor(event_list, "Link");
        link->staff_type = 0;

        const Prop eyeProp("Eye", vec3<float>{600.0f, -460.0f, -320.0f});
        const Prop centerProp("Center", vec3<float>{600.0f, -480.0f, -200.0f});
        const Prop fovyProp("Fovy", 60.0f);
        const Prop timerProp("Timer", 30);
        const std::vector<Prop> props{
            eyeProp,
            centerProp,
            fovyProp,
            timerProp
        };
        std::shared_ptr<Action> act = camera->add_action(event_list, "FIXEDFRM", props);
        act = aryll_actor->add_action(event_list, "LOK_PLYER", {Prop{"prm_0", 8}});
        act = aryll_actor->add_action(event_list, "ANM_CHG", {Prop{"AnmNo", 8}});
        act = aryll_actor->add_action(event_list, "WAIT", {Prop{"Timer", 30}});

        act = aryll_actor->add_action(event_list, "TALK_MSG", {Prop{"msg_num", 849}});
        act = aryll_actor->add_action(event_list, "ANM_CHG", {Prop{"AnmNo", 4}});

        act = aryll_actor->add_action(event_list, "TALK_MSG", {Prop{"msg_num", 850}});
        act = aryll_actor->add_action(event_list, "ANM_CHG", {Prop{"AnmNo", 5}});

        act = link->add_action(event_list, "001wait", {});

        event.ending_flags[0] = aryll_actor->actions.back()->flag_id_to_set;

        return true;
    });
    g_session.openGameFile("code/cking.rpx@RPX@ELF").addAction([=](RandoSession* session, FileType* data) -> int {
        CAST_ENTRY_TO_FILETYPE(elf, FileTypes::ELF, data)

        RPX_ERROR_CHECK(elfUtil::write_u8(elf, elfUtil::AddressToOffset(elf, 0x101BFFC4), 5));

        return true;
    });
    RandoSession::CacheEntry& shipStage = g_session.openGameFile("content/Common/Stage/Asoko_Stage.szs@YAZ0@SARC@Stage.bfres@BFRES@stage.dzs@DZX");
    shipStage.addAction([](RandoSession* session, FileType* data) -> int {
        CAST_ENTRY_TO_FILETYPE(shipDzs, FileTypes::DZXFile, data)

        ChunkEntry& new_evnt = shipDzs.add_entity("EVNT");
        new_evnt.data = "\xFF" "AryllOpensDoor\x00\xFF\xFF\x00\xFF\xFF\xFF\xFF\xFF"s;

        return true;
    });

    return TweakError::NONE;
}

struct CyclicWarpPotData {
    std::string stage_name;
    uint8_t room_num;
    float x, y, z;
    uint16_t y_rot;
    uint8_t event_reg_index;
};

static const std::array<std::array<CyclicWarpPotData, 3>, 2> loops = {{
    {{
        {"M_NewD2", 2, 2185.0f, 0.0f, 590.0f, 0xA000, 2},
        {"kindan", 1, 986.0f, 3956.43f, 9588.0f, 0xB929, 2},
        {"Siren", 6, 277.0f, 229.42f, -6669.0f, 0xC000, 2}
    }},
    {{
        {"ma2room", 2, 1556.0f, 728.46f, -7091.0f, 0xEAA6, 5},
        {"M_Dai", 1, -8010.0f, 1010.0f, -1610.0f, 0, 5},
        {"kaze", 3, -4333.0f, 1100.0f, 48.0f, 0x4000, 5}
    }}
}};

TweakError add_cross_dungeon_warps() {
    for(const auto& loop : loops) {
        uint8_t warp_index = 0;
        for (const CyclicWarpPotData& warp : loop) {
            RandoSession::CacheEntry& stage = g_session.openGameFile(getStageFilePath(warp.stage_name).concat("@YAZ0@SARC@Stage.bfres@BFRES@stage.dzs@DZX"));
            RandoSession::CacheEntry& room = g_session.openGameFile(getRoomDzrPath(warp.stage_name, warp.room_num));

            RandoSession::CacheEntry* dzx_for_spawn;
            dzx_for_spawn = &room;
            if(warp.stage_name == "M_Dai" || warp.stage_name == "kaze") {
                dzx_for_spawn = &stage;
            }

            const float pos_x = Utility::Endian::toPlatform(eType::Big, warp.x);
            const float pos_y = Utility::Endian::toPlatform(eType::Big, warp.y);
            const float pos_z = Utility::Endian::toPlatform(eType::Big, warp.z);
            const uint16_t y_rot = Utility::Endian::toPlatform(eType::Big, warp.y_rot);

            dzx_for_spawn->addAction([warp, pos_x, pos_y, pos_z, y_rot](RandoSession* session, FileType* data) -> int {
                CAST_ENTRY_TO_FILETYPE(dzx, FileTypes::DZXFile, data)

                ChunkEntry& spawn = dzx.add_entity("PLYR");
                spawn.data = "Link\x00\x00\x00\x00\xFF\xFF\x70"s;
                spawn.data.resize(0x20);
                spawn.data[0xB] = (spawn.data[0xB] & ~0x3F) | (warp.room_num & 0x3F);
                spawn.data.replace(0xC, 4, reinterpret_cast<const char*>(&pos_x), 4);
                spawn.data.replace(0x10, 4, reinterpret_cast<const char*>(&pos_y), 4);
                spawn.data.replace(0x14, 4, reinterpret_cast<const char*>(&pos_z), 4);
                spawn.data.replace(0x18, 2, "\x00\x00", 2);
                spawn.data.replace(0x1A, 2, reinterpret_cast<const char*>(&y_rot), 2);
                spawn.data.replace(0x1C, 4, "\xFF\x45\xFF\xFF", 4);

                std::vector<ChunkEntry*> spawns = dzx.entries_by_type("PLYR");
                std::vector<ChunkEntry*> spawn_id_69;
                for (ChunkEntry* spawn_to_check : spawns) {
                    if (std::strncmp(&spawn_to_check->data[0x1D], "\x45", 1) == 0) spawn_id_69.push_back(spawn_to_check);
                }
                if (spawn_id_69.size() != 1) LOG_ERR_AND_RETURN_BOOL(TweakError::MISSING_ENTITY); //technically too many and not missing, but its close enough with line number

                return true;
            });

            room.addAction([loop, warp, warp_index, pos_x, pos_y, pos_z, y_rot](RandoSession* session, FileType* data) -> int {
                CAST_ENTRY_TO_FILETYPE(room, FileTypes::DZXFile, data)

                std::vector<uint8_t> pot_index_to_exit;
                for (const CyclicWarpPotData& other_warp : loop) {
                    ChunkEntry& scls_exit = room.add_entity("SCLS");
                    std::string dest_stage = other_warp.stage_name;
                    dest_stage.resize(8, '\0');
                    scls_exit.data.resize(0xC);
                    scls_exit.data.replace(0, 8, dest_stage);
                    scls_exit.data.replace(8, 1, "\x45", 1);
                    scls_exit.data.replace(0x9, 1, reinterpret_cast<const char*>(&other_warp.room_num), 1);
                    scls_exit.data.replace(0xA, 2, "\x04\xFF", 2);
                    pot_index_to_exit.push_back(room.entries_by_type("SCLS").size() - 1);
                }

                uint32_t params = 0x00000000;
                params = (params & ~0xFF000000) | ((pot_index_to_exit[2] << 24) & 0xFF000000);
                params = (params & ~0x00FF0000) | ((pot_index_to_exit[1] << 16) & 0x00FF0000);
                params = (params & ~0x0000FF00) | ((pot_index_to_exit[0] << 8) & 0x0000FF00);
                params = (params & ~0x000000F0) | ((warp.event_reg_index << 4) & 0x000000F0);
                params = (params & ~0x0000000F) | ((warp_index + 2) & 0x0000000F);
                Utility::Endian::toPlatform_inplace(eType::Big, params);

                ChunkEntry& warp_pot = room.add_entity("ACTR");
                warp_pot.data = "Warpts" + std::to_string(warp_index + 1);
                warp_pot.data.resize(0x20);
                warp_pot.data.replace(0x8, 4, reinterpret_cast<const char*>(&params), 4);
                warp_pot.data.replace(0xC, 4, reinterpret_cast<const char*>(&pos_x), 4);
                warp_pot.data.replace(0x10, 4, reinterpret_cast<const char*>(&pos_y), 4);
                warp_pot.data.replace(0x14, 4, reinterpret_cast<const char*>(&pos_z), 4);
                warp_pot.data.replace(0x18, 2, "\xFF\xFF", 2);
                warp_pot.data.replace(0x1A, 2, reinterpret_cast<const char*>(&y_rot), 2);
                warp_pot.data.replace(0x1C, 4, "\xFF\xFF\xFF\xFF", 4);

                return true;
            });

            warp_index++;
        }
    }

    RandoSession::CacheEntry& drc = g_session.openGameFile("content/Common/Particle/Particle.szs@YAZ0@SARC@Particle.bfres@BFRES@Pscene035.jpc@JPC");
    RandoSession::CacheEntry& totg = g_session.openGameFile("content/Common/Particle/Particle.szs@YAZ0@SARC@Particle.bfres@BFRES@Pscene050.jpc@JPC");
    RandoSession::CacheEntry& ff = g_session.openGameFile("content/Common/Particle/Particle.szs@YAZ0@SARC@Particle.bfres@BFRES@Pscene043.jpc@JPC");

    drc.addAction([&totg, &ff](RandoSession* session, FileType* data) -> int {
        CAST_ENTRY_TO_FILETYPE(drc, FileTypes::JPC, data)

        for (const uint16_t particle_id : {0x8161, 0x8162, 0x8165, 0x8166, 0x8112}) {
            const Particle& particle = drc.particles[drc.particle_index_by_id[particle_id]];

            totg.addAction([particle](RandoSession* session, FileType* data) -> int {
                CAST_ENTRY_TO_FILETYPE(totg, FileTypes::JPC, data)

                FILETYPE_ERROR_CHECK(totg.addParticle(particle));

                return true;
            });
            ff.addAction([particle](RandoSession* session, FileType* data) -> int {
                CAST_ENTRY_TO_FILETYPE(ff, FileTypes::JPC, data)

                FILETYPE_ERROR_CHECK(ff.addParticle(particle));

                return true;
            });

            for (const std::string& textureFilename : particle.texDatabase.value().texFilenames) {
                totg.addAction([textureFilename](RandoSession* session, FileType* data) -> int {
                    CAST_ENTRY_TO_FILETYPE(totg, FileTypes::JPC, data)

                    if (totg.textures.find(textureFilename) == totg.textures.end()) {
                        FILETYPE_ERROR_CHECK(totg.addTexture(textureFilename));
                    }

                    return true;
                });
            
                ff.addAction([textureFilename](RandoSession* session, FileType* data) -> int {
                    CAST_ENTRY_TO_FILETYPE(ff, FileTypes::JPC, data)

                    if (ff.textures.find(textureFilename) == ff.textures.end()) {
                        FILETYPE_ERROR_CHECK(ff.addTexture(textureFilename));
                    }

                    return true;
                });
            }
        }

        return true;
    });
    
    drc.addDependent(totg.getParent()); //Want the file above the JPC entry
    drc.addDependent(ff.getParent()); //Want the file above the JPC entry

    return TweakError::NONE;
}

TweakError restore_cross_dungeon_warps() {
    for(const auto& loop : loops) {
        for (const CyclicWarpPotData& warp : loop) {
            if(const fspath path = getRoomFilePath(warp.stage_name, warp.room_num); !g_session.restoreGameFile(path)) {
                ErrorLog::getInstance().log("Failed to restore " + path.string() + '\n');
                return TweakError::FILE_OPEN_FAILED;
            }
            if(const fspath path = getStageFilePath(warp.stage_name); !g_session.restoreGameFile(path)) {
                ErrorLog::getInstance().log("Failed to restore " + path.string() + '\n');
                return TweakError::FILE_OPEN_FAILED;
            }
        }
    }

    // Particle.szs is restored elsewhere

    return TweakError::NONE;
}

// Add in return spawns for boss doors which don't have them
TweakError add_boss_door_return_spawns() {
    struct BossDoorSpawnData {
        std::string stage_name;
        uint8_t room_num;
        float x, y, z;
        uint16_t y_rot;
    };

    std::list<BossDoorSpawnData> newSpawns = {
        {"M_NewD2", 10,     400.0f,  5950.0f,    -450.0f, 0x0000},
        {"Siren",   18,   -2200.0f,  9245.0f,   -9775.0f, 0xC000},
        {"sea",      1, -302800.0f,  4544.0f, -304400.0f, 0xD555},
        {"kaze",    12,   13965.0f, -5060.0f,    9600.0f, 0x8000},
    };

    for (auto& newSpawn : newSpawns) {

        RandoSession::CacheEntry* dzx_for_spawn;
        if(isAnyOf(newSpawn.stage_name, "M_Dai", "kaze")) {
            dzx_for_spawn = &g_session.openGameFile(getStageFilePath(newSpawn.stage_name).concat("@YAZ0@SARC@Stage.bfres@BFRES@stage.dzs@DZX"));
        } else {
            dzx_for_spawn = &g_session.openGameFile(getRoomDzrPath(newSpawn.stage_name, newSpawn.room_num));
        }

        Utility::Endian::toPlatform_inplace(eType::Big, newSpawn.x);
        Utility::Endian::toPlatform_inplace(eType::Big, newSpawn.y);
        Utility::Endian::toPlatform_inplace(eType::Big, newSpawn.z);
        Utility::Endian::toPlatform_inplace(eType::Big, newSpawn.y_rot);

        dzx_for_spawn->addAction([newSpawn](RandoSession* session, FileType* data) -> int {
            CAST_ENTRY_TO_FILETYPE(dzx, FileTypes::DZXFile, data)

            ChunkEntry& spawn = dzx.add_entity("PLYR");
            spawn.data = "Link\x00\x00\x00\x00\xFF\xFF\x60"s;

            // Spawn type 6 doesn't work properly for the wind 
            // temple boss door for some reason, so use 0 instead
            // Also use 0 for Forsaken Fortress
            if (isAnyOf(newSpawn.stage_name, "kaze", "sea")) {
                spawn.data[0xA] = '\x00';
            }

            spawn.data.resize(0x20);
            spawn.data[0xB] = (spawn.data[0xB] & ~0x3F) | (newSpawn.room_num & 0x3F);
            spawn.data.replace(0xC, 4, reinterpret_cast<const char*>(&newSpawn.x), 4);
            spawn.data.replace(0x10, 4, reinterpret_cast<const char*>(&newSpawn.y), 4);
            spawn.data.replace(0x14, 4, reinterpret_cast<const char*>(&newSpawn.z), 4);
            spawn.data.replace(0x18, 2, "\x00\x00", 2);
            spawn.data.replace(0x1A, 2, reinterpret_cast<const char*>(&newSpawn.y_rot), 2);
            spawn.data.replace(0x1C, 4, "\xFF\x46\xFF\xFF", 4);
            if(newSpawn.stage_name == "sea") {
                spawn.data[0x1C] = '\x00'; //use ship spawn ID 0 if this exits to sea
            }
            else {
                spawn.data[0x1C] = '\xFF'; //otherwise don't do anything with KoRL
            }

            return true;
        });
    }
    return TweakError::NONE;
}

TweakError remove_makar_kidnapping() {
    RandoSession::CacheEntry& entry = g_session.openGameFile("content/Common/Stage/kaze_Room3.szs@YAZ0@SARC@Room3.bfres@BFRES@room.dzr@DZX");
    entry.addAction([](RandoSession* session, FileType* data) -> int {
        CAST_ENTRY_TO_FILETYPE(dzr, FileTypes::DZXFile, data)

        std::vector<ChunkEntry*> actors = dzr.entries_by_type("ACTR");

        ChunkEntry* switch_actor = nullptr; //initialization is just to make compiler happy
        for (ChunkEntry* actor : actors) {
            if (std::strncmp(&actor->data[0], "AND_SW2\x00", 8) == 0) switch_actor = actor;
        }
        if(switch_actor == nullptr) LOG_ERR_AND_RETURN_BOOL(TweakError::MISSING_ENTITY);
        dzr.remove_entity(switch_actor);

        for (ChunkEntry* actor : actors) {
            if (std::strncmp(&actor->data[0], "wiz_r\x00\x00\x00", 8) == 0) {
                actor->data.replace(0x9, 1, "\xFF", 1);
            }
        }

        return true;
    });

    return TweakError::NONE;
}

TweakError increase_crawl_speed() {
    //The 3.0 float crawling uses is shared with other things in HD, can't change it directly
    //Redirect both instances to load 6.0 from elsewhere
    g_session.openGameFile("code/cking.rpx@RPX@ELF").addAction([](RandoSession* session, FileType* data) -> int {
        CAST_ENTRY_TO_FILETYPE(elf, FileTypes::ELF, data)
        
        RPX_ERROR_CHECK(elfUtil::write_u32(elf, elfUtil::AddressToOffset(elf, 0x0014EC04, 7), 0x000355C4)); //update .rela.text entry
        RPX_ERROR_CHECK(elfUtil::write_u32(elf, elfUtil::AddressToOffset(elf, 0x0014EC4C, 7), 0x000355C4)); //update .rela.text entry

        return true;
    });
    
    return TweakError::NONE;
}

TweakError add_chart_number_to_item_get_messages(World& world) {

    std::unordered_map<std::string, size_t> replacementData = {
        {"English", 12},
        {"Spanish", 18},
        {"French", 17},
    };

    for (const auto& language : Text::supported_languages) {
        RandoSession::CacheEntry& entry = g_session.openGameFile("content/Common/Pack/permanent_2d_Us" + language + ".pack@SARC@message_msbt.szs@YAZ0@SARC@message.msbt@MSBT");

        auto replacementIndex = replacementData[language];

        for (uint8_t item_id = 0xCC; item_id < 0xFF; item_id++) {
            if (item_id == 0xDB || item_id == 0xDC) continue; // Skip ghost ship chart and tingle's chart

            // Get the properly formated item name
            auto itemName = gameItemToName(idToGameItem(item_id));
            const std::u16string u16itemName = world.getItem(itemName).getUTF16Name(language, Text::Type::PRETTY);

            // The message between the "You obtained " and the next '!' will be replaced
            entry.addAction([=](RandoSession* session, FileType* data) -> int {
                CAST_ENTRY_TO_FILETYPE(msbt, FileTypes::MSBTFile, data)
                
                auto& message = msbt.messages_by_label["00" + std::to_string(101 + item_id)].text.message;
                auto replacementLength = message.find('!') - replacementIndex;

                message.replace(replacementIndex, replacementLength, u16itemName);

                return true;
            });
        }
    }

    return TweakError::NONE;
}

TweakError increase_grapple_animation_speed() {
    g_session.openGameFile("code/cking.rpx@RPX@ELF").addAction([](RandoSession* session, FileType* data) -> int {
        CAST_ENTRY_TO_FILETYPE(elf, FileTypes::ELF, data)
        
        RPX_ERROR_CHECK(elfUtil::write_u32(elf, elfUtil::AddressToOffset(elf, 0x02170250), 0x394B000A));
        RPX_ERROR_CHECK(elfUtil::write_u32(elf, elfUtil::AddressToOffset(elf, 0x00075170, 7), 0x00010FFC)); //update .rela.text entry
        RPX_ERROR_CHECK(elfUtil::write_u32(elf, elfUtil::AddressToOffset(elf, 0x100110C8), 0x41C80000));
        RPX_ERROR_CHECK(elfUtil::write_u32(elf, elfUtil::AddressToOffset(elf, 0x021711D4), 0x390B0006));

        return true;
    });

    return TweakError::NONE;
}

TweakError increase_block_move_animation() {
    g_session.openGameFile("code/cking.rpx@RPX@ELF").addAction([](RandoSession* session, FileType* data) -> int {
        CAST_ENTRY_TO_FILETYPE(elf, FileTypes::ELF, data)
        
        RPX_ERROR_CHECK(elfUtil::write_u32(elf, elfUtil::AddressToOffset(elf, 0x00153b00, 7), 0x00035AAC)); //update .rela.text entries
        RPX_ERROR_CHECK(elfUtil::write_u32(elf, elfUtil::AddressToOffset(elf, 0x00153b48, 7), 0x00035AAC));

        uint32_t offset = 0x101CB424;
        for (unsigned int i = 0; i < 13; i++) { //13 types of blocks total
            RPX_ERROR_CHECK(elfUtil::write_u16(elf, elfUtil::AddressToOffset(elf, offset + 0x04), 0x000C)); // Reduce number frames for pushing to last from 20 to 12
            RPX_ERROR_CHECK(elfUtil::write_u16(elf, elfUtil::AddressToOffset(elf, offset + 0x0A), 0x000C)); // Reduce number frames for pulling to last from 20 to 12
            offset += 0x9C;
        }

        return true;
    });

    return TweakError::NONE;
}

TweakError increase_misc_animations() {
    //Float is shared, redirect it to read another float with the right value
    g_session.openGameFile("code/cking.rpx@RPX@ELF").addAction([](RandoSession* session, FileType* data) -> int {
        CAST_ENTRY_TO_FILETYPE(elf, FileTypes::ELF, data)
        
        RPX_ERROR_CHECK(elfUtil::write_u32(elf, elfUtil::AddressToOffset(elf, 0x00148820, 7), 0x000358D8));
        RPX_ERROR_CHECK(elfUtil::write_u32(elf, elfUtil::AddressToOffset(elf, 0x001482a4, 7), 0x00035124));
        RPX_ERROR_CHECK(elfUtil::write_u32(elf, elfUtil::AddressToOffset(elf, 0x00148430, 7), 0x000358D8));
        RPX_ERROR_CHECK(elfUtil::write_u32(elf, elfUtil::AddressToOffset(elf, 0x00148310, 7), 0x00035AAC));
        RPX_ERROR_CHECK(elfUtil::write_u32(elf, elfUtil::AddressToOffset(elf, 0x0014e2d4, 7), 0x00035530));

        RPX_ERROR_CHECK(elfUtil::write_u32(elf, elfUtil::AddressToOffset(elf, 0x02508b50), 0x3880000A));

        return true;
    });
    
    return TweakError::NONE;
}

TweakError set_casual_clothes() {
    if(custom_symbols.count("should_start_with_heros_clothes") == 0) LOG_ERR_AND_RETURN(TweakError::MISSING_SYMBOL);
    const uint32_t starting_clothes_addr = custom_symbols.at("should_start_with_heros_clothes");
    g_session.openGameFile("code/cking.rpx@RPX@ELF").addAction([=](RandoSession* session, FileType* data) -> int {
        CAST_ENTRY_TO_FILETYPE(elf, FileTypes::ELF, data)
        
        RPX_ERROR_CHECK(elfUtil::write_u8(elf, elfUtil::AddressToOffset(elf, starting_clothes_addr), 0));
    
        return true;
    });

    return TweakError::NONE;
}

TweakError hide_ship_sail() {
    g_session.openGameFile("code/cking.rpx@RPX@ELF").addAction([](RandoSession* session, FileType* data) -> int {
        CAST_ENTRY_TO_FILETYPE(elf, FileTypes::ELF, data)
        
        RPX_ERROR_CHECK(elfUtil::write_u32(elf, elfUtil::AddressToOffset(elf, 0x02162B04), 0x4E800020));

        return true;
    });

    return TweakError::NONE;
}

TweakError shorten_auction_intro_event() {
    RandoSession::CacheEntry& entry = g_session.openGameFile("content/Common/Stage/Orichh_Stage.szs@YAZ0@SARC@Stage.bfres@BFRES@event_list.dat@EVENTS");
    entry.addAction([](RandoSession* session, FileType* data) -> int {
        CAST_ENTRY_TO_FILETYPE(event_list, FileTypes::EventList, data)

        if(event_list.Events_By_Name.count("AUCTION_START") == 0) LOG_ERR_AND_RETURN_BOOL(TweakError::MISSING_EVENT);
        std::shared_ptr<Event> auction_start_event = event_list.Events_By_Name.at("AUCTION_START");
        std::shared_ptr<Actor> camera = auction_start_event->get_actor("CAMERA");
        if (camera == nullptr) {
            LOG_ERR_AND_RETURN_BOOL(TweakError::MISSING_EVENT);
        }

        camera->actions.erase(camera->actions.begin() + 3, camera->actions.begin() + 5); //last iterator not inclusive, only erase actions 3-4

        return true;
    });

    return TweakError::NONE;
}

TweakError disable_invisible_walls() {
    RandoSession::CacheEntry& entry = g_session.openGameFile("content/Common/Stage/M_NewD2_Room2.szs@YAZ0@SARC@Room2.bfres@BFRES@room.dzr@DZX");
    entry.addAction([](RandoSession* session, FileType* data) -> int {
        CAST_ENTRY_TO_FILETYPE(dzr, FileTypes::DZXFile, data)
        std::vector<ChunkEntry*> scobs = dzr.entries_by_type("SCOB");

        for (ChunkEntry* scob : scobs) {
            if (std::strncmp(&scob->data[0], "Akabe\x00\x00\x00", 8) == 0) {
                scob->data[0xB] = '\xFF';
            }
        }

        return true;
    });

    return TweakError::NONE;
}

TweakError update_skip_rematch_bosses_game_variable(const bool& skipRefights) {
    if(custom_symbols.count("skip_rematch_bosses") == 0) LOG_ERR_AND_RETURN(TweakError::MISSING_SYMBOL);
    const uint32_t skip_rematch_bosses_addr = custom_symbols.at("skip_rematch_bosses");

    RandoSession::CacheEntry& entry = g_session.openGameFile("code/cking.rpx@RPX@ELF");
    entry.addAction([=](RandoSession* session, FileType* data) -> int {
        CAST_ENTRY_TO_FILETYPE(elf, FileTypes::ELF, data)

        if (skipRefights) {
            RPX_ERROR_CHECK(elfUtil::write_u8(elf, elfUtil::AddressToOffset(elf, skip_rematch_bosses_addr), 0x01));
        }
        else {
            RPX_ERROR_CHECK(elfUtil::write_u8(elf, elfUtil::AddressToOffset(elf, skip_rematch_bosses_addr), 0x00));
        }

        return true;
    });

    return TweakError::NONE;
}

TweakError update_sword_mode_game_variable(const bool& remove_swords) {
    if(custom_symbols.count("swordless") == 0) LOG_ERR_AND_RETURN(TweakError::MISSING_SYMBOL);
    const uint32_t swordless_addr = custom_symbols.at("swordless");

    RandoSession::CacheEntry& entry = g_session.openGameFile("code/cking.rpx@RPX@ELF");
    entry.addAction([=](RandoSession* session, FileType* data) -> int {
        CAST_ENTRY_TO_FILETYPE(elf, FileTypes::ELF, data)

        if (remove_swords) {
            RPX_ERROR_CHECK(elfUtil::write_u8(elf, elfUtil::AddressToOffset(elf, swordless_addr), 0x01));
        }
        else {
            RPX_ERROR_CHECK(elfUtil::write_u8(elf, elfUtil::AddressToOffset(elf, swordless_addr), 0x00));
        }

        return true;
    });

    return TweakError::NONE;
}

TweakError update_starting_gear(const std::vector<GameItem>& startingItems) {
    std::vector<GameItem> startingGear = startingItems; //copy so we can edit without causing problems

    // Changing starting magic doesn't work when done via our normal starting items initialization code, so we need to handle it specially.
    LOG_AND_RETURN_IF_ERR(set_starting_magic(16 * std::count(startingGear.begin(), startingGear.end(), GameItem::ProgressiveMagicMeter)));
    auto it = std::find(startingGear.begin(), startingGear.end(), GameItem::ProgressiveMagicMeter);
    while (it != startingGear.end()) {
        startingGear.erase(it);
        it = std::find(startingGear.begin(), startingGear.end(), GameItem::ProgressiveMagicMeter);
    }

    if (startingGear.size() > MAXIMUM_ADDITIONAL_STARTING_ITEMS) {
        LOG_ERR_AND_RETURN(TweakError::UNEXPECTED_VALUE); //error
    }

    if(custom_symbols.count("starting_gear") == 0) LOG_ERR_AND_RETURN(TweakError::MISSING_SYMBOL);
    const uint32_t starting_gear_array_addr = custom_symbols.at("starting_gear");

    g_session.openGameFile("code/cking.rpx@RPX@ELF").addAction([=](RandoSession* session, FileType* data) -> int {
        CAST_ENTRY_TO_FILETYPE(elf, FileTypes::ELF, data)

        for (size_t i = 0; i < startingGear.size(); i++) {
            const uint8_t item_id = static_cast<std::underlying_type_t<GameItem>>(startingGear[i]);
            RPX_ERROR_CHECK(elfUtil::write_u8(elf, elfUtil::AddressToOffset(elf, starting_gear_array_addr + i), item_id));
        }

        RPX_ERROR_CHECK(elfUtil::write_u8(elf, elfUtil::AddressToOffset(elf, starting_gear_array_addr + startingGear.size()), 0xFF));

        return true;
    });
    
    return TweakError::NONE;
}

TweakError add_hint_signs() {
    for (const auto& language : Text::supported_languages) {
        RandoSession::CacheEntry& entry = g_session.openGameFile("content/Common/Pack/permanent_2d_Us" + language + ".pack@SARC@message_msbt.szs@YAZ0@SARC@message.msbt@MSBT");
    
        entry.addAction([](RandoSession* session, FileType* data) -> int {
            CAST_ENTRY_TO_FILETYPE(msbt, FileTypes::MSBTFile, data)

            const std::string new_message_label = "00847";
            Attributes attributes;
            attributes.character = 0xF; //sign
            attributes.boxStyle = 0x2;
            attributes.drawType = 0x1;
            attributes.screenPos = 0x2;
            attributes.lineAlignment = 3;
            TSY1Entry tsy;
            tsy.styleIndex = 0x12B;
            const std::u16string message = IMAGE(ImageTags::R_ARROW) + u"\0"s;
            msbt.addMessage(new_message_label, attributes, tsy, message);

            return true;
        });
    }

    RandoSession::CacheEntry& room = g_session.openGameFile("content/Common/Stage/M_NewD2_Room2.szs@YAZ0@SARC@Room2.bfres@BFRES@room.dzr@DZX");
    room.addAction([](RandoSession* session, FileType* data) -> int {
        CAST_ENTRY_TO_FILETYPE(dzr, FileTypes::DZXFile, data)

        std::vector<ChunkEntry*> actors = dzr.entries_by_type("ACTR");

        std::vector<ChunkEntry*> bomb_flowers;
        for (ChunkEntry* actor : actors) {
            if (std::strncmp(&actor->data[0], "BFlower\0", 8) == 0) bomb_flowers.push_back(actor);
        }
        bomb_flowers[1]->data = "Kanban\x00\x00\x00\x00\x03\x4F\x44\x34\x96\xEB\x42\x47\xFF\xFF\xC2\x40\xB0\x3A\x00\x00\x20\x00\x00\x00\xFF\xFF"s;

        return true;
    });

    return TweakError::NONE;
}

TweakError prevent_reverse_door_softlocks() {

    // Add switch triggers to remove boulders blocking DRC doors
    RandoSession::CacheEntry& drc_room13 = g_session.openGameFile("content/Common/Stage/M_NewD2_Room13.szs@YAZ0@SARC@Room13.bfres@BFRES@room.dzr@DZX");

    drc_room13.addAction([](RandoSession* session, FileType* data) -> int {
        CAST_ENTRY_TO_FILETYPE(drc_room13, FileTypes::DZXFile, data)

        ChunkEntry& swc00_13 = drc_room13.add_entity("SCOB");
        swc00_13.data = "SW_C00\x00\x00\x00\x03\xFF\x05\x45\x24\xB0\x00\x00\x00\x00\x00\x43\x63\x00\x00\x00\x00\xC0\x00\xFF\xFF\xFF\xFF\x20\x10\x10\xFF"s;

        return true;
    });

    RandoSession::CacheEntry& drc_room14 = g_session.openGameFile("content/Common/Stage/M_NewD2_Room14.szs@YAZ0@SARC@Room14.bfres@BFRES@room.dzr@DZX");
    
    drc_room14.addAction([](RandoSession* session, FileType* data) -> int {
        CAST_ENTRY_TO_FILETYPE(drc_room14, FileTypes::DZXFile, data)

        ChunkEntry& swc00_14 = drc_room14.add_entity("SCOB");
        swc00_14.data = "SW_C00\x00\x00\x00\x03\xFF\x06\xC5\x7A\x20\x00\x44\xF3\xC0\x00\xC5\x06\xC0\x00\x00\x00\xA0\x00\xFF\xFF\xFF\xFF\x20\x10\x10\xFF"s;

        return true;
    });

    // Add switch triggers to remove obstacles blocking ET doors if someone enters ET from the boss door loading zone
    // 1st song stone
    RandoSession::CacheEntry& et_room10 = g_session.openGameFile("content/Common/Stage/M_Dai_Room10.szs@YAZ0@SARC@Room10.bfres@BFRES@room.dzr@DZX");

    et_room10.addAction([](RandoSession* session, FileType* data) -> int {
        CAST_ENTRY_TO_FILETYPE(et_room10, FileTypes::DZXFile, data)

        ChunkEntry& swc00_10 = et_room10.add_entity("SCOB");
        swc00_10.data = "SW_C00\x00\x00\x00\x03\xFF\x45\x45\x9F\x05\x90\xC4\xA2\x80\x00\x45\x98\x15\xF3\x00\x00\x00\x00\x00\x00\xFF\xFF\x1E\x14\x0A\xFF"s;

        return true;
    });

    // Elephant statue 
    RandoSession::CacheEntry& et_room14 = g_session.openGameFile("content/Common/Stage/M_Dai_Room14.szs@YAZ0@SARC@Room14.bfres@BFRES@room.dzr@DZX");

    et_room14.addAction([](RandoSession* session, FileType* data) -> int {
        CAST_ENTRY_TO_FILETYPE(et_room14, FileTypes::DZXFile, data)

        ChunkEntry& swc00_14 = et_room14.add_entity("SCOB");
        swc00_14.data = "SW_C00\x00\x00\x00\x03\xFF\x52\x45\x54\x80\x00\xC4\x96\x00\x00\x46\x16\x00\x00\x00\x00\x00\x00\x00\x00\xFF\xFF\x1E\x14\x0A\xFF"s;

        return true;
    });

    // 2nd song stone
    RandoSession::CacheEntry& et_room15 = g_session.openGameFile("content/Common/Stage/M_Dai_Room15.szs@YAZ0@SARC@Room15.bfres@BFRES@room.dzr@DZX");

    et_room15.addAction([](RandoSession* session, FileType* data) -> int {
        CAST_ENTRY_TO_FILETYPE(et_room15, FileTypes::DZXFile, data)

        ChunkEntry& swc00_15 = et_room15.add_entity("SCOB");
        swc00_15.data = "SW_C00\x00\x00\x00\x03\xFF\x59\x44\x22\x80\x00\xC4\x8F\xC0\x00\x45\xF0\xA0\x00\x00\x00\x00\x00\x00\x00\xFF\xFF\x1E\x14\x0A\xFF"s;

        return true;
    });

    // Walls before Boss Key room incase players clip in early
    // This solves the whole puzzle, so it's a bit cheap and will leave out for now
    // RandoSession::CacheEntry& et_room17 = g_session.openGameFile("content/Common/Stage/M_Dai_Room17.szs@YAZ0@SARC@Room17.bfres@BFRES@room.dzr@DZX");

    // et_room17.addAction([](RandoSession* session, FileType* data) -> int {
    //     CAST_ENTRY_TO_FILETYPE(et_room17, FileTypes::DZXFile, data)

    //     ChunkEntry& swc00_17 = et_room17.add_entity("SCOB");
    //     swc00_17.data = "SW_C00\x00\x00\x00\x03\xFF\x62\xC5\x54\x85\x61\xC5\xB0\x22\x41\x44\x9F\x2A\xD7\x00\x00\x00\x00\x00\x00\xFF\xFF\x5F\xEC\x5F\x04"s;

    //     return true;
    // });

    // First Song Stone in Wind Temple if players clip past it
    RandoSession::CacheEntry& wt_room8 = g_session.openGameFile("content/Common/Stage/kaze_Room8.szs@YAZ0@SARC@Room8.bfres@BFRES@room.dzr@DZX");

    wt_room8.addAction([](RandoSession* session, FileType* data) -> int {
        CAST_ENTRY_TO_FILETYPE(wt_room8, FileTypes::DZXFile, data)

        ChunkEntry& swc00_8 = wt_room8.add_entity("SCOB");
        swc00_8.data = "SW_C00\x00\x00\x00\x03\xFF\x1E\x46\x13\xDB\x2A\x44\x85\x9F\x3A\xC0\xF3\x0B\xAC\x00\x00\x00\x00\x00\x00\xFF\xFF\x1E\x0A\x0A\xFF"s;

        return true;
    });

    return TweakError::NONE;
}

TweakError add_barren_dungeon_hint_triggers(World& world) {

    if (world.getSettings().progression_dungeons != ProgressionDungeons::RaceMode)
    {
        return TweakError::NONE;
    }

    for (auto& [name, dungeon] : world.dungeons)
    {
        if (dungeon.isRequiredDungeon)
        {
            continue;
        }

        else if (name == "Dragon Roost Cavern")
        {
            // Add barren hint text to DRC first room
            RandoSession::CacheEntry& drc_room0 = g_session.openGameFile("content/Common/Stage/M_NewD2_Room0.szs@YAZ0@SARC@Room0.bfres@BFRES@room.dzr@DZX");

            drc_room0.addAction([](RandoSession* session, FileType* data) -> int {
                CAST_ENTRY_TO_FILETYPE(drc_room0, FileTypes::DZXFile, data)

                ChunkEntry& tagHt_00 = drc_room0.add_entity("SCOB");
                tagHt_00.data = "TagHt\x00\x00\x00\xFF\xFF\xFF\x3F\xC4\x72\x20\xC4\x43\x16\x00\x00\x46\x08\xFF\x59\x05\xE5\x00\x00\xFF\xFF\xFF\xFF\x1E\x0A\x0A\xFF"s;

                return true;
            });

            // Add barren hint text in front of DRC boss door
            RandoSession::CacheEntry& drc_room10 = g_session.openGameFile("content/Common/Stage/M_NewD2_Room10.szs@YAZ0@SARC@Room10.bfres@BFRES@room.dzr@DZX");

            drc_room10.addAction([](RandoSession* session, FileType* data) -> int {
                CAST_ENTRY_TO_FILETYPE(drc_room10, FileTypes::DZXFile, data)

                ChunkEntry& tagHt_10 = drc_room10.add_entity("SCOB");
                tagHt_10.data = "TagHt\x00\x00\x00\xFF\xFF\xFF\x3F\x43\xCC\x83\x95\x45\xB9\xF0\x00\x41\xA8\x8A\x60\x05\xE5\x00\x00\xFF\xFF\xFF\xFF\x1E\x0A\x0A\xFF"s;

                return true;
            });
        }

        else if (name == "Forbidden Woods")
        {
            // Add barren hint text to FW first room
            RandoSession::CacheEntry& fw_room0 = g_session.openGameFile("content/Common/Stage/kindan_Room0.szs@YAZ0@SARC@Room0.bfres@BFRES@room.dzr@DZX");

            fw_room0.addAction([](RandoSession* session, FileType* data) -> int {
                CAST_ENTRY_TO_FILETYPE(fw_room0, FileTypes::DZXFile, data)

                ChunkEntry& tagHt_00 = fw_room0.add_entity("SCOB");
                tagHt_00.data = "TagHt\x00\x00\x00\xFF\xFF\xFF\x3F\x40\x81\xE1\xC0\x45\x16\x00\x00\x46\x6F\x52\xEF\x05\xE5\x00\x00\xFF\xFF\xFF\xFF\x1E\x0A\x0A\xFF"s;

                return true;
            });

            // Add barren hint text to FW room before miniboss
            RandoSession::CacheEntry& fw_room9 = g_session.openGameFile("content/Common/Stage/kindan_Room9.szs@YAZ0@SARC@Room9.bfres@BFRES@room.dzr@DZX");

            fw_room9.addAction([](RandoSession* session, FileType* data) -> int {
                CAST_ENTRY_TO_FILETYPE(fw_room9, FileTypes::DZXFile, data)

                ChunkEntry& tagHt_09 = fw_room9.add_entity("SCOB");
                tagHt_09.data = "TagHt\x00\x00\x00\xFF\xFF\xFF\x3F\xC3\x96\x5D\xF1\x45\xE8\xC2\x41\xC6\x0D\x6D\x3D\x05\xE5\x00\x00\xFF\xFF\xFF\xFF\x1E\x0A\x0A\xFF"s;

                return true;
            });

            // Add barren hint text to FW boss door room
            RandoSession::CacheEntry& fw_room16 = g_session.openGameFile("content/Common/Stage/kindan_Room16.szs@YAZ0@SARC@Room16.bfres@BFRES@room.dzr@DZX");

            fw_room16.addAction([](RandoSession* session, FileType* data) -> int {
                CAST_ENTRY_TO_FILETYPE(fw_room16, FileTypes::DZXFile, data)

                ChunkEntry& tagHt_16 = fw_room16.add_entity("SCOB");
                tagHt_16.data = "TagHt\x00\x00\x00\xFF\xFF\xFF\x3F\x46\x06\x49\x3E\x45\x8A\x5B\xAA\xC4\xB8\x46\x34\x05\xE5\x00\x00\xFF\xFF\xFF\xFF\x1E\x0A\x0A\xFF"s;

                return true;
            });
        }

        else if (name == "Tower of the Gods")
        {
            // Add barren hint texts to TotG Start
            RandoSession::CacheEntry& totg_room0 = g_session.openGameFile("content/Common/Stage/Siren_Room0.szs@YAZ0@SARC@Room0.bfres@BFRES@room.dzr@DZX");

            totg_room0.addAction([](RandoSession* session, FileType* data) -> int {
                CAST_ENTRY_TO_FILETYPE(totg_room0, FileTypes::DZXFile, data)

                // Multiple triggers all around the room (one giant trigger doesn't work for some reason)
                auto datas = {
                    "TagHt\x00\x00\x00\xFF\xFF\xFF\x3F\x45\x48\xCB\x26\xC4\xCA\x23\x9A\xC5\x22\xDA\x18\x05\xE5\x00\x00\xFF\xFF\xFF\xFF\x1E\x0A\x0A\xFF"s,
                    "TagHt\x00\x00\x00\xFF\xFF\xFF\x3F\x45\x49\x4A\x6F\xC4\xC2\x73\x68\xC5\x02\xAA\x6C\x05\xE5\x00\x00\xFF\xFF\xFF\xFF\x1E\x0A\x0A\xFF"s,
                    "TagHt\x00\x00\x00\xFF\xFF\xFF\x3F\x45\x49\x4A\x6F\xC4\xC2\x73\x68\xC4\xC4\x43\x35\x05\xE5\x00\x00\xFF\xFF\xFF\xFF\x1E\x0A\x0A\xFF"s,
                    "TagHt\x00\x00\x00\xFF\xFF\xFF\x3F\x45\x32\x0B\x7D\xC4\xC2\x77\x51\xC4\x9E\xE3\x2F\x05\xE5\x00\x00\xFF\xFF\xFF\xFF\x1E\x0A\x0A\xFF"s,
                    "TagHt\x00\x00\x00\xFF\xFF\xFF\x3F\x45\x60\x7B\x07\xC5\x00\x32\x6E\x44\x64\xC2\x9A\x05\xE5\x00\x00\xFF\xFF\xFF\xFF\x1E\x0A\x0A\xFF"s,
                    "TagHt\x00\x00\x00\xFF\xFF\xFF\x3F\xC5\x4D\xB4\x38\xC4\xBF\x71\x0C\xC4\x81\xD0\x5C\x05\xE5\x00\x00\xFF\xFF\xFF\xFF\x1E\x0A\x0A\xFF"s,
                    "TagHt\x00\x00\x00\xFF\xFF\xFF\x3F\xC5\x62\x87\xFE\xC4\xFC\xCB\x3B\x44\x38\x0E\x3F\x05\xE5\x00\x00\xFF\xFF\xFF\xFF\x28\x0A\x0A\xFF"s,
                };

                for (auto& tagHtData : datas)
                {
                    ChunkEntry& tagHt = totg_room0.add_entity("SCOB");
                    tagHt.data = tagHtData;
                }

                return true;
            });

            // Add barren hint text to TotG Outside Miniboss Room
            RandoSession::CacheEntry& totg_room14 = g_session.openGameFile("content/Common/Stage/Siren_Room14.szs@YAZ0@SARC@Room14.bfres@BFRES@room.dzr@DZX");

            totg_room14.addAction([](RandoSession* session, FileType* data) -> int {
                CAST_ENTRY_TO_FILETYPE(totg_room14, FileTypes::DZXFile, data)

                ChunkEntry& tagHt_14 = totg_room14.add_entity("SCOB");
                tagHt_14.data = "TagHt\x00\x00\x00\xFF\xFF\xFF\x3F\xC5\x5A\xF7\x98\x43\x6B\x6B\xB2\xC5\xE5\x65\xDF\x05\xE5\x00\x00\xFF\xFF\xFF\xFF\x1E\x0A\x0A\xFF"s;

                return true;
            });

            // Add barren hint text to TotG Outside Boss Room
            RandoSession::CacheEntry& totg_room18 = g_session.openGameFile("content/Common/Stage/Siren_Room18.szs@YAZ0@SARC@Room18.bfres@BFRES@room.dzr@DZX");

            totg_room18.addAction([](RandoSession* session, FileType* data) -> int {
                CAST_ENTRY_TO_FILETYPE(totg_room18, FileTypes::DZXFile, data)

                ChunkEntry& tagHt_18 = totg_room18.add_entity("SCOB");
                tagHt_18.data = "TagHt\x00\x00\x00\xFF\xFF\xFF\x3F\xC5\x16\x56\x78\x46\x11\x35\xA3\xC6\x18\xCF\xD1\x05\xE5\x00\x00\xFF\xFF\xFF\xFF\x1E\x0A\x0A\xFF"s;

                return true;
            });
        }

        else if (name == "Earth Temple")
        {
            // Add barren hint text to ET first room
            RandoSession::CacheEntry& et_room0 = g_session.openGameFile("content/Common/Stage/M_Dai_Room0.szs@YAZ0@SARC@Room0.bfres@BFRES@room.dzr@DZX");

            et_room0.addAction([](RandoSession* session, FileType* data) -> int {
                CAST_ENTRY_TO_FILETYPE(et_room0, FileTypes::DZXFile, data)

                ChunkEntry& tagHt_00 = et_room0.add_entity("SCOB");
                tagHt_00.data = "TagHt\x00\x00\x00\xFF\xFF\xFF\x3F\xC5\xDA\xC0\x00\xC3\x15\xFF\xFF\x45\xB2\x20\x00\x05\xE5\x00\x00\xFF\xFF\xFF\xFF\x1E\x0A\x0A\xFF"s;

                return true;
            });

            // Add barren hint text to ET outside miniboss room
            RandoSession::CacheEntry& et_room7 = g_session.openGameFile("content/Common/Stage/M_Dai_Room7.szs@YAZ0@SARC@Room7.bfres@BFRES@room.dzr@DZX");

            et_room7.addAction([](RandoSession* session, FileType* data) -> int {
                CAST_ENTRY_TO_FILETYPE(et_room7, FileTypes::DZXFile, data)

                ChunkEntry& tagHt_00 = et_room7.add_entity("SCOB");
                tagHt_00.data = "TagHt\x00\x00\x00\xFF\xFF\xFF\x3F\x45\xC0\x3D\x58\x44\xAF\x00\x00\x45\x17\x10\xD7\x05\xE5\x00\x00\xFF\xFF\xFF\xFF\x1E\x0A\x0A\xFF"s;

                return true;
            });

            // Add barren hint text to ET outside boss room
            RandoSession::CacheEntry& et_room15 = g_session.openGameFile("content/Common/Stage/M_Dai_Room15.szs@YAZ0@SARC@Room15.bfres@BFRES@room.dzr@DZX");

            et_room15.addAction([](RandoSession* session, FileType* data) -> int {
                CAST_ENTRY_TO_FILETYPE(et_room15, FileTypes::DZXFile, data)

                ChunkEntry& tagHt_00 = et_room15.add_entity("SCOB");
                tagHt_00.data = "TagHt\x00\x00\x00\xFF\xFF\xFF\x3F\xC5\xB4\xDD\xE4\xC5\x0F\x93\x3D\x46\x14\x8F\x16\x05\xE5\x00\x00\xFF\xFF\xFF\xFF\x1E\x0A\x0A\xFF"s;

                return true;
            });
        }

        else if (name == "Wind Temple")
        {
            // Add barren hint text to WT first room
            RandoSession::CacheEntry& wt_room15 = g_session.openGameFile("content/Common/Stage/kaze_Room15.szs@YAZ0@SARC@Room15.bfres@BFRES@room.dzr@DZX");

            wt_room15.addAction([](RandoSession* session, FileType* data) -> int {
                CAST_ENTRY_TO_FILETYPE(wt_room15, FileTypes::DZXFile, data)

                ChunkEntry& tagHt_15 = wt_room15.add_entity("SCOB");
                tagHt_15.data = "TagHt\x00\x00\x00\xFF\xFF\xFF\x3F\xC5\x6F\x04\x20\x44\xC2\xB5\x4F\x46\x51\x6B\xEB\x05\xE5\x00\x00\xFF\xFF\xFF\xFF\x1E\x0A\x0A\xFF"s;

                return true;
            });

            // Add barren hint text to WT outside miniboss room
            RandoSession::CacheEntry& wt_room2 = g_session.openGameFile("content/Common/Stage/kaze_Room2.szs@YAZ0@SARC@Room2.bfres@BFRES@room.dzr@DZX");

            wt_room2.addAction([](RandoSession* session, FileType* data) -> int {
                CAST_ENTRY_TO_FILETYPE(wt_room2, FileTypes::DZXFile, data)

                ChunkEntry& tagHt_02 = wt_room2.add_entity("SCOB");
                tagHt_02.data = "TagHt\x00\x00\x00\xFF\xFF\xFF\x3F\x3F\x2E\xF5\x80\xC5\x61\x00\x00\xC4\xE2\x0D\x42\x05\xE5\x00\x00\xFF\xFF\xFF\xFF\x1E\x0A\x0A\xFF"s;

                return true;
            });

            // Add barren hint text to WT outside boss room
            RandoSession::CacheEntry& wt_room12 = g_session.openGameFile("content/Common/Stage/kaze_Room12.szs@YAZ0@SARC@Room12.bfres@BFRES@room.dzr@DZX");

            wt_room12.addAction([](RandoSession* session, FileType* data) -> int {
                CAST_ENTRY_TO_FILETYPE(wt_room12, FileTypes::DZXFile, data)

                ChunkEntry& tagHt_12 = wt_room12.add_entity("SCOB");
                tagHt_12.data = "TagHt\x00\x00\x00\xFF\xFF\xFF\x3F\x46\x5A\x4F\xEB\xC5\x9E\x33\xE4\x46\x15\xEB\x03\x05\xE5\x00\x00\xFF\xFF\xFF\xFF\x1E\x0A\x0A\xFF"s;

                return true;
            });
        }
    }

    return TweakError::NONE;
}

TweakError update_tingle_statue_item_get_funcs() {
    const uint32_t item_get_func_ptr = 0x0001DA54; //First relevant relocation entry in .rela.data (overwrites .data section when loaded)
    const std::unordered_map<int, std::string> symbol_name_by_item_id = { {0xA3, "dragon_tingle_statue_item_get_func"}, {0xA4, "forbidden_tingle_statue_item_get_func"}, {0xA5, "goddess_tingle_statue_item_get_func"}, {0xA6, "earth_tingle_statue_item_get_func"}, {0xA7, "wind_tingle_statue_item_get_func"} };
    RandoSession::CacheEntry& rpx = g_session.openGameFile("code/cking.rpx@RPX@ELF");

    for (const uint8_t statue_id : {0xA3, 0xA4, 0xA5, 0xA6, 0xA7}) {
        if(custom_symbols.count(symbol_name_by_item_id.at(statue_id)) == 0) LOG_ERR_AND_RETURN(TweakError::MISSING_SYMBOL);

        const uint32_t item_func_addr = item_get_func_ptr + (statue_id * 0xC) + 8;
        const uint32_t item_func_ptr = custom_symbols.at(symbol_name_by_item_id.at(statue_id));
        
        rpx.addAction([=](RandoSession* session, FileType* data) -> int {
            CAST_ENTRY_TO_FILETYPE(elf, FileTypes::ELF, data)

            RPX_ERROR_CHECK(elfUtil::write_u32(elf, elfUtil::AddressToOffset(elf, item_func_addr, 9), item_func_ptr - 0x02000000));

            return true;
        });
    }

    return TweakError::NONE;
}

TweakError make_tingle_statue_reward_rupee_rainbow_colored() {
    const uint32_t item_resources_list_start = 0x101e4674;
    const uint32_t rainbow_rupee_item_resource_addr = item_resources_list_start + 0xB8 * 0x24;

    g_session.openGameFile("code/cking.rpx@RPX@ELF").addAction([=](RandoSession* session, FileType* data) -> int {
        CAST_ENTRY_TO_FILETYPE(elf, FileTypes::ELF, data)

        RPX_ERROR_CHECK(elfUtil::write_u8(elf, elfUtil::AddressToOffset(elf, rainbow_rupee_item_resource_addr + 0x14), 0x07));

        return true;
    });

    return TweakError::NONE;
}

TweakError show_seed_hash_on_title_screen(const std::u16string& hash) { //make sure hash is null terminated
    using namespace NintendoWare::Layout;

    RandoSession::CacheEntry& entry = g_session.openGameFile("content/Common/Layout/Title_00.szs@YAZ0@SARC@blyt/Title_00.bflyt@BFLYT");
    
    //add hash
    entry.addAction([hash](RandoSession* sessio, FileType* data) -> int {
        CAST_ENTRY_TO_FILETYPE(layout, FileTypes::FLYTFile, data)

        Pane& newPane = layout.rootPane.children[0].children[1].children[3].duplicateChildPane(1); //hidden version number text
        txt1& textPane = *dynamic_cast<txt1*>(newPane.pane.get());
        textPane.name = "T_Hash";
        textPane.name.resize(0x18);
        textPane.text = u"Seed Hash:\n" + hash;
        textPane.fontIndex = 0;
        textPane.restrictedLen = (12 + hash.length()) * 2;
        textPane.lineAlignment = txt1::LineAlignment::CENTER;
        textPane.translation.X = -491.0f;
        textPane.translation.Y = -113.0f;
        textPane.width = 205.0f;
        textPane.height = 100.0f;

        return true;
    });
    
    return TweakError::NONE;
}

TweakError implement_key_bag() {
    // Add a "unit" string for the big key
    const std::unordered_map<std::string, std::u16string> messages = {
        {"English", u" +Big\0"s},
        {"Spanish", u" +Jefe\0"s},
        {"French", u" +Boss\0"s},
    };

    for (const auto& language : Text::supported_languages) {
        RandoSession::CacheEntry& charm = g_session.openGameFile("content/Common/Pack/permanent_2d_Us" + language + ".pack@SARC@BtnCollectIcon_00.szs@YAZ0@SARC@timg/CollectIcon118_08^l.bflim@BFLIM");
        
        charm.addAction([](RandoSession* session, FileType* data) -> int {
            CAST_ENTRY_TO_FILETYPE(pirates_charm, FileTypes::FLIMFile, data)

            FILETYPE_ERROR_CHECK(pirates_charm.replaceWithDDS(Utility::get_data_path() / "assets/KeyBag.dds", GX2TileMode::GX2_TILE_MODE_DEFAULT, 0, true));

            return true;
        });

        RandoSession::CacheEntry& units = g_session.openGameFile("content/Common/Pack/permanent_2d_Us" + language + ".pack@SARC@unitString_msbt.szs@YAZ0@SARC@unitString.msbt@MSBT");
        units.addAction([messages, language](RandoSession* session, FileType* data) -> int {
            CAST_ENTRY_TO_FILETYPE(msbt, FileTypes::MSBTFile, data)

            const Message& to_copy = msbt.messages_by_label["Unit_Rupee_00"];
            msbt.addMessage("Unit_Key_00", to_copy.attributes, to_copy.style, u"\0"s);
            msbt.addMessage("Unit_Key_01", to_copy.attributes, to_copy.style, messages.at(language));

            return true;
        });
    }

    return TweakError::NONE;
}

TweakError show_dungeon_markers_on_chart(World& world) {
    using namespace NintendoWare::Layout;

    std::unordered_multiset<uint8_t> room_numbers;
    for(const auto& [name, dungeon] : world.dungeons) {
        if (dungeon.isRequiredDungeon)
        {
            // Get island of associated boss room
            auto raceModeArea = dungeon.raceModeLocation->accessPoints.front()->area;
            auto bossIslands = raceModeArea->findIslands();

            const std::string& islandName = bossIslands.front();
            room_numbers.emplace(islandNameToRoomNum(islandName));
        }
    }

    for (const auto& language : Text::supported_languages) {
        RandoSession::CacheEntry& map = g_session.openGameFile("content/Common/Pack/permanent_2d_Us" + language + ".pack@SARC@Map_00.szs@YAZ0@SARC@blyt/Map_00.bflyt@BFLYT");

        map.addAction([room_numbers](RandoSession* session, FileType* data) -> int {
            CAST_ENTRY_TO_FILETYPE(map, FileTypes::FLYTFile, data)

            std::vector<size_t> quest_marker_indexes = {
                144, 145, 146, 147, 148, 149, 150, 151
            };

            std::set<uint8_t> used_room_numbers = {};

            // Index of L_BtnMapBalloon_00. Currently the last element in the vector of children.
            // Save it now so that we can swap this pane to the back of the vector after we potentially add some more panes.
            // If this is not done, then any panes we add will be layered over the balloon element when players zoom in on 
            // an island.
            auto balloonIndex = map.rootPane.children[0].children.size() - 1;

            for(const uint8_t& index : room_numbers) {
                if (used_room_numbers.contains(index))
                {
                    continue;
                }
                used_room_numbers.insert(index);

                const uint32_t column = (index - 1) % 7;
                const uint32_t row = (index - 1) / 7;
                float x_pos = (column * 73.0f) - 148.0f;
                float y_pos = 175.0f - (row * 73.0f);

                pan1* marker = dynamic_cast<pan1*>(map.rootPane.children[0].children[quest_marker_indexes.back()].pane.get());
                quest_marker_indexes.pop_back();
                marker->translation.X = x_pos;
                marker->translation.Y = y_pos;

                // Only display the number of required dungeons on the sector if there's more than 1
                auto number_of_dungeons = room_numbers.count(index);
                if (number_of_dungeons <= 1)
                {
                    continue;
                }
                
                // X and Y for new pane displaying the number of requried dungeons
                x_pos = (column * 73.0f) - 155.0f;
                y_pos = 156.0f - (row * 73.0f);

                // Create the new pane element by duplicating the one existing text element
                Pane& newPane = map.rootPane.children[0].duplicateChildPane(1); // T_Deco_00
                txt1* text = dynamic_cast<txt1*>(newPane.pane.get());
                text->name = "Required_Dungeon_Text_" + std::to_string(index);
                text->translation.X = x_pos;
                text->translation.Y = y_pos;
                text->width = 40.0f;
                text->height = 40.0f;
                text->text = Utility::Str::toUTF16(std::to_string(number_of_dungeons)) + TEXT_END;
                text->fontSizeX = 40.0f;
                text->fontSizeY = 40.0f;
                text->fontColorTop = RGBA8(255, 255, 255, 255);
                text->fontColorBottom = RGBA8(255, 255, 255, 255);
                text->alpha = 200;
                
                // Set the material index to be the current size of the vector of materials since we're going to create
                // a new one for these numbers
                text->matIndex = map.materials.value().materials.size();
            }

            // Swap the L_BtnMapBalloon pane to the back of the vector
            std::swap(map.rootPane.children[0].children[balloonIndex], map.rootPane.children[0].children.back());

            // Hide any remaining markers
            for(const auto& index : quest_marker_indexes) { 
                pan1* marker = dynamic_cast<pan1*>(map.rootPane.children[0].children[index].pane.get());
                marker->alpha = 0;
            }
            
            // Create the new material for the number of required dungeons text by copying the material for the markers
            auto newMat = map.materials.value().materials.back();

            // blackColor is the outline color for our text. whiteColor is currently the blue used for the map markers, so
            // set this blue as the new outline color.
            newMat.blackColor = newMat.whiteColor;
            // set whiteColor to white so that the inside of the text is white.
            newMat.whiteColor = RGBA8(255, 255, 255, 255);
            map.materials.value().materials.push_back(newMat);

            return true;
        });
    }

    return TweakError::NONE;
}

TweakError add_chest_in_place_jabun_cutscene() {
    RandoSession::CacheEntry& entry = g_session.openGameFile("content/Common/Stage/Pjavdou_Room0.szs@YAZ0@SARC@Room0.bfres@BFRES@room.dzr");
    entry.addAction([](RandoSession* session, FileType* data) -> int {
        CAST_ENTRY_TO_FILETYPE(generic, RawFile, data) //do this on the stream so it happens before location mod

        FileTypes::DZXFile dzr;
        LOG_AND_RETURN_BOOL_IF_ERR(dzr.loadFromBinary(generic.data));

        ChunkEntry& raft = dzr.add_entity("ACTR");
        ChunkEntry& chest = dzr.add_entity("TRES");
        raft.data = "Ikada\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x80\x00\x00\x00\xFF\xFF"s;
        chest.data = "takara3\x00\xFF\x2F\xF3\x05\x00\x00\x00\x00\x43\x96\x00\x00\xC3\x48\x00\x00\x00\x00\x80\x00\x05\xFF\xFF\xFF"s;

        LOG_AND_RETURN_BOOL_IF_ERR(dzr.writeToStream(generic.data));

        return true;
    });

    return TweakError::NONE;
}

TweakError add_jabun_obstacles_to_default_layer() {
    RandoSession::CacheEntry& entry = g_session.openGameFile("content/Common/Pack/szs_permanent2.pack@SARC@sea_Room44.szs@YAZ0@SARC@Room44.bfres@BFRES@room.dzr@DZX");
    entry.addAction([](RandoSession* session, FileType* data) -> int {
        CAST_ENTRY_TO_FILETYPE(dzr, FileTypes::DZXFile, data)

        std::vector<ChunkEntry*> layer_5_actors = dzr.entries_by_type_and_layer("ACTR", 5);
        const std::string layer_5_door_data = layer_5_actors[0]->data;
        const std::string layer_5_whirlpool_data = layer_5_actors[1]->data;

        dzr.remove_entity(layer_5_actors[0]);
        dzr.remove_entity(layer_5_actors[1]);

        ChunkEntry& newDoor = dzr.add_entity("ACTR");
        ChunkEntry& newWhirlpool = dzr.add_entity("ACTR");
        newDoor.data = layer_5_door_data;
        newWhirlpool.data = layer_5_whirlpool_data;

        return true;
    });

    return TweakError::NONE;
}

TweakError remove_jabun_stone_door_event() {
    RandoSession::CacheEntry& entry = g_session.openGameFile("content/Common/Pack/first_szs_permanent.pack@SARC@sea_Stage.szs@YAZ0@SARC@Stage.bfres@BFRES@event_list.dat@EVENTS");
    entry.addAction([](RandoSession* session, FileType* data) -> int {
        CAST_ENTRY_TO_FILETYPE(event_list, FileTypes::EventList, data)
        
        if(event_list.Events_By_Name.count("ajav_uzu") == 0) LOG_ERR_AND_RETURN_BOOL(TweakError::MISSING_EVENT);
        std::shared_ptr<Event> unlock_cave_event = event_list.Events_By_Name.at("ajav_uzu");
        std::shared_ptr<Actor> director = unlock_cave_event->get_actor("DIRECTOR");
        if (director == nullptr) {
            LOG_ERR_AND_RETURN_BOOL(TweakError::MISSING_EVENT);
        }
        std::shared_ptr<Actor> camera = unlock_cave_event->get_actor("CAMERA");
        if (camera == nullptr) {
            LOG_ERR_AND_RETURN_BOOL(TweakError::MISSING_EVENT);
        }
        std::shared_ptr<Actor> ship = unlock_cave_event->get_actor("Ship");
        if (ship == nullptr) {
            LOG_ERR_AND_RETURN_BOOL(TweakError::MISSING_EVENT);
        }

        director->actions.erase(director->actions.begin() + 1, director->actions.end());
        camera->actions.erase(camera->actions.begin() + 2, camera->actions.end());
        ship->actions.erase(ship->actions.begin() + 2, ship->actions.end());
        unlock_cave_event->ending_flags = {
            director->actions.back()->flag_id_to_set,
            camera->actions.back()->flag_id_to_set,
            -1
        };

        return true;
    });

    return TweakError::NONE;
}

TweakError add_chest_in_place_master_sword() {
    RandoSession::CacheEntry& entry = g_session.openGameFile("content/Common/Stage/kenroom_Room0.szs@YAZ0@SARC@Room0.bfres@BFRES@room.dzr");
    entry.addAction([](RandoSession* session, FileType* data) -> int {
        CAST_ENTRY_TO_FILETYPE(generic, RawFile, data) //do this on the stream so it happens before location mod

        FileTypes::DZXFile dzr;
        LOG_AND_RETURN_BOOL_IF_ERR(dzr.loadFromBinary(generic.data));

        const std::vector<ChunkEntry*> default_layer_actors = dzr.entries_by_type_and_layer("ACTR", DEFAULT_LAYER);
        dzr.remove_entity(default_layer_actors[5]);
        dzr.remove_entity(default_layer_actors[6]);

        const std::vector<ChunkEntry*> layer_5_actors = dzr.entries_by_type_and_layer("ACTR", 5);
        const std::array<ChunkEntry*, 4> layer_5_to_copy = { layer_5_actors[0], layer_5_actors[2], layer_5_actors[3], layer_5_actors[4] };

        for (ChunkEntry* orig_actor : layer_5_to_copy) {
            ChunkEntry& new_actor = dzr.add_entity("ACTR");
            new_actor.data = orig_actor->data;
            dzr.remove_entity(orig_actor);
        }

        ChunkEntry& chest = dzr.add_entity("TRES");
        chest.data = "takara3\x00\xFF\x20\x50\x04\xc2\xf6\xfd\x71\xc5\x49\x40\x00\xc5\xf4\xe9\x0a\x00\x00\x00\x00\x6a\xff\xff\xff"s;

        const std::vector<ChunkEntry*> spawns = dzr.entries_by_type("PLYR");
        for (ChunkEntry* spawn : spawns) {
            if (spawn->data[29] == '\x0A') {
                spawn->data.replace(0x14, 4, "\xC5\x84\x85\x9A", 4);
                spawn->data.replace(0x10, 4, "\xC5\x38\x56\x3D", 4);
                break;
            }
        }

        LOG_AND_RETURN_BOOL_IF_ERR(dzr.writeToStream(generic.data));

        return true;
    });

    return TweakError::NONE;
}

TweakError fix_totg_warp_spawn() {
    RandoSession::CacheEntry& entry = g_session.openGameFile("content/Common/Stage/sea_Room26.szs@YAZ0@SARC@Room26.bfres@BFRES@room.dzr@DZX");
    entry.addAction([](RandoSession* session, FileType* data) -> int {
        CAST_ENTRY_TO_FILETYPE(dzr, FileTypes::DZXFile, data)

        const std::vector<ChunkEntry*> spawns = dzr.entries_by_type("PLYR");
        ChunkEntry* spawn = spawns[9];
        spawn->data = "\x4C\x69\x6E\x6B\x00\x00\x00\x00\x32\xFF\x20\x1A\x47\xC3\x4F\x5F\x00\x00\x00\x00\x44\x7A\x00\x00\x00\x00\x00\x00\x01\x01\xFF\xFF"s;

        return true;
    });

    return TweakError::NONE;
}

TweakError remove_phantom_ganon_req_for_reefs() {
    for (const uint8_t room_num : {24, 46, 22, 8, 37, 25}) {
        const fspath path = getRoomDzrPath("sea", room_num);
        RandoSession::CacheEntry& entry = g_session.openGameFile(path);
        
        entry.addAction([](RandoSession* session, FileType* data) -> int {
            CAST_ENTRY_TO_FILETYPE(room_dzr, FileTypes::DZXFile, data)

            const std::vector<ChunkEntry*> actors = room_dzr.entries_by_type("ACTR");
            for (ChunkEntry* actor : actors) {
                if (std::strncmp(&actor->data[0], "Ocanon\x00\x00", 8) == 0) {
                    if (std::strncmp(&actor->data[0xA], "\x2A", 1) == 0) {
                        actor->data[0xA] = '\xFF';
                    }
                }
                else if (std::strncmp(&actor->data[0], "Oship\x00\x00\x00", 8) == 0) {
                    if (std::strncmp(&actor->data[0x19], "\x2A", 1) == 0) {
                        actor->data[0x19] = '\xFF';
                    }
                }
            }

            return true;
        });
    }

    return TweakError::NONE;
}

TweakError fix_ff_door() {
    RandoSession::CacheEntry& entry = g_session.openGameFile("content/Common/Pack/szs_permanent1.pack@SARC@sea_Room1.szs@YAZ0@SARC@Room1.bfres@BFRES@room.dzb");
    entry.addAction([](RandoSession* session, FileType* data) -> int {
        static constexpr int32_t face_index = 0x1493;
        static constexpr uint16_t new_prop_index = 0x0011;

        CAST_ENTRY_TO_FILETYPE(file, RawFile, data)
        std::stringstream& stream = file.data;

        stream.seekg(0xC, std::ios::beg);
        
        uint32_t face_list_offset;
        stream.read(reinterpret_cast<char*>(&face_list_offset), 4);
        Utility::Endian::toPlatform_inplace(eType::Big, face_list_offset);

        stream.seekp((face_list_offset + face_index * 0xA) + 6, std::ios::beg);
        stream.write(reinterpret_cast<const char*>(&new_prop_index), 2);

        return true;
    });

    return TweakError::NONE;
}

//rat hole culling

//not needed until enemy rando is a thing
/*TweakError add_failsafe_id_0_spawns() {
    struct spawn_data {
        std::string stage_name;
        uint8_t room_num = 0;
    };
    
    struct spawn_data_to_copy {
        std::string stage_name;
        uint8_t room_num = 0;
        uint8_t spawn_id_to_copy = 0;
    };

    const std::array<spawn_data_to_copy, 32> spawns_to_copy{
    {
        {"Asoko", 0, 255},
        {"I_TestM", 0, 1},
        {"M_Dai", 20, 23},
        {"M_NewD2", 1, 20},
        {"M_NewD2", 2, 1},
        {"M_NewD2", 3, 6},
        {"M_NewD2", 4, 7},
        {"M_NewD2", 6, 9},
        {"M_NewD2", 8, 14},
        {"M_NewD2", 11, 2},
        {"M_NewD2", 12, 3},
        {"M_NewD2", 13, 4},
        {"M_NewD2", 14, 5},
        {"M_NewD2", 15, 18},
        {"TF_06", 1, 1},
        {"TF_06", 2, 2},
        {"TF_06", 3, 2},
        {"TF_06", 4, 2},
        {"TF_06", 5, 2},
        {"TF_06", 6, 6},
        {"ma2room", 1, 2},
        {"ma2room", 2, 15}, // Front door
        {"ma2room", 3, 9}, // In the water
        {"ma2room", 4, 6},
        {"ma3room", 1, 2},
        {"ma3room", 2, 15}, // Front door
        {"ma3room", 3, 9}, // In the water
        {"ma3room", 4, 6},
        {"majroom", 1, 2},
        {"majroom", 2, 15}, // Front door
        {"majroom", 3, 9}, // In the water
        {"majroom", 4, 6}
    }
    };

    const std::array<spawn_data, 32> spawns_to_create{
    {
        {"TF_01", 1},
        {"TF_01", 2},
        {"TF_01", 3},
        {"TF_01", 4},
        {"TF_01", 5},
        {"TF_01", 6},
        {"TF_02", 1},
        {"TF_02", 2},
        {"TF_02", 3},
        {"TF_02", 4},
        {"TF_02", 5},
        {"TF_02", 6}
    }
    };



    for (const spawn_data_to_copy& spawn_info : spawns_to_copy) {
        std::string path = "content/Common/Stage/" + spawn_info.stage_name + "_Room" + std::to_string(spawn_info.room_num) + ".szs@YAZ0@SARC@Room" + std::to_string(spawn_info.room_num) + ".bfres@BFRES@room.dzr";
        RandoSession::fspath filePath = g_session.openGameFile(path);
        FileTypes::DZXFile room_dzr;
        room_dzr.loadFromBinary(filePath);

        std::vector<ChunkEntry*> spawns = room_dzr.entries_by_type("PLYR");

        for (ChunkEntry* spawn : spawns) {
            if (spawn->data[0x1D] == (char)spawn_info.spawn_id_to_copy) {
                ChunkEntry& new_spawn = room_dzr.add_entity("PLYR");
                new_spawn.data = spawn->data;
                new_spawn.data[0x1D] = '\x00';
            }
        }

        room_dzr.writeToStream(filePath);
    }

    for (const spawn_data& spawn_info : spawns_to_create) {
        std::string path = "content/Common/Stage/" + spawn_info.stage_name + "_Room" + std::to_string(spawn_info.room_num) + ".szs@YAZ0@SARC@Room" + std::to_string(spawn_info.room_num) + ".bfres@BFRES@room.dzr";
        RandoSession::fspath filePath = g_session.openGameFile(path);
        FileTypes::DZXFile room_dzr;
        room_dzr.loadFromBinary(filePath);

        std::vector<ChunkEntry*> doors = room_dzr.entries_by_type("TGDR");

        float spawn_dist_from_door = 200.0f;
        float x_pos = 0.0f;
        float y_pos = 0.0f;
        float z_pos = 0.0f;
        uint16_t y_rot = 0;
        for (const ChunkEntry* door : doors) {
            if (((*(uint16_t*)&door->data[0x18]) & 0x0FC0) == spawn_info.room_num || ((*(uint16_t*)&door->data[0x18]) & 0x003F) == spawn_info.room_num) {
                y_rot = *(uint16_t*)&door->data[0x1A];
                if (((*(uint16_t*)&door->data[0x18]) & 0x003F) != spawn_info.room_num) {
                    y_rot = (y_rot + 0x8000) % 0x10000;
                }

                int y_rot_degrees = y_rot * (90.0 / 0x4000);
                float x_offset = sin((y_rot_degrees * M_PI) / 180.0) * spawn_dist_from_door;
                float z_offset = cos((y_rot_degrees * M_PI) / 180.0) * spawn_dist_from_door;

                float door_x_pos = Utility::Endian::toPlatform(eType::Big, *(float*)&door->data[0xC]);
                float door_y_pos = Utility::Endian::toPlatform(eType::Big, *(float*)&door->data[0x10]);
                float door_z_pos = Utility::Endian::toPlatform(eType::Big, *(float*)&door->data[0x14]);
                x_pos = door_x_pos + x_offset;
                y_pos = door_y_pos;
                z_pos = door_z_pos + z_offset;
                break;
            }
        }

        ChunkEntry& newSpawn = room_dzr.add_entity("PLYR");
        newSpawn.data = "Link\x00\x00\x00\x00"s;
        newSpawn.data.resize(0x20);

        uint32_t params = 0xFFFFFFFF;
        //https://github.com/LagoLunatic/wwrando/blob/master/tweaks.py#L2160
    }
}*/

TweakError remove_minor_pan_cs() {
    struct pan_cs_info {
        std::string stage_name;
        int roomNum;
        uint8_t evnt_index;
    };

    const std::array<pan_cs_info, 7> panning_cs{
        {
            {"M_NewD2", 2, 4},
            {"kindan", -1, 2},
            {"Siren", 18, 2},
            {"M_Dai", 3, 7},
            {"sea", 41, 19},
            {"sea", 41, 22},
            {"sea", 41, 23}
        }
    };

    for (const pan_cs_info& cs_info : panning_cs) {
        fspath path;
        if(cs_info.roomNum != -1) {
            path = getRoomDzrPath(cs_info.stage_name, cs_info.roomNum);
        }
        else {
            path = getStageFilePath(cs_info.stage_name).concat("@YAZ0@SARC@Stage.bfres@BFRES@stage.dzs@DZX");
        }

        RandoSession::CacheEntry& entry = g_session.openGameFile(path);
        entry.addAction([cs_info](RandoSession* session, FileType* data) -> int {
            CAST_ENTRY_TO_FILETYPE(dzx, FileTypes::DZXFile, data)

            std::vector<ChunkEntry*> scobs = dzx.entries_by_type("SCOB");
            for (ChunkEntry* scob : scobs) {
                if (std::strncmp(&scob->data[0], "TagEv\x00\x00\x00", 8) == 0) {
                    if (scob->data[0x8] == cs_info.evnt_index) {
                        dzx.remove_entity(scob);
                    }
                }
            }

            std::vector<ChunkEntry*> spawns = dzx.entries_by_type("PLYR");
            for (ChunkEntry* spawn : spawns) {
                if (spawn->data[8] == cs_info.evnt_index) {
                    spawn->data[8] = '\xFF';
                }
            }

            return true;
        });
    }

    return TweakError::NONE;
}

TweakError fix_stone_head_bugs() {
    g_session.openGameFile("code/cking.rpx@RPX@ELF").addAction([](RandoSession* session, FileType* data) -> int {
        CAST_ENTRY_TO_FILETYPE(elf, FileTypes::ELF, data)

        uint32_t status_bits = elfUtil::read_u32(elf, elfUtil::AddressToOffset(elf, 0x101ca100));
        Utility::Endian::toPlatform_inplace(eType::Big, status_bits);
        status_bits &= ~0x00000080;
        RPX_ERROR_CHECK(elfUtil::write_u32(elf, elfUtil::AddressToOffset(elf, 0x101ca100), status_bits)); //write function handles byteswap back

        return true;
    });
    
    return TweakError::NONE;
}

TweakError show_tingle_statues_on_quest_screen() {
    for (std::string language : Text::supported_languages) {
        RandoSession::CacheEntry& icon = g_session.openGameFile("content/Common/Pack/permanent_2d_Us" + language + ".pack@SARC@BtnMapIcon_00.szs@YAZ0@SARC@timg/MapBtn_00^l.bflim@BFLIM");
        icon.addAction([](RandoSession* session, FileType* data) -> int {
            CAST_ENTRY_TO_FILETYPE(tingle, FileTypes::FLIMFile, data)
            FILETYPE_ERROR_CHECK(tingle.replaceWithDDS(Utility::get_data_path() / "assets/Tingle.dds", GX2TileMode::GX2_TILE_MODE_DEFAULT, 0, true));

            return true;
        });

        RandoSession::CacheEntry& shadow = g_session.openGameFile("content/Common/Pack/permanent_2d_Us" + language + ".pack@SARC@BtnMapIcon_00.szs@YAZ0@SARC@timg/MapBtn_07^t.bflim@BFLIM");
        shadow.addAction([](RandoSession* session, FileType* data) -> int {
            CAST_ENTRY_TO_FILETYPE(shadow, FileTypes::FLIMFile, data)
            FILETYPE_ERROR_CHECK(shadow.replaceWithDDS(Utility::get_data_path() / "assets/TingleShadow.dds", GX2TileMode::GX2_TILE_MODE_DEFAULT, 0, false));

            return true;
        });
    }

    return TweakError::NONE;
}

TweakError add_shortcut_warps_into_dungeons() {
    RandoSession::CacheEntry& entry = g_session.openGameFile("content/Common/Pack/szs_permanent2.pack@SARC@sea_Room41.szs@YAZ0@SARC@Room41.bfres@BFRES@room.dzr@DZX");
    entry.addAction([](RandoSession* session, FileType* data) -> int {
        CAST_ENTRY_TO_FILETYPE(dzr, FileTypes::DZXFile, data)

        ChunkEntry& sw_c00 = dzr.add_entity("SCOB");
        sw_c00.data = "SW_C00\x00\x00\x00\x03\xFF\x7F\x48\x40\x24\xED\x45\x44\x99\xB1\x48\x41\x7B\x63\x00\x00\x00\x00\x00\x00\xFF\xFF\x96\x14\x28\xFF"s;

        ChunkEntry& warp = dzr.add_entity("SCOB");
        warp.data = "Ysdls00\x00\x10\xFF\x06\x7F\x48\x54\x16\x86\x42\x0B\xFF\xF8\x48\x3E\xD3\xED\x00\x00\x00\x00\x00\x00\xFF\xFF\x0A\x0A\x0A\xFF"s;

        return true;
    });

    return TweakError::NONE;
}

TweakError update_entrance_events() {
    //Some entrances have event triggers with hardcoded stages rather than a load zone with a SCLS entry
    //Update those edge cases based on the SCLS entries in their respective dzr

    RandoSession::CacheEntry& dzr = g_session.openGameFile("content/Common/Pack/szs_permanent2.pack@SARC@sea_Room41.szs@YAZ0@SARC@Room41.bfres@BFRES@room.dzr@DZX");
    RandoSession::CacheEntry& entry = g_session.openGameFile("content/Common/Pack/first_szs_permanent.pack@SARC@sea_Stage.szs@YAZ0@SARC@Stage.bfres@BFRES@event_list.dat@EVENTS");
    
    dzr.addAction([&entry](RandoSession* session, FileType* data) -> int {
        CAST_ENTRY_TO_FILETYPE(dzr, FileTypes::DZXFile, data)

        const ChunkEntry* waterfall = dzr.entries_by_type("SCLS")[7];
        //ChunkEntry* gallery = dzr.entries_by_type("SCLS")[8]; // not currently randomized

        entry.addAction([waterfall = *waterfall](RandoSession* session, FileType* data) -> int {
            CAST_ENTRY_TO_FILETYPE(event_list, FileTypes::EventList, data)

            if(event_list.Events_By_Name.count("fall") == 0) LOG_ERR_AND_RETURN_BOOL(TweakError::MISSING_EVENT);
            std::shared_ptr<Action> loadRoom = event_list.Events_By_Name.at("fall")->get_actor("DIRECTOR")->actions[1];
            if(loadRoom == nullptr) LOG_ERR_AND_RETURN_BOOL(TweakError::MISSING_EVENT);
            loadRoom->get_prop("Stage")->value = waterfall.data.substr(0, 8);
            std::get<std::vector<int32_t>>(loadRoom->get_prop("StartCode")->value)[0] = waterfall.data[8]; // spawn ID
            std::get<std::vector<int32_t>>(loadRoom->get_prop("RoomNo")->value)[0] = waterfall.data[9];

            // The Nintendo Gallery entrance isn't currently randomized, this can patch it if it is ever shuffled
            //std::shared_ptr<Action> next2 = event_list.Events_By_Name.at("nitendo")->get_actor("DIRECTOR")->actions[1];
            //next2->get_prop("Stage")->value = gallery.data.substr(0, 8);
            //std::get<std::vector<int32_t>>(next2->get_prop("StartCode")->value)[0] = gallery.data[8]; // spawn ID
            //std::get<std::vector<int32_t>>(next2->get_prop("RoomNo")->value)[0] = gallery.data[9];

            return true;
        });

        return true;
    });

    dzr.addDependent(entry.getRoot());

    return TweakError::NONE;
}

TweakError fix_entrance_params() {
    //Some entrances have params that cause issues when they're randomized

    //set our custom param for the crawlspaces so they always have Link crawling
    RandoSession::CacheEntry& link_ug = g_session.openGameFile("content/Common/Stage/LinkUG_Room0.szs@YAZ0@SARC@Room0.bfres@BFRES@room.dzr@DZX");
    link_ug.addAction([](RandoSession* session, FileType* data) -> int {
        CAST_ENTRY_TO_FILETYPE(dzr, FileTypes::DZXFile, data)

        const std::vector<ChunkEntry*> spawns = dzr.entries_by_type("PLYR");
        spawns[1]->data.data()[0x19] |= 0x01; //last byte of X rotation

        return true;
    });

    RandoSession::CacheEntry& outset_exit = g_session.openGameFile("content/Common/Pack/szs_permanent2.pack@SARC@sea_Room44.szs@YAZ0@SARC@Room44.bfres@BFRES@room.dzr@DZX");
    outset_exit.addAction([](RandoSession* session, FileType* data) -> int {
        CAST_ENTRY_TO_FILETYPE(dzr, FileTypes::DZXFile, data)

        const std::vector<ChunkEntry*> spawns = dzr.entries_by_type("PLYR");
        spawns[23]->data.data()[0x19] |= 0x01;

        return true;
    });

    RandoSession::CacheEntry& bomb_shop = g_session.openGameFile("content/Common/Stage/Obombh_Room0.szs@YAZ0@SARC@Room0.bfres@BFRES@room.dzr@DZX");
    bomb_shop.addAction([](RandoSession* session, FileType* data) -> int {
        CAST_ENTRY_TO_FILETYPE(dzr, FileTypes::DZXFile, data)

        const std::vector<ChunkEntry*> spawns = dzr.entries_by_type("PLYR");
        spawns[1]->data.data()[0x19] |= 0x01;

        // also update the crawlspace exit to have a different fade type
        // for some reason fades 4 and 5 break sometimes
        const std::vector<ChunkEntry*> exits = dzr.entries_by_type("SCLS");
        exits[2]->data.data()[0xA] = 9;

        return true;
    });

    RandoSession::CacheEntry& windfall_exit = g_session.openGameFile("content/Common/Pack/szs_permanent1.pack@SARC@sea_Room11.szs@YAZ0@SARC@Room11.bfres@BFRES@room.dzr@DZX");
    windfall_exit.addAction([](RandoSession* session, FileType* data) -> int {
        CAST_ENTRY_TO_FILETYPE(dzr, FileTypes::DZXFile, data)

        const std::vector<ChunkEntry*> spawns = dzr.entries_by_type("PLYR");
        spawns[2]->data.data()[0x19] |= 0x01;

        // also update the crawlspace exit to have a different fade type
        // for some reason fades 4 and 5 break sometimes
        const std::vector<ChunkEntry*> exits = dzr.entries_by_type("SCLS");
        exits[2]->data.data()[0xA] = 0;

        return true;
    });

    return TweakError::NONE;
}

TweakError replace_ctmc_chest_texture() {
    RandoSession::CacheEntry& entry = g_session.openGameFile("content/Common/Pack/permanent_3d.pack@SARC@Dalways.szs@YAZ0@SARC@Dalways.bfres@BFRES");
    entry.addAction([](RandoSession* session, FileType* data) -> int {
        CAST_ENTRY_TO_FILETYPE(bfres, FileTypes::resFile, data)

        FILETYPE_ERROR_CHECK(bfres.textures[3].replaceImageData(Utility::get_data_path() / "assets/KeyChest.dds", GX2TileMode::GX2_TILE_MODE_TILED_2D_THIN1, 0, true, true));

        return true;
    });

    return TweakError::NONE;
}

TweakError apply_ingame_preferences(const Settings& settings) {
    if(custom_symbols.count("target_type_preference") == 0) LOG_ERR_AND_RETURN(TweakError::MISSING_SYMBOL);
    if(custom_symbols.count("camera_preference") == 0) LOG_ERR_AND_RETURN(TweakError::MISSING_SYMBOL);
    if(custom_symbols.count("first_person_camera_preference") == 0) LOG_ERR_AND_RETURN(TweakError::MISSING_SYMBOL);
    if(custom_symbols.count("gyroscope_preference") == 0) LOG_ERR_AND_RETURN(TweakError::MISSING_SYMBOL);
    if(custom_symbols.count("ui_display_preference") == 0) LOG_ERR_AND_RETURN(TweakError::MISSING_SYMBOL);

    const uint32_t target_type_preference_addr = custom_symbols.at("target_type_preference");
    const uint32_t camera_preference_addr = custom_symbols.at("camera_preference");
    const uint32_t first_person_camera_preference_addr = custom_symbols.at("first_person_camera_preference");
    const uint32_t gyroscope_preference_addr = custom_symbols.at("gyroscope_preference");
    const uint32_t ui_display_preference_addr = custom_symbols.at("ui_display_preference");

    RandoSession::CacheEntry& entry = g_session.openGameFile("code/cking.rpx@RPX@ELF");
    entry.addAction([=](RandoSession* session, FileType* data) -> int {
        CAST_ENTRY_TO_FILETYPE(elf, FileTypes::ELF, data);

        RPX_ERROR_CHECK(elfUtil::write_u8(elf, elfUtil::AddressToOffset(elf, target_type_preference_addr), static_cast<std::underlying_type_t<TargetTypePreference>>(settings.target_type)));
        RPX_ERROR_CHECK(elfUtil::write_u8(elf, elfUtil::AddressToOffset(elf, camera_preference_addr), static_cast<std::underlying_type_t<CameraPreference>>(settings.camera)));
        RPX_ERROR_CHECK(elfUtil::write_u8(elf, elfUtil::AddressToOffset(elf, first_person_camera_preference_addr), static_cast<std::underlying_type_t<FirstPersonCameraPreference>>(settings.first_person_camera)));
        RPX_ERROR_CHECK(elfUtil::write_u8(elf, elfUtil::AddressToOffset(elf, gyroscope_preference_addr), static_cast<std::underlying_type_t<GyroscopePreference>>(settings.gyroscope)));
        RPX_ERROR_CHECK(elfUtil::write_u8(elf, elfUtil::AddressToOffset(elf, ui_display_preference_addr), static_cast<std::underlying_type_t<UIDisplayPreference>>(settings.ui_display)));

        return true;
    });

    return TweakError::NONE;
}

TweakError updateCodeSize() {
    //Increase the max codesize in cos.xml to load all our code
    RandoSession::CacheEntry& cosEntry = g_session.openGameFile("code/cos.xml");
    cosEntry.addAction([](RandoSession* session, FileType* data) -> int {
        CAST_ENTRY_TO_FILETYPE(generic, RawFile, data)\
        std::stringstream& cosStream = generic.data;

        tinyxml2::XMLDocument cos;
        cos.Parse(cosStream.str().c_str(), cosStream.str().size());
        tinyxml2::XMLElement* root = cos.RootElement();
        root->FirstChildElement("max_codesize")->SetText("02080000");
        tinyxml2::XMLPrinter printer;
        cos.Print(&printer);
        cosStream.str(printer.CStr());

        return true;
    });

    //Also update the RPL info section of the RPX
    //Change the textSize and loadSize to be large enough for the new code/relocations
    g_session.openGameFile("code/cking.rpx@RPX@ELF").addAction([](RandoSession* session, FileType* data) -> int {
        CAST_ENTRY_TO_FILETYPE(elf, FileTypes::ELF, data)
        
        RPX_ERROR_CHECK(elfUtil::write_u32(elf, elfUtil::AddressToOffset(elf, 0x00000004, 32), 0x00909510));
        RPX_ERROR_CHECK(elfUtil::write_u32(elf, elfUtil::AddressToOffset(elf, 0x0000001C, 32), 0x00379000));

        return true;
    });
    
    return TweakError::NONE;
}


TweakError fix_needle_rock_island_salvage_flags() {
    // Salvage flags 0 and 1 for Needle Rock Island are each duplicated in the vanilla game.
    // There are two light ring salvages, using flags 0 and 1.
    // There are three gunboat salvages, using flags 0, 1, and 2. 2 is for the golden gunboat.
    // This causes a bug where you can't get all of these sunken treasures if you salvage the light
    // rings first, or if you salvage the gunboats first and then reload the room.
    // So we have to change the flags used by the two light ring salvages so that they don't conflict
    // with the two non-golden gunboat salvages.
  
    RandoSession::CacheEntry& entry = g_session.openGameFile("content/Common/Stage/sea_Room29.szs@YAZ0@SARC@Room29.bfres@BFRES@room.dzr@DZX");
    entry.addAction([](RandoSession* session, FileType* data) -> int {
        CAST_ENTRY_TO_FILETYPE(dzr, FileTypes::DZXFile, data)
        std::vector<ChunkEntry*> salvages;
        static const std::unordered_set<std::string> salvage_object_names = {
            "Salvage\0"s,
            "SwSlvg\0\0"s,
            "Salvag2\0"s,
            "SalvagN\0"s,
            "SalvagE\0"s,
            "SalvFM\0\0"s
        };
        static const std::unordered_set<uint8_t> types = {2, 3, 4};
        static const std::unordered_set<uint8_t> flag = {0, 1};

        std::vector<ChunkEntry*> scobs = dzr.entries_by_type("SCOB");
        for (ChunkEntry* scob : scobs) {
            if (salvage_object_names.count(scob->data.substr(0, 8)) > 0 && types.count((scob->data[8] & 0xF0) >> 4) > 0 && flag.count((scob->data[8] & 0x0F) << 4 | (scob->data[9] & 0xF0) >> 4) > 0) {
                salvages.push_back(scob);
            }
        }

        salvages[0]->data[9] = (0x08 << 4) | (salvages[0]->data[9] & ~0xF0);
        salvages[1]->data[9] = (0x09 << 4) | (salvages[1]->data[9] & ~0xF0);

        return true;
    });
    return TweakError::NONE;
}

TweakError add_ff_warp_button() {
    using namespace NintendoWare::Layout;

    for (const auto& language : Text::supported_languages) {
        RandoSession::CacheEntry& map = g_session.openGameFile("content/Common/Pack/permanent_2d_Us" + language + ".pack@SARC@WarpMap_00.szs@YAZ0@SARC@blyt/WarpMap_00.bflyt@BFLYT");

        //add another WarpArea pane
        map.addAction([](RandoSession* sessio, FileType* data) -> int {
            CAST_ENTRY_TO_FILETYPE(layout, FileTypes::FLYTFile, data)

            Pane& newPane = layout.rootPane.children[0].duplicateChildPane(4); //L_WarpArea_00
            prt1& partPane = *dynamic_cast<prt1*>(newPane.pane.get());
            partPane.name = "L_WarpArea_09";
            partPane.translation.X = -158.0f;
            partPane.translation.Y = 198.0f;

            return true;
        });

        
        RandoSession::CacheEntry& text = g_session.openGameFile("content/Common/Pack/permanent_2d_Us" + language + ".pack@SARC@message_msbt.szs@YAZ0@SARC@message.msbt@MSBT");
    
        text.addAction([](RandoSession* session, FileType* data) -> int {
            CAST_ENTRY_TO_FILETYPE(msbt, FileTypes::MSBTFile, data)

            const Message& to_copy = msbt.messages_by_label["00075"];
            msbt.addMessage("00076", to_copy.attributes, to_copy.style, u"");

            return true;
        });
    
    }
    
    return TweakError::NONE;
}

TweakError fix_vanilla_text() {
    //The spanish text for the 99 quiver says that you can hold up to 99 bombs
    RandoSession::CacheEntry& text = g_session.openGameFile("content/Common/Pack/permanent_2d_UsSpanish.pack@SARC@message_msbt.szs@YAZ0@SARC@message.msbt@MSBT");
    text.addAction([](RandoSession* session, FileType* data) -> int {
        CAST_ENTRY_TO_FILETYPE(msbt, FileTypes::MSBTFile, data)

        std::u16string& message = msbt.messages_by_label["00277"].text.message;
        message.replace(message.find(u"bombas"), 6, u"flechas", 7);

        return true;
    });
    
    return TweakError::NONE;
}

TweakError allow_nonlinear_servants_of_the_towers() {
    // Allow the sections of Tower of the Gods where you bring three Servants of the Tower into the hub room to be done nonlinearly, so you can return the servants in any order.
    // We change it so the Command Melody tablet appears when any one of the three servants is returned (originally it would only appear when returning the east servant).
    // We also change the final warp upwards to appear only after all three servants have been returned, *and* the item from the Command Melody tablet has been obtained (since that tablet would softlock the game if it was still there when you try to enter the warp).
    // However, the various events for the servants being returned do not behave well with these modifications. So we will need to substantially edit these events.
    RandoSession::CacheEntry& totg = g_session.openGameFile("content/Common/Stage/Siren_Stage.szs@YAZ0@SARC@Stage.bfres@BFRES@stage.dzs@DZX");
    RandoSession::CacheEntry& list = g_session.openGameFile("content/Common/Stage/Siren_Stage.szs@YAZ0@SARC@Stage.bfres@BFRES@event_list.dat@EVENTS");
    RandoSession::CacheEntry& hub_room = g_session.openGameFile("content/Common/Stage/Siren_Room7.szs@YAZ0@SARC@Room7.bfres@BFRES@room.dzr@DZX");
    
    totg.addAction([](RandoSession* session, FileType* data) -> int {
        CAST_ENTRY_TO_FILETYPE(dzs, FileTypes::DZXFile, data)

        std::vector<ChunkEntry*> doors = dzs.entries_by_type("TGDR");
        ChunkEntry* north_door = doors[6];
        ChunkEntry* west_door = doors[8];

        // Remove the open condition switches from the doors, making them unlocked from the start.
        north_door->data[0xB] = 0xFF;
        west_door->data[0xB] = 0xFF;
        
        ChunkEntry& tablet_appear_evnt = dzs.add_entity("EVNT");
        tablet_appear_evnt.data = "\xFF" "hsehi1_appear\x00\x00\xFF\xFF\x00\xFF\xFF\xFF\xFF\xFF"s;

        return true;
    });

    // Note: In vanilla, 0x29 was not set directly by the east servant.
    // Instead, the east servant's event caused the tablet to appear, and then after getting
    // the Command Melody from the tablet, the tablet would set switch 0x29.
    // We change the east servant to work like the others, and directly set the switch.
    //const uint8_t east_servant_returned_switch = 0x29;
    //const uint8_t west_servant_returned_switch = 0x2A;
    //const uint8_t north_servant_returned_switch = 0x28;

    // These switches should be unused in vanilla TotG.
    const uint8_t tablet_item_obtained_switch = 0x2B; // Must be contiguous with 0x28-0x2A.
    //const uint8_t any_servant_returned_switch = 0x7E;
    const uint8_t all_servants_returned_switch = 0x7F;

    const uint8_t original_all_servants_returned_switch = 0x28;
    
    // In vanilla, the tablet and the east servant both had their switch set to 0x29.
    // The east servant would start an event that makes the tablet appear, and then after you
    // get the Command Melody from the tablet, the tablet would set switch 0x29.
    // The east servant would check for switch 0x29 to be set, and once it is, start another
    // event where it tells you about its kin and makes the tablet disappear.

    // We change how this works so that the east servant sets switch 0x29 in its event.
    // Then we have a custom event that triggers when any of the three servant returned
    // switches have been set. This custom event makes the tablet appear.
    // The switch set by the tablet when you get its item is changed to 0x2B (unused in vanilla).
    // Once all fourth switches are set, the light beam warp appears.

    hub_room.addAction([=](RandoSession* session, FileType* data) -> int {
        CAST_ENTRY_TO_FILETYPE(dzr, FileTypes::DZXFile, data)

        std::vector<ChunkEntry*> actors = dzr.entries_by_type("ACTR");
        for (ChunkEntry* actor : actors) {
            if (std::strncmp(&actor->data[0], "Hsh\0\0\0\0\0", 8) == 0) { //stone tablet
                actor->data[0xB] = tablet_item_obtained_switch; //params & 0x000000FF
                break;
            }
        }
        for (ChunkEntry* actor : actors) {
            if (std::strncmp(&actor->data[0], "Ywarp00\0", 8) == 0) { //warp beam
                actor->data[0xB] = all_servants_returned_switch; //params & 0x000000FF
                break;
            }
        }
        std::vector<ChunkEntry*> scobs = dzr.entries_by_type("ACTR");
        for (ChunkEntry* scob : scobs) {
            if (std::strncmp(&scob->data[0], "kytag00\0", 8) == 0) { //weather trigger
                scob->data[0x19] = all_servants_returned_switch; //X rotation & 0x00FF
                break;
            }
        }
        for (ChunkEntry* scob : scobs) {
            if (std::strncmp(&scob->data[0], "AttTag\0\0", 8) == 0 && scob->data[0xB] == original_all_servants_returned_switch && (scob->data[0xA] & 0x03) == 1) {
                scob->data[0xB] = all_servants_returned_switch; //params & 0x000000FF
                break;
            }
        }

        return true;
    });

    // East servant returned.
    // Make this servant set its switch directly, instead of making the Command Melody tablet appear and then having the tablet set the switch.
    list.addAction([](RandoSession* session, FileType* data) -> int {
        CAST_ENTRY_TO_FILETYPE(event_list, FileTypes::EventList, data)
        
        if(event_list.Events_By_Name.count("Os_Finish") == 0) LOG_ERR_AND_RETURN_BOOL(TweakError::MISSING_EVENT);
        std::shared_ptr<Event> os0_finish = event_list.Events_By_Name.at("Os_Finish");

        std::shared_ptr<Actor> os0 = os0_finish->get_actor("Os");
        if (os0 == nullptr) {
            LOG_ERR_AND_RETURN_BOOL(TweakError::MISSING_EVENT);
        }
        // Remove the tablet.
        std::erase_if(os0_finish->actors, [](std::shared_ptr<Actor> actor) { return actor->name == "Hsh"; });
        std::shared_ptr<Actor> timekeeper = os0_finish->get_actor("TIMEKEEPER");
        if (timekeeper == nullptr) {
            LOG_ERR_AND_RETURN_BOOL(TweakError::MISSING_EVENT);
        }
        std::shared_ptr<Actor> camera = os0_finish->get_actor("CAMERA");
        if (camera == nullptr) {
            LOG_ERR_AND_RETURN_BOOL(TweakError::MISSING_EVENT);
        }

        // Set the switch.
        std::shared_ptr<Action> set_switch_action = os0->add_action(event_list, "SW_ON", {});
        os0->actions.pop_back();
        os0->actions.insert(os0->actions.begin() + 7, set_switch_action);
        
        // Do not make the other actors wait for the tablet.
        os0->actions.back()->starting_flags[0] = -1;
        timekeeper->actions.end()[-2]->starting_flags[0] = -1;
        camera->actions[6]->starting_flags[0] = -1;
        
        // Remove the final countdown and wait, they won't be used for anything.
        timekeeper->actions.pop_back();
        timekeeper->actions.pop_back();
        
        // Adjust the camera angle so the beam doesn't pierce the camera.
        std::shared_ptr<Action> os0_unitrans = camera->actions[3];
        std::shared_ptr<Property> eye_prop = os0_unitrans->get_prop("Eye");
        eye_prop->value = std::vector<vec3<float>>{vec3<float>{546.0f, 719.0f, -8789.0f}};
        std::shared_ptr<Property> center_prop = os0_unitrans->get_prop("Center");
        center_prop->value = std::vector<vec3<float>>{vec3<float>{783.0f, 582.0f, -9085.0f}};
        
        // Do not make the camera look at the tablet appearing.
        camera->actions.erase(camera->actions.begin() + 7);
        std::shared_ptr<Action> camera_tablet_fixedfrm_act = camera->actions[6];
        const std::vector<std::shared_ptr<Property>> camera_tablet_fixedfrm_props =  camera_tablet_fixedfrm_act->properties;
        camera->actions.erase(camera->actions.begin() + 6);

        // Make it shoot a light beam.
        std::shared_ptr<Action> finish_action = *std::find_if(os0->actions.begin(), os0->actions.end(), [](std::shared_ptr<Action> act) { return act->name == "FINISH"; });
        std::shared_ptr<Property> finish_type_prop = finish_action->get_prop("Type");
        finish_type_prop->value = std::vector<int32_t>{2};



        // West servant returned.
        if(event_list.Events_By_Name.count("Os1_Finish") == 0) LOG_ERR_AND_RETURN_BOOL(TweakError::MISSING_EVENT);
        std::shared_ptr<Event> os1_finish = event_list.Events_By_Name.at("Os1_Finish");

        std::shared_ptr<Actor> os1 = os1_finish->get_actor("Os1");
        camera = os1_finish->get_actor("CAMERA");
        
        // Make it shoot a light beam.
        std::vector<std::shared_ptr<Action>> finish_actions;
        std::for_each(os1->actions.begin(), os1->actions.end(), [&](std::shared_ptr<Action> act) { if(act->name == "FINISH") finish_actions.push_back(act); });
        finish_type_prop = finish_actions[1]->get_prop("Type");
        finish_type_prop->value = std::vector<int32_t>{2};

        // Adjust the camera angle so the beam doesn't pierce the camera.
        std::shared_ptr<Action> os1_unitrans = camera->actions[3];
        const std::vector<vec3<float>> os1_cam_eye = {vec3<float>{-512.0, 626.0, -8775.0}};
        const std::vector<vec3<float>> os1_cam_center = {vec3<float>{-790.0, 667.0, -9065.0}};
        eye_prop = os1_unitrans->get_prop("Eye");
        eye_prop->value = os1_cam_eye;
        center_prop = os1_unitrans->get_prop("Center");
        center_prop->value = os1_cam_center;

        // Remove the camera zooming in on the north door.
        camera->actions.erase(camera->actions.end() - 2);
        os1->actions.pop_back();
        os1_finish->ending_flags[0] = os1->actions.back()->flag_id_to_set;

        // Don't make it wait for the countdown before shooting the beam.
        // Instead make it wait for the camera zooming in on the servant.
        std::erase_if(os1->actions, [finish_actions](std::shared_ptr<Action> act) { return act == finish_actions[0]; });
        finish_actions[1]->starting_flags[0] = (*(camera->actions.end() - 2))->flag_id_to_set;


        // After west servant returned.
        if(event_list.Events_By_Name.count("Os1_Message") == 0) LOG_ERR_AND_RETURN_BOOL(TweakError::MISSING_EVENT);
        std::shared_ptr<Event> os1_message = event_list.Events_By_Name.at("Os1_Message");
        os1 = os1_message->get_actor("Os1");
        camera = os1_message->get_actor("CAMERA");
        // Remove all but the last action to effecitvely remove the event.
        os1->actions.erase(os1->actions.begin(), os1->actions.end() - 1);
        camera->actions.erase(camera->actions.begin(), camera->actions.end() - 1);



        // North servant returned.
        if(event_list.Events_By_Name.count("Os2_Finish") == 0) LOG_ERR_AND_RETURN_BOOL(TweakError::MISSING_EVENT);
        std::shared_ptr<Event> os2_finish = event_list.Events_By_Name.at("Os2_Finish");

        // Remove the east and west servants from being a part of this event.
        std::erase_if(os2_finish->actors, [](std::shared_ptr<Actor> actor) { return actor->name == "Os"; });
        std::erase_if(os2_finish->actors, [](std::shared_ptr<Actor> actor) { return actor->name == "Os1"; });

        std::shared_ptr<Actor> os2 = os2_finish->get_actor("Os2");
        camera = os2_finish->get_actor("CAMERA");

        // Do not make the north servant wait for the east servant to finish before it ends the event.
        std::shared_ptr<Action> os2_sw_on = *std::find_if(os2->actions.begin(), os2->actions.end(), [](std::shared_ptr<Action> act) { return act->name == "SW_ON"; });
        os2_sw_on->starting_flags[0] = -1;

        // Do not make the camera wait for the west servant to finish before it ends the event.
        camera->actions.back()->starting_flags[0] = -1;

        // Do not make the camera look at the east and west servants.
        camera->actions.erase(camera->actions.end() - 3);
        camera->actions.erase(camera->actions.end() - 2);

        // Adjust the camera angle while the camera is following the platform and the servant up.
        // Normally it would adjust the angle after the platform is fully up, but we just skip a step.
        std::shared_ptr<Action> os2_unitrans = camera->actions[3];
        const std::vector<vec3<float>> os2_cam_eye = {vec3<float>{124.0, 589.0, -9482.0}};
        const std::vector<vec3<float>> os2_cam_center = {vec3<float>{-7.0, 644.0, -9625.0}};
        eye_prop = os2_unitrans->get_prop("Eye");
        eye_prop->value = os2_cam_eye;
        center_prop =  os2_unitrans->get_prop("Center");
        center_prop->value = os2_cam_center;

        // Remove the third unitrans, which is when the original event adjusted the camera angle.
        camera->actions.erase(camera->actions.end() - 1);
        camera->actions.erase(camera->actions.end() - 1);

        // And don't make the beam shooting action depend on the deleted unitrans.
        // Instead make it wait for the camera zooming in on the servant.
        finish_actions.clear();
        std::for_each(os2->actions.begin(), os2->actions.end(), [&](std::shared_ptr<Action> act) { if(act->name == "FINISH") finish_actions.push_back(act); });

        std::erase_if(os2->actions, [finish_actions](std::shared_ptr<Action> act) { return act == finish_actions[0]; });
        finish_actions[1]->starting_flags[0] = camera->actions.back()->flag_id_to_set;
        
        
        
        // Tablet event where you play the Command Melody and get an item.
        if(event_list.Events_By_Name.count("hsehi1_tact") == 0) LOG_ERR_AND_RETURN_BOOL(TweakError::MISSING_EVENT);
        std::shared_ptr<Event> hsehi1_tact = event_list.Events_By_Name.at("hsehi1_tact");

        camera = hsehi1_tact->get_actor("CAMERA");
        std::shared_ptr<Actor> hsh = hsehi1_tact->get_actor("Hsh");
        timekeeper = hsehi1_tact->get_actor("TIMEKEEPER");
        std::shared_ptr<Actor> link = hsehi1_tact->get_actor("Link");

        // Remove the camera zooming in on the west door.
        camera->actions.pop_back();
        // Don't make the table wait for the camera to zoom in on the west door.
        hsh->actions.pop_back();
        // Make the tablet disappear at the end.
        hsh->actions.pop_back();
        std::shared_ptr<Action> tablet_hide_player_act = hsh->add_action(event_list, "Disp", std::vector<Prop>{Prop{"target", "@PLAYER"}, Prop{"disp", "off"}});
        std::shared_ptr<Action> tablet_delete_action = hsh->add_action(event_list, "Delete", {});
        // Make the camera zoom in on the tablet while it's disappearing.
        std::shared_ptr<Action> camera_fixedfrm = camera->add_action(event_list, "FIXEDFRM", std::vector<Prop>{
          {"Eye", vec3<float>{3.314825f, 690.2266f, -8600.536f}},
          {"Center", vec3<float>{0.82259f, 677.7084f, -8721.426f}},
          {"Fovy", 60.0f},
          {"Timer", 30}
        });
        std::shared_ptr<Action> link_get_song_action = link->actions[4]; // 059get_dance
        camera_fixedfrm->starting_flags[0] = link_get_song_action->flag_id_to_set;
        tablet_delete_action->starting_flags[0] = camera_fixedfrm->flag_id_to_set;
        hsh->add_action(event_list, "Disp", std::vector<Prop>{{"target", "@PLAYER"}, {"disp", "on"}});
        hsh->add_action(event_list, "WAIT", {});
        hsehi1_tact->ending_flags[0] = hsh->actions.back()->flag_id_to_set;


        // Create the custom event that causes the Command Melody tablet to appear.
        Event& appear_event = event_list.add_event("hsehi1_appear");
      
        camera = appear_event.add_actor(event_list, "CAMERA");
        camera->staff_type = 2;
      
        std::shared_ptr<Actor> tablet_actor = appear_event.add_actor(event_list, "Hsh");
        tablet_actor->staff_type = 0;
        std::shared_ptr<Action> tablet_wait_action = tablet_actor->add_action(event_list, "WAIT", {});
      
        // Make sure Link still animates during the event instead of freezing.
        link = appear_event.add_actor(event_list, "Link");
        link->staff_type = 0;
        link->add_action(event_list, "001n_wait", {});
      
        timekeeper = appear_event.add_actor(event_list, "TIMEKEEPER");
        timekeeper->staff_type = 4;
        timekeeper->add_action(event_list, "WAIT", {});
      
        std::shared_ptr<Action> camera_fixedfrm_action = camera->add_action(event_list, "FIXEDFRM", {});
        for (const std::shared_ptr<Property>& property : camera_tablet_fixedfrm_props) {
            Property& prop = camera_fixedfrm_action->add_property(property->name);
            prop.value = property->value;
        }
      
        camera->add_action(event_list, "PAUSE", {});
      
        std::shared_ptr<Action> tablet_appear_action = tablet_actor->add_action(event_list, "Appear", {});
        tablet_appear_action->starting_flags[0] = camera_fixedfrm_action->flag_id_to_set;
      
        std::shared_ptr<Action> timekeeper_countdown_90_action = timekeeper->add_action(event_list, "COUNTDOWN", std::vector<Prop>{{"Timer", 90}});
        timekeeper_countdown_90_action->duplicate_id = 1;
        timekeeper_countdown_90_action->starting_flags[0] = tablet_appear_action->flag_id_to_set;
      
        tablet_wait_action = tablet_actor->add_action(event_list, "WAIT", {});
        tablet_wait_action->duplicate_id = 1;
        tablet_wait_action->starting_flags[0] = timekeeper_countdown_90_action->flag_id_to_set;
      
        appear_event.ending_flags[0] = tablet_wait_action->flag_id_to_set;

        // Also add SwOps to all four events, with a dummy action. This is so their code still runs during these events, allowing them to seamlessly
        // start events, instead of having a janky one or two frame delay where the camera tries to zoom back to the player before realizing it needs to go to the tablet.
        for (std::shared_ptr<Event> event : {os0_finish, os1_finish, os2_finish, hsehi1_tact}) {
          std::shared_ptr<Actor>swop_actor = event->add_actor(event_list, "SwOp");
          swop_actor->add_action(event_list, "DUMMY", {});
        }

        // Also speed up the events where the servants walk to their respective platforms.
        // They're so slow that it's painful to watch, so we give them a big speed boost.
        const uint8_t servant_speed_multiplier = 4;
        const uint8_t platform_speed_multiplier = 2;
        const uint8_t beam_delay_multiplier = 2;
        for (std::shared_ptr<Event> finish_event : {os0_finish, os1_finish, os2_finish}) {
            const std::unordered_set<std::string> servant_names = {"Os", "Os1", "Os2"};
            const std::unordered_set<std::string> platform_names = {"Hdai1", "Hdai2", "Hdai3"};
            std::shared_ptr<Actor> servant = *std::find_if(finish_event->actors.begin(), finish_event->actors.end(), [servant_names](std::shared_ptr<Actor> actor) { return servant_names.contains(actor->name); });
            std::shared_ptr<Actor> platform = *std::find_if(finish_event->actors.begin(), finish_event->actors.end(), [platform_names](std::shared_ptr<Actor> actor) { return platform_names.contains(actor->name); });
            timekeeper = finish_event->get_actor("TIMEKEEPER");
            camera = finish_event->get_actor("CAMERA");

            std::shared_ptr<Action> servant_move_action = *std::find_if(servant->actions.begin(), servant->actions.end(), [](std::shared_ptr<Action> act) { return act->name == "MOVE"; });
            std::shared_ptr<Property> stick_prop = servant_move_action->get_prop("Stick");
            std::get<std::vector<float>>(stick_prop->value)[0] *= servant_speed_multiplier; // Originally 0.5

            std::shared_ptr<Action> platform_move_action = *std::find_if(platform->actions.begin(), platform->actions.end(), [](std::shared_ptr<Action> act) { return act->name == "MOVE"; });
            std::shared_ptr<Property> speed_prop = platform_move_action->get_prop("Speed");
            std::get<std::vector<float>>(speed_prop->value)[0] *= platform_speed_multiplier; // Originally 2.5

            std::vector<std::shared_ptr<Action>> countdown_actions;
            std::for_each(timekeeper->actions.begin(), timekeeper->actions.end(), [&](std::shared_ptr<Action> act) { if(act->name == "COUNTDOWN") countdown_actions.push_back(act); });
            std::vector<std::shared_ptr<Property>> countdown_timer_props;
            for (std::shared_ptr<Action> act : countdown_actions) {
                std::for_each(act->properties.begin(), act->properties.end(), [&](std::shared_ptr<Property> prop) { if(prop->name == "Timer") countdown_timer_props.push_back(prop); });
            }
            std::get<std::vector<int32_t>>(countdown_timer_props[0]->value)[0] /= servant_speed_multiplier; // Originally 60 frames (2s)
            std::get<std::vector<int32_t>>(countdown_timer_props[1]->value)[0] /= servant_speed_multiplier; // Originally 210 frames (7s)
            // countdown_timer_props[2] is 10 frames
            std::get<std::vector<int32_t>>(countdown_timer_props[3]->value)[0] /= beam_delay_multiplier; // Originally 60 frames (2s)

            std::vector<std::shared_ptr<Action>> unitrans_actions;
            std::for_each(camera->actions.begin(), camera->actions.end(), [&](std::shared_ptr<Action> act) { if(act->name == "UNITRANS") unitrans_actions.push_back(act); });
            std::vector<std::shared_ptr<Property>> unitrans_timer_props;
            for (std::shared_ptr<Action> act : unitrans_actions) {
                std::for_each(act->properties.begin(), act->properties.end(), [&](std::shared_ptr<Property> prop) { if(prop->name == "Timer") unitrans_timer_props.push_back(prop); });
            }
            std::get<std::vector<int32_t>>(unitrans_timer_props[0]->value)[0] /= platform_speed_multiplier; // Originally 90 frames (3s)
            // unitrans_timer_props[1] is 30 frames
        }

        return true;
    });

    hub_room.addAction([](RandoSession* session, FileType* data) -> int {
        CAST_ENTRY_TO_FILETYPE(dzr, FileTypes::DZXFile, data)

        // Detect when any servant has been returned and start the tablet event (hardcode evnt index because it avoids a file dependency, not great but whatever).
        ChunkEntry& swop = dzr.add_entity("ACTR");
        swop.data = "SwOp\x00\x00\x00\x00\x03\x28\x7E\x02\xC4\x48\x00\x00\x44\x7A\x00\x00\xC6\x0C\xA0\x00\x00\x08\x00\x00\x00\x00\xFF\xFF"s;

        // Detect when all servants have been returned and the tablet item is also obtained, and make the warp appear.
        ChunkEntry& swop2 = dzr.add_entity("ACTR");
        swop2.data = "SwOp\x00\x00\x00\x00\x04\x28\x7F\x00\x44\x48\x00\x00\x44\x7A\x00\x00\xC6\x0C\xA0\x00\x00\xFF\x00\x00\x00\x00\xFF\xFF"s;

        return true;
    });

    
    return TweakError::NONE;
}

TweakError make_dungeon_joy_pendants_flexible() {
    // The joy pendant pot in the FW maze room doesn't have an item pickup flag set, which seems accidental
    // Give it an unused flag so you can only get the item once
    RandoSession::CacheEntry& maze_room = g_session.openGameFile("content/Common/Stage/kindan_Room7.szs@YAZ0@SARC@Room7.bfres@BFRES@room.dzr@DZX");
    maze_room.addAction([](RandoSession* session, FileType* data) -> int {
        CAST_ENTRY_TO_FILETYPE(dzr, FileTypes::DZXFile, data)

        ChunkEntry* pot = dzr.entries_by_type_and_layer("ACTR", DEFAULT_LAYER)[0xE0];
        pot->data[9] = 0x16;

        return true;
    });

    // The stone heads with joy pendants will stay destroyed until you reload the dungeon, even if you haven't collected the item
    // This can be confusing and look like a softlock under some circumstances (even though it isn't), so change them to always reappear
    RandoSession::CacheEntry& turn_floor_room = g_session.openGameFile("content/Common/Stage/kaze_Room1.szs@YAZ0@SARC@Room1.bfres@BFRES@room.dzr@DZX");
    turn_floor_room.addAction([](RandoSession* session, FileType* data) -> int {
        CAST_ENTRY_TO_FILETYPE(dzr, FileTypes::DZXFile, data)

        ChunkEntry* stone_head = dzr.entries_by_type_and_layer("ACTR", DEFAULT_LAYER)[0x10];
        stone_head->data[8] = 0xFF;

        return true;
    });

    RandoSession::CacheEntry& many_cyclones_room = g_session.openGameFile("content/Common/Stage/kaze_Room7.szs@YAZ0@SARC@Room7.bfres@BFRES@room.dzr@DZX");
    many_cyclones_room.addAction([](RandoSession* session, FileType* data) -> int {
        CAST_ENTRY_TO_FILETYPE(dzr, FileTypes::DZXFile, data)

        ChunkEntry* stone_head = dzr.entries_by_type_and_layer("ACTR", DEFAULT_LAYER)[0x2E];
        stone_head->data[8] = 0xFF;

        return true;
    });

    return TweakError::NONE;
}

TweakError prevent_fairy_island_softlocks() {
    // Exiting Western Fairy Island without previously solving the puzzle (decoupled entrances) will knock Link back into the fountain before he can land
    // Move out the ring of flames slightly so Link doesn't immediately get hit
    RandoSession::CacheEntry& wfi_dzr = g_session.openGameFile("content/Common/Stage/sea_Room15.szs@YAZ0@SARC@Room15.bfres@BFRES@room.dzr@DZX");

    wfi_dzr.addAction([](RandoSession* session, FileType* data) -> int {
        CAST_ENTRY_TO_FILETYPE(dzr, FileTypes::DZXFile, data)
        
        std::vector<ChunkEntry*> spawns = dzr.entries_by_type("PLYR");
        for (ChunkEntry* spawn : spawns) {
            if (spawn->data[29] == '\x01') { // find spawn ID 1
                spawn->data.replace(0xC, 0x4, "\xC8\x9C\x55\x40", 0x0, 0x4); // change X position to -320170.0
                break;
            }
        }

        return true;
    });

    return TweakError::NONE;
}

TweakError give_fairy_fountains_distinct_colors() {
    double hue = 0.0;
    for (const std::string& stageName : {"Fairy01", "Fairy02", "Fairy03", "Fairy04", "Fairy05", "Fairy06"}) {
        RandoSession::CacheEntry& fountain = g_session.openGameFile("content/Common/Stage/" + stageName + "_Stage.szs@YAZ0@SARC@Stage.bfres@BFRES@stage.dzs@DZX");

        fountain.addAction([hue](RandoSession* session, FileType* data) -> int {
            CAST_ENTRY_TO_FILETYPE(dzs, FileTypes::DZXFile, data)

            for (ChunkEntry* pale : dzs.entries_by_type("Pale")) {
                HSV bg0_c0 = RGBToHSV((uint8_t)pale->data[6] / 255.0, (uint8_t)pale->data[7] / 255.0, (uint8_t)pale->data[8] / 255.0);
                HSV bg0_k0 = RGBToHSV((uint8_t)pale->data[9] / 255.0, (uint8_t)pale->data[10] / 255.0, (uint8_t)pale->data[11] / 255.0);

                bg0_c0.H = hue;
                bg0_k0.H = hue;
                const RGBA<double> new_c0 = HSVToRGB(bg0_c0);
                const RGBA<double> new_k0 = HSVToRGB(bg0_k0);

                pale->data[6] = new_c0.R * 255.0;
                pale->data[7] = new_c0.G * 255.0;
                pale->data[8] = new_c0.B * 255.0;
                pale->data[9] = new_k0.R * 255.0;
                pale->data[10] = new_k0.G * 255.0;
                pale->data[11] = new_k0.B * 255.0;
            }

            return true;
        });

        hue += 360.0 / 6.0;
    }

    return TweakError::NONE;
}

TweakError fix_second_quest_locations() {
    // The Outset chart's salvage point is inside the whirlpool in NG+, locking it behind bombs
    // Logic does not account for this (normal files never require bombs) so it could be problematic in rare cases
    RandoSession::CacheEntry& outset = g_session.openGameFile("content/Common/Pack/szs_permanent2.pack@SARC@sea_Room44.szs@YAZ0@SARC@Room44.bfres@BFRES@room.dzr@DZX");
    outset.addAction([](RandoSession* session, FileType* data) -> int {
        CAST_ENTRY_TO_FILETYPE(dzr, FileTypes::DZXFile, data)

        const std::vector<ChunkEntry*> scobs = dzr.entries_by_type_and_layer("SCOB", DEFAULT_LAYER);
        for(const size_t& salvage_scob_index : {21, 49, 50, 51, 55}) {
            float& x_pos = *reinterpret_cast<float*>(&scobs[salvage_scob_index]->data.data()[0xC]);
            Utility::Endian::toPlatform_inplace(eType::Big, x_pos);
            x_pos += 7000.0f;
            Utility::Endian::toPlatform_inplace(eType::Big, x_pos);
        }

        return true;
    });

    RandoSession::CacheEntry& charts = g_session.openGameFile("content/Common/Misc/Misc.szs@YAZ0@SARC@Misc.bfres@BFRES@cmapdat.bin@CHARTS");
    charts.addAction([](RandoSession* session, FileType* data) -> int {
        CAST_ENTRY_TO_FILETYPE(charts, FileTypes::ChartList, data)

        const auto outset_chart = std::find_if(charts.charts.begin(), charts.charts.end(), [&](const Chart& chart) { return chart.getIslandNumber() == 44; });
        if(outset_chart == charts.charts.end()) return false;

        // Update the texture location to match the new salvage point
        ChartPos& pos = outset_chart->possible_positions[3]; // NG+ chart set
        pos.salvage_x_pos += 7000;

        return true;
    });

    return TweakError::NONE;
}

TweakError apply_necessary_tweaks(const Settings& settings) {
    LOG_AND_RETURN_IF_ERR(Load_Custom_Symbols(Utility::get_data_path() / "asm/custom_symbols.yaml"));

    const std::string seedHash = LogInfo::getSeedHash();
    const std::u16string u16_seedHash = Utility::Str::toUTF16(seedHash);

    TWEAK_ERR_CHECK(updateCodeSize());

    LOG_AND_RETURN_IF_ERR(Apply_Patch(Utility::get_data_path() / "asm/patch_diffs/custom_funcs_diff.yaml"));
    LOG_AND_RETURN_IF_ERR(Apply_Patch(Utility::get_data_path() / "asm/patch_diffs/make_game_nonlinear_diff.yaml"));
    LOG_AND_RETURN_IF_ERR(Apply_Patch(Utility::get_data_path() / "asm/patch_diffs/make_all_figurines_obtainable_diff.yaml"));
    LOG_AND_RETURN_IF_ERR(Apply_Patch(Utility::get_data_path() / "asm/patch_diffs/remove_cutscenes_diff.yaml"));
    LOG_AND_RETURN_IF_ERR(Apply_Patch(Utility::get_data_path() / "asm/patch_diffs/flexible_entrances_diff.yaml"));
    LOG_AND_RETURN_IF_ERR(Apply_Patch(Utility::get_data_path() / "asm/patch_diffs/flexible_hint_locations_diff.yaml"));
    LOG_AND_RETURN_IF_ERR(Apply_Patch(Utility::get_data_path() / "asm/patch_diffs/flexible_item_locations_diff.yaml"));
    LOG_AND_RETURN_IF_ERR(Apply_Patch(Utility::get_data_path() / "asm/patch_diffs/fix_vanilla_bugs_diff.yaml"));
    LOG_AND_RETURN_IF_ERR(Apply_Patch(Utility::get_data_path() / "asm/patch_diffs/misc_rando_features_diff.yaml"));
    LOG_AND_RETURN_IF_ERR(Apply_Patch(Utility::get_data_path() / "asm/patch_diffs/switch_op_diff.yaml"));

    g_session.openGameFile("code/cking.rpx@RPX@ELF").addAction([](RandoSession* session, FileType* data) -> int {
        CAST_ENTRY_TO_FILETYPE(elf, FileTypes::ELF, data)

        //Elf32_Rela blockMoveReloc;
        //blockMoveReloc.r_offset = custom_symbols.at("load_uncompressed_szs") + 0x28;
        //blockMoveReloc.r_info = 0x00015b0a;
        //blockMoveReloc.r_addend = 0;
        //RPX_ERROR_CHECK(elfUtil::addRelocation(elf, 7, blockMoveReloc));

        //blockMoveReloc.r_offset = custom_symbols.at("copy_sarc_to_output") + 0x1C;
        //blockMoveReloc.r_info = 0x00015b0a;
        //blockMoveReloc.r_addend = 0;
        //RPX_ERROR_CHECK(elfUtil::addRelocation(elf, 7, blockMoveReloc));

        RPX_ERROR_CHECK(elfUtil::removeRelocation(elf, {7, 0x001c0ae8})); //would mess with save init
        RPX_ERROR_CHECK(elfUtil::removeRelocation(elf, {7, 0x00160224})); //would mess with salvage point patch
        RPX_ERROR_CHECK(elfUtil::removeRelocation(elf, {7, 0x00199854})); //would overwrite getLayerNo patch

        return true;
    });

    //Update hurricane spin item func, not done through asm because of relocation things
    if(custom_symbols.count("hurricane_spin_item_func") == 0) LOG_ERR_AND_RETURN(TweakError::MISSING_SYMBOL);
    g_session.openGameFile("code/cking.rpx@RPX@ELF").addAction([](RandoSession* session, FileType* data) -> int {
        CAST_ENTRY_TO_FILETYPE(elf, FileTypes::ELF, data)
        
        RPX_ERROR_CHECK(elfUtil::write_u32(elf, elfUtil::AddressToOffset(elf, 0x0001DA54 + (0xAA * 0xC) + 8, 9), custom_symbols.at("hurricane_spin_item_func") - 0x02000000));
        return true;
    });

    if (settings.instant_text_boxes) {
        LOG_AND_RETURN_IF_ERR(Apply_Patch(Utility::get_data_path() / "asm/patch_diffs/b_button_skips_text_diff.yaml"));
        TWEAK_ERR_CHECK(make_all_text_instant());
    }
    if (settings.quiet_swift_sail) {
        LOG_AND_RETURN_IF_ERR(Apply_Patch(Utility::get_data_path() / "asm/patch_diffs/quiet_swift_sail_diff.yaml"));
    }
    if (settings.fix_rng) {
        LOG_AND_RETURN_IF_ERR(Apply_Patch(Utility::get_data_path() / "asm/patch_diffs/fix_rng_diff.yaml"));
    }
    if (settings.performance) {
        LOG_AND_RETURN_IF_ERR(Apply_Patch(Utility::get_data_path() / "asm/patch_diffs/performance_diff.yaml"));
    }
    if (settings.classic_mode) {
        LOG_AND_RETURN_IF_ERR(Apply_Patch(Utility::get_data_path() / "asm/patch_diffs/classic_features_diff.yaml"));
    }
    if (settings.reveal_full_sea_chart) {
        LOG_AND_RETURN_IF_ERR(Apply_Patch(Utility::get_data_path() / "asm/patch_diffs/reveal_sea_chart_diff.yaml"));
    }
    if (settings.invert_sea_compass_x_axis) {
        LOG_AND_RETURN_IF_ERR(Apply_Patch(Utility::get_data_path() / "asm/patch_diffs/invert_sea_compass_x_axis_diff.yaml"));
    }
    if (settings.remove_swords) {
        LOG_AND_RETURN_IF_ERR(Apply_Patch(Utility::get_data_path() / "asm/patch_diffs/swordless_diff.yaml"));
        g_session.openGameFile("code/cking.rpx@RPX@ELF").addAction([](RandoSession* session, FileType* data) -> int {
            CAST_ENTRY_TO_FILETYPE(elf, FileTypes::ELF, data)

            RPX_ERROR_CHECK(elfUtil::removeRelocation(elf, {7, 0x001C1ED4})); //would overwrite branch to custom code
            return true;
        });
    }
    if (settings.remove_music) {
        LOG_AND_RETURN_IF_ERR(Apply_Patch(Utility::get_data_path() / "asm/patch_diffs/remove_music_diff.yaml"));
    }
    if(settings.chest_type_matches_contents) {
        TWEAK_ERR_CHECK(replace_ctmc_chest_texture());
    }
    if(settings.anyEntrancesShuffled()) {
        LOG_AND_RETURN_IF_ERR(Apply_Patch(Utility::get_data_path() / "asm/patch_diffs/entrance_rando_features_diff.yaml"));
    }

    TWEAK_ERR_CHECK(fix_deku_leaf_model());
    TWEAK_ERR_CHECK(allow_all_items_to_be_field_items());
    TWEAK_ERR_CHECK(remove_shop_item_forced_uniqueness_bit());
    TWEAK_ERR_CHECK(remove_ff2_cutscenes(settings.randomize_boss_entrances));
    TWEAK_ERR_CHECK(make_items_progressive());
    TWEAK_ERR_CHECK(add_chest_in_place_medli_gift());
    TWEAK_ERR_CHECK(add_chest_in_place_queen_fairy_cutscene());
    TWEAK_ERR_CHECK(modify_title_screen());
    TWEAK_ERR_CHECK(update_name_and_icon());
    TWEAK_ERR_CHECK(fix_shop_item_y_offsets());
    TWEAK_ERR_CHECK(set_starting_health(settings.starting_pohs, settings.starting_hcs));
    TWEAK_ERR_CHECK(set_damage_multiplier(settings.damage_multiplier));
    TWEAK_ERR_CHECK(remove_makar_kidnapping());
    TWEAK_ERR_CHECK(increase_crawl_speed());
    TWEAK_ERR_CHECK(increase_grapple_animation_speed());
    TWEAK_ERR_CHECK(increase_block_move_animation());
    TWEAK_ERR_CHECK(increase_misc_animations());
    TWEAK_ERR_CHECK(disable_invisible_walls());
    TWEAK_ERR_CHECK(update_tingle_statue_item_get_funcs());
    TWEAK_ERR_CHECK(make_tingle_statue_reward_rupee_rainbow_colored());
    TWEAK_ERR_CHECK(show_seed_hash_on_title_screen(u16_seedHash));
    TWEAK_ERR_CHECK(implement_key_bag());
    TWEAK_ERR_CHECK(add_chest_in_place_jabun_cutscene());
    TWEAK_ERR_CHECK(add_chest_in_place_master_sword());
    TWEAK_ERR_CHECK(fix_totg_warp_spawn());
    TWEAK_ERR_CHECK(remove_phantom_ganon_req_for_reefs());
    TWEAK_ERR_CHECK(fix_ff_door());
    TWEAK_ERR_CHECK(fix_stone_head_bugs());
    TWEAK_ERR_CHECK(show_tingle_statues_on_quest_screen());
    TWEAK_ERR_CHECK(apply_ingame_preferences(settings));
    TWEAK_ERR_CHECK(add_ff_warp_button());
    TWEAK_ERR_CHECK(fix_entrance_params());
    TWEAK_ERR_CHECK(fix_vanilla_text());
    TWEAK_ERR_CHECK(make_dungeon_joy_pendants_flexible());
    TWEAK_ERR_CHECK(prevent_fairy_island_softlocks());
    TWEAK_ERR_CHECK(give_fairy_fountains_distinct_colors());
    //rat hole visibility
    //failsafe id 0 spawns

    TWEAK_ERR_CHECK(update_skip_rematch_bosses_game_variable(settings.skip_rematch_bosses));
    TWEAK_ERR_CHECK(update_sword_mode_game_variable(settings.remove_swords));
    TWEAK_ERR_CHECK(update_starting_gear(settings.starting_gear));
    if(settings.selectedModel.casual) {
        TWEAK_ERR_CHECK(set_casual_clothes());
    }
    TWEAK_ERR_CHECK(set_pig_color(settings.pig_color));

    return TweakError::NONE;
}

TweakError apply_necessary_post_randomization_tweaks(World& world/* , const bool& randomizeItems */) {
    const uint8_t startIsland = islandNameToRoomNum(world.getArea("Link's Spawn")->exits.front().getConnectedArea()->name);

    TWEAK_ERR_CHECK(set_new_game_starting_location(0, startIsland));
    TWEAK_ERR_CHECK(change_ship_starting_island(startIsland));

    TWEAK_ERR_CHECK(add_ganons_tower_warp_to_ff2());
    TWEAK_ERR_CHECK(add_more_magic_jars());
    TWEAK_ERR_CHECK(add_pirate_ship_to_windfall());
    TWEAK_ERR_CHECK(add_hint_signs());
    TWEAK_ERR_CHECK(prevent_reverse_door_softlocks());
    TWEAK_ERR_CHECK(add_barren_dungeon_hint_triggers(world));
    TWEAK_ERR_CHECK(add_shortcut_warps_into_dungeons());
    TWEAK_ERR_CHECK(add_boss_door_return_spawns());
    TWEAK_ERR_CHECK(shorten_zephos_event());
    TWEAK_ERR_CHECK(shorten_auction_intro_event());
    TWEAK_ERR_CHECK(add_jabun_obstacles_to_default_layer());
    TWEAK_ERR_CHECK(remove_jabun_stone_door_event());
    TWEAK_ERR_CHECK(remove_minor_pan_cs());
    TWEAK_ERR_CHECK(show_dungeon_markers_on_chart(world));
    TWEAK_ERR_CHECK(update_entrance_events());
    TWEAK_ERR_CHECK(allow_dungeon_items_to_appear_anywhere(world));
    TWEAK_ERR_CHECK(fix_needle_rock_island_salvage_flags());
    TWEAK_ERR_CHECK(allow_nonlinear_servants_of_the_towers());
    TWEAK_ERR_CHECK(fix_second_quest_locations());

    if(world.getSettings().add_shortcut_warps_between_dungeons) {
        TWEAK_ERR_CHECK(add_cross_dungeon_warps());
    }
    else {
        TWEAK_ERR_CHECK(restore_cross_dungeon_warps());
    }

    //update text last so everything has a chance to add textboxes
    TWEAK_ERR_CHECK(update_text_replacements(world));
    TWEAK_ERR_CHECK(update_korl_dialog(world));
    TWEAK_ERR_CHECK(update_ho_ho_dialog(world));
    TWEAK_ERR_CHECK(rotate_ho_ho_to_face_hints(world));
    TWEAK_ERR_CHECK(add_chart_number_to_item_get_messages(world));

    return TweakError::NONE;
}

std::string errorGetName(TweakError err) {
    switch(err) {
        case TweakError::NONE:
            return "NONE";
        case TweakError::DATA_FILE_MISSING:
            return "DATA_FILE_MISSING";
        case TweakError::PATCH_MISSING_KEY:
            return "PATCH_MISSING_KEY";
        case TweakError::RELOCATION_MISSING_KEY:
            return "RELOCATION_MISSING_KEY";
        case TweakError::FILE_OPEN_FAILED:
            return "FILE_OPEN_FAILED";
        case TweakError::FILE_COPY_FAILED:
            return "FILE_COPY_FAILED";
        case TweakError::FILE_SAVE_FAILED:
            return "FILE_SAVE_FAILED";
        case TweakError::RPX_OPERATION_FAILED:
            return "RPX_OPERATION_FAILED";
        case TweakError::FILETYPE_ERROR:
            return "FILETYPE_ERROR";
        case TweakError::MISSING_SYMBOL:
            return "MISSING_SYMBOL";
        case TweakError::MISSING_EVENT:
            return "MISSING_EVENT";
        case TweakError::MISSING_ENTITY:
            return "MISSING_ENTITY";
        case TweakError::UNEXPECTED_VALUE:
            return "UNEXPECTED_VALUE";
        default:
            return "UNKNOWN";
    }
}
