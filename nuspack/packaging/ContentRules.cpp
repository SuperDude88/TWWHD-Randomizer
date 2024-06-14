#include "ContentRules.hpp"

#include <algorithm>

enum FSTFlags : uint16_t {
    NOT_HASHED = 0x0000,
    BYTE_OFFSET = 0x0004, //offset in bytes instead of sectors
    HASHED_META = 0x0040,
    HASHED_CONTENT = 0x0400
};

static constexpr uint16_t GROUPID_CODE = 0x0000;
static constexpr uint16_t GROUPID_META = 0x0400;
    
static constexpr uint16_t FSTFLAGS_CODE = NOT_HASHED;
static constexpr uint16_t FSTFLAGS_META = HASHED_META;
static constexpr uint16_t FSTFLAGS_CONTENT = HASHED_CONTENT;



ContentRule& ContentRules::addRule(const ContentRule& rule) {
    if(auto it = std::find_if(rules.begin(), rules.end(), [&](const ContentRule& r) { return r == rule; }); it != rules.end()) {
        return *it;
    }

    rules.push_back(rule);
    return rules.back();
}

ContentRule& ContentRules::createNewRule(const std::string& pattern, const ContentDetails& details, const bool& contentPerMatch){
    return rules.emplace_back(pattern, details, contentPerMatch);
}

ContentRules getCommonRules(const uint16_t& group, const uint64_t& titleID) {
    ContentRules ret;
    std::vector<ContentRule>& rules = ret.rules;

    ContentDetails codeDetails(false, GROUPID_CODE, 0, FSTFLAGS_CODE);
    rules.emplace_back("/code/app.xml", codeDetails);
    rules.emplace_back("/code/cos.xml", codeDetails);
    
    ContentDetails metaDetails(true, GROUPID_META, 0, FSTFLAGS_META);
    rules.emplace_back("/meta/meta.xml", metaDetails);

    rules.emplace_back("/meta/.*[^.xml)]+", metaDetails);
        
    rules.emplace_back("/meta/bootMovie.h264", metaDetails);
    rules.emplace_back("/meta/bootLogoTex.tga", metaDetails);

    rules.emplace_back("/meta/Manual.bfma", metaDetails);
    
    rules.emplace_back("/meta/.*.jpg", metaDetails);

    rules.emplace_back("/code/.*(.rpx|.rpl)", codeDetails, true);

    ContentDetails preloadDetails(true, GROUPID_CODE, 0, FSTFLAGS_CODE);
    rules.emplace_back("/code/preload.txt", preloadDetails);
    
    rules.emplace_back("/code/fw.img", codeDetails);
    rules.emplace_back("/code/fw.tmd", codeDetails);
    rules.emplace_back("/code/htk.bin", codeDetails);
    rules.emplace_back("/code/rvlt.tik", codeDetails);
    rules.emplace_back("/code/rvlt.tmd", codeDetails);
        
    ContentDetails contentDetails(true, group, titleID, FSTFLAGS_CONTENT);
    rules.emplace_back("/content/.*", contentDetails);
    
    return ret;
}
