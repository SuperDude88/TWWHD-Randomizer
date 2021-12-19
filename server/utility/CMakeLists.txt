cmake_minimum_required(VERSION 3.7)


if(CMAKE_VERSION VERSION_LESS "3.13")
	if (POLICY CMP0076)
		cmake_policy(SET CMP0076 OLD)
	endif()
	target_sources(wwhd_rando_patch PRIVATE server/utility/byteswap.cpp server/utility/platform.cpp server/utility/platform_socket.cpp)
	if(DEFINED DEVKITPRO)
		# devkitpro specific sources
		target_sources(wwhd_rando_patch PRIVATE server/utility/wiiutitles.cpp)
	endif()
else()
	cmake_policy(SET CMP0076 NEW)
	target_sources(wwhd_rando_patch PRIVATE byteswap.cpp platform.cpp)
	if(DEFINED DEVKITPRO)
		target_sources(wwhd_rando_patch PRIVATE wiiutitles.cpp)
	endif()
endif()
