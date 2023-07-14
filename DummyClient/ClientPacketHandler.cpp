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
		size += buffsCount * sizeof(BuffsListItem);
		if (size != packetSize)
			return false;

		//패킷에 명시한 가변 데이터 길이를 벗어나는 경우
		if (buffOffset + buffsCount * sizeof(BuffsListItem) > packetSize)
			return false;

		return true;
	}
};
#pragma pack()

void ClientPacketHandler::Handle_S_TEST(BYTE* buffer, int32 len)
{
	BufferReader br(buffer, len);

	if (len < sizeof(PKT_S_TEST))
		return;

	PKT_S_TEST pkt;
	br >> pkt;

	if (pkt.Validate() == false)
		return;

	vector<PKT_S_TEST::BuffsListItem> buffs;
	buffs.resize(pkt.buffsCount);

	for (int32 i = 0; i < pkt.buffsCount; i++)
		br >> buffs[i];


	cout << "BuffCount : " << pkt.buffsCount << endl;
	for (int32 i = 0; i < pkt.buffsCount; i++)
	{
		cout << "BuffInfo : [" << buffs[i].buffid << ", " << buffs[i].remainTime << "]" << endl;
	}
}
