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
	ASSERT_CRASH(Capacity() >= len); //��뷮���� ũ�� CRASH
	::memcpy(_buffer.data(), data, len); //���ۿ� ������ ����
	_writeSize = len;
}
