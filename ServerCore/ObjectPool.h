#pragma once
#include "Types.h"
#include "MemoryPool.h"

//template으로 만들게 되면, static 속성 들이 단 하나 존재하는 것은 맞지만, 다형성에 의해서 각각의 타입에 대해서 유일하게 존재한다.
template<typename Type>
class ObjectPool
{
	//클래스 별로 풀을 생성하는 정책
	//어떤 타입 T에 대한 전용 pool을 만들어서 관리
public:
	template<typename ...Args>
	static Type* Pop(Args&&... args)
	{ 
#ifdef _STOMP
		MemoryHeader* ptr = reinterpret_cast<MemoryHeader*>(StompAllocator::Alloc(s_allocSize));
		Type* memory = static_cast<Type*>(MemoryHeader::AttachHeader(ptr, s_allocSize));
#else
		//pool에서 메모리를 하나 꺼내고 , 그곳에다가 헤더를 붙인다.
		Type* memory = static_cast<Type*>(MemoryHeader::AttachHeader(s_pool.Pop(), s_allocSize));
#endif
		//생성자를 호출하여 memory반환
		new(memory) Type(std::forward<Args>(args)...);
		return memory;
	}

	static void Push(Type* obj)
	{
		obj->~Type(); //객체의 소멸자 호출
#ifdef _STOMP
		StompAllocator::Release(MemoryHeader::DetachHeader(obj));
#else
		s_pool.Push(MemoryHeader::DetachHeader(obj)); //헤더를 꺼내서 다시 pool에 넣어준다.
#endif
	}

	template<typename... Args>
	static shared_ptr<Type> MakeShared(Args&& ...args)
	{
		shared_ptr<Type> sptr = { Pop(forward<Args>(args)...) , Push };
		return sptr;
	}

private:
	static int32 s_allocSize;
	static MemoryPool s_pool;
	
};

template<typename Type>
int32 ObjectPool<Type>::s_allocSize = sizeof(Type) + sizeof(MemoryHeader);

template<typename Type>
MemoryPool ObjectPool<Type>::s_pool{ s_allocSize };

