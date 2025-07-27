#include "PacketManager.h"
#include "PacketHandler.h"
#include <assert.h>

namespace sniperholdem::server
{
	std::unordered_map<int, PacketManager::PROCESS_RECV_PACKET_FUNCTION> PacketManager::RecvFunctionDictionary{};
	bool PacketManager::IsRunProcessThread = false;
	std::thread PacketManager::ProcessThread{};
	std::mutex PacketManager::Lock{};
	std::deque<packet::PacketInfo*> PacketManager::PacketQueue{};

	void PacketManager::Init(const UINT32 maxClient)
	{
		UserManager::Init(maxClient);
		PacketHandler::Init();
	}

	void PacketManager::Run()
	{
		IsRunProcessThread = true;
		ProcessThread = std::thread([]() { processPacket(); });
	}

	void PacketManager::End()
	{
		IsRunProcessThread = false;

		if (ProcessThread.joinable())
			ProcessThread.join();
	}

	void PacketManager::ReceivePacket(const UINT32 clientIndex, const UINT32 dataSize, char* pData)
	{ 
		auto pUser = UserManager::GetUserByIndex(clientIndex);
		if (pUser->Write(dataSize, pData))
		{
			enqueuePacket(pUser);

		}
		else
		{
			assert(1);
		}
	}

	void PacketManager::RegisterPacketFunc(packet::ePacketID packetID, PROCESS_RECV_PACKET_FUNCTION function)
	{
		RecvFunctionDictionary[static_cast<int>(packetID)] = function;
	}

	void PacketManager::enqueuePacket(User* pUser)
	{
		packet::PacketInfo* packet = nullptr;
		
		std::lock_guard<std::mutex> lock(Lock);
		while ((packet = pUser->Read()) != nullptr)
		{
			PacketQueue.push_back(packet);
		}
	}

	packet::PacketInfo* PacketManager::dequeuePacket()
	{
		std::lock_guard<std::mutex> lock(Lock);

		if (PacketQueue.empty())
			return nullptr;

		auto packet = PacketQueue.front();
		PacketQueue.pop_front();

		return packet;
	}

	void PacketManager::processPacket()
	{
		while (IsRunProcessThread)
		{
			bool isIdle = true;

			auto packet = dequeuePacket();
			if (packet != nullptr)
			{
				isIdle = false;
				processRecvPacket
				(
					packet->ClientIndex,
					packet->PacketId,
					packet->DataSize,
					packet->PacketData
				);
			}

			if (isIdle)
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}
	}

	void PacketManager::processRecvPacket(const UINT32 clientIndex, const UINT32 packetId, const UINT32 packetSize, char* pPacket)
	{
		auto itr = RecvFunctionDictionary.find(packetId);
		if (itr != RecvFunctionDictionary.end())
		{
			(itr->second)(clientIndex, packetSize, pPacket);
		}
	}
}