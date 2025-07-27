#pragma once

#include "../IOCPServer/IOCPServer.h"

namespace sniperholdem::server
{
	class Server : public iocpserver::IOCPServer
	{
	public:
		Server();
		~Server();

		void OnConnect(const UINT32 clientIndex) override;
		void OnClose(const UINT32 clientIndex) override;
		void OnReceive(const UINT32 clientIndex, const UINT32 size, char* pData) override;
	};
}