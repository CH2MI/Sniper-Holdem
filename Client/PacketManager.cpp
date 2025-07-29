#include "PacketManager.h"
#include "PacketHandler.h"

#include <assert.h>


namespace sniperholdem::client
{
	std::unordered_map<int, packet::PROCESS_RECV_PACKET_FUNCTION> PacketManager::RecvFunctionDictionary{};

	bool PacketManager::IsRunProcessThread = false;
	std::thread PacketManager::ProcessThread{};
	std::mutex PacketManager::Lock{};

	std::deque<packet::PacketInfo*> PacketManager::PacketQueue{};

	RingBuffer PacketManager::Buffer{};

	void PacketManager::Initialize()
	{
		network::ClientNetwork::OnReceive = std::bind(PacketManager::ReceivePacket,
			std::placeholders::_1, std::placeholders::_2);

		PacketHandler::Initialize();
	}

	void PacketManager::Start()
	{ 
		IsRunProcessThread = true;
		ProcessThread = std::thread([]() { processPacket(); });
	}
	void PacketManager::Stop()
	{
		IsRunProcessThread = false;

		if (ProcessThread.joinable())
			ProcessThread.join();
	}

	void PacketManager::ReceivePacket(const UINT32 dataSize, char* pData)
	{ 
		if (Buffer.Write(dataSize, pData))
		{
			enqueuePacket();
		}
		else
		{
			assert(1);
		}
	}

	void PacketManager::RegisterPacketFunc(packet::ePacketID packetID, packet::PROCESS_RECV_PACKET_FUNCTION funcion)
	{
		RecvFunctionDictionary[static_cast<int>(packetID)] = funcion;
	}

	void PacketManager::enqueuePacket()
	{
		packet::PacketInfo* packet = nullptr;

		std::lock_guard<std::mutex> lock(Lock);
		while ((packet = Buffer.Read()) != nullptr)
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
					packet->PacketId,
					packet->DataSize,
					packet->PacketData
				);
			}

			if (isIdle)
				std::this_thread::sleep_for(std::chrono::milliseconds(1));

		}
	}

	void PacketManager::processRecvPacket(const UINT32 packetId, const UINT32 packetSize, char* pPacket)
	{ 
		auto itr = RecvFunctionDictionary.find(packetId);
		if (itr != RecvFunctionDictionary.end())
			(itr->second)(0, packetSize, pPacket);
	}

}