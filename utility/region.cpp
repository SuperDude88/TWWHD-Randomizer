#include "utility/region.hpp"

#include <algorithm>
#include <unordered_set>
#include <fstream>
#include <map>
#include <sstream>

#include <command/Log.hpp>
#include <filetypes/msbt.hpp>
#include <filetypes/sarc.hpp>
#include <filetypes/yaz0.hpp>
#include <libs/tinyxml2.hpp>
#include <utility/file.hpp>
#include <utility/platform.hpp>
#include <utility/text.hpp>

using namespace std::literals;

namespace Region {
namespace {

Release g_release = Release::USA;
std::map<uint32_t, uint32_t> g_at, g_rela;  // explicit addresses, .rela.text offsets
std::map<uint32_t, int32_t> g_data;         // data: from this USA address on, this offset (until the next)
struct Func {
    uint32_t other, size;
};
std::map<uint32_t, Func> g_funcs;  // matched functions: USA entry -> this release's entry, size
uint32_t g_usaTextEnd = 0x028F87F4, g_textEnd = 0x028F87F4;
bool g_failed = false;
std::string g_errors;
std::vector<Pack> g_packs;

void fail(const std::string& what) {
    g_failed = true;
    if (g_errors.size() < 2000) g_errors += what + "\n";
    ErrorLog::getInstance().log(what);
}

bool load_map(const fspath& path, std::string& err) {
    std::string contents;
    if (Utility::getFileContents(path, contents, true) != 0) return err = "missing " + Utility::toUtf8String(path), false;
    std::istringstream f(contents);
    g_at.clear(), g_rela.clear(), g_data.clear(), g_funcs.clear();
    std::string line;
    while (std::getline(f, line)) {
        std::istringstream in(line);
        std::string kind, a, b, c;
        in >> kind >> a >> b >> c;
        auto hex = [](const std::string& s) { return (uint32_t)std::stoul(s, nullptr, 16); };
        if (kind == "at") g_at[hex(a)] = hex(b);
        else if (kind == "rela") g_rela[hex(a)] = hex(b);
        else if (kind == "data") g_data[hex(a)] = (int32_t)std::stol(b, nullptr, 16);
        else if (kind == "func") g_funcs[hex(a)] = {hex(b), hex(c)};
        else if (kind == "text_end") g_usaTextEnd = hex(a), g_textEnd = hex(b);
    }
    if (g_at.empty() || g_funcs.empty()) return err = "empty address map " + Utility::toUtf8String(path), false;
    return true;
}

}  // namespace

bool load(const fspath& gameBaseDir, std::string& err) {
    tinyxml2::XMLDocument meta;
    if (LoadXML(meta, gameBaseDir / "meta" / "meta.xml") != tinyxml2::XMLError::XML_SUCCESS || !meta.RootElement() ||
        !meta.RootElement()->FirstChildElement("title_id"))
        return err = "cannot read the game's meta/meta.xml", false;
    const std::string id = meta.RootElement()->FirstChildElement("title_id")->GetText();
    g_failed = false;
    g_errors.clear();
    if (id == "0005000010143500") {
        g_release = Release::USA;
        g_packs = {{"UsEnglish", "English"}, {"UsSpanish", "Spanish"}, {"UsFrench", "French"}};
        return true;
    }
    if (id == "0005000010143600") {
        g_release = Release::EUR;
        // German and Italian: the randomizer has no text in them; its changed English messages go there (mergePacks)
        g_packs = {{"EuEnglish", "English"}, {"EuSpanish", "Spanish"}, {"EuFrench", "French"}, {"EuGerman", "English"}, {"EuItalian", "English"}};
    } else if (id == "0005000010143400") {
        g_release = Release::JPN;
        g_packs = {{"JpJapanese", "English"}};  // (the randomizer's messages in English, the game's own stay Japanese)
    } else {
        return err = "not a known release of the game (title id " + id + ")", false;
    }
    return load_map(asmDir() / "address_map.txt", err);
}

Release current() { return g_release; }
const char* name() { return g_release == Release::USA ? "USA" : g_release == Release::EUR ? "EUR" : "JPN"; }
const char* metaRegion() { return g_release == Release::USA ? "00000002" : g_release == Release::EUR ? "00000004" : "00000001"; }
const char* randomizedTitleId() {
    return g_release == Release::USA ? "0005000010143599" : g_release == Release::EUR ? "0005000010143699" : "0005000010143499";
}
uint64_t randomizedTitleIdValue() { return std::stoull(randomizedTitleId(), nullptr, 16); }

fspath asmDir() {
    switch (g_release) {
        case Release::EUR: return Utility::get_data_path() / "asm" / "eur";
        case Release::JPN: return Utility::get_data_path() / "asm" / "jpn";
        default: return Utility::get_data_path() / "asm";
    }
}

uint32_t port(uint32_t usa) {
    if (g_release == Release::USA) return usa;
    if (auto it = g_at.find(usa); it != g_at.end()) return it->second;
    if (usa >= g_usaTextEnd && usa < 0x03000000) return usa;  // the randomizer's own code (this release's addresses)
    if (usa >= 0x02000000 && usa < g_usaTextEnd) {
        auto it = g_funcs.upper_bound(usa);
        if (it != g_funcs.begin()) {
            --it;
            if (usa < it->first + it->second.size) return it->second.other + (usa - it->first);
        }
    } else if (usa >= 0x10000000 && usa < 0x10800000) {
        auto it = g_data.upper_bound(usa);
        if (it != g_data.begin()) return (uint32_t)((int64_t)usa + std::prev(it)->second);
    }
    char t[96];
    snprintf(t, sizeof t, "no %s address for USA %08X", name(), usa);
    fail(t);
    return usa;
}

uint32_t portRelaText(uint32_t usaOffset) {
    if (g_release == Release::USA) return usaOffset;
    if (auto it = g_rela.find(usaOffset); it != g_rela.end()) return it->second;
    char t[96];
    snprintf(t, sizeof t, "no %s .rela.text entry for USA +%X", name(), usaOffset);
    fail(t);
    return usaOffset;
}

uint32_t portRodataOffset(uint32_t usaOffset) { return port(0x10000000 + usaOffset) - 0x10000000; }

// make_all_text_instant (tweaks.cpp) for one message
void instantText(const std::string& label, Message& message) {
    std::u16string& String = message.text.message;

    message.attributes.drawType = 1; // draw instant

    std::u16string::size_type wait = String.find(u"\x0e\x01\x06\x02"s); // dont use macro because duration shouldnt matter
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
        std::u16string::size_type wait_dismiss = String.find(u"\x0e\x01\x03\x02"s); // dont use macro because duration shouldnt matter
        while (wait_dismiss != std::u16string::npos) {
            String.erase(wait_dismiss, 5);
            wait_dismiss = String.find(u"\x0e\x01\x03\x02"s);
        }
    }

