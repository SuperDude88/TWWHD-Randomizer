#include "randomizer.hpp"

#include <string>
#include <vector>
#include <filesystem>

#include <libs/tinyxml2.hpp>
#include <libs/zlib-ng.hpp>

#include <tweaks.hpp>
#include <seedgen/config.hpp>
#include <seedgen/random.hpp>
#include <seedgen/seed.hpp>
#include <logic/SpoilerLog.hpp>
#include <logic/Generate.hpp>
#include <logic/LogicTests.hpp>
#include <command/WriteLocations.hpp>
#include <command/WriteEntrances.hpp>
#include <command/WriteCharts.hpp>
#include <command/RandoSession.hpp>
#include <command/Log.hpp>
#include <utility/platform.hpp>
#include <utility/path.hpp>
#include <utility/file.hpp>
#include <utility/time.hpp>

#include <gui/desktop/update_dialog_header.hpp>

#ifdef DEVKITPRO
    #include <sysapp/title.h>
    #include <platform/channel.hpp>
    #include <gui/wiiu/InstallMenu.hpp>
#endif

class Randomizer {
private:
    Config config;

    #ifdef DRY_RUN
        const bool dryRun = true;
    #else
        const bool dryRun = false;
    #endif
    //bool randomizeItems = true; not currently used
    unsigned int numPlayers = 1;
    //int playerId = 1;

    [[nodiscard]] bool verifyBase() {
        using namespace std::filesystem;

        Utility::platformLog("Verifying dump...");
        UPDATE_DIALOG_LABEL("Verifying dump...");
        UPDATE_DIALOG_VALUE(25);

        const fspath& base = g_session.getBaseDir();
        if(!is_directory(base / "code") || !is_directory(base / "content") || !is_directory(base / "meta")) {
            ErrorLog::getInstance().log("Invalid base path: could not find code/content/meta folders at " + Utility::toUtf8String(base) + "!");
            return false;
        }

        //Check the meta.xml for other platforms (+ a sanity check on console)
        const fspath& metaPath = g_session.getBaseDir() / "meta/meta.xml";
        if(!is_regular_file(metaPath)) {
            ErrorLog::getInstance().log("Failed finding meta.xml");
            return false;
        }

        tinyxml2::XMLDocument metaXml;
        if(const tinyxml2::XMLError err = LoadXML(metaXml, metaPath); err != tinyxml2::XMLError::XML_SUCCESS) {
            ErrorLog::getInstance().log(std::string("Could not parse input's meta.xml, ") + metaXml.ErrorStr());
            return false;
        }

        const tinyxml2::XMLElement* root = metaXml.RootElement();

        const std::string titleId = root->FirstChildElement("title_id")->GetText();
        const std::string nameEn = root->FirstChildElement("longname_en")->GetText();
        if(titleId != "0005000010143500" || nameEn != "THE LEGEND OF ZELDA\nThe Wind Waker HD")  {
            if(titleId == "0005000010143400" || titleId == "0005000010143600") {
                ErrorLog::getInstance().log("Incorrect region - game must be a NTSC-U / US copy");
            }
            else {
                ErrorLog::getInstance().log("meta.xml does not match base game - dump is not valid");
                ErrorLog::getInstance().log("ID " + titleId);
                ErrorLog::getInstance().log("Name " + nameEn);
            }

            return false;
        }

        const std::string region = root->FirstChildElement("region")->GetText();
        if(region != "00000002") {
            ErrorLog::getInstance().log("Incorrect region - game must be a NTSC-U / US copy");
            return false;
        }

        return true;
    }

