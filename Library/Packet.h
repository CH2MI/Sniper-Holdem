#pragma once

#include <Windows.h>

namespace sniperholdem::packet
{
	struct PacketInfo
	{
		UINT16 ClientIndex;
		UINT16 PacketId;
		UINT16 DataSize;
		char* PacketData;
	};

	enum class ePacketID : UINT16
	{
		User_Connect = 11,
		User_Disconnect = 12,

		Login_Request = 201,
		Login_Response = 202,
	};

#pragma pack(push, 1)

	struct PacketHeader
	{
		UINT16 PacketLength;
		UINT16 PacketID;
	};

	constexpr UINT32 PACKET_HEADER_LENGTH = sizeof(PacketHeader);

	constexpr UINT32 MAX_USER_ID_LEN = 32;

	struct LoginRequestPacket : public PacketHeader
	{
		char UserID[MAX_USER_ID_LEN + 1];
	};
	constexpr size_t LOGIN_REQUEST_PACKET_SIZE = sizeof(LoginRequestPacket);

	struct LoginResponsePacket : public PacketHeader
	{
		UINT16 Result;
	};


#pragma pack(pop)
}