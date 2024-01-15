cmake_minimum_required(VERSION 3.7)
if(CMAKE_VERSION VERSION_LESS "3.13")
	if (POLICY CMP0076)
		cmake_policy(SET CMP0076 OLD)
	endif()
	
    target_sources(wwhd_rando PRIVATE platform/gui/screen.cpp platform/gui/LogConsole.cpp platform/gui/ConfirmMenu.cpp platform/gui/InstallMenu.cpp platform/gui/SettingsMenu.cpp platform/gui/Page.cpp platform/gui/Button.cpp platform/gui/OptionActions.cpp platform/gui/TextWrap.cpp platform/gui/Keyboard.cpp)
else()
	cmake_policy(SET CMP0076 NEW)

	target_sources(wwhd_rando PRIVATE screen.cpp LogConsole.cpp ConfirmMenu.cpp InstallMenu.cpp SettingsMenu.cpp Page.cpp Button.cpp OptionActions.cpp TextWrap.cpp Keyboard.cpp)
endif()