    [[nodiscard]] bool verifyOutput() {
        using namespace std::filesystem;

        Utility::platformLog("Verifying output...");
        UPDATE_DIALOG_LABEL("Verifying output...");
        UPDATE_DIALOG_VALUE(25);
        
        const fspath& out = g_session.getOutputDir();
        if(out.empty() || !is_directory(out)) { // we tried to create this earlier so error if that failed
            ErrorLog::getInstance().log("Invalid output path: " + Utility::toUtf8String(out) + " is not a valid folder!");
            return false;
        }
        if(!is_directory(out / "code") || !is_directory(out / "content") || !is_directory(out / "meta")) {
            #ifdef DEVKITPRO // this should all exist on console
                ErrorLog::getInstance().log("Invalid output path: could not find code/content/meta folders at " + out.string() + "!");
                return false;
            #else // copy over the files on desktop
                g_session.setFirstTimeSetup(true);
                return true; // skip the rest of the checks, don't error
            #endif
        }

        //Double check the meta.xml
        const fspath& metaPath = out / "meta/meta.xml";
        if(!is_regular_file(metaPath)) {
            ErrorLog::getInstance().log("Failed finding meta.xml");
            return false;
        }

        tinyxml2::XMLDocument metaXml;
        if(const tinyxml2::XMLError err = LoadXML(metaXml, metaPath); err != tinyxml2::XMLError::XML_SUCCESS) {
            ErrorLog::getInstance().log(std::string("Could not parse output's meta.xml, ") + metaXml.ErrorStr());
            return false;
        }
        const tinyxml2::XMLElement* root = metaXml.RootElement();

        // Title ID won't be updated until after the first randomization on PC so it's not a very useful check
        // But on console it should be correct once the channel is installed so it's a good sanity check
        #ifdef DEVKITPRO
            const std::string titleId = root->FirstChildElement("title_id")->GetText();
            if(titleId != "0005000010143599")  {
                ErrorLog::getInstance().log("meta.xml does not match - custom channel is not valid");
                ErrorLog::getInstance().log("ID " + titleId);
                return false;
            }
        #endif

        const std::string region = root->FirstChildElement("region")->GetText();
        if(region != "00000002") {
            ErrorLog::getInstance().log("Incorrect region - game must be a NTSC-U / US copy");
            return false;
        }

        return true;
    }

public:
    Randomizer(const Config& config_) :
        config(config_)
    {}

    int randomize() {
        // Go through the setting testing process if mass testing is turned on and ignore everything else
        #ifdef LOGIC_TESTS
            #if TEST_COUNT
                testSettings(config, TEST_COUNT);
            #else
                runLogicTests(config);
            #endif
            return 0;
        #endif
        
        // Only set up the session if we actually need it
        if(!dryRun) {
            if(!g_session.init(config.gameBaseDir, config.outputDir)) {
                ErrorLog::getInstance().log("Failed to initialize session");
                return 1;
            }
            Utility::platformLog("Initialized session");
        }

        LogInfo::setConfig(config);

        LOG_TO_DEBUG("Permalink: " + config.getPermalink());

        // Seed RNG
        const std::string permalink = config.getPermalink(true);
        if(permalink.empty()) {
            ErrorLog::getInstance().log("Could not generate permalink for RNG seeding.");
            return 1;
        }
        const size_t integer_seed = zng_crc32(0L, reinterpret_cast<const uint8_t*>(permalink.data()), permalink.length());
        Random_Init(integer_seed);

        LogInfo::setSeedHash(generate_seed_hash());

        UPDATE_DIALOG_TITLE("Randomizing - Hash: " + LogInfo::getSeedHash());

        // Create all necessary worlds (for any potential multiworld support in the future)
        WorldPool worlds(numPlayers);
        std::vector<Settings> settingsVector (numPlayers, config.settings);

        Utility::platformLog("Randomizing...");
        UPDATE_DIALOG_VALUE(5);
        if (generateWorlds(worlds, settingsVector) != 0) {
            return 1;
        }

        generateNonSpoilerLog(worlds);
        if (!config.settings.do_not_generate_spoiler_log) {
            generateSpoilerLog(worlds);
        }

        // Skip all game modification stuff if we're doing a dry run (fill testing)
        if (dryRun) return 0;

        if(!verifyBase()) {
            return 1;
        }
        if(!verifyOutput()) {
            return 1;
        }

        //IMPROVEMENT: custom model things
        if(const ModelError err = config.settings.selectedModel.applyModel(); err != ModelError::NONE) {
            ErrorLog::getInstance().log("Failed to apply custom model, error " + errorToName(err));
            return 1;
        }

        Utility::platformLog("Modifying game code...");
        UPDATE_DIALOG_VALUE(30);
        UPDATE_DIALOG_LABEL("Modifying game code...");
        // TODO: update worlds indexing for multiworld eventually
        if(const TweakError err = apply_necessary_tweaks(worlds[0].getSettings()); err != TweakError::NONE) {
            ErrorLog::getInstance().log("Encountered " + errorGetName(err) + " in pre-randomization tweaks!");
            return 1;
        }

        // Assume 1 world for now, modifying multiple copies needs work
        // These work directly on the data stream so RandoSession applies them first
        if(!writeLocations(worlds)) {
            ErrorLog::getInstance().log("Failed to save items!");
            return 1;
        }

        // Charts/entrances work through the actor list, which is applied after any stream modifications
        // This prevents them from interfering with the hardcoded item offsets
        if(config.settings.randomize_charts) {
            if(!writeCharts(worlds)) {
                ErrorLog::getInstance().log("Failed to save charts!");
                return 1;
            }
        }

        if (config.settings.anyEntrancesShuffled()) {
            if(!writeEntrances(worlds)) {
                ErrorLog::getInstance().log("Failed to save entrances!");
                return 1;
            }
        }

        Utility::platformLog("Applying final patches...");
        UPDATE_DIALOG_VALUE(50);
        UPDATE_DIALOG_LABEL("Applying final patches...");
        if(const TweakError err = apply_necessary_post_randomization_tweaks(worlds[0]/* , randomizeItems */); err != TweakError::NONE) {
            ErrorLog::getInstance().log("Encountered " + errorGetName(err) + " in post-randomization tweaks!");
            return 1;
        }

        // Restore files that aren't changed (chart list, entrances, etc) so they don't persist across seeds
        // restoreGameFile() does not need to check if the file is cached because it only tries to get the cache entry
        // Getting a cache entry doesn't overwrite anything if the file already had modifications
        Utility::platformLog("Restoring outdated files...");
        if(!g_session.restoreGameFile("content/Common/Misc/Misc.szs")) {
            ErrorLog::getInstance().log("Failed to restore Misc.szs!");
            return 1;
        }
        if(!g_session.restoreGameFile("content/Common/Particle/Particle.szs")) {
            ErrorLog::getInstance().log("Failed to restore Particle.szs!");
            return 1;
        }
        if(!g_session.restoreGameFile("content/Common/Pack/permanent_3d.pack")) {
            ErrorLog::getInstance().log("Failed to restore permanent_3d.pack!");
            return 1;
        }
        if(!restoreEntrances(worlds)) {
            ErrorLog::getInstance().log("Failed to restore entrances!");
            return 1;
        }

        Utility::platformLog("Preparing to edit files...");
        if(!g_session.modFiles()) {
            ErrorLog::getInstance().log("Failed to edit file cache!");
            return 1;
        }

        //done!
        return 0;
    }
};

int mainRandomize() {
    #ifdef ENABLE_TIMING
        ScopedTimer<"Total process took "> timer;
    #endif

    // Make sure we have a logs folder
    if (!Utility::create_directories(Utility::get_logs_path())) {
        ErrorLog::getInstance().log("Failed to create logs folder");
        return 1;
    }

    // Create default configs/preferences if they don't exist
    ConfigError err = Config::writeDefault(Utility::get_app_save_path() / "config.yaml", Utility::get_app_save_path() / "preferences.yaml");
    if(err != ConfigError::NONE) {
        ErrorLog::getInstance().log("Failed to create config, error " + ConfigErrorGetName(err));
        return 1;
    }

    Utility::platformLog("Reading config");
    Config load;
    err = load.loadFromFile(Utility::get_app_save_path() / "config.yaml", Utility::get_app_save_path() / "preferences.yaml");
    if(err != ConfigError::NONE && err != ConfigError::DIFFERENT_RANDO_VERSION) {
        ErrorLog::getInstance().log("Failed to read config, error " + ConfigErrorGetName(err));

        return 1;
    }

    #ifdef DEVKITPRO
        if(!SYSCheckTitleExists(0x0005000010143500)) {
            ErrorLog::getInstance().log("Could not find game: you must have a NTSC-U / US copy of TWWHD!");

            return 1;
        }
        if(const auto& err = getTitlePath(0x0005000010143500, load.gameBaseDir); err < 0) {
            return 1;
        }
        if(!Utility::mountDeviceAndConvertPath(load.gameBaseDir)) {
            ErrorLog::getInstance().log("Failed mounting input device!");
            return 1;
        }
        
        if(!SYSCheckTitleExists(0x0005000010143599)) {
            if(!createOutputChannel(load.gameBaseDir, pickInstallLocation())) {
                return 1;
            }
            g_session.setFirstTimeSetup(true);
        }
        if(const auto& err = getTitlePath(0x0005000010143599, load.outputDir); err < 0) {
            return 1;
        }
        if(!Utility::mountDeviceAndConvertPath(load.outputDir)) {
            ErrorLog::getInstance().log("Failed mounting output device!");
            return 1;
        }
    #endif

    Randomizer rando(load);

    // IMPROVEMENT: issue with seekp, find better solution than manual padding?
    // TODO: make things zoom

    return rando.randomize();
}
