#pragma once

#include <string>

#include "../Library/RingBuffer.h"

namespace sniperholdem::server
{
	class User
	{
	public:

		User();
		~User();

		void Clear();

		bool Write(const UINT32 dataSize, char* pData) { return mBuffer.Write(dataSize, pData); }
		packet::PacketInfo* Read()
		{
			auto packet = mBuffer.Read();
			if (packet != nullptr)
			{
				packet->ClientIndex = mIndex;
			}
			return packet;
		}

		void SetUserID(char* userID) { mUserID = userID; }
		std::string GetUserID() { return mUserID; }
		void SetIndex(UINT32 index) { mIndex = index; }
		UINT32 GetIndex() { return mIndex; }


	private:
		
		UINT32 mIndex;
		std::string mUserID;

		RingBuffer mBuffer;

	};
}