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
SendBufferRef ServerPacketHandler::Make_S_TEST(uint64 id, uint32 hp, uint32 power, vector<BuffData> buffs, wstring name)
{
	SendBufferRef sendBuffer = GSendBufferManager->Open(4096);

	BufferWriter bw(sendBuffer->Buffer(), sendBuffer->AllocSize());

	PacketHeader* header = bw.Reserve<PacketHeader>();

	bw << id << hp << power;

	//���� ������
	bw << (uint16)buffs.size();

	for (BuffData& buff : buffs)
	{
		bw << buff.buffid << buff.remainTime;
	}
	
	bw << (uint16)name.size();
	bw.Write((void*)name.data(), name.size() * sizeof(WCHAR));

	header->size = bw.WriteSize();
	header->id = S_TEST;

	sendBuffer->Close(bw.WriteSize());

	return sendBuffer;
}
