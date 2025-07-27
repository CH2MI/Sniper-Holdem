#include "PacketHandler.h"
#include "../Library/Packet.h"

namespace sniperholdem::server
{
	void PacketHandler::Init()
	{
		PacketManager::RegisterPacketFunc(packet::ePacketID::User_Connect, processUserConnect);
		PacketManager::RegisterPacketFunc(packet::ePacketID::User_Disconnect, processUserDisconnect);
	}

	void PacketHandler::processUserConnect(const UINT32 clientIndex, const UINT32 packetSize, char* pPacket)
	{
		printf("[UserConnect] clientIndex : %d\n", clientIndex);

		auto pUser = UserManager::GetUserByIndex(clientIndex);
		pUser->Clear();
	}

	void PacketHandler::processUserDisconnect(const UINT32 clientIndex, const UINT32 packetSize, char* pPacket)
	{
		printf("[UserDisconnect] clientIndex : %d\n", clientIndex);
	}
}