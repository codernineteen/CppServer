#include "pch.h"
#include "ServerPacketHandler.h"
#include "BufferReader.h"
#include "BufferWriter.h"

void ServerPacketHandler::HandlePacket(BYTE* buffer, int32 len)
{
	BufferReader br(buffer, len);

	PacketHeader header;
	br.Peek(&header);

	switch (header.id)
	{
	default:
		break;
	}
}

//���۸� �����ϴ� �κ��� �����ϱ� ���� �Լ� ����
SendBufferRef ServerPacketHandler::Make_S_TEST(uint64 id, uint32 hp, uint16 attack, vector<BuffData> buffs, wstring name)
{
	SendBufferRef sendBuffer = GSendBufferManager->Open(4096);

	BufferWriter bw(sendBuffer->Buffer(), sendBuffer->AllocSize());

	PacketHeader* header = bw.Reserve<PacketHeader>();

	bw << id << hp << attack;

	struct ListHeader
	{
		uint16 offset;
		uint16 count;
	};

	//���� ������
	ListHeader* buffsHeader = bw.Reserve<ListHeader>();

	buffsHeader->offset = bw.WriteSize(); //������� �� ������ offset�� ����
	buffsHeader->count = buffs.size();

	for (BuffData& buff : buffs)
		bw << buff.buffid << buff.remainTime;

	header->size = bw.WriteSize();
	header->id = S_TEST;

	sendBuffer->Close(bw.WriteSize());

	return sendBuffer;
}
