#pragma once

#include <Windows.h>


namespace sniperholdem::client
{
	class PacketHandler
	{
	public:

		static void Initialize();

	private:

		static void processLoginResponse(const UINT32 clientIndex, const UINT32 packetSize, char* pPacket);
		
	};
}

