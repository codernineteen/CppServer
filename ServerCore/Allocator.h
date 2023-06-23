#pragma once

/**
	base allocator
*/

class BaseAllocator
{

public:
	static void* Alloc(int32 size);
	static void Release(void* ptr);
};

/**
	Stomp Allocator
*/

class StompAllocator
{
	enum {PAGE_SIZE = 0x10000};

public:
	static void* Alloc(int32 size);
	static void Release(void* ptr);
};

/**
	STL Allocator
*/
template<typename T>
class StlAllocator
{
public:
	using value_type = T;

	StlAllocator() {}

	template<typename Other>
	StlAllocator(const StlAllocator<Other>&) {}

	T* allocate(size_t count)
	{
		const int32 size = static_cast<int32>(count * sizeof(T));
		return static_cast<T*>(XALLOC(size));
	}

	void deallocate(T* ptr, size_t count)
	{
		XRELEASE(ptr);
	}
};

