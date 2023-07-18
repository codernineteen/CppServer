#pragma once
#include "BufferReader.h"
#include "BufferWriter.h"

enum
{
	S_TEST = 1
};

template<typename T, typename C>
class PacketIterator
{
public:
	PacketIterator(C& container, uint16 index) : _container(container), _index(index) {}

	bool operator!=(const PacketIterator& other) const { return _index != other._index; }
	// index access
	const T& operator*() const { return _container[_index]; }
	const T& operator*() { return _container[_index]; }
	T* operator->() { return &_container[_index]; }
	PacketIterator& operator++() { _index++; return *this; }
	PacketIterator operator++(int32) { PacketIterator ret = *this; ++_index; return ret; }

private:
	C& _container;
	uint16 _index;
};

template<typename T>
class PacketList
{
public:
	PacketList() : _data(nullptr), _count(0) {};
	PacketList(T* data, uint16 count) : _data(data), _count(count) {};

	T& operator[](uint16 index)
	{
		ASSERT_CRASH(index < _count);
		return _data[index];
	}

	uint16 Count() { return _count; }

	// range-based for support
	PacketIterator<T, PacketList<T>> begin() { return PacketIterator<T, PacketList<T>>(*this, 0); }
	PacketIterator<T, PacketList<T>> end() { return PacketIterator<T, PacketList<T>>(*this, _count); }

private:
	T* _data;
	uint16 _count;
};


#pragma pack(1)
//packet 설계
struct PKT_S_TEST
{
	struct BuffsListItem
	{
		uint64 buffid;
		float remainTime;

		// 중첩 리스트 정보
		uint16 victimsOffset;
		uint16 victimsCount;
	};

	uint16 packetSize; //공용 헤더
	uint16 packetId; //공용 헤더
	uint64 id;
	uint32 hp;
	uint16 attack;
	uint16 buffOffset; //가변 데이터 시작위치
	uint16 buffsCount; //가변 데이터 원소 개수

	//보내는 쪽에서 validate는 필요하지 않다.
};

// 패킷 구조 : [PKT_S_TEST(정적)][BuffsListItem(동적)][BuffsListItem][victim0][victim1]
class PKT_S_TEST_WRITE
{
public:
	using BuffsListItem = PKT_S_TEST::BuffsListItem;
	using BuffsList = PacketList<PKT_S_TEST::BuffsListItem>;
	using BuffsVitimsList = PacketList<uint64>;

	PKT_S_TEST_WRITE(uint64 id, uint32 hp, uint16 attack)
	{
		_sendBuffer = GSendBufferManager->Open(4096);
		_bw = BufferWriter(_sendBuffer->Buffer(), _sendBuffer->AllocSize());

		//고정 길이에 대한 메모리 예약 및 할당
		_pkt = _bw.Reserve<PKT_S_TEST>();
		_pkt->packetSize = 0; //채워줘야 할 부분
		_pkt->packetId = S_TEST;
		_pkt->id = id;
		_pkt->hp = hp;
		_pkt->buffOffset = 0;//채워줘야 할 부분
		_pkt->buffsCount = 0;//채워줘야 할 부분
	}

	//가변 데이터가 항상 하나의 타입만 존재하는 것이 아니기 때문에
	//어떤 데이터 타입에 대한 필요 공간을 미리 예약하고 사용하는 것이 일반적이다.
	BuffsList ReserveBuffsList(uint16 buffCount)
	{

		BuffsListItem* firstBuffsListItem = _bw.Reserve<BuffsListItem>(buffCount);
		_pkt->buffOffset = (uint64)firstBuffsListItem - (uint64)_pkt;
		_pkt->buffsCount = buffCount;
		return BuffsList(firstBuffsListItem, buffCount);
	}

	BuffsVitimsList ReserveBuffsVictimsList(BuffsListItem* buffsItem, uint16 victimsCount)
	{
		uint64* firstVictimsListItem = _bw.Reserve<uint64>(victimsCount);
		buffsItem->victimsOffset = (uint64)firstVictimsListItem - (uint64)_pkt;
		buffsItem->victimsCount = victimsCount;
		return BuffsVitimsList(firstVictimsListItem, victimsCount);
	}

	SendBufferRef CloseAndReturn()
	{
		//패킷 사이즈 계산
		_pkt->packetSize = _bw.WriteSize();
		cout << _pkt->packetSize << endl;
		_sendBuffer->Close(_bw.WriteSize());
		return _sendBuffer;
	}

private:
	PKT_S_TEST* _pkt = nullptr;
	SendBufferRef _sendBuffer;
	BufferWriter _bw;
};
#pragma pack()

class ServerPacketHandler
{
public:
	static void HandlePacket(BYTE* buffer, int32 len);

	
};

