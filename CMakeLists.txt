cmake_minimum_required(VERSION 3.7)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(wwhd_rando CXX)
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DENABLE_DEBUG=0 -Weverything -Wno-c++98-compat -Wno-c++98-compat-pedantic -O3") # -Weverything isn't great but other flags don't want to work
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DENABLE_DEBUG=1")
add_compile_definitions(JSON_NOEXCEPTION=1 JSON_USE_IMPLICIT_CONVERSIONS=0)

if(DEFINED DEVKITPRO)
  add_definitions(-DDEVKITPRO)
endif()

if(ENABLE_DEBUG)
  message("Debugging is ON")
  add_compile_definitions(ENABLE_DEBUG)
endif()

# Version strings for RNG and compatibility checks
add_compile_definitions(RANDOMIZER_VERSION="01.00.00a")
add_compile_definitions(CONFIG_VERSION="1.0")

# Path strings for logging
string(LENGTH "${CMAKE_SOURCE_DIR}/" SOURCE_PATH_SIZE)
add_definitions("-DSOURCE_PATH_SIZE=${SOURCE_PATH_SIZE}")

# zlib for RPX (de)compression
if(WIN32)
  # Bundle a zlib static library for Windows
  set(ZLIB_LIBRARY "${CMAKE_SOURCE_DIR}/libs/zlib_win/zlibstat.lib")
  set(ZLIB_INCLUDE_DIR "libs/zlib_win")
  add_library(zlib STATIC IMPORTED)
  set_target_properties(zlib PROPERTIES IMPORTED_LOCATION ${ZLIB_LIBRARY})
else()
  include(FindPkgConfig)
  pkg_check_modules(ZLIB zlib REQUIRED)
endif()

add_executable(wwhd_rando randomizer.cpp)
target_sources(wwhd_rando PRIVATE "options.cpp" "tweaks.cpp")
add_subdirectory("libs")
add_subdirectory("server/utility")
add_subdirectory("server/command")
add_subdirectory("server/filetypes")
add_subdirectory("seedgen")
add_subdirectory("logic")

if(WIN32)
  target_link_libraries(wwhd_rando zlib)
  target_include_directories(wwhd_rando PRIVATE ${ZLIB_INCLUDE_DIR})
else()
  target_link_libraries(wwhd_rando ${ZLIB_LDFLAGS})
  target_compile_options(wwhd_rando PRIVATE ${ZLIB_CFLAGS})
endif()

if(DEFINED DEVKITPRO)
  # Some code specific to Wii U
  add_subdirectory("server/platform")
  
  find_library(LIBIOSUHAX iosuhax REQUIRED HINTS "${DEVKITPRO}/wut/usr/lib")
  target_link_libraries(wwhd_rando "${LIBIOSUHAX}")

  # TODO: Use libmocha for filesystem access
  # find_library(LIBMOCHA mocha REQUIRED HINTS "${DEVKITPRO}/wut/usr/lib")
  # target_link_libraries(wwhd_rando "${LIBMOCHA}")
  
  # Copy some files for the homebrew launcher
  file(COPY "server/platform/meta.xml" DESTINATION "${CMAKE_BINARY_DIR}/rando")
  file(COPY "server/platform/icon.png" DESTINATION "${CMAKE_BINARY_DIR}/rando")
  
  # Put data files together so they can be easily copied to the SD card
  file(COPY "asm/custom_symbols.json" DESTINATION "${CMAKE_BINARY_DIR}/rando/asm")                # Custom symbols to get addresses of some inserted code
  file(COPY "asm/patch_diffs" DESTINATION "${CMAKE_BINARY_DIR}/rando/asm")                        # Diffs for precompiled ASM patches
  file(COPY assets DESTINATION "${CMAKE_BINARY_DIR}/rando")                                       # Images or other assets used in patching
  file(COPY "logic/data" DESTINATION "${CMAKE_BINARY_DIR}/rando/logic")                           # World, macros, and location info
  configure_file("seedgen/default_config.yaml" "${CMAKE_BINARY_DIR}/rando/config.yaml" COPYONLY)  # Config for settings and seed

  wut_create_rpx(wwhd_rando)
else()
  # Copy data files to build directory (need to be in the same folder as the executable)
  file(COPY "asm/custom_symbols.json" DESTINATION "${CMAKE_BINARY_DIR}/asm")                # Custom symbols to get addresses of some inserted code
  file(COPY "asm/patch_diffs" DESTINATION "${CMAKE_BINARY_DIR}/asm")                        # Diffs for precompiled ASM patches
  file(COPY assets DESTINATION "${CMAKE_BINARY_DIR}")                                       # Images or other assets used in patching
  file(COPY "logic/data" DESTINATION "${CMAKE_BINARY_DIR}/logic")                           # World, macros, and location info
  configure_file("seedgen/default_config.yaml" "${CMAKE_BINARY_DIR}/config.yaml" COPYONLY)  # Config for settings and seed
endif()
