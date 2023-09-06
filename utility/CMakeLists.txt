cmake_minimum_required(VERSION 3.7)


if(CMAKE_VERSION VERSION_LESS "3.13")
	if (POLICY CMP0076)
		cmake_policy(SET CMP0076 OLD)
	endif()
	target_sources(wwhd_rando PRIVATE utility/platform.cpp utility/endian.cpp utility/common.cpp utility/file.cpp utility/string.cpp utility/text.cpp utility/time.cpp utility/color.cpp utility/pointer.cpp)
else()
	cmake_policy(SET CMP0076 NEW)
	target_sources(wwhd_rando PRIVATE platform.cpp endian.cpp common.cpp file.cpp string.cpp text.cpp time.cpp color.cpp pointer.cpp)
endif()
