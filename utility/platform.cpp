#include "platform.hpp"
#include <thread>
#include <csignal>
#include <mutex>

#ifdef PLATFORM_DKP
	#include <whb/proc.h>
	#include <whb/log.h>
	#include <whb/log_console.h>
	#include <coreinit/mcp.h>
	#include <coreinit/thread.h>

	#include <mocha/mocha.h>

	#define PRINTF_BUFFER_LENGTH 2048

	static bool whbInit = false;
	static bool mochaOpen = false;
	static bool systemMLCMounted = false;
	static std::vector<Utility::titleEntry> wiiuTitlesList{};
#endif 

static bool _platformIsRunning = true;
static std::mutex printMut;

static void sigHandler(int signal)
{
	switch (signal)
	{
	case SIGINT:
#ifdef SIGBREAK
	case SIGBREAK:
#endif
		printf("ctrl+c\n");
		_platformIsRunning = false;
		break;
	}
}

#ifdef PLATFORM_DKP
bool initMocha()
{
	Utility::platformLog("Starting libmocha...\n");
	
	if(MochaUtilsStatus status = Mocha_InitLibrary(); status != MOCHA_RESULT_SUCCESS) {
		Utility::platformLog("Mocha_InitLibrary() failed\n");
		Utility::platformShutdown();
		return false;
	}

	Utility::platformLog("attempting to mount mlc\n");
	if(MochaUtilsStatus status = Mocha_MountFS("storage_mlc01", nullptr, "/vol/storage_mlc01"); status != MOCHA_RESULT_SUCCESS)
	{
		Utility::platformLog("Failed to mount mlc: %s\n", Mocha_GetStatusStr(status));
		OSSleepTicks(OSSecondsToTicks(1));
		return false;
	}

	systemMLCMounted = true;

	Utility::platformLog("Mocha initialized, MLC mounted\n");
	return true;
}

void closeMocha() {
	if(MochaUtilsStatus status = Mocha_UnmountFS("storage_mlc01"); status != MOCHA_RESULT_SUCCESS) {
		Utility::platformLog("Error unmounting mlc: %s\n", Mocha_GetStatusStr(status));
	}
	systemMLCMounted = false;

	if(MochaUtilsStatus status = Mocha_DeInitLibrary(); status != MOCHA_RESULT_SUCCESS) {
		Utility::platformLog("Mocha_DeinitLibrary() failed\n");
		Utility::platformShutdown();
	}

	return;
}
#endif

namespace Utility
{
	void platformLog(const char* f, ...)
	{
#ifdef PLATFORM_DKP
		char buf[PRINTF_BUFFER_LENGTH];
#endif
		va_list args;
		va_start(args, f);
		std::unique_lock<std::mutex> lock(printMut);
#ifdef PLATFORM_DKP
		vsnprintf(buf, PRINTF_BUFFER_LENGTH - 1, f, args);
		
		WHBLogWrite(buf);
		WHBLogConsoleDraw();
#else
		vprintf(f, args);
#endif
		lock.unlock();
		va_end(args);
	}

	void platformLog(const std::string& str)
	{
		std::unique_lock<std::mutex> lock(printMut);
#ifdef PLATFORM_DKP
		WHBLogWrite(str.c_str());
		WHBLogConsoleDraw();
#else
		printf("%s", str.c_str());
		fflush(stdout); //vscode debug console works better with this
#endif
		lock.unlock();
	}

	bool platformInit()
	{
#ifdef PLATFORM_DKP
		WHBProcInit();
		WHBLogConsoleInit();
		whbInit = true;

		// retrieve WiiU title information
		std::vector<MCPTitleListType> rawTitles{};
		if(!getRawTitles(rawTitles))
		{
			Utility::platformLog("Failed to get raw titles\n");
			platformShutdown();
			return false;
		}

		if(!initMocha())
		{
			Utility::platformLog("Failed to init libmocha\n");
			platformShutdown();
			return false;
		}
		mochaOpen = true;

		if(!loadDetailedTitles(rawTitles, wiiuTitlesList))
		{
			Utility::platformLog("Failed to load detailed titles\n");
			platformShutdown();
			return false;
		}
#else
		signal(SIGINT, sigHandler);
#ifdef SIGBREAK
		signal(SIGBREAK, sigHandler);
#endif
#endif
		return true;
	}

	

	bool platformIsRunning()
	{
#ifdef PLATFORM_DKP
		return WHBProcIsRunning();
#else
		return _platformIsRunning;
#endif
	}

	void waitForPlatformStop()
	{
		while (platformIsRunning())
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(200));
		}
	}

	void platformShutdown()
	{
#ifdef PLATFORM_DKP
		if (mochaOpen)
		{
			closeMocha();
			mochaOpen = false;
		}

		wiiuTitlesList.clear();

		if(whbInit) {
			WHBLogConsoleFree();
			WHBProcShutdown();
		}
#endif
	}

#ifdef PLATFORM_DKP
	const std::vector<Utility::titleEntry>* getLoadedTitles() {
		return &wiiuTitlesList;
	}
#endif
}
