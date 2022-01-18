
#include "Fill.hpp"
#include "Search.hpp"

FillError fill(std::vector<World>& worlds) {



    return FillError::NONE;
}

const char* errorToName(FillError err)
{
    switch(err)
    {
    case FillError::NONE:
        return "NONE";
    case FillError::RAN_OUT_OF_RETRIES:
        return "RAN_OUT_OF_RETRIES";
    case FillError::MORE_ITEMS_THAN_LOCATIONS:
        return "MORE_ITEMS_THAN_LOCATIONS";
    default:
        return "UNKNOWN";
    }
}
