#include "packer.hpp"

#include "../../command/Log.hpp"
#include "packaging/NUSPackage.hpp"



PackError createPackage(const std::filesystem::path& dirPath, const std::filesystem::path& out, const Key& encryptionKey, const Key& encryptKeyWith) {
    AppInfo info;
    LOG_AND_RETURN_IF_ERR(info.parseFromXML(dirPath / "code/app.xml"));

    const uint64_t parentID = info.titleID & ~0x0000000F00000000;

    const ContentRules rules = getCommonRules(info.groupID, parentID);

    PackageConfig config(dirPath, info, encryptionKey, encryptKeyWith, rules);
    NUSPackage package = NUSPackage::createNew(config);
    package.PackContents(out);

    return PackError::NONE;
}
