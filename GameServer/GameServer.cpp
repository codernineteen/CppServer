#include "pch.h"
#include <iostream>

#include "ThreadManager.h"
#include "Service.h"
#include "GameSession.h"
#include "GameSessionManager.h"
#include "BufferWriter.h"

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
	char sendData[] = "Hello world!";
	while (true)
	{
		SendBufferRef sendBuffer = GSendBufferManager->Open(4096);

		BufferWriter bw(sendBuffer->Buffer(), sendBuffer->AllocSize());

		PacketHeader* header = bw.Reserve<PacketHeader>();

		// id(uint64) , 체력(uint32), 공격력(uint16)
		bw << (uint64)1001 << (uint32)100 << (uint16)10;
		// 특정 데이터를 쓰고 싶을 때
		bw.Write(sendData, sizeof(sendData));
		
		header->size = bw.WriteSize();
		header->id = 1; // 1 : Msg
		
		sendBuffer->Close(bw.WriteSize());

		GameSessionManager::GetInstance().Broadcast(sendBuffer);

		this_thread::sleep_for(250ms);
	}

	GThreadManager->Join();
}