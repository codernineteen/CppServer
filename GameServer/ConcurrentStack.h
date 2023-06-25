#pragma once

#include <mutex>
#include <atomic>

/**
	Version 1
*/

//노드 안에 데이터와 다음 노드에 대한 포인터가 중첩된 형태의 구조를 사용하는 대신
// 그 둘을 분리해서 개별 구조로 관리하는 방법
//struct SListEntrySingle
//{
//	SListEntrySingle* next;
//};


// 헤더가 첫번째 데이터를 가르키고 있고, 첫번째 데이터는 다음 데이터를 가르킨다.
// [data ][  ][  ]
// Header[ next ] : next는 어떤 데이터를 가르키고 있다.
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
