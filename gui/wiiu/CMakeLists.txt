cmake_minimum_required(VERSION 3.13)

target_sources(wwhd_rando PRIVATE screen.cpp LogConsole.cpp ConfirmMenu.cpp InstallMenu.cpp ExitMenu.cpp SettingsMenu.cpp Page.cpp Button.cpp OptionActions.cpp TextWrap.cpp Keyboard.cpp)

# Use librpxloader to get the redirected save path
find_library(LIBRPXLOADER rpxloader HINTS "${DEVKITPRO}/wums/lib")
if(LIBRPXLOADER)
	message("Building with librpxloader")
	target_compile_definitions(wwhd_rando PRIVATE USE_LIBRPXLOADER)
	target_include_directories(wwhd_rando PRIVATE "${DEVKITPRO}/wums/include")
	target_link_libraries(wwhd_rando PRIVATE "${LIBRPXLOADER}")
endif()
