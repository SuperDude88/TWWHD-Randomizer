cmake_minimum_required(VERSION 3.7)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(wwhd_rando C CXX)
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Weverything -Wno-c++98-compat -Wno-c++98-compat-pedantic -O3") # -Weverything isn't great but other flags don't want to work
add_compile_definitions(JSON_NOEXCEPTION=1 JSON_USE_IMPLICIT_CONVERSIONS=0 NO_GZIP=1)
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

if(DEFINED DEVKITPRO)
  add_compile_definitions(DEVKITPRO)
  add_compile_options(-O3 -flto -Wall)
  add_link_options(-flto -fno-fat-lto-objects)
else()
  add_compile_options(-g)
endif()

if(GET_THREADS)
  find_package(Threads REQUIRED)
endif()

if(ENABLE_DEBUG)
  message("Debugging is ON")
  add_compile_definitions(ENABLE_DEBUG)
endif()

if(ENABLE_TIMING)
  message("Some events will be timed")
  add_compile_definitions(ENABLE_TIMING)
endif()

if(FILL_TESTING)
  message("Configuring for Fill Testing")
  add_compile_definitions(FILL_TESTING)
elseif(MASS_TESTING)
  message("Configuring for Mass Testing")
  add_compile_definitions(MASS_TESTING)
  if(TEST_COUNT)
    message("Test Count: " ${TEST_COUNT})
    add_compile_definitions(TEST_COUNT=${TEST_COUNT})
  endif()
endif()

# Version strings for RNG and compatibility checks
add_compile_definitions(RANDOMIZER_VERSION="01.00.00a")
add_compile_definitions(CONFIG_VERSION="1.0")

# Path strings for logging
string(LENGTH "${CMAKE_SOURCE_DIR}/" SOURCE_PATH_SIZE)
add_compile_definitions(SOURCE_PATH_SIZE=${SOURCE_PATH_SIZE})

add_executable(wwhd_rando main.cpp)
target_sources(wwhd_rando PRIVATE "randomizer.cpp" "options.cpp" "tweaks.cpp" "text_replacements.cpp")
add_subdirectory("libs")
add_subdirectory("utility")
add_subdirectory("command")
add_subdirectory("filetypes")
add_subdirectory("seedgen")
add_subdirectory("logic")
add_subdirectory("nuspack")
# Modded zlib-ng for fast yaz0
add_subdirectory("libs/zlib-ng")

target_include_directories(wwhd_rando PUBLIC ${CMAKE_SOURCE_DIR})

# Put data files together for easier manipulation
file(COPY "asm/custom_symbols.json" DESTINATION "${CMAKE_BINARY_DIR}/data/asm")                    # Custom symbols for inserted code
file(COPY "asm/patch_diffs" DESTINATION "${CMAKE_BINARY_DIR}/data/asm")                            # Diffs for precompiled ASM patches
file(COPY assets DESTINATION "${CMAKE_BINARY_DIR}/data" REGEX "^.*tracker.*$" EXCLUDE)             # Images and other assets used in patching
file(COPY "logic/data" DESTINATION "${CMAKE_BINARY_DIR}/data/logic" REGEX "^.*example.*$" EXCLUDE) # World, macros, and location info

if(CMAKE_USE_PTHREADS_INIT AND GET_THREADS)
  target_link_libraries(wwhd_rando Threads::Threads)
endif()

if(DEFINED DEVKITPRO)
  # Some code specific to Wii U
  add_subdirectory("platform")

  # Use libmocha for filesystem access
  find_library(LIBMOCHA mocha REQUIRED HINTS "${DEVKITPRO}/wut/usr/lib")
  target_link_libraries(wwhd_rando "${LIBMOCHA}")
  
  add_compile_definitions(DATA_PATH="/vol/content/" APP_SAVE_PATH="/vol/save/" TEMP_DIR="/vol/save/temp/")

  wut_create_rpx(wwhd_rando)
  wut_create_wuhb(wwhd_rando
    NAME       "The Legend of Zelda: The Wind Waker HD Randomizer"

    CONTENT    "${CMAKE_BINARY_DIR}/data"
    SHORTNAME  "TWWHD Randomizer"
    AUTHOR     "SuperDude88, csunday95, gymnast86"
    #ICON       "${CMAKE_SOURCE_DIR}/platform/Icon.png"
    #TVSPLASH   "${CMAKE_SOURCE_DIR}/platform/Splash.png"
    #DRCSPLASH  "${CMAKE_SOURCE_DIR}/platform/Splash.png"
  )
else()
  add_compile_definitions(DATA_PATH="./data/" APP_SAVE_PATH="./" TEMP_DIR="./temp/")
endif()
