#include "ServerNetwork.h"

namespace sniperholdem::server::network
{
	std::function<bool(const UINT32, const UINT32, char*)> ServerNetwork::SendFunc{};

	ServerNetwork::ServerNetwork() :
		mClientCnt(0),
		mClientInfos{},
		mListenSocket(INVALID_SOCKET),
		mIOCPHandle(INVALID_HANDLE_VALUE),
		mAccepterThread{},
		mIsAccepterRun(false),
		mIOWorkerThreads{},
		mIsWorkerRun{}
	{
	}


	ServerNetwork::~ServerNetwork()
	{
		for (auto client : mClientInfos)
		{
			delete client;
			client = nullptr;
		}

		WSACleanup();
	}

	bool ServerNetwork::InitSocket()
	{
		WSADATA wsaData;

		int ret = WSAStartup(MAKEWORD(2, 2), &wsaData);
		if (0 != ret)
		{
			printf("[Error] WSAStartup Error : %d\n", WSAGetLastError());
			return false;
		}

		mListenSocket = WSASocket
		(
			AF_INET,
			SOCK_STREAM,
			0,
			nullptr,
			0,
			WSA_FLAG_OVERLAPPED
		);
		if (INVALID_SOCKET == mListenSocket)
		{
			printf("[Error] WSASocket Error : %d\n", WSAGetLastError());
			return false;
		}

		printf("Socket Init Complete\n");

		return true;
	}
	bool ServerNetwork::BindAndListen(int bindPort)
	{
		SOCKADDR_IN serverAddr;
		serverAddr.sin_family = AF_INET;
		serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
		serverAddr.sin_port = htons(bindPort);

		int ret = bind
		(
			mListenSocket,
			reinterpret_cast<SOCKADDR*>(&serverAddr),
			sizeof(SOCKADDR_IN)
		);
		if (0 != ret)
		{
			printf("[Error] bind Error : %d\n", WSAGetLastError());
			return false;
		}

		ret = listen(mListenSocket, 5);
		if (0 != ret)
		{
			printf("[Error] listen Error : %d\n", WSAGetLastError());
			return false;
		}

		mIOCPHandle = CreateIoCompletionPort
		(
			INVALID_HANDLE_VALUE,
			nullptr,
			0,
			0
		);
		if (INVALID_HANDLE_VALUE == mIOCPHandle)
		{
			printf("[Error] CreateIoCompletionPort create Error : %d\n", WSAGetLastError());
			return false;
		}

		HANDLE iocpHandle = CreateIoCompletionPort
		(
			reinterpret_cast<HANDLE>(mListenSocket),
			mIOCPHandle,
			0,
			0
		);
		if (nullptr == iocpHandle)
		{
			printf("[Error] CreateIoCompletionPort bind Error : %d\n", WSAGetLastError());
			return false;
		}

		printf("Socket setting complete\n");

		return true;
	}
	void ServerNetwork::StartServer(const UINT32 maxClientCount)
	{
		createClient(maxClientCount);
		createWorkerThread();
		createAccepterThread();
		
		printf("Server start\n");
	}
	void ServerNetwork::DestroyThread()
	{
		mIsAccepterRun = false;
		closesocket(mListenSocket);

		if (mAccepterThread.joinable())
			mAccepterThread.join();

		mIsWorkerRun = false;
		CloseHandle(mIOCPHandle);

		for (auto& thread : mIOWorkerThreads)
		{
			if (thread.joinable())
				thread.join();
		}

	}

	bool ServerNetwork::SendMsg(const UINT32 sessionIndex, const UINT32 dataSize, char* pData)
	{
		auto pClient = mClientInfos[sessionIndex];
		return pClient->SendMsg(dataSize, pData);
	}

	void ServerNetwork::createClient(const UINT32 maxClientCount)
	{
		for (size_t i = 0; i < maxClientCount; i++)
		{
			auto client = new ClientInfo();
			client->Init(i, mIOCPHandle);

			mClientInfos.push_back(client);
		}

		SendFunc = std::bind(&ServerNetwork::SendMsg, this,
			std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	}

	ClientInfo* ServerNetwork::getEmptyClientInfo()
	{
		for (auto& client : mClientInfos)
		{
			if (!client->IsConnect())
				return client;
		}

		return nullptr;
	}

	void ServerNetwork::createAccepterThread()
	{
		mIsAccepterRun = true;

		mAccepterThread = std::thread([this]() { accepterThread(); });

		printf("AccpeterThread starts\n");
	}

	void ServerNetwork::accepterThread()
	{
		while (mIsAccepterRun)
		{
			UINT64 curTimeSec = 
				std::chrono::duration_cast<std::chrono::seconds>
				(
					std::chrono::steady_clock::now().time_since_epoch()
				).count();

			for (auto client : mClientInfos)
			{
				if (client->IsConnect())
					continue;

				if (curTimeSec < client->GetLatestClosedTimeSec())
					continue;

				if (curTimeSec - client->GetLatestClosedTimeSec() <= RE_USE_SESSION_WAIT_TIME_SEC)
					continue;

				client->ReserveAccept(mListenSocket, curTimeSec);
			}

			std::this_thread::sleep_for(std::chrono::milliseconds(32));
		}
	}

	void ServerNetwork::createWorkerThread()
	{
		mIsWorkerRun = true;

		for (size_t i = 0; i < MAX_WORKERTHREAD; i++)
		{
			mIOWorkerThreads.emplace_back([this]() { workerThread(); });
		}

		printf("WorkerThread starts\n");
	}

	void ServerNetwork::workerThread()
	{
		ClientInfo* pClientInfo = nullptr;

		BOOL isSuccess = true;
		DWORD dwIoSize = 0;

		LPOVERLAPPED lpoverlapped = nullptr;

		while (mIsWorkerRun)
		{
			isSuccess = GetQueuedCompletionStatus
			(
				mIOCPHandle,
				&dwIoSize,
				reinterpret_cast<PULONG_PTR>(&pClientInfo),
				&lpoverlapped,
				INFINITE
			);

			if (FALSE == isSuccess && 0 == dwIoSize && nullptr == lpoverlapped)
			{
				mIsWorkerRun = false;
				continue;
			}

			if (nullptr == lpoverlapped)
				continue;

			auto pOverlappedEx = reinterpret_cast<OverlappedEx*>(lpoverlapped);

			if (FALSE == isSuccess) 
			{
				if (0 == dwIoSize && IOOperation::Accpet != pOverlappedEx->Operation)
					closeSocket(pClientInfo);
					
				continue;
			}

			if (IOOperation::Accpet == pOverlappedEx->Operation)
			{
				pClientInfo = mClientInfos[pOverlappedEx->SessionIndex];

				if (pClientInfo->AcceptCompleted())
				{
					mClientCnt++;

					OnConnect(pClientInfo->GetIndex());
				}
				else
				{
					closeSocket(pClientInfo, true);
				}
			}
			else if (IOOperation::Recv == pOverlappedEx->Operation)
			{
				OnReceive(pClientInfo->GetIndex(), (UINT32)dwIoSize, pClientInfo->GetRecvBuffer());
				pClientInfo->BindRecv();
			}
			else if (IOOperation::Send == pOverlappedEx->Operation)
			{
				pClientInfo->SendCompleted(dwIoSize);
			}
			else
			{
				printf("[Error] Exception Socket : %d\n", pClientInfo->GetIndex());
			}

		}
	}
	void ServerNetwork::closeSocket(ClientInfo* pClientInfo, bool isForce)
	{
		auto clientIndex = pClientInfo->GetIndex();
		pClientInfo->Close(isForce);

		OnClose(clientIndex);
	}
}