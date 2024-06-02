cmake_minimum_required(VERSION 3.7)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(wwhd_rando VERSION 0.1 LANGUAGES C CXX) # not sure how to handle this version variable, putting it here for the mac bundle
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

if(DEFINED DEVKITPRO)
  add_compile_definitions(DEVKITPRO DATA_PATH="/vol/content/" APP_SAVE_PATH="/vol/save/" TEMP_DIR="/vol/save/temp/" LOGS_PATH="/vol/save/logs/")
  add_compile_options(-Ofast -mcpu=750 -meabi -mhard-float -ffunction-sections -flto -Wall)
  add_link_options(-flto -fno-fat-lto-objects)
else()
  add_compile_definitions(APP_SAVE_PATH="./" TEMP_DIR="./temp/" LOGS_PATH="./logs/")

  if(APPLE)
    add_compile_options(-fexperimental-library)
    if(MAC_APP_DIRS)
      add_compile_definitions(MAC_APP_DIRS)
    endif()
  endif()

  if(NOT DEFINED EMBED_DATA)
    if(APPLE)
      add_compile_definitions(DATA_PATH="../../../data/")
    else()
      add_compile_definitions(DATA_PATH="./data/")
    endif()
  endif()
endif()

if(ENABLE_DEBUG)
  message("Debugging is ON")

  add_compile_options(-g)
  add_compile_definitions(ENABLE_DEBUG)
endif()

if(ENABLE_TIMING)
  message("Some events will be timed")

  add_compile_definitions(ENABLE_TIMING)
endif()

if(DRY_RUN)
  message("Game patching will be skipped")

  add_compile_definitions(DRY_RUN)
endif()

if(LOGIC_TESTS)
  message("Configuring for Logic Tests")

  add_compile_definitions(LOGIC_TESTS)

  if(TEST_COUNT)
    message("Test Count: " ${TEST_COUNT})
    add_compile_definitions(TEST_COUNT=${TEST_COUNT})
  endif()
endif()

# Versioning
if(DEFINED RELEASE_TAG)
  # Use the new tag we're creating for this release (passed in workflow)
  set(GIT_TAG "${RELEASE_TAG}")

  # Also use the seed key to change item placement when a spoiler log isn't generated
  set(SEED_KEY "$ENV{SEED_KEY}")
else()
  # Get previous tag and commit hash
  find_package(Git REQUIRED)
  execute_process(COMMAND "${GIT_EXECUTABLE}" describe --tags --always --dirty --broken
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
    OUTPUT_VARIABLE GIT_TAG
    ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE
  )
endif()
configure_file(version.hpp.in version.hpp @ONLY)
configure_file(keys/keys.hpp.in keys/keys.hpp @ONLY)

# Path strings for logging
string(LENGTH "${CMAKE_SOURCE_DIR}/" SOURCE_PATH_SIZE)
add_compile_definitions(SOURCE_PATH_SIZE=${SOURCE_PATH_SIZE})

# Put data files together for easier manipulation
file(COPY "asm/custom_symbols.yaml" DESTINATION "${CMAKE_BINARY_DIR}/data/asm")                    # Custom symbols for inserted code
file(COPY "asm/patch_diffs" DESTINATION "${CMAKE_BINARY_DIR}/data/asm")                            # Diffs for precompiled ASM patches
file(COPY "assets" DESTINATION "${CMAKE_BINARY_DIR}/data")                                         # Assets for the patcher
file(COPY "logic/data/" DESTINATION "${CMAKE_BINARY_DIR}/data/logic" REGEX "^.*example.*$" EXCLUDE) # World, macros, and location info
file(COPY "customizer/data/" DESTINATION "${CMAKE_BINARY_DIR}/data/customizer")                     # Default model info

if(DEFINED QT_GUI)
  message("Building with Qt GUI")

  find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
  find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

  add_compile_definitions(QT_GUI)

  # Set Icon on Mac (has to be done out here)
  set(MACOSX_BUNDLE_ICON_FILE "app.icns")
  set(app_icon_macos "${CMAKE_CURRENT_SOURCE_DIR}/gui/app.icns")
  set_source_files_properties(${app_icon_macos} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")

  if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(wwhd_rando MACOSX_BUNDLE main.cpp ${app_icon_macos})
  else()
    add_executable(wwhd_rando main.cpp)
  endif()

  add_subdirectory("gui")
else()
  add_executable(wwhd_rando main.cpp)
endif()

if(GET_THREADS)
  find_package(Threads REQUIRED)

  if(CMAKE_USE_PTHREADS_INIT)
    target_link_libraries(wwhd_rando PRIVATE Threads::Threads)
  endif()
endif()

target_sources(wwhd_rando PRIVATE "randomizer.cpp" "options.cpp" "tweaks.cpp" "text_replacements.cpp")
add_subdirectory("libs")
add_subdirectory("utility")
add_subdirectory("command")
add_subdirectory("filetypes")
add_subdirectory("seedgen")
add_subdirectory("logic")
add_subdirectory("nuspack")
add_subdirectory("customizer")

target_include_directories(wwhd_rando PRIVATE ${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR})

if(DEFINED DEVKITPRO)
  # Some code specific to Wii U
  add_subdirectory("platform")

  # Use libmocha for filesystem access
  find_library(LIBMOCHA mocha REQUIRED HINTS "${DEVKITPRO}/wut/usr/lib")
  target_include_directories(wwhd_rando PRIVATE "${DEVKITPRO}/wut/usr/include")
  target_link_libraries(wwhd_rando PRIVATE "${LIBMOCHA}")
  
  wut_create_rpx(wwhd_rando)
  wut_create_wuhb(wwhd_rando
    NAME       "The Legend of Zelda: The Wind Waker HD Randomizer"

    CONTENT    "${CMAKE_BINARY_DIR}/data"
    SHORTNAME  "TWWHD Randomizer"
    AUTHOR     "SuperDude88, csunday95, gymnast86"
    ICON       "${CMAKE_SOURCE_DIR}/platform/Icon.png"
    #TVSPLASH   "${CMAKE_SOURCE_DIR}/platform/Splash.png"
    #DRCSPLASH  "${CMAKE_SOURCE_DIR}/platform/Splash.png"
  )
endif()
