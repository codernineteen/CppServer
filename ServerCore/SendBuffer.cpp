#include "pch.h"
#include "SendBuffer.h"

/**
	Send Buffer
*/

SendBuffer::SendBuffer(int32 bufferSize)
{
	_buffer.resize(bufferSize);
}

SendBuffer::~SendBuffer()
{
}

void SendBuffer::CopyData(void* data, int32 len)
{
	ASSERT_CRASH(Capacity() >= len); //허용량보다 크면 CRASH
	::memcpy(_buffer.data(), data, len); //버퍼에 데이터 복사
	_writeSize = len;
}
