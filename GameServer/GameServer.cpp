#include "pch.h"
#include <iostream>
#include <tchar.h>

#include "ThreadManager.h"
#include "Service.h"
#include "GameSession.h"
#include "GameSessionManager.h"
#include "BufferWriter.h"
#include "ServerPacketHandler.h"

int main()
{	
	ServerServiceRef service = MakeShared<ServerService>(
		NetworkAddress(L"127.0.0.1", 7777), // network address
		MakeShared<IocpCore>(), // iocp core
		MakeShared<GameSession>, // session (no parentheses needed here)
		100 // max count
	);

	ASSERT_CRASH(service->Start());


	for (int32 i = 0; i < 4; i++)
	{
		GThreadManager->Launch([=]()
			{
				while (true)
				{
					service->GetIocpCore()->Dispatch();
				}
			}
		);
	}

	//main thread broad cast
	while (true)
	{
		vector<BuffData> buffs { BuffData{100, 1.5f}, BuffData{ 200, 2.3f }};
		SendBufferRef sendBuffer = ServerPacketHandler::Make_S_TEST(1001, 100, 10, buffs, L"¾È³çÇÏ¼¼¿ä");
		GameSessionManager::GetInstance().Broadcast(sendBuffer);

		this_thread::sleep_for(250ms);
	}

	GThreadManager->Join();
}