    std::u16string::size_type wait_dismiss_prompt = String.find(u"\x0e\x01\x02\x02"s); // dont use macro because duration shouldnt matter
    while (wait_dismiss_prompt != std::u16string::npos) {
        String.erase(wait_dismiss_prompt, 5);
        wait_dismiss_prompt = String.find(u"\x0e\x01\x02\x02"s);
    }
}

bool replaceFound(std::u16string& message, const std::u16string& what, const std::u16string& with) {
    const size_t at = message.find(what);
    if (at == std::u16string::npos) return false;
    message.replace(at, what.size(), with);
    return true;
}

bool portFailed() { return g_failed; }
const std::string& portErrors() { return g_errors; }

const std::vector<Pack>& packs() { return g_packs; }

std::string packFor(const std::string& language) {
    for (const Pack& p : g_packs)
        if (p.language == language) return p.pack;  // (the first: EuEnglish for English)
    return "";
}

// ---- EUR: the German and Italian packs get what the randomizer changed in the English one

namespace {

bool read_file(const fspath& p, std::string& out) {
    std::ifstream f(p, std::ios::binary);
    if (!f) return false;
    std::stringstream s;
    s << f.rdbuf();
    out = s.str();
    return true;
}

bool load_sarc(const std::string& bytes, FileTypes::SARCFile& sarc) {
    std::istringstream in(bytes);
    return sarc.loadFromBinary(in) == SARCError::NONE;
}

// a "*_msbt.szs" (Yaz0 SARC with one .msbt): its messages by label
bool load_msbt_szs(const std::string& szs, FileTypes::SARCFile& inner, std::string& name, FileTypes::MSBTFile& msbt) {
    std::stringstream in(szs), dec;
    if (FileTypes::yaz0Decode(in, dec) != YAZ0Error::NONE) return false;
    if (!load_sarc(dec.str(), inner)) return false;
    for (auto& [n, data] : inner.files)
        if (n.size() > 5 && n.substr(n.size() - 5) == ".msbt") {
            name = n;
            std::istringstream m(data);
            return msbt.loadFromBinary(m) == LMSError::NONE;
        }
    return false;
}

bool store_msbt_szs(FileTypes::SARCFile& inner, const std::string& name, FileTypes::MSBTFile& msbt, std::string& szs) {
    std::ostringstream m;
    if (msbt.writeToStream(m) != LMSError::NONE) return false;
    inner.files[name] = m.str();
    std::ostringstream s;
    if (inner.writeToStream(s) != SARCError::NONE) return false;
    std::stringstream raw(s.str()), enc;
    if (FileTypes::yaz0Encode(raw, enc) != YAZ0Error::NONE) return false;
    szs = enc.str();
    return true;
}

}  // namespace

