#include "PacketManager.h"
#include "PacketHandler.h"
#include "../Library/ErrorCode.h"	


namespace sniperholdem::client
{
	void PacketHandler::Initialize()
	{
		PacketManager::RegisterPacketFunc(packet::ePacketID::Login_Response, processLoginResponse);
	}

	void PacketHandler::processLoginResponse(const UINT32 clientIndex, const UINT32 packetSize, char* pPacket)
	{
		if (packetSize != packet::LOGIN_RESPONSE_PACKET_SIZE)
			return;

		auto loginResponsePacket = reinterpret_cast<packet::LoginResponsePacket*>(pPacket);

		if ((packet::eErrorCode)loginResponsePacket->Result == packet::eErrorCode::None)
		{
			printf("Login Success!!!\n");
		}
		else
		{
			printf("Login Failed");
		}
	}
}