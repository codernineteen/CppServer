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
		//��� ������ ���ؼ��� ���������� �׿� �����鿡 ���ؼ� ������ �� ���⿡ üũ�� ���ش�.
		if (packetSize < size)
			return false;
		
		//��Ŷ�� ����� ���� ������ ���̸� ����� ���
		if (buffOffset + buffsCount * sizeof(BuffsListItem) > packetSize)
			return false;

		//���� ������ ũ�� ��ȿ�� �˻�
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
		BYTE* data = reinterpret_cast<BYTE*>(this); //����Ʈ �����ͷ� ��ȯ
		data += buffOffset;
		return BuffsList(reinterpret_cast<PKT_S_TEST::BuffsListItem*>(data), buffsCount);
	}

	VictimsList GetVictimsList(BuffsListItem* buffsItem)
	{
		BYTE* data = reinterpret_cast<BYTE*>(this); //����Ʈ �����ͷ� ��ȯ
		data += buffsItem->victimsOffset;
		return VictimsList(reinterpret_cast<uint64*>(data), buffsItem->victimsCount);
	}
};
#pragma pack()

void ClientPacketHandler::Handle_S_TEST(BYTE* buffer, int32 len)
{
	BufferReader br(buffer, len);

	//������ ���۸� PKT_S_TEST�� ĳ�����ϸ� ���� �����ּҺ��� ��Ŷ�� �ʿ��� ������ŭ ĳ������ �ȴ�.
	PKT_S_TEST* pkt = reinterpret_cast<PKT_S_TEST*>(buffer);

	if (pkt->Validate() == false)
		return;

	//buffer���� ���� ������ �κ��� vector�� �����ϴ� ��ſ�
	//��Ŷ���� ������ BuffsList ����ü�� �ִ� �����ͷ� ĳ���� �ؼ� ������ �ϵ�����
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
