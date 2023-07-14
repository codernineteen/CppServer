#include "pch.h"
#include "ClientPacketHandler.h"
#include "BufferReader.h"

void ClientPacketHandler::HandlePacket(BYTE* buffer, int32 len)
{
	BufferReader br(buffer, len);
	PacketHeader header;
	br >> header;

	//packet id�� ���� ����
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
//packet ����
struct PKT_S_TEST
{
	struct BuffsListItem
	{
		uint64 buffid;
		float remainTime;
	};

	uint16 packetSize; //���� ���
	uint16 packetId; //���� ���
	uint64 id;
	uint32 hp;
	uint16 attack;
	uint16 buffOffset; //���� ������ ������ġ
	uint16 buffsCount; //���� ������ ���� ����

	bool Validate()
	{
		uint32 size = 0;

		size += sizeof(PKT_S_TEST);
		size += buffsCount * sizeof(BuffsListItem);
		if (size != packetSize)
			return false;

		//��Ŷ�� ����� ���� ������ ���̸� ����� ���
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
