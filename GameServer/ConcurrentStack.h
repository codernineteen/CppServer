#pragma once

#include <mutex>
#include <atomic>

/**
	Version 1
*/

//��� �ȿ� �����Ϳ� ���� ��忡 ���� �����Ͱ� ��ø�� ������ ������ ����ϴ� ���
// �� ���� �и��ؼ� ���� ������ �����ϴ� ���
//struct SListEntrySingle
//{
//	SListEntrySingle* next;
//};


// ����� ù��° �����͸� ����Ű�� �ְ�, ù��° �����ʹ� ���� �����͸� ����Ų��.
// [data ][  ][  ]
// Header[ next ] : next�� � �����͸� ����Ű�� �ִ�.
//struct SListHeaderSingle
//{
//	SListEntrySingle* next = nullptr;
//};
//
//void InitHead(SListHeaderSingle* header);
//
//void PushEntrySList(SListHeaderSingle* header, SListEntrySingle* entry);
//
//SListEntrySingle* PopEntrySList(SListHeaderSingle* header);



/**
	Version 2 - multi thread env
*/

DECLSPEC_ALIGN(16) // 16 byte alignment
struct SListEntry
{
	SListEntry* next;
};


DECLSPEC_ALIGN(16)
class Data
{
public:
	SListEntry _entry;
	int64 _rand = rand() % 1000;
};

DECLSPEC_ALIGN(16)
struct SListHeader
{
	SListHeader()
	{
		alignment = 0;
		region = 0;
	}

	union
	{
		struct
		{
			uint64 alignment;
			uint64 region;
		} DUMMYSTRUCTNAME;

		struct
		{
			uint64 depth : 16;
			uint64 sequence : 48;
			uint64 reserved : 4;
			uint64 next : 60;
		} HeaderX64;
	};
};

void InitHead(SListHeader* header);

void PushEntrySList(SListHeader* header, SListEntry* entry);

SListEntry* PopEntrySList(SListHeader* header);
