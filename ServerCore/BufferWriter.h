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
	uint32 WriteSize() { return _pos; } //������� ���� ����
	uint32 FreeSize() { return _size - _pos; } //���� ����


	template<typename T>
	bool Write(T* src) { return Read(src, sizeof(T)); }
	bool Write(void* src, uint32 len);

	// << operation�� �����ε��ؼ� �����͸� �о�ִ´�.
	template<typename T>
	BufferWriter& operator<<(const T& src); // lvalue reference
	
	template<typename T>
	BufferWriter& operator<<(T&& src); // rvalue reference

	template<typename T>
	T* Reserve();

private:
	BYTE* _buffer = nullptr; // ���� ���� �ּ�
	uint32 _size = 0; // ���� ũ��
	uint32 _pos = 0; // ���� ���� ��ġ
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

//reference�� ��ȯ�ϴ� ������ operator�� ���������� ����ϱ� �����̴�.
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