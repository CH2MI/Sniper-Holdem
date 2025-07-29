#pragma once

#include <thread>
#include <unordered_map>
#include <mutex>
#include <deque>

#include "../ClientNetwork/ClientNetwork.h"
#include "../Library/RingBuffer.h"

namespace sniperholdem::client
{
	class PacketManager
	{
	public:
		static void Initialize();
		static void Start();
		static void Stop();

		static void ReceivePacket(const UINT32 dataSize, char* pData);

		static void RegisterPacketFunc(packet::ePacketID packetID, packet::PROCESS_RECV_PACKET_FUNCTION funcion);

	private:
		static void enqueuePacket();
		static packet::PacketInfo* dequeuePacket();
		static void processPacket();
		static void processRecvPacket(const UINT32 packetId, const UINT32 packetSize, char* pPacket);


	private:
		static std::unordered_map<int, packet::PROCESS_RECV_PACKET_FUNCTION> RecvFunctionDictionary;

		static bool IsRunProcessThread;
		static std::thread ProcessThread;
		static std::mutex Lock;

		static std::deque<packet::PacketInfo*> PacketQueue;

		static RingBuffer Buffer;
	};
}