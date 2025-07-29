#pragma once

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Mswsock.lib")

#include <functional>
#include <thread>
#include <vector>

#include "ClientInfo.h"
#include "Define.h"

namespace sniperholdem::server::network
{
	

	class ServerNetwork
	{
	public:
		ServerNetwork();
		virtual ~ServerNetwork();

		bool InitSocket();
		bool BindAndListen(int bindPort);
		void StartServer(const UINT32 maxClientCount);
		void DestroyThread();
		bool SendMsg(const UINT32 sessionIndex, const UINT32 dataSize, char* pData);

		virtual void OnConnect(const UINT32 clientIndex) = 0;
		virtual void OnClose(const UINT32 clientIndex) = 0;
		virtual void OnReceive(const UINT32 clientIndex, const UINT32 size, char* pData) = 0;

	public:
		static std::function<bool(const UINT32, const UINT32, char*)> SendFunc;

	private:
		void createClient(const UINT32 maxClientCount);
		ClientInfo* getEmptyClientInfo();

		void createAccepterThread();
		void accepterThread();

		void createWorkerThread();
		void workerThread();

		void closeSocket(ClientInfo* pClientInfo, bool isForce = false);

	private:
		std::vector<ClientInfo*> mClientInfos;

		int mClientCnt;
		
		SOCKET mListenSocket;
		HANDLE mIOCPHandle;
		
		std::thread mAccepterThread;
		bool mIsAccepterRun;
		
		std::vector<std::thread> mIOWorkerThreads;
		bool mIsWorkerRun;
	};


}

