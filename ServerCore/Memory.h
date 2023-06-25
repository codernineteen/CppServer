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
		// 0 ~ 1024���� : 32bit������ ����
		// 1024 ~ 2048 : 128bit������ ����
		// 2048 ~ 4096 : 256bit������ ����
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
	new(memory) Type(std::forward<Args>(args)...); //we already have memory space, just call constructor on the space.
	//...Arg achieves Polymorphism by taking arguments dynamically
	return memory;
}

template<typename Type>
void xdelete(Type* obj)
{
	obj->~Type(); //call destructor
	PoolAllocator::Release(obj);
}

//Pooling�� ������� ���� ��, shared ptr ����
template<typename Type>
shared_ptr<Type> MakeShared()
{
	return shared_ptr<Type> { xnew<Type>(), xdelete<Type> }; 
}