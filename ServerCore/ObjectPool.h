#pragma once
#include "Types.h"
#include "MemoryPool.h"

//template���� ����� �Ǹ�, static �Ӽ� ���� �� �ϳ� �����ϴ� ���� ������, �������� ���ؼ� ������ Ÿ�Կ� ���ؼ� �����ϰ� �����Ѵ�.
template<typename Type>
class ObjectPool
{
	//Ŭ���� ���� Ǯ�� �����ϴ� ��å
	//� Ÿ�� T�� ���� ���� pool�� ���� ����
public:
	template<typename ...Args>
	static Type* Pop(Args&&... args)
	{ 
#ifdef _STOMP
		MemoryHeader* ptr = reinterpret_cast<MemoryHeader*>(StompAllocator::Alloc(s_allocSize));
		Type* memory = static_cast<Type*>(MemoryHeader::AttachHeader(ptr, s_allocSize));
#else
		//pool���� �޸𸮸� �ϳ� ������ , �װ����ٰ� ����� ���δ�.
		Type* memory = static_cast<Type*>(MemoryHeader::AttachHeader(s_pool.Pop(), s_allocSize));
#endif
		//�����ڸ� ȣ���Ͽ� memory��ȯ
		new(memory) Type(std::forward<Args>(args)...);
		return memory;
	}

	static void Push(Type* obj)
	{
		obj->~Type(); //��ü�� �Ҹ��� ȣ��
#ifdef _STOMP
		StompAllocator::Release(MemoryHeader::DetachHeader(obj));
#else
		s_pool.Push(MemoryHeader::DetachHeader(obj)); //����� ������ �ٽ� pool�� �־��ش�.
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

