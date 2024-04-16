#include "WriteCharts.hpp"

#include <unordered_set>

#include <logic/World.hpp>
#include <utility/endian.hpp>
#include <utility/platform.hpp>
#include <filetypes/charts.hpp>
#include <filetypes/dzx.hpp>
#include <command/RandoSession.hpp>
#include <command/GamePath.hpp>

#include <gui/update_dialog_header.hpp>

bool writeCharts(const WorldPool& worlds) {
    using namespace std::literals::string_literals;
    using eType = Utility::Endian::Type;

    Utility::platformLog("Saving randomized charts...\n");
    UPDATE_DIALOG_LABEL("Saving randomized charts...");

    RandoSession::CacheEntry& entry = g_session.openGameFile("content/Common/Misc/Misc.szs@YAZ0@SARC@Misc.bfres@BFRES@cmapdat.bin@CHARTS");

    static const std::unordered_set<std::string> salvage_object_names = {
        "Salvage\0"s,
        "SwSlvg\0\0"s,
        "Salvag2\0"s,
        "SalvagN\0"s,
        "SalvagE\0"s,
        "SalvFM\0\0"s
    };

    for (uint8_t islandNumber = 1; islandNumber <= 49; islandNumber++) {
        const std::string dzrPath = getRoomDzrPath("sea", islandNumber);
        RandoSession::CacheEntry& dzrEntry = g_session.openGameFile(dzrPath);

        entry.addAction([&worlds, &dzrEntry, islandNumber](RandoSession* session, FileType* data) -> int
        {
            CAST_ENTRY_TO_FILETYPE(charts, FileTypes::ChartList, data)
            static const auto original_charts = charts.charts;

            const Chart& original_chart = *std::find_if(original_charts.begin(), original_charts.end(), [islandNumber](const Chart& chart) {return (chart.type == 0 || chart.type == 1 || chart.type == 2 || chart.type == 5 || chart.type == 6 || chart.type == 8) && chart.getIslandNumber() == islandNumber; });
            const GameItem new_chart_item = worlds[0].chartMappings.at(islandNumber);
            const auto new_chart = std::find_if(charts.charts.begin(), charts.charts.end(), [&](const Chart& chart) {return chart.getItem() == new_chart_item;});
            if(new_chart == charts.charts.end()) return false;

            new_chart->texture_id = original_chart.texture_id;
            new_chart->sector_x = original_chart.sector_x;
            new_chart->sector_y = original_chart.sector_y;

            //Probably not needed on HD since they removed chart sets, but update anyway
            for(uint8_t pos_index = 0; pos_index < 4; pos_index++) {
                ChartPos& new_pos = new_chart->possible_positions[pos_index];
                const ChartPos& original_pos = original_chart.possible_positions[pos_index];

                new_pos.tex_x_offset = original_pos.tex_x_offset;
                new_pos.tex_y_offset = original_pos.tex_y_offset;
                new_pos.salvage_x_pos = original_pos.salvage_x_pos;
                new_pos.salvage_y_pos = original_pos.salvage_y_pos;
            }

            dzrEntry.addAction([new_chart = *new_chart](RandoSession* session, FileType* data) -> int
            {
                CAST_ENTRY_TO_FILETYPE(dzr, FileTypes::DZXFile, data)

                for(ChunkEntry* scob : dzr.entries_by_type("SCOB")) {
                    if(salvage_object_names.count(scob->data.substr(0, 8)) > 0 && ((scob->data[8] & 0xF0) >> 4) == 0) {
                        uint32_t& params = *reinterpret_cast<uint32_t*>(&scob->data[8]);
                        Utility::Endian::toPlatform_inplace(eType::Big, params);
                        const uint32_t mask = 0x0FF00000;
                        const uint8_t shiftAmount = 20;

                        params = (params & (~mask)) | (uint32_t(new_chart.owned_chart_index_plus_1 << shiftAmount) & mask);
                        Utility::Endian::toPlatform_inplace(eType::Big, params);
                    }
                }

                return true;
            });

            return true;
        });

        entry.addDependent(dzrEntry.getRoot());
    }

    return true;
}
