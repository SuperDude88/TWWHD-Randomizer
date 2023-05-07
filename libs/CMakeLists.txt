cmake_minimum_required(VERSION 3.7)
if(CMAKE_VERSION VERSION_LESS "3.13")
	if (POLICY CMP0076)
		cmake_policy(SET CMP0076 OLD)
	endif()
	target_sources(wwhd_rando PRIVATE "libs/tinyxml2.cpp" "libs/Yaml.cpp" "libs/base64pp.cpp" "libs/AES.cpp" "libs/hashing/sha1.cpp" "libs/hashing/sha256.cpp")
else()
	cmake_policy(SET CMP0076 NEW)
	target_sources(wwhd_rando PRIVATE tinyxml2.cpp base64pp.cpp AES.cpp hashing/sha1.cpp hashing/sha256.cpp)
endif()

set(BUILD_SHARED_LIBS OFF)

# Modded zlib-ng for fast yaz0
add_compile_definitions(NO_GZIP=1)
add_subdirectory("zlib-ng")

# Disable extra stuff for yaml-cpp
option(YAML_CPP_BUILD_CONTRIB OFF)
option(YAML_CPP_BUILD_TOOLS OFF)
option(YAML_BUILD_SHARED_LIBS ${BUILD_SHARED_LIBS})
option(YAML_CPP_BUILD_TESTS OFF)
option(YAML_CPP_INSTALL OFF)
option(YAML_MSVC_SHARED_RT OFF)
add_subdirectory("yaml-cpp")
