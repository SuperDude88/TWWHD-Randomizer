#include "contentInfo.hpp"

#include <utility/endian.hpp>

using eType = Utility::Endian::Type;



void ContentInfo::writeToStream(std::ostream& out) {
    const auto idx = Utility::Endian::toPlatform(eType::Big, indexOffset);
    const auto count = Utility::Endian::toPlatform(eType::Big, contentCount);

    out.write(reinterpret_cast<const char*>(&idx), sizeof(idx));
    out.write(reinterpret_cast<const char*>(&count), sizeof(count));
    out.write(reinterpret_cast<const char*>(&hash[0]), hash.size());
}
