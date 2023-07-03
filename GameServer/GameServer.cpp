#include "pch.h"
#include <iostream>
#include "CorePch.h"
#include <atomic>
#include <mutex>
#include <windows.h>
#include <future>
#include "ThreadManager.h"

#include "SocketUtils.h"
#include "Listener.h"
#include "Service.h"
#include "Session.h"

void MakeSharedSession()
{
	MakeShared<Session>();
}

int main()
{	
	ServerServiceRef service = MakeShared<ServerService>(
		NetworkAddress(L"127.0.0.1", 7777), // network address
		MakeShared<IocpCore>(), // iocp core
		MakeShared<Session>, // session (no parentheses needed here)
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

	GThreadManager->Join();
}