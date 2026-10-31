#pragma once

// Support for the EUR and JPN releases. The C++ code keeps the USA addresses it was written with; port() gives the
// release's own (asm/<region>/address_map.txt, made by asm/port/port_rando.py). The release's patch diffs and custom
// symbols are in asm/<region>/, and the text goes into the release's language packs.
#include <cstdint>
#include <string>
#include <vector>

#include <filetypes/msbt.hpp>
#include <utility/path.hpp>

namespace Region {

enum class Release { USA, EUR, JPN };

// from the game's meta.xml title id; false (and `err`) for an unknown game or a release without data
bool load(const fspath& gameBaseDir, std::string& err);
Release current();
const char* name();
// meta.xml's <region> of this release, and the title ID the randomized game gets (its own channel / Cemu entry)
const char* metaRegion();
const char* randomizedTitleId();
uint64_t randomizedTitleIdValue();

// the folder with patch_diffs/ and custom_symbols.yaml for this release
fspath asmDir();

// USA address -> this release's; the randomizer's own code area (after the USA .text) stays. An address without a
// mapping is logged, fails the run (portFailed) and comes back unchanged.
uint32_t port(uint32_t usa);
// a byte offset into .rela.text (USA) -> this release's
uint32_t portRelaText(uint32_t usaOffset);
// a .rodata offset (relocation addend) -> this release's
uint32_t portRodataOffset(uint32_t usaOffset);
bool portFailed();
const std::string& portErrors();

// language packs: {pack file name part ("UsEnglish", "EuGerman", ...), text language ("English", "Spanish", "French")};
// a pack whose language the randomizer has no text for gets the English text
struct Pack {
    std::string pack, language;
};
const std::vector<Pack>& packs();
// the pack for a text language ("English" -> "EuEnglish"), "" if this release has none for it
std::string packFor(const std::string& language);
// make_all_text_instant's change of one message
void instantText(const std::string& label, Message& message);
// `what` in `message` replaced by `with`; false (message as it is) when the release's text doesn't have it
bool replaceFound(std::u16string& message, const std::u16string& what, const std::u16string& with);
// EUR: the German and Italian packs in `outDir` get the messages and files the randomizer changed in the English one
bool mergePacks(const fspath& gameBaseDir, const fspath& outDir, bool instant, std::string& err);

}  // namespace Region
