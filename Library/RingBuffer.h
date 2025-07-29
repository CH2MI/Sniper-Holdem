#pragma once

#include "Packet.h"

namespace sniperholdem
{
	constexpr UINT32 MAX_BUFFER_SIZE = 8096;

	class RingBuffer
	{
	public:
		RingBuffer();
		~RingBuffer();

		bool Write(const UINT32 dataSize, char* pData);
		packet::PacketInfo* Read();

		void Clear();

	private:

		char* mBuffer;

		UINT32 mBufferWritePos;
		UINT32 mBufferReadPos;

	};
}