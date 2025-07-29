#pragma once

#include <winsock2.h>
#include <Ws2tcpip.h>
#include <mswsock.h>
#include <cstdio>
#include <string>
#include <deque>
#include <mutex>
#include <thread>
#include <functional>

#include "../Library/Packet.h"

namespace sniperholdem::client::network
{
	constexpr UINT32 MAX_SOCKBUF = 1024;

	class ClientNetwork
	{

	public:

		static bool Initialize();
		static bool Connect(std::string serverIP, int serverPort);
		static void Start();
		static void Stop();

		static void Send(packet::PacketHeader* packet);

	public:

		static std::function<void()> OnConnect;
		static std::function<void()> OnDisconnect;
		static std::function<void(const UINT32, char* pData)> OnReceive;

	private:

		static void createRecvThread();
		static void recvPacket();

		static void createSendThread();
		static void sendPacket();

	private:

		static SOCKET ConnectSocket;
		static char RecvBuffer[MAX_SOCKBUF];

		static std::thread RecvThread;
		static bool IsRecvThreadRun;

		static std::thread SendThread;
		static bool IsSendThreadRun;

		static std::deque<packet::PacketHeader*> SendQueue;
		static std::mutex SendLock;
		

	};

}

