#pragma once

#include <string>
#include <vector>
#include <filesystem>
#include <fstream>
#include <cstdarg>
#include <cstdio>
#include <cstdint>
#include <cstring>

#ifdef DEVKITPRO
	#define PLATFORM_DKP
	#include <platform/wiiutitles.hpp>
#elif defined(_MSC_VER)
	#define PLATFORM_MSVC
#elif defined(__GNUC__) || defined(__GNUG__)
	#define PLATFORM_GCC
#elif defined(__clang__)
	#define PLATFORM_CLANG
#else 
    #error UNKNOWN PLATFORM 
#endif

namespace Utility
{
	void platformLog(const char* f, ...);

	void platformLog(const std::string& str);

	bool platformInit();

	bool platformIsRunning();

	void waitForPlatformStop();

	void platformShutdown();

	#ifdef PLATFORM_DKP
		const std::vector<Utility::titleEntry>* getLoadedTitles();
	#endif
}
