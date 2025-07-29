#pragma once

#include <Windows.h>

#include "PacketManager.h"

namespace sniperholdem::server
{
	class PacketHandler
	{
	public:

		static void Init();

	private:

		static void processUserConnect(const UINT32 clientIndex, const UINT32 packetSize, char* pPacket);
		static void processUserDisconnect(const UINT32 clientIndex, const UINT32 packetSize, char* pPacket);
		static void processLoginRequest(const UINT32 clientIndex, const UINT32 packetSize, char* pPacket);
	};
}

