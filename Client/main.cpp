#define WIN32_LEAN_AND_MEAN

#include <iostream>
#include <string>

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Mswsock.lib")

#include "PacketManager.h"

using namespace sniperholdem;

int main() 
{
	bool bret = client::network::ClientNetwork::Initialize();
	if (!bret)
		return 1;

	bret = client::network::ClientNetwork::Connect("127.0.0.1", 8888);
	if (!bret)
		return 1;

	client::PacketManager::Initialize();
	client::PacketManager::Start();


	client::network::ClientNetwork::Start();

	std::string s;
	std::getline(std::cin, s);

	packet::LoginRequestPacket* loginRequestPacket = new packet::LoginRequestPacket;
	loginRequestPacket->PacketID = (UINT16)packet::ePacketID::Login_Request;
	loginRequestPacket->PacketLength = packet::LOGIN_REQUEST_PACKET_SIZE;
	CopyMemory(loginRequestPacket->UserID, s.c_str(), max(packet::MAX_USER_ID_LEN, s.size()));
	client::network::ClientNetwork::Send(loginRequestPacket);

	while (true)
	{

	}
}
