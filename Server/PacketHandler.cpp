#include "Server.h"
#include "PacketHandler.h"
#include "../Library/Packet.h"

namespace sniperholdem::server
{
	void PacketHandler::Init()
	{
		PacketManager::RegisterPacketFunc(packet::ePacketID::User_Connect, processUserConnect);
		PacketManager::RegisterPacketFunc(packet::ePacketID::User_Disconnect, processUserDisconnect);
		PacketManager::RegisterPacketFunc(packet::ePacketID::Login_Request, processLoginRequest);

	}

	void PacketHandler::processUserConnect(const UINT32 clientIndex, const UINT32 packetSize, char* pPacket)
	{
		printf("[UserConnect] clientIndex : %d\n", clientIndex);

		auto pUser = UserManager::GetUserByIndex(clientIndex);
		UserManager::DeleteUserInfo(pUser);
	}

	void PacketHandler::processUserDisconnect(const UINT32 clientIndex, const UINT32 packetSize, char* pPacket)
	{
		printf("[UserDisconnect] clientIndex : %d\n", clientIndex);
	}

	void PacketHandler::processLoginRequest(const UINT32 clientIndex, const UINT32 packetSize, char* pPacket)
	{
		if (packetSize != packet::LOGIN_REQUEST_PACKET_SIZE)
			return;

		packet::LoginRequestPacket* loginRequestPacket = reinterpret_cast<packet::LoginRequestPacket*>(pPacket);

		packet::LoginResponsePacket* loginResponsePacket = new packet::LoginResponsePacket;
		loginResponsePacket->PacketID = (UINT16)packet::ePacketID::Login_Response;
		loginResponsePacket->PacketLength = packet::LOGIN_RESPONSE_PACKET_SIZE;

		if (UserManager::FindUserIndexByID(loginRequestPacket->UserID) == UINT32_MAX)
		{
			// Login Success !
			loginResponsePacket->Result = (UINT16)packet::eErrorCode::None;
			UserManager::AddUser(loginRequestPacket->UserID, clientIndex);
			printf("Client %s(%d) Login success!\n", loginRequestPacket->UserID, clientIndex);
		}
		else
		{
			// Login Failed !
			loginResponsePacket->Result = (UINT16)packet::eErrorCode::Login_User_Already;
			printf("Client %s(%d) Login failed!\n", loginRequestPacket->UserID, clientIndex);

		}

		delete loginRequestPacket;
		
		Server::SendFunc(clientIndex, loginResponsePacket->PacketLength, reinterpret_cast<char*>(loginResponsePacket));
	}
}