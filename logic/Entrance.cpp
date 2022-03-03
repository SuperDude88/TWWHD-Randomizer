
#include "Entrance.hpp"
#include "Random.hpp"

static Area roomIndexToStartingIslandArea(const uint8_t& startingIslandRoomIndex)
{
    constexpr std::array<Area, 49> startingIslandAreaArray = {
        Area::ForsakenFortress,
        Area::StarIsland,
        Area::NorthernFairyIsland,
        Area::GaleIsle,
        Area::CrescentMoonIsland,
        Area::SevenStarIsles,
        Area::OverlookIsland,
        Area::FourEyeReef,
        Area::MotherAndChildIsles,
        Area::SpectacleIsland,
        Area::WindfallIsland,
        Area::PawprintIsle,
        Area::DragonRoostIsland,
        Area::FlightControlPlatform,
        Area::WesternFairyIsland,
        Area::RockSpireIsle,
        Area::TingleIsland,
        Area::NorthernTriangleIsland,
        Area::EasternFairyIsland,
        Area::FireMountain,
        Area::StarBeltArchipelago,
        Area::ThreeEyeReef,
        Area::GreatfishIsle,
        Area::CyclopsReef,
        Area::SixEyeReef,
        Area::TowerOfTheGods,
        Area::EasternTriangleIsland,
        Area::ThornedFairyIsland,
        Area::NeedleRockIsle,
        Area::IsletOfSteel,
        Area::StoneWatcherIsland,
        Area::SouthernTriangleIsland,
        Area::PrivateOasis,
        Area::BombIsland,
        Area::BirdsPeakRock,
        Area::DiamondSteppeIsland,
        Area::FiveEyeReef,
        Area::SharkIsland,
        Area::SouthernFairyIsland,
        Area::IceRingIsle,
        Area::ForestHaven,
        Area::CliffPlateauIsles,
        Area::HorseshoeIsle,
        Area::OutsetIsland,
        Area::HeadstoneIsland,
        Area::TwoEyeReef,
        Area::AngularIsles,
        Area::BoatingCourse,
        Area::FiveStarIsles,
    };

    return startingIslandAreaArray[startingIslandRoomIndex];
}

EntranceError randomizeEntrances(WorldPool& worlds)
{
    for (auto& world : worlds)
    {
        if (world.getSettings().randomize_starting_island)
        {
            // Rooms 2 - 49 include every island except Forsaken Fortress
            world.startingIslandRoomIndex = Random(2, 50);
            auto startingIsland = roomIndexToStartingIslandArea(world.startingIslandRoomIndex);

            // Set the new starting island in the world graph
            auto& linksSpawnExit = world.getExit(Area::LinksSpawn, Area::OutsetIsland);
            linksSpawnExit.connectedArea = startingIsland;
        }
    }

    return EntranceError::NONE;
}
