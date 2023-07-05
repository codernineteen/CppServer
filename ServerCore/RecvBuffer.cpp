#include "pch.h"
#include "RecvBuffer.h"

/**
	Recv Buffer
*/

RecvBuffer::RecvBuffer(int32 bufferSize) : _bufferSize(bufferSize)
{
	_capacity = bufferSize * BUFFER_COUNT;
	_buffer.resize(_capacity);
}

RecvBuffer::~RecvBuffer()
{
	
}

void RecvBuffer::Clean()
{
	int32 dataSize = DataSize();

	if (dataSize == 0)
	{
		//읽기, 쓰기 위치가 겹칠 때
		_writePos = _readPos = 0;
	}
	else
	{
		//여유 공간이 버퍼 1개 크기 미만이면, 데이터를 앞으로 당긴다.
		if (FreeSize() < _bufferSize)
		{
			::memcpy(&_buffer[0], &_buffer[_readPos], DataSize());
			_readPos = 0;
			_writePos = dataSize;
		}
	}
}

bool RecvBuffer::OnRead(int32 numOfBytes)
{
	if (numOfBytes > DataSize())
		return false;

	_readPos += numOfBytes;

	return true;
}

bool RecvBuffer::OnWrite(int32 numOfBytes)
{
	if (numOfBytes > FreeSize())
		return false;

	_writePos += numOfBytes;

	return true;
}
