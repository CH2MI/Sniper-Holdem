#include "ClientNetwork.h"

namespace sniperholdem::client::network
{
	std::function<void()> ClientNetwork::OnConnect{};
	std::function<void()> ClientNetwork::OnDisconnect{};
	std::function<void(const UINT32, char* pData)> ClientNetwork::OnReceive{};

	SOCKET ClientNetwork::ConnectSocket{};
	char ClientNetwork::RecvBuffer[MAX_SOCKBUF] = {};
	
	std::thread ClientNetwork::RecvThread{};
	bool ClientNetwork::IsRecvThreadRun = false;

	std::thread ClientNetwork::SendThread{};
	bool ClientNetwork::IsSendThreadRun = false;

	std::deque<packet::PacketHeader*> ClientNetwork::SendQueue{};
	std::mutex ClientNetwork::SendLock{};

	bool ClientNetwork::Initialize()
	{
		WSADATA wsaData;

		int ret = WSAStartup(MAKEWORD(2, 2), &wsaData);
		if (0 != ret)
		{
			printf("[Error] WSAStartup Error : %d\n", WSAGetLastError());
			WSACleanup();
			return false;
		}

		ConnectSocket = WSASocket
		(
			AF_INET,
			SOCK_STREAM,
			0,
			nullptr,
			0,
			WSA_FLAG_OVERLAPPED
		);
		if (INVALID_SOCKET == ConnectSocket)
		{
			printf("[Error] WSASocket Error : %d\n", WSAGetLastError());
			WSACleanup();
			return false;
		}

		printf("Socket Init Complete\n");

		return true;
	}

	bool ClientNetwork::Connect(std::string serverIP, int serverPort)
	{
		SOCKADDR_IN serverAddr;
		serverAddr.sin_family = AF_INET;
		inet_pton(AF_INET, serverIP.c_str(), &serverAddr.sin_addr);
		serverAddr.sin_port = htons(serverPort);

		int ret = connect(ConnectSocket, reinterpret_cast<SOCKADDR*>(&serverAddr), sizeof(SOCKADDR_IN));
		if (SOCKET_ERROR == ret)
		{
			printf("[Error] connect Error %d\n", WSAGetLastError());
			closesocket(ConnectSocket);
			WSACleanup();
			return false;
		}

		printf("Server Connect Complete\n");
		
		// OnConnect
		if (OnConnect)
			OnConnect();

		return true;
	}

	void ClientNetwork::Start()
	{
		createSendThread();
		createRecvThread();
	}

	void ClientNetwork::Stop()
	{
		IsRecvThreadRun = false;
		IsSendThreadRun = false;

		shutdown(ConnectSocket, SD_BOTH);

		if (RecvThread.joinable())
			RecvThread.join();

		if (SendThread.joinable())
			SendThread.join();

		closesocket(ConnectSocket);
		WSACleanup();
	}

	void ClientNetwork::createRecvThread()
	{
		IsRecvThreadRun = true;
		RecvThread = std::thread([]() { recvPacket(); });
	}

	void ClientNetwork::recvPacket()
	{
		while (IsRecvThreadRun)
		{
			int ret = recv(ConnectSocket, RecvBuffer, sizeof(RecvBuffer), 0);
			if (ret > 0)
			{
				if (OnReceive)
					OnReceive(ret, RecvBuffer);
			}
			else if (ret == 0)
			{
				// OnClose
				if (OnDisconnect)
					OnDisconnect();
				Stop();
			}
			else
			{
				// Except
				printf("[Error] Recv Expection! %d\n", WSAGetLastError());
				Stop();
			}
		}
	}

	void ClientNetwork::createSendThread()
	{
		IsSendThreadRun = true;
		SendThread = std::thread([]() { sendPacket(); });
	}

	void ClientNetwork::sendPacket()
	{
		while (IsSendThreadRun)
		{
			bool isIdle = true;
			packet::PacketHeader* packet = nullptr;
			{
				std::lock_guard<std::mutex> lock(SendLock);
				if (!SendQueue.empty())
				{
					isIdle = false;
					packet = SendQueue.front();
					SendQueue.pop_front();
				}
			}

			if (isIdle)
			{ 
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
			}
			else
			{
				int ret = send(ConnectSocket, reinterpret_cast<char*>(packet), packet->PacketLength, 0);
				if (SOCKET_ERROR == ret)
				{
					std::lock_guard<std::mutex> lock(SendLock);
					SendQueue.push_front(packet);
				}
				else
				{
					delete packet;
				}
			}
		}
	}


}