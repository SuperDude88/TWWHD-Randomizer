
#pragma once

#include <string>
#include <vector>


enum struct LocationID
{
    UNDER_GRANDMAS_HOUSE,
    PIG_DIG,
    MESA_HOUSE
};

struct ItemLocation
{
    ItemLocation(LocationID id, std::string reqData) : id(id), requirementsData(reqData) {}
    LocationID id;
    std::string requirementsData;
    int32_t currentItem = -1;
};

void assumedFill(const std::vector<uint8_t>& items, std::vector<ItemLocation>& locations);
