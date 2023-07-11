#include "pch.h"
#include <iostream>

#include "ThreadManager.h"
#include "Service.h"
#include "GameSession.h"
#include "GameSessionManager.h"

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
	char sendData[1000] = "Hello world!";
	while (true)
	{
		SendBufferRef sendBuffer = GSendBufferManager->Open(4096);

		BYTE* buffer = sendBuffer->Buffer();
		((PacketHeader*)buffer)->size = (sizeof(sendData) + sizeof(PacketHeader));
		((PacketHeader*)buffer)->id = 1;

		::memcpy(&buffer[4], sendData, sizeof(sendData));
		sendBuffer->Close((sizeof(sendData) + sizeof(PacketHeader)));

		GameSessionManager::GetInstance().Broadcast(sendBuffer);

		this_thread::sleep_for(250ms);
	}

	GThreadManager->Join();
}