#include "RingBuffer.h"

namespace sniperholdem
{
	RingBuffer::RingBuffer() :
		mBufferWritePos(0),
		mBufferReadPos(0)
	{
		mBuffer = new char[MAX_BUFFER_SIZE];
	}

	RingBuffer::~RingBuffer()
	{
		delete[] mBuffer;
	}

	bool RingBuffer::Write(const UINT32 dataSize, char* pData)
	{
		// 버퍼 데이터를 맨 앞으로 댕긴다.
		if ((mBufferWritePos + dataSize) >= MAX_BUFFER_SIZE)
		{
			UINT32 remainDataSize = mBufferWritePos - mBufferReadPos;

			if (remainDataSize > 0)
			{
				CopyMemory(&mBuffer[0], &mBuffer[mBufferReadPos], remainDataSize);
				mBufferWritePos = remainDataSize;
			}
			else
				mBufferWritePos = 0;
			
			mBufferReadPos = 0;
		}

		// 버퍼에 더이상 담을 수 없을 때
		if ((mBufferWritePos + dataSize) >= MAX_BUFFER_SIZE)
			return false;

		CopyMemory(&mBuffer[mBufferWritePos], pData, dataSize);
		mBufferWritePos += dataSize;

		return true;
	}

	packet::PacketInfo* RingBuffer::Read()
	{
		short packetSize = 0;

		UINT32 remainByte = mBufferWritePos - mBufferReadPos;

		// 헤더가 완성되지 않을 때
		if (remainByte < packet::PACKET_HEADER_LENGTH)
			return nullptr;

		auto pHeader = reinterpret_cast<packet::PacketHeader*>(&mBuffer[mBufferReadPos]);

		// 패킷이 완성되지 않을 때
		if (pHeader->PacketLength > remainByte)
			return nullptr;

		auto packet = new packet::PacketInfo;
		packet->PacketId = pHeader->PacketID;
		packet->DataSize = pHeader->PacketLength;
		packet->PacketData = new char[packet->DataSize];
		CopyMemory(packet->PacketData, &mBuffer[mBufferReadPos], packet->DataSize);

		mBufferReadPos += packet->DataSize;

		return packet;
	}

	void RingBuffer::Clear()
	{
		mBufferReadPos = mBufferWritePos = 0;
	}
}