cmake_minimum_required(VERSION 3.13)

target_sources(wwhd_rando PRIVATE GameItem.cpp Location.cpp World.cpp ItemPool.cpp Area.cpp Fill.cpp Search.cpp SpoilerLog.cpp Dungeon.cpp Generate.cpp Requirements.cpp Entrance.cpp EntranceShuffle.cpp LogicTests.cpp Hints.cpp Plandomizer.cpp)

add_subdirectory("flatten")
