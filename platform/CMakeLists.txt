# devkitpro specific sources

cmake_minimum_required(VERSION 3.7)
if(CMAKE_VERSION VERSION_LESS "3.13")
	if (POLICY CMP0076)
		cmake_policy(SET CMP0076 OLD)
	endif()
	target_sources(wwhd_rando PRIVATE platform/channel.cpp platform/proc.cpp platform/heap.cpp platform/home.cpp platform/energy_saver.cpp platform/input.cpp)
else()
	cmake_policy(SET CMP0076 NEW)
	target_sources(wwhd_rando PRIVATE channel.cpp proc.cpp heap.cpp home.cpp energy_saver.cpp input.cpp)
endif()
