#include "pch.h"
#include "ConcurrentStack.h"

//version 1
//void InitHead(SListHeaderSingle* header)
//{
//	header->next = nullptr;
//}
//
//
//void PushEntrySList(SListHeaderSingle* header, SListEntrySingle* entry)
//{
//	entry->next = header->next;
//	header->next = entry;
//}
//
//SListEntrySingle* PopEntrySList(SListHeaderSingle* header)
//{
//	SListEntrySingle* first = header->next; //header가 가르키는 대상
//	if (first != nullptr)
//	{
//		header가 가르키는 대상이 가르키는 대상으로 교체한다.
//		header->next = first->next;
//	}
//
//	return first;
//}

//void InitHead(SListHeader* header)
//{
//	header->next = nullptr;
//}
//
//
//void PushEntrySList(SListHeader* header, SListEntry* entry)
//{
//	entry->next = header->next;
//
//	//expected : entry->next
//	//desired : &header->next
//	//if same , &header->next = entry
//	while (::InterlockedCompareExchange64((int64*)&header->next, (int64)entry, (int64)entry->next) == 0)
//	{
//
//	}
//
//	header->next = entry;
//}
//
//SListEntry* PopEntrySList(SListEntry* header)
//{
//	SListEntry* expected = header->next; //header가 가르키는 대상
//
//	while (expected && ::InterlockedCompareExchange64((int64*)&header->next, (int64)expected->next, (int64)expected) == 0)
//	{
//		
//	}
//
//	return expected;
//}

/**
	Version 3
*/

void InitHead(SListHeader* header)
{
	header->alignment = 0;
	header->region = 0;
}


void PushEntrySList(SListHeader* header, SListEntry* entry)
{
	SListHeader expected = {};
	SListHeader desired = {};

	desired.HeaderX64.next = (((uint64)entry) >> 4); // 앞의 구조체에서 next는 60비트를 차지 하기 때문에, 진짜 next의 숫자를 알기 위해서 4비트를 오른쪽으로 shift시켜준다.
	//참고로 특정 운영체제에서도 64비트 주소체계를 사용할 때 충분히 48비트로 표현 가능하여
	//그만큼의 공간만 할당하고, shift연산을 사용하는 경우가 있다.

	while (true)
	{
		expected = *header;

		//4비트 왼쪽 이동 시켰던 것을 포인터로 캐스팅 하기 위하여, 다시 오른쪽으로 밀어넣어주고, 포인터로 캐스팅
		entry->next = (SListEntry*)((uint64)expected.HeaderX64.next << 4); 
		//카운터를 증가시켜서 해당 데이터에 대한 상태를 변경시켜준다.
		desired.HeaderX64.depth = expected.HeaderX64.depth + 1;
		desired.HeaderX64.sequence = expected.HeaderX64.sequence + 1;

		if (::InterlockedCompareExchange128((int64*)header, desired.region, desired.alignment, (int64*)&expected) == 1)
			break;
	}
}

SListEntry* PopEntrySList(SListHeader* header)
{
	SListHeader expected = {};
	SListHeader desired = {};
	SListEntry* entry = nullptr;

	while (true)
	{
		expected = *header;

		entry = (SListEntry*)(((uint64)expected.HeaderX64.next) << 4);
		if (entry == nullptr)
			break;

		desired.HeaderX64.next = ((uint64)entry->next >> 4);
		desired.HeaderX64.depth = expected.HeaderX64.depth - 1;
		desired.HeaderX64.sequence = expected.HeaderX64.depth + 1;

		if (::InterlockedCompareExchange128((int64*)header, desired.region, desired.alignment, (int64*)&expected) == 1)
			break;
	}

	return entry;
}

