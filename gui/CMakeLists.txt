cmake_minimum_required(VERSION 3.5)

project(wwhd_rando VERSION 0.1 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_definitions(JSON_NOEXCEPTION=1 JSON_USE_IMPLICIT_CONVERSIONS=0)
add_compile_options(-O3)

add_compile_definitions(QT_GUI=1)

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
endif()

# Version strings for RNG and compatibility checks
add_compile_definitions(RANDOMIZER_VERSION="01.00.00a")
add_compile_definitions(CONFIG_VERSION="1.0")

# Path strings for logging
string(LENGTH "${CMAKE_SOURCE_DIR}/" SOURCE_PATH_SIZE)
add_compile_definitions(SOURCE_PATH_SIZE=${SOURCE_PATH_SIZE})

# zlib for RPX (de)compression
if(WIN32)
  # Bundle a zlib static library for Windows
  set(ZLIB_LIBRARY "${CMAKE_SOURCE_DIR}/../libs/zlib_win/zlibstat.lib")
  set(ZLIB_INCLUDE_DIR "../libs/zlib_win")
  add_library(zlib STATIC IMPORTED)
  set_target_properties(zlib PROPERTIES IMPORTED_LOCATION ${ZLIB_LIBRARY})
else()
  include(FindPkgConfig)
  pkg_check_modules(ZLIB zlib REQUIRED)
endif()

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

add_compile_definitions(DATA_PATH="./data/" APP_SAVE_PATH="./" TEMP_DIR="./temp/" RESOURCE_PATH=":/")

# This flag should only be used when generating static executables for release
if(EMBED_DATA)
  message("Data will be embeded")
  add_compile_definitions(EMBED_DATA)
  find_package(PythonInterp REQUIRED)
  execute_process(COMMAND "${PYTHON_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/generate_qrc_file.py" WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} RESULT_VARIABLE QRC_GENERATE_RESULT)
  if(NOT QRC_GENERATE_RESULT EQUAL "0")
    message("Failed to generate data.qrc")
  else()
    set(PROJECT_SOURCES data.qrc ${PROJECT_SOURCES})
  endif()
else()
  # Put data files together for easier manipulation
  file(COPY "../asm/custom_symbols.json" DESTINATION "${CMAKE_BINARY_DIR}/data/asm")                    # Custom symbols for inserted code
  file(COPY "../asm/patch_diffs" DESTINATION "${CMAKE_BINARY_DIR}/data/asm")                            # Diffs for precompiled ASM patches
  file(COPY  ../assets DESTINATION "${CMAKE_BINARY_DIR}/data")                                          # Images and other assets used in patching
  file(COPY "../logic/data" DESTINATION "${CMAKE_BINARY_DIR}/data/logic" REGEX "^.*example.*$" EXCLUDE) # World, macros, and location info
endif()

file(WRITE "${CMAKE_CURRENT_SOURCE_DIR}/icon.rc" "IDI_ICON1 ICON DISCARDABLE \"appicon.ico\"")
set(app_icon_resource_windows "${CMAKE_CURRENT_SOURCE_DIR}/icon.rc")

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
        randomizer_thread.cpp
        ../randomizer.cpp
        ../tweaks.cpp
        ../options.cpp
        ../text_replacements.cpp
        ${app_icon_resource_windows}
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

target_include_directories(wwhd_rando PUBLIC ${CMAKE_SOURCE_DIR}/..)

if(WIN32)
  target_link_libraries(wwhd_rando PRIVATE zlib)
  target_include_directories(wwhd_rando PRIVATE ${ZLIB_INCLUDE_DIR})
else()
  target_link_libraries(wwhd_rando PRIVATE ${ZLIB_LDFLAGS})
  target_compile_options(wwhd_rando PRIVATE ${ZLIB_CFLAGS})
endif()

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(wwhd_rando)
endif()