bool mergePacks(const fspath& gameBaseDir, const fspath& outDir, bool instant, std::string& err) {
    if (g_release != Release::EUR) return true;
    const fspath packDir = "content/Common/Pack";
    std::string vanillaBytes, randoBytes;
    if (!read_file(gameBaseDir / packDir / "permanent_2d_EuEnglish.pack", vanillaBytes) ||
        !read_file(outDir / packDir / "permanent_2d_EuEnglish.pack", randoBytes))
        return true;  // (the English pack unchanged: nothing to carry over)
    FileTypes::SARCFile vanilla, rando;
    if (!load_sarc(vanillaBytes, vanilla) || !load_sarc(randoBytes, rando)) return err = "cannot read the English pack", false;
    for (const char* lang : {"EuGerman", "EuItalian"}) {
        std::string bytes;
        if (!read_file(gameBaseDir / packDir / (std::string("permanent_2d_") + lang + ".pack"), bytes)) continue;
        FileTypes::SARCFile target;
        if (!load_sarc(bytes, target)) return err = std::string("cannot read ") + lang, false;
        size_t files = 0, messages = 0;
        for (auto& [name, data] : rando.files) {
            auto v = vanilla.files.find(name);
            if (v != vanilla.files.end() && v->second == data) continue;  // not changed by the randomizer
            auto t = target.files.find(name);
            const bool isMsbt = name.size() > 9 && name.substr(name.size() - 9) == "_msbt.szs";
            if (!isMsbt || t == target.files.end() || v == vanilla.files.end()) {
                target.files[name] = data;  // layouts, pictures: as in English
                files++;
                continue;
            }
            // messages: the changed and new English ones replace the language's, its others stay
            FileTypes::SARCFile vi, ri, ti;
            FileTypes::MSBTFile vm, rm, tm;
            std::string vn, rn, tn;
            if (!load_msbt_szs(v->second, vi, vn, vm) || !load_msbt_szs(data, ri, rn, rm) || !load_msbt_szs(t->second, ti, tn, tm))
                return err = "cannot read the messages of " + name, false;
            // the randomizer's changes to all messages (instant text) done to the language's own; then only the
            // messages whose English content changed beyond that are taken over
            if (instant) {
                for (auto& [label, msg] : vm.messages_by_label) instantText(label, msg);
                for (auto& [label, msg] : tm.messages_by_label) instantText(label, msg);
            }
            size_t n = instant ? 1 : 0;  // (rewritten either way when instant)
            for (auto& [label, msg] : rm.messages_by_label) {
                auto old = vm.messages_by_label.find(label);
                if (old != vm.messages_by_label.end() && old->second.text.message == msg.text.message &&
                    old->second.attributes.character == msg.attributes.character)
                    continue;
                auto here = tm.messages_by_label.find(label);
                if (here == tm.messages_by_label.end()) tm.addMessage(label, msg.attributes, msg.style, msg.text.message);
                else {
                    here->second.attributes = msg.attributes;
                    here->second.style = msg.style;
                    here->second.text.message = msg.text.message;
                }
                n++;
                messages++;
            }
            if (n && !store_msbt_szs(ti, tn, tm, t->second)) return err = "cannot write the messages of " + name, false;
        }
        std::ostringstream out;
        if (target.writeToStream(out) != SARCError::NONE) return err = std::string("cannot write ") + lang, false;
        std::ofstream f(outDir / packDir / (std::string("permanent_2d_") + lang + ".pack"), std::ios::binary);
        f << out.str();
        if (!f) return err = std::string("cannot write ") + lang, false;
        Utility::platformLog(std::string(lang) + ": " + std::to_string(messages) + " messages and " + std::to_string(files) + " files from the English pack");
    }
    return true;
}

}  // namespace Region
