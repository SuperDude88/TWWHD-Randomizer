cmake_minimum_required(VERSION 3.7)
if(CMAKE_VERSION VERSION_LESS "3.13")
	if (POLICY CMP0076)
		cmake_policy(SET CMP0076 OLD)
	endif()
	
    target_sources(wwhd_rando_t4b PRIVATE nuspack/packer.cpp nuspack/appinfo.cpp)
else()
	cmake_policy(SET CMP0076 NEW)

        target_sources(wwhd_rando_t4b PRIVATE packer.cpp appinfo.cpp)
endif()

add_subdirectory("packaging")
add_subdirectory("fst")
add_subdirectory("crypto")
add_subdirectory("contents")
