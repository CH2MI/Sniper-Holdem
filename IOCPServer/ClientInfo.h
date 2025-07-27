#pragma once

#include <mutex>
#include <queue>

#include "Define.h"

namespace sniperholdem::iocpserver
{
	class ClientInfo
	{
	public:

		ClientInfo();
		~ClientInfo();

		void Init(const UINT32 index, HANDLE iocpHandle);
		bool Connect();
		void Close(bool isForce = false);

		bool ReserveAccept(SOCKET listenSock, const UINT64 curTimeSec);
		bool AcceptCompleted();
		bool BindIOCP();
		
		bool BindRecv();
		bool SendMsg(const UINT32 dataSize, char* pMsg);

		void SendCompleted(const UINT32 dataSize);

		UINT32 GetIndex() { return mIndex; }
		bool IsConnect() { return mIsConnect; }
		UINT64 GetLatestClosedTimeSec() { return mLatestClosedTimeSec; }
		SOCKET GetSocket() { return mSocket; }
		char* GetRecvBuffer() { return mRecvBuf; }

	private:

		bool sendIO();

	private:

		UINT32 mIndex;
		
		bool mIsConnect;
		
		UINT64 mLatestClosedTimeSec;
		
		SOCKET mSocket;

		HANDLE mIOCPHandle;
		
		OverlappedEx mAcceptOverlappedEx;
		char mAcceptBuf[64];

		OverlappedEx mRecvOverlappedEx;
		char mRecvBuf[MAX_SOCKBUF];

		std::mutex mSendLock;
		std::queue<OverlappedEx*> mSendDataQueue;


	};
}

