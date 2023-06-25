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
//	SListEntrySingle* first = header->next; //header�� ����Ű�� ���
//	if (first != nullptr)
//	{
//		header�� ����Ű�� ����� ����Ű�� ������� ��ü�Ѵ�.
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
//	SListEntry* expected = header->next; //header�� ����Ű�� ���
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

	desired.HeaderX64.next = (((uint64)entry) >> 4); // ���� ����ü���� next�� 60��Ʈ�� ���� �ϱ� ������, ��¥ next�� ���ڸ� �˱� ���ؼ� 4��Ʈ�� ���������� shift�����ش�.
	//����� Ư�� �ü�������� 64��Ʈ �ּ�ü�踦 ����� �� ����� 48��Ʈ�� ǥ�� �����Ͽ�
	//�׸�ŭ�� ������ �Ҵ��ϰ�, shift������ ����ϴ� ��찡 �ִ�.

	while (true)
	{
		expected = *header;

		//4��Ʈ ���� �̵� ���״� ���� �����ͷ� ĳ���� �ϱ� ���Ͽ�, �ٽ� ���������� �о�־��ְ�, �����ͷ� ĳ����
		entry->next = (SListEntry*)((uint64)expected.HeaderX64.next << 4); 
		//ī���͸� �������Ѽ� �ش� �����Ϳ� ���� ���¸� ��������ش�.
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

