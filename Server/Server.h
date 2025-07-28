#pragma once

#include "../IOCPServer/ServerNetwork.h"

namespace sniperholdem::server
{
	class Server : public network::IOCPServer
	{
	public:
		Server();
		~Server();

		void OnConnect(const UINT32 clientIndex) override;
		void OnClose(const UINT32 clientIndex) override;
		void OnReceive(const UINT32 clientIndex, const UINT32 size, char* pData) override;
	};
}