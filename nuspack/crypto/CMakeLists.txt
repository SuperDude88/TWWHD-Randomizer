cmake_minimum_required(VERSION 3.7)
if(CMAKE_VERSION VERSION_LESS "3.13")
	if (POLICY CMP0076)
		cmake_policy(SET CMP0076 OLD)
	endif()
	
    target_sources(wwhd_rando_t4b PRIVATE nuspack/crypto/ContentHashes.cpp nuspack/crypto/Encryption.cpp)
else()
	cmake_policy(SET CMP0076 NEW)

        target_sources(wwhd_rando_t4b PRIVATE ContentHashes.cpp Encryption.cpp)
endif()
