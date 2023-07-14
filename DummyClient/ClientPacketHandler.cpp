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

struct BuffData
{
	uint64 buffid;
	float remainTime;
};

//packet 설계
struct S_TEST
{
	uint64 id;
	uint32 hp;
	uint16 attack;
	//가변 데이터
	vector<BuffData> buffs;
	
	wstring name;
};

void ClientPacketHandler::Handle_S_TEST(BYTE* buffer, int32 len)
{
	BufferReader br(buffer, len);

	PacketHeader header;
	br >> header;

	uint64 id;
	uint32 hp;
	uint16 attack;
	vector<BuffData> buffs;
	uint16 buffCount;

	br >> id >> hp >> attack >> buffCount;

	cout << "id : " << id << " hp : " << hp << " attack : " << attack << endl;
	buffs.resize(buffCount);
	for (int32 i = 0; i < buffCount; i++)
	{
		br >> buffs[i].buffid >> buffs[i].remainTime;
	}

	cout << "BuffCount : " << buffCount << endl;
	for (int32 i = 0; i < buffCount; i++)
	{
		cout << "BuffInfo : [" << buffs[i].buffid << ", " << buffs[i].remainTime << "]" << endl;
	}

	wstring name;
	uint16 nameLen;
	br >> nameLen;
	name.resize(nameLen);
	br.Read((void*)name.data(), nameLen + sizeof(WCHAR));

	wcout.imbue(std::locale("kor"));
	wcout << "name" << endl;
}
