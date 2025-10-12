cmake_minimum_required(VERSION 3.13)

target_sources(wwhd_rando PRIVATE wiiurpx.cpp yaz0.cpp sarc.cpp msbt.cpp elf.cpp events.cpp bfres.cpp jpc.cpp dzx.cpp charts.cpp bflyt.cpp dds.cpp bflim.cpp msbp.cpp bdt.cpp bffnt.cpp util/elfUtil.cpp shared/lms.cpp)

add_subdirectory("texture")
add_subdirectory("subfiles")
