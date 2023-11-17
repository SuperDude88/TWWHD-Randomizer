#pragma once

#include <string>

inline std::string getRoomFilePath(const std::string& stageName, const uint8_t roomNum) {
    static const std::unordered_set<uint8_t> pack1 = {0, 1, 11, 13, 17, 23};
    static const std::unordered_set<uint8_t> pack2 = {9, 39, 41, 44};

    const std::string room = std::to_string(roomNum);

    if(stageName == "sea") {
        if(pack1.contains(roomNum)) {
            return "content/Common/Pack/szs_permanent1.pack@SARC@" + stageName + "_Room" + room + ".szs";
        }
        else if(pack2.contains(roomNum)) {
            return "content/Common/Pack/szs_permanent2.pack@SARC@" + stageName + "_Room" + room + ".szs";
        }
    }

    return "content/Common/Stage/" + stageName + "_Room" + room + ".szs";
}

inline std::string getStageFilePath(const std::string& stageName) {
    if(stageName == "sea") {
        return "content/Common/Pack/first_szs_permanent.pack@SARC@sea_Stage.szs";
    }
    
    return "content/Common/Stage/" + stageName + "_Stage.szs";
}

inline std::string getRoomDzrPath(const std::string& stageName, const uint8_t roomNum) {
    return getRoomFilePath(stageName, roomNum) + "@YAZ0@SARC@Room" + std::to_string(roomNum) + ".bfres@BFRES@room.dzr@DZX";
}
