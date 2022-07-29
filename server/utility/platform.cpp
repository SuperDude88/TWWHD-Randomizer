#include "platform.hpp"
#include <thread>
#include <csignal>
#include <mutex>

#ifdef PLATFORM_DKP
	#include <whb/proc.h>
	#include <whb/log.h>
	#include <whb/log_console.h>
	#include <coreinit/mcp.h>
	#include <coreinit/ios.h>
	#include <coreinit/thread.h>
	#include <iosuhax.h>
	#include <iosuhax_devoptab.h>
	#include "../platform/wiiutitles.hpp"
	#define PRINTF_BUFFER_LENGTH 2048
	static int32_t mcpHookHandle = -1;
	static int32_t fsaHandle = -1;
	static int32_t iosuhaxHandle = -1;
	static bool iosuhaxOpen = false;
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
static bool initIOSUHax();
static void closeIosuhax();
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
#endif
		lock.unlock();
	}

	bool platformInit()
	{
#ifdef PLATFORM_DKP
		WHBProcInit();
		WHBLogConsoleInit();

		// retrieve WiiU title information
		std::vector<MCPTitleListType> rawTitles{};
		if(!getRawTitles(rawTitles))
		{
			Utility::platformLog("unable to get raw titles\n");
			platformShutdown();
			return false;
		}

		if(!initIOSUHax())
		{
			Utility::platformLog("unable to init IOSUHAX\n");
			platformShutdown();
			return false;
		}
		iosuhaxOpen = true;
		if(mount_fs("storage_mlc01", getFSAHandle(), NULL, "/vol/storage_mlc01") != 0)
		{
			Utility::platformLog("unable to mount MLC\n");
			platformShutdown();
			return false;
		}
		systemMLCMounted = true;

		if(!loadDetailedTitles(rawTitles, wiiuTitlesList))
		{
			Utility::platformLog("unable to load detailed titles\n");
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
		if (systemMLCMounted)
		{
			unmount_fs("storage_mlc01");
		}
		if (iosuhaxOpen)
		{
			closeIosuhax();
		}
		wiiuTitlesList.clear();
		WHBLogConsoleFree();
		WHBProcShutdown();
#endif
	}

	int32_t getFSAHandle()
	{
#ifdef PLATFORM_DKP
		return fsaHandle;
#else 
		return -1;
#endif
	}

	int32_t getMCPHandle()
	{
#ifdef PLATFORM_DKP
		return mcpHookHandle;
#else 
		return -1;
#endif
	}

	#ifdef PLATFORM_DKP
		const std::vector<Utility::titleEntry>* getLoadedTitles() {
			return &wiiuTitlesList;
		}
	#endif
}

#ifdef PLATFORM_DKP
void haxStartCallback(IOSError arg1, void *arg2) {
	Utility::platformLog("hasStartCallback arg1 = %d, arg2 = %p\n", (int)arg1, (int)arg2);
}


bool initIOSUHax()
{
	Utility::platformLog("starting IOSUHax...\n");

	iosuhaxHandle = IOSUHAX_Open(nullptr);
	if (iosuhaxHandle < 0) 
	{
		Utility::platformLog("Couldn't immediately open IOSUHAX, attempting to start\n");
		mcpHookHandle = MCP_Open();
		if (mcpHookHandle < 0)
		{
			Utility::platformLog("Unable to acquire mcp Hook handle\n");
			return false;
		}
		IOS_IoctlAsync(mcpHookHandle, 0x62, nullptr, 0, nullptr, 0, haxStartCallback, (void*)0);
		OSSleepTicks(OSSecondsToTicks(1));
		if(IOSUHAX_Open("/dev/mcp") < 0)
		{
			MCP_Close(mcpHookHandle);
			mcpHookHandle = -1;
			Utility::platformLog("Unable to open iosuhax /dev/mcp\n");
			return false;
		}
		OSSleepTicks(OSSecondsToTicks(5));
	}

	fsaHandle = IOSUHAX_FSA_Open();
    if (fsaHandle < 0) {
		closeIosuhax();
        Utility::platformLog("Couldn't open iosuhax FSA!\n");
        return false;
    }

	Utility::platformLog("attempting to mount mlc\n");
	if(mount_fs("storage_mlc01", fsaHandle, NULL, "/vol/storage_mlc01") < 0)
	{
		Utility::platformLog("failed to mount mlc: %d\n", errno);
		OSSleepTicks(OSSecondsToTicks(1));
		return false;
	}

	Utility::platformLog("IOSUHAX initialized, MLC mounted\n");
	return true;
}

void closeIosuhax() {
	if (fsaHandle >= 0) IOSUHAX_FSA_Close(fsaHandle);
	if (iosuhaxHandle >= 0) IOSUHAX_Close();
    if (mcpHookHandle >= 0) MCP_Close(mcpHookHandle);
    mcpHookHandle = -1;
    fsaHandle = -1;
    iosuhaxHandle = -1;
	iosuhaxOpen = false;
}

#endif
