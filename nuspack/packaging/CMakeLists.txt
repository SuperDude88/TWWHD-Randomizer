cmake_minimum_required(VERSION 3.7)
if(CMAKE_VERSION VERSION_LESS "3.13")
	if (POLICY CMP0076)
		cmake_policy(SET CMP0076 OLD)
	endif()
	
    target_sources(wwhd_rando PRIVATE nuspack/packaging/ContentRules.cpp nuspack/packaging/ContentRulesService.cpp nuspack/packaging/fst.cpp nuspack/packaging/NUSPackage.cpp nuspack/packaging/ticket.cpp nuspack/packaging/tmd.cpp)
else()
	cmake_policy(SET CMP0076 NEW)

	target_sources(wwhd_rando PRIVATE ContentRules.cpp ContentRulesService.cpp fst.cpp NUSPackage.cpp ticket.cpp tmd.cpp)
endif()
