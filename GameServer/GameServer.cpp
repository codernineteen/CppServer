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
		PKT_S_TEST_WRITE pktWriter(1001, 100, 10);
		PKT_S_TEST_WRITE::BuffsList buffList = pktWriter.ReserveBuffsList(3);
		buffList[0] = { 100, 1.5f };
		buffList[1] = { 200, 2.5f };
		buffList[2] = { 300, 3.5f };

		PKT_S_TEST_WRITE::BuffsVitimsList vic0 = pktWriter.ReserveBuffsVictimsList(&buffList[0], 3);
		{
			vic0[0] = 1000;
			vic0[1] = 2000;
			vic0[2] = 3000;
		}

		PKT_S_TEST_WRITE::BuffsVitimsList vic1 = pktWriter.ReserveBuffsVictimsList(&buffList[1], 2);
		{
			vic1[0] = 1000;
			vic1[1] = 2000;
		}

		PKT_S_TEST_WRITE::BuffsVitimsList vic2 = pktWriter.ReserveBuffsVictimsList(&buffList[2], 2);
		{
			vic2[0] = 1000;
			vic2[1] = 2000;
		}

		SendBufferRef sendBuffer = pktWriter.CloseAndReturn();
		GameSessionManager::GetInstance().Broadcast(sendBuffer);

		this_thread::sleep_for(250ms);
	}

	GThreadManager->Join();
}