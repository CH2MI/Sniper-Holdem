#include "Server.h"
#include "../Library/Packet.h"
#include "PacketManager.h"

namespace sniperholdem::server
{
	Server::Server() : ServerNetwork::ServerNetwork()
	{
	}

	Server::~Server()
	{
	}

	void Server::OnConnect(const UINT32 clientIndex)
	{
		//printf("[OnConnect] %d\n", clientIndex);
		
		auto packetHeader = packet::PacketHeader{};
		packetHeader.PacketID = static_cast<UINT16>(packet::ePacketID::User_Connect);
		packetHeader.PacketLength = sizeof(packet::PacketHeader);
		PacketManager::ReceivePacket(clientIndex, sizeof(packet::PacketHeader), reinterpret_cast<char*>(&packetHeader));
	}

	void Server::OnClose(const UINT32 clientIndex)
	{
		//printf("[OnClose] %d\n", clientIndex);

		auto packetHeader = packet::PacketHeader{};
		packetHeader.PacketID = static_cast<UINT16>(packet::ePacketID::User_Disconnect);
		packetHeader.PacketLength = sizeof(packet::PacketHeader);
		PacketManager::ReceivePacket(clientIndex, sizeof(packet::PacketHeader), reinterpret_cast<char*>(&packetHeader));
	}

	void Server::OnReceive(const UINT32 clientIndex, const UINT32 size, char* pData)
	{
		/*pData[size] = '\n';
		printf("[OnReceive] %d : %s\n", clientIndex, pData);*/
		PacketManager::ReceivePacket(clientIndex, size, pData);
		//SendFunc(clientIndex, size, pData);
	}
}