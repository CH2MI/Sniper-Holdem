#include "ClientInfo.h"

namespace sniperholdem::iocpserver
{
	ClientInfo::ClientInfo() :
		mIndex(0),
		mIsConnect(false),
		mLatestClosedTimeSec(0),
		mSocket(INVALID_SOCKET),
		mIOCPHandle(INVALID_HANDLE_VALUE),
		mAcceptOverlappedEx{},
		mAcceptBuf{},
		mRecvOverlappedEx{},
		mRecvBuf{},
		mSendLock{},
		mSendDataQueue{}
	{
	}

	ClientInfo::~ClientInfo()
	{
		while (!mSendDataQueue.empty())
		{
			delete[] mSendDataQueue.front()->WsaBuf.buf;
			delete mSendDataQueue.front();

			mSendDataQueue.pop();
		}

		if (mSocket != INVALID_SOCKET)
		{
			closesocket(mSocket);
		}
	}

	void ClientInfo::Init(const UINT32 index, HANDLE iocpHandle)
	{
		mIndex = index;
		mIOCPHandle = iocpHandle;
	}

	bool ClientInfo::Connect()
	{
		mIsConnect = true;

		if (!BindIOCP())
			return false;

		return BindRecv();
	}

	void ClientInfo::Close(bool isForce)
	{
		struct linger stLinger = { 0, 0 };

		if (isForce)
			stLinger.l_onoff = 1;

		shutdown(mSocket, SD_BOTH);

		setsockopt(mSocket, SOL_SOCKET, SO_LINGER,
			reinterpret_cast<char*>(&stLinger), sizeof(stLinger));

		mIsConnect = false;
		mLatestClosedTimeSec =
			std::chrono::duration_cast<std::chrono::seconds>
			(
				std::chrono::steady_clock::now().time_since_epoch()
			).count();

		closesocket(mSocket);
		mSocket = INVALID_SOCKET;
	}

	bool ClientInfo::ReserveAccept(SOCKET listenSock, const UINT64 curTimeSec)
	{
		mLatestClosedTimeSec = UINT64_MAX;

		mSocket = WSASocket(AF_INET, SOCK_STREAM, 0, nullptr, 0, WSA_FLAG_OVERLAPPED);
		if (INVALID_SOCKET == mSocket)
		{
			printf("[Error] %d Client WSASocket Error : %d\n", mIndex, WSAGetLastError());
			return false;
		}

		ZeroMemory(&mAcceptOverlappedEx, sizeof(mAcceptOverlappedEx));

		DWORD bytes = 0;
		DWORD flags = 0;
		
		mAcceptOverlappedEx.WsaBuf.len = 0;
		mAcceptOverlappedEx.WsaBuf.buf = nullptr;
		mAcceptOverlappedEx.Operation = IOOperation::Accpet;
		mAcceptOverlappedEx.SessionIndex = mIndex;

		BOOL bRet = AcceptEx
		(
			listenSock, 
			mSocket, 
			mAcceptBuf,
			0, 
			sizeof(SOCKADDR_IN) + 16, 
			sizeof(SOCKADDR_IN) + 16,
			&bytes, 
			reinterpret_cast<LPWSAOVERLAPPED>(&mAcceptOverlappedEx)
		);

		if (!bRet)
		{
			if (WSA_IO_PENDING != WSAGetLastError())
			{
				printf("[Error] %d Client AcceptEx Error : %d\n", mIndex, WSAGetLastError());
				return false;
			}
		}

		return true;
	}

	bool ClientInfo::AcceptCompleted()
	{
		return Connect();
	}

	bool ClientInfo::BindIOCP()
	{
		HANDLE hIOCP = CreateIoCompletionPort
		(
			reinterpret_cast<HANDLE>(mSocket),
			mIOCPHandle,
			reinterpret_cast<ULONG_PTR>(this),
			0
		);

		if (INVALID_HANDLE_VALUE == hIOCP)
		{
			printf("[Error] %d Client CreateIoCompletionPort Error : %d\n", mIndex, WSAGetLastError());
			return false;
		}

		return true;
	}
	bool ClientInfo::BindRecv()
	{
		DWORD dwFlag = 0;
		DWORD dwRecvNumBytes = 0;

		mRecvOverlappedEx.WsaBuf.len = MAX_SOCKBUF;
		mRecvOverlappedEx.WsaBuf.buf = mRecvBuf;
		mRecvOverlappedEx.Operation = IOOperation::Recv;

		int ret = WSARecv
		(
			mSocket,
			&(mRecvOverlappedEx.WsaBuf),
			1,
			&dwRecvNumBytes,
			&dwFlag,
			reinterpret_cast<LPWSAOVERLAPPED>(&mRecvOverlappedEx),
			nullptr
		);

		if (SOCKET_ERROR == ret && ERROR_IO_PENDING != WSAGetLastError())
		{
			printf("[Error] %d Client WSARecv Error : %d\n", mIndex, WSAGetLastError());
			return false;
		}

		return true;
	}

	bool ClientInfo::SendMsg(const UINT32 dataSize, char* pMsg)
	{
		auto sendOverlappedEx = new OverlappedEx;
		ZeroMemory(sendOverlappedEx, sizeof(OverlappedEx));

		sendOverlappedEx->WsaBuf.len = dataSize;
		sendOverlappedEx->WsaBuf.buf = new char[dataSize];
		CopyMemory(sendOverlappedEx->WsaBuf.buf, pMsg, dataSize);
		sendOverlappedEx->Operation = IOOperation::Send;
		sendOverlappedEx->SessionIndex = mIndex;

		std::lock_guard<std::mutex> lock(mSendLock);

		mSendDataQueue.push(sendOverlappedEx);

		if (1 == mSendDataQueue.size())
			sendIO();

		return true;
	}

	void ClientInfo::SendCompleted(const UINT32 dataSize)
	{
		std::lock_guard<std::mutex> lock(mSendLock);
		
		//printf("SendCompleted %d\n", mSendDataQueue.front()->SessionIndex);

		delete[] mSendDataQueue.front()->WsaBuf.buf;
		delete mSendDataQueue.front();

		mSendDataQueue.pop();

		if (!mSendDataQueue.empty())
			sendIO();
	}


	bool ClientInfo::sendIO()
	{
		auto sendOverlappedEx = mSendDataQueue.front();

		DWORD dwNumBytesSent = 0;

		int ret = WSASend
		(
			mSocket,
			&(sendOverlappedEx->WsaBuf),
			1,
			&dwNumBytesSent,
			0,
			reinterpret_cast<LPWSAOVERLAPPED>(sendOverlappedEx),
			nullptr
		);

		if (SOCKET_ERROR == ret && ERROR_IO_PENDING != WSAGetLastError())
		{
			printf("[Error] %d Client WSASend Error : %d\n", mIndex, WSAGetLastError());
			return false;
		}

		return true;
	}
}