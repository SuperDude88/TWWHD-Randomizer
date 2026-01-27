cmake_minimum_required(VERSION 3.13)

set_target_properties(wwhd_rando PROPERTIES AUTOUIC ON)
set_target_properties(wwhd_rando PROPERTIES AUTOMOC ON)
set_target_properties(wwhd_rando PROPERTIES AUTORCC ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_compile_options(-O3)

# Add UI assets to our data folder
file(COPY "tracker/data/" DESTINATION "${CMAKE_BINARY_DIR}/data/tracker") # Tracker assets

# Embeds the data files into the Qt executable
if(DEFINED EMBED_DATA)
  message("Data will be embedded")

  target_compile_definitions(wwhd_rando PRIVATE EMBED_DATA)

  find_package(Python REQUIRED)
  execute_process(COMMAND "${Python_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/generate_qrc_file.py" "${CMAKE_BINARY_DIR}/data/" WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} RESULT_VARIABLE QRC_GENERATE_RESULT)
  if(NOT QRC_GENERATE_RESULT EQUAL "0")
    message("Failed to generate data.qrc")
  else()
    target_sources(wwhd_rando PRIVATE data.qrc)
  endif()
endif()

# Set Icon on Windows
file(WRITE "${CMAKE_CURRENT_SOURCE_DIR}/icon.rc" "IDI_ICON1 ICON DISCARDABLE \"appicon.ico\"")
set(app_icon_resource_windows "${CMAKE_CURRENT_SOURCE_DIR}/icon.rc")

target_sources(wwhd_rando PRIVATE mainwindow.cpp mainwindow.ui player_customization.cpp
                                  randomizer_thread.cpp palette.cpp tracker/tracker_area_label.cpp
                                  tracker/tracker_area_widget.cpp tracker/tracker_inventory_button.cpp
                                  tracker/tracker_label.cpp tracker/tracker.cpp
                                  tracker/tracker_preferences_dialog.ui tracker/tracker_preferences_dialog.cpp
                                  tracker/tracker_required_boss_checkbox.cpp
                                  ${app_icon_resource_windows} ${app_icon_macos})

target_link_libraries(wwhd_rando PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

set_target_properties(wwhd_rando PROPERTIES
    MACOSX_BUNDLE_wwhd_rando_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

install(TARGETS wwhd_rando BUNDLE DESTINATION . LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
