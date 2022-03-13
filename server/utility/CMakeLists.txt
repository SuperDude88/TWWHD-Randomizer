cmake_minimum_required(VERSION 3.7)


if(CMAKE_VERSION VERSION_LESS "3.13")
	if (POLICY CMP0076)
		cmake_policy(SET CMP0076 OLD)
	endif()
	target_sources(wwhd_rando PRIVATE server/utility/endian.cpp server/utility/platform.cpp server/utility/common.cpp)
	if(DEFINED DEVKITPRO)
		# devkitpro specific sources
		target_sources(wwhd_rando PRIVATE server/utility/wiiutitles.cpp)
	endif()
else()
	cmake_policy(SET CMP0076 NEW)
	target_sources(wwhd_rando PRIVATE endian.cpp platform.cpp common.cpp)
	if(DEFINED DEVKITPRO)
		target_sources(wwhd_rando PRIVATE wiiutitles.cpp)
	endif()
endif()
