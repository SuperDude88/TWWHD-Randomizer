cmake_minimum_required(VERSION 3.13)

target_sources(wwhd_rando PRIVATE packer.cpp appinfo.cpp)

add_subdirectory("packaging")
add_subdirectory("fst")
add_subdirectory("crypto")
add_subdirectory("contents")
