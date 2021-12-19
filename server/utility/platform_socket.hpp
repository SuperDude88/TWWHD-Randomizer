#pragma once

#include "platform.hpp"

#ifdef PLATFORM_MSVC
	#include <winsock2.h>
	#include <ws2tcpip.h>
	#pragma comment(lib, "Ws2_32.lib")
	#define SocketType SOCKET
	#define SOCK_POLL WSAPoll
	#define SOCK_CLOSE closesocket
	#define NONBLOCK_RECV_FLAGS 0
#else
	#include <netinet/in.h>
	#define SocketType int
	#define SOCK_POLL poll
	#define SOCK_CLOSE close
	#define INVALID_SOCKET -1
	#define NONBLOCK_RECV_FLAGS MSG_DONTWAIT
#endif

#if defined(PLATFORM_DKP)
	#include <nn/ac.h>
#endif

namespace Utility
{
	bool netInit();

	void netShutdown();

	bool isSocketInvalid(SocketType sock);

	bool setSocketBlockingFlag(SocketType sock, bool blocking);

	bool getSocketBytesAvailable(SocketType sock, size_t& bytesAvailable);
}
