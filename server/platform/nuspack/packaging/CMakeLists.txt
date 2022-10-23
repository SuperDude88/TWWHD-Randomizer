cmake_minimum_required(VERSION 3.7)
if(CMAKE_VERSION VERSION_LESS "3.13")
	if (POLICY CMP0076)
		cmake_policy(SET CMP0076 OLD)
	endif()
	
    target_sources(wwhd_rando PRIVATE server/platform/nuspack/packaging/ContentRules.cpp server/platform/nuspack/packaging/ContentRulesService.cpp server/platform/nuspack/packaging/fst.cpp server/platform/nuspack/packaging/NUSPackage.cpp server/platform/nuspack/packaging/ticket.cpp server/platform/nuspack/packaging/tmd.cpp)
else()
	cmake_policy(SET CMP0076 NEW)

	target_sources(wwhd_rando PRIVATE ContentRules.cpp ContentRulesService.cpp fst.cpp NUSPackage.cpp ticket.cpp tmd.cpp)
endif()
