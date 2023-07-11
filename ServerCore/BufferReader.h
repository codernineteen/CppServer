#pragma once

/*
	Buffer Reader
*/

class BufferReader
{
public:
	BufferReader();
	BufferReader(BYTE* buffer, uint32 size, uint32 pos=0);
	~BufferReader();

	BYTE* Buffer() { return _buffer; }
	uint32 Size() { return _size; } 
	uint32 ReadSize() { return _pos; } //현재까지 읽은 공간
	uint32 FreeSize() { return _size - _pos; } //남은 공간

	//해당 위치에서 데이터를 꺼내쓸 수 있는지 확인하고 싶을 때 사용
	template<typename T>
	bool Peek(T* dest) { return Peek(dest, sizeof(T)); }
	bool Peek(void* dest, uint32 len);

	template<typename T>
	bool Read(T* dest) { return Read(dest, sizeof(T)); }
	bool Read(void* dest, uint32 len);

	// >> operation을 오버로딩해서 데이터를 꺼내쓴다
	template<typename T>
	BufferReader& operator>>(OUT T& dest);

private:
	BYTE* _buffer = nullptr; // 버퍼 시작 주소
	uint32 _size = 0; // 버퍼 크기
	uint32 _pos = 0; // 버퍼 현재 위치
};

template<typename T>
inline BufferReader& BufferReader::operator>>(OUT T& dest)
{
	//버퍼에서 현재 커서가 가르키는 곳부터 데이터를 꺼내쓰고, T의 크기만큼 더해주어서 커서를 이동
	dest = *reinterpret_cast<T*>(&_buffer[_pos]);
	_pos += sizeof(T);
	return *this;
}