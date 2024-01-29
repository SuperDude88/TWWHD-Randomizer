cmake_minimum_required(VERSION 3.5)

project(wwhd_rando VERSION 0.1 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options(-O3)
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

add_compile_definitions(QT_GUI=1)

if(ENABLE_DEBUG)
  message("Debugging is ON")
  add_compile_definitions(ENABLE_DEBUG)
  add_compile_options(-g)
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
endif()

# Versioning
find_package(Git REQUIRED)
if(DEFINED RELEASE_TAG)
  # use the new tag we're creating for this release (passed in workflow)
  set(GIT_TAG "${RELEASE_TAG}")
else()
  # get previous tag and commit hash
  execute_process(COMMAND "${GIT_EXECUTABLE}" describe --tags --always --dirty --broken
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
    OUTPUT_VARIABLE GIT_TAG
    ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE
  )
endif()
configure_file("../version.hpp.in" "../version.hpp" @ONLY)

# Path strings for logging
string(LENGTH "${CMAKE_SOURCE_DIR}/" SOURCE_PATH_SIZE)
add_compile_definitions(SOURCE_PATH_SIZE=${SOURCE_PATH_SIZE})

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

add_compile_definitions(APP_SAVE_PATH="./" TEMP_DIR="./temp/")

if (EMBED_DATA)
    add_compile_definitions(DATA_PATH=":/")
else()
    if(APPLE)
        add_compile_definitions(DATA_PATH="../../../data")
    else()
        add_compile_definitions(DATA_PATH="./data/")
    endif()
endif()

# Embeds the data files into the Qt executable
if(EMBED_DATA)
  message("Data will be embeded")
  add_compile_definitions(EMBED_DATA)
  find_package(Python REQUIRED)
  execute_process(COMMAND "${Python_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/generate_qrc_file.py" WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} RESULT_VARIABLE QRC_GENERATE_RESULT)
  if(NOT QRC_GENERATE_RESULT EQUAL "0")
    message("Failed to generate data.qrc")
  else()
    set(PROJECT_SOURCES data.qrc ${PROJECT_SOURCES})
  endif()
else()
  # Put data files together for easier manipulation
  file(COPY "../asm/custom_symbols.yaml" DESTINATION "${CMAKE_BINARY_DIR}/data/asm")                    # Custom symbols for inserted code
  file(COPY "../asm/patch_diffs" DESTINATION "${CMAKE_BINARY_DIR}/data/asm")                            # Diffs for precompiled ASM patches
  file(COPY  ../assets DESTINATION "${CMAKE_BINARY_DIR}/data")                                          # Images and other assets used in patching
  file(COPY "../logic/data" DESTINATION "${CMAKE_BINARY_DIR}/data/logic" REGEX "^.*example.*$" EXCLUDE) # World, macros, and location info
  file(COPY "../customizer/data" DESTINATION "${CMAKE_BINARY_DIR}/data/customizer")                     # Default model info
endif()

# Set Icon on Windows
file(WRITE "${CMAKE_CURRENT_SOURCE_DIR}/icon.rc" "IDI_ICON1 ICON DISCARDABLE \"appicon.ico\"")
set(app_icon_resource_windows "${CMAKE_CURRENT_SOURCE_DIR}/icon.rc")

# Set Icon on Mac
set(MACOSX_BUNDLE_ICON_FILE "app.icns")
set(app_icon_macos "${CMAKE_CURRENT_SOURCE_DIR}/app.icns")
set_source_files_properties(${app_icon_macos} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
        player_customization.cpp
        randomizer_thread.cpp
        tracker/set_font.h
        tracker/tracker_area_label.h
        tracker/tracker_area_label.cpp
        tracker/tracker_area_widget.h
        tracker/tracker_area_widget.cpp
        tracker/tracker_inventory_button.h
        tracker/tracker_inventory_button.cpp
        tracker/tracker_label.h
        tracker/tracker_label.cpp
        tracker/tracker.cpp
        ../randomizer.cpp
        ../tweaks.cpp
        ../options.cpp
        ../text_replacements.cpp
        ${app_icon_resource_windows}
        ${app_icon_macos}
        ${PROJECT_SOURCES}
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(wwhd_rando
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET wwhd_rando APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(wwhd_rando SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(wwhd_rando
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(wwhd_rando PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

set_target_properties(wwhd_rando PROPERTIES
    MACOSX_BUNDLE_wwhd_rando_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

install(TARGETS wwhd_rando
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

add_subdirectory("../libs" "libs")
add_subdirectory("../utility" "utility")
add_subdirectory("../command" "command")
add_subdirectory("../filetypes" "filetypes")
add_subdirectory("../seedgen" "seedgen")
add_subdirectory("../logic" "logic")
add_subdirectory("../nuspack" "nuspack")
add_subdirectory("../customizer" "customizer")

target_include_directories(wwhd_rando PUBLIC ${CMAKE_SOURCE_DIR}/..)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(wwhd_rando)
endif()
