#pragma once

/*
	Buffer Writer
*/

class BufferWriter
{
public:
	BufferWriter();
	BufferWriter(BYTE* buffer, uint32 size, uint32 pos=0);
	~BufferWriter();

	BYTE* Buffer() { return _buffer; }
	uint32 Size() { return _size; }
	uint32 WriteSize() { return _pos; } //현재까지 읽은 공간
	uint32 FreeSize() { return _size - _pos; } //남은 공간


	template<typename T>
	bool Write(T* src) { return Read(src, sizeof(T)); }
	bool Write(void* src, uint32 len);

	// << operation을 오버로딩해서 데이터를 밀어넣는다.
	template<typename T>
	BufferWriter& operator<<(const T& src); // lvalue reference
	
	template<typename T>
	BufferWriter& operator<<(T&& src); // rvalue reference

	template<typename T>
	T* Reserve();

private:
	BYTE* _buffer = nullptr; // 버퍼 시작 주소
	uint32 _size = 0; // 버퍼 크기
	uint32 _pos = 0; // 버퍼 현재 위치
};

template<typename T>
T* BufferWriter::Reserve()
{
	if (FreeSize() < sizeof(T))
		return nullptr;

	T* ret = reinterpret_cast<T*>(&_buffer[_pos]);
	_pos += sizeof(T);

	return ret;
}

//reference를 반환하는 이유는 operator를 연속적으로 사용하기 위함이다.
template<typename T>
BufferWriter& BufferWriter::operator<<(const T& src)
{
	*reinterpret_cast<T*>(&_buffer[_pos]) = src;
	_pos += sizeof(T);
	return *this;
}

template<typename T>
BufferWriter& BufferWriter::operator<<(T&& src)
{
	*reinterpret_cast<T*>(&_buffer[_pos]) = std::move(src);
	_pos += sizeof(T);
	return *this;
}