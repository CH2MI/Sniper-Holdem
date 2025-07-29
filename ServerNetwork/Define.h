#pragma once

#include <winsock2.h>
#include <Ws2tcpip.h>
#include <mswsock.h>

namespace sniperholdem
{
	constexpr UINT32 MAX_SOCKBUF = 1024;
	constexpr UINT32 MAX_WORKERTHREAD = 4;
	constexpr UINT64 RE_USE_SESSION_WAIT_TIME_SEC = 3;

	enum class IOOperation
	{
		Accpet,
		Recv,
		Send
	};

	struct OverlappedEx
	{
		WSAOVERLAPPED WsaOverlapped;
		WSABUF WsaBuf;
		IOOperation Operation;
		UINT32 SessionIndex;
	};
}