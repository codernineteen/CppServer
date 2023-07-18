#include "pch.h"
#include "ClientPacketHandler.h"
#include "BufferReader.h"

void ClientPacketHandler::HandlePacket(BYTE* buffer, int32 len)
{
	BufferReader br(buffer, len);
	PacketHeader header;
	br >> header;

	//packet id에 따라서 관리
	switch (header.id)
	{
	case S_TEST:
		Handle_S_TEST(buffer, len);
		break;

	default:
		break;
	}

}

#pragma pack(1)
//packet 설계
struct PKT_S_TEST
{
	struct BuffsListItem
	{
		uint64 buffid;
		float remainTime;

		uint16 victimsOffset;
		uint16 victimsCount;

		bool Validate(BYTE* packetStart, uint16 packetSize, OUT uint32& size)
		{
			if (victimsOffset + victimsCount * sizeof(uint64) > packetSize)
				return false;

			size += victimsCount * sizeof(uint64);
			return true;
		}
	};

	uint16 packetSize; //공용 헤더
	uint16 packetId; //공용 헤더
	uint64 id;
	uint32 hp;
	uint16 attack;
	uint16 buffOffset; //가변 데이터 시작위치
	uint16 buffsCount; //가변 데이터 원소 개수

	bool Validate()
	{
		uint32 size = 0;
		size += sizeof(PKT_S_TEST);
		//헤더 정보에 관해서는 안전하지만 그외 정보들에 대해서 보장할 수 없기에 체크를 해준다.
		if (packetSize < size)
			return false;
		
		//패킷에 명시한 가변 데이터 길이를 벗어나는 경우
		if (buffOffset + buffsCount * sizeof(BuffsListItem) > packetSize)
			return false;

		//가변 데이터 크기 유효성 검사
		size += buffsCount * sizeof(BuffsListItem);
		BuffsList buffList = GetBuffList();
		for (int32 i = 0; i < buffList.Count(); i++)
		{
			if (buffList[i].Validate((BYTE*)this, packetSize, OUT size) == false)
				return false;
		}
		if (size != packetSize)
			return false;

		return true;
	}

	using BuffsList = PacketList<PKT_S_TEST::BuffsListItem>;
	using VictimsList = PacketList<uint64>;

	BuffsList GetBuffList()
	{
		BYTE* data = reinterpret_cast<BYTE*>(this); //바이트 포인터로 변환
		data += buffOffset;
		return BuffsList(reinterpret_cast<PKT_S_TEST::BuffsListItem*>(data), buffsCount);
	}

	VictimsList GetVictimsList(BuffsListItem* buffsItem)
	{
		BYTE* data = reinterpret_cast<BYTE*>(this); //바이트 포인터로 변환
		data += buffsItem->victimsOffset;
		return VictimsList(reinterpret_cast<uint64*>(data), buffsItem->victimsCount);
	}
};
#pragma pack()

void ClientPacketHandler::Handle_S_TEST(BYTE* buffer, int32 len)
{
	BufferReader br(buffer, len);

	//어차피 버퍼를 PKT_S_TEST로 캐스팅하면 버퍼 시작주소부터 패킷이 필요한 공간만큼 캐스팅이 된다.
	PKT_S_TEST* pkt = reinterpret_cast<PKT_S_TEST*>(buffer);

	if (pkt->Validate() == false)
		return;

	//buffer에서 가변 데이터 부분을 vector로 복사하는 대신에
	//패킷에서 정의한 BuffsList 구조체에 있는 포인터로 캐스팅 해서 참조만 하도록함
	PKT_S_TEST::BuffsList buffs = pkt->GetBuffList();

	cout << "BuffCount : " << buffs.Count() << endl;
	for (int32 i = 0; i < buffs.Count(); i++)
	{
		cout << "BuffInfo : [" << buffs[i].buffid << ", " << buffs[i].remainTime << "]" << endl;

		PKT_S_TEST::VictimsList victims = pkt->GetVictimsList(&buffs[0]);
		for (auto& vic : victims)
		{
			cout << "victim : " << vic << endl;
		}
	}
}
