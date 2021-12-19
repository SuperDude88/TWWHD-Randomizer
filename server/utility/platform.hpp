#pragma once

#include <cstdarg>
#include <cstdio>
#include <cstdint>

#ifdef DEVKITPRO
	#define PLATFORM_DKP
#elif defined(_MSC_VER)
	#define PLATFORM_MSVC
	#define ZLIB_WINAPI 
#elif defined(__GNUC__) || defined(__GNUG__)
	#define PLATFORM_GCC
#elif defined(__clang__)
	#define PLATFORM_CLANG
#else 
    #error UNKNOWN PLATFORM 
#endif


// if this gets sufficiently extensive, we'll want to break out into different
// target files/directories and use cmake to subtitute pre-makefile

namespace Utility
{
	void platformLog(const char* f, ...);

	bool platformInit();

	bool platformIsRunning();

	void waitForPlatformStop();

	void platformShutdown();

	// these functions are always -1 in a non-homebrew context
	int32_t getMCPHandle();

	int32_t getFSAHandle();
}
