#pragma once

enum
{
	S_TEST = 1
};

struct BuffData
{
	uint64 buffid;
	float remainTime;
};

class ServerPacketHandler
{
public:
	static void HandlePacket(BYTE* buffer, int32 len);

	static SendBufferRef Make_S_TEST(uint64 id, uint32 hp, uint32 power, vector<BuffData> buffs, wstring name);
};

