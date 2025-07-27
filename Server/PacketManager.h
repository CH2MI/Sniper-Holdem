#pragma once

#include <thread>
#include <unordered_map>
#include <Windows.h>
#include <mutex>
#include <deque>

#include "../Library/Packet.h"
#include "UserManager.h"

namespace sniperholdem::server
{
	

	class PacketManager
	{
		typedef void(*PROCESS_RECV_PACKET_FUNCTION)(UINT32, UINT32, char*);

	public:
		static void Init(const UINT32 maxClient);
		static void Run();
		static void End();

		static void ReceivePacket(const UINT32 clientIndex, const UINT32 dataSize, char* pData);

		static void RegisterPacketFunc(packet::ePacketID packetID, PROCESS_RECV_PACKET_FUNCTION funcion);

	private:

		static void enqueuePacket(User* pUser);
		static packet::PacketInfo* dequeuePacket();
		static void processPacket();
		static void processRecvPacket(const UINT32 clientIndex, const UINT32 packetId, const UINT32 packetSize, char* pPacket);


	private:

		static std::unordered_map<int, PROCESS_RECV_PACKET_FUNCTION> RecvFunctionDictionary;

		static bool IsRunProcessThread;
		static std::thread ProcessThread;
		static std::mutex Lock;

		static std::deque<packet::PacketInfo*> PacketQueue;

	};
}