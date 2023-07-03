#pragma once
#include "Allocator.h"

class MemoryPool; //forward declaration

/**
	Memory Manager
*/
class Memory
{
	enum
	{
		// 0 ~ 1024까지 : 32bit단위로 생성
		// 1024 ~ 2048 : 128bit단위로 생성
		// 2048 ~ 4096 : 256bit단위로 생성
		POOL_COUNT = (1024 / 32) + (1024 / 128) + (2048 / 256),
		MAX_ALLOC_SIZE = 4096
	};

public:
	Memory();
	~Memory();

	void* Allocate(int32 size);
	void Release(void* ptr);

private:
	vector<MemoryPool*> _pools;

	MemoryPool* _poolTable[MAX_ALLOC_SIZE + 1];
};

template<typename Type, typename ...Args>
Type* xnew(Args&&... args)
{
	Type* memory = static_cast<Type*>(PoolAllocator::Alloc(sizeof(Type)));

	//placement new
	new(memory)Type(forward<Args>(args)...); //we already have memory space, just call constructor on the space.
	//...Arg achieves Polymorphism by taking arguments dynamically
	return memory;
}

template<typename Type>
void xdelete(Type* obj)
{
	obj->~Type(); //call destructor
	PoolAllocator::Release(obj);
}


//인자를 받는 shared_ptr 생성자
template<typename Type, typename... Args>
shared_ptr<Type> MakeShared(Args&&... args)
{
	return shared_ptr<Type> { xnew<Type>(forward<Args>(args)...), xdelete<Type> };
}
