#include "pch.h"
#include "ThreadManager.h"
#include <iostream>

#include <Service.h>
#include <Session.h>

char sendBuffer[] = "Hello world!";

class ServerSession : public Session
{
public:
	~ServerSession()
	{
		cout << "client destructed" << endl;
	}


	virtual void OnConnected() override
	{
		cout << "Connected to Server" << endl;
		Send((BYTE*)sendBuffer, sizeof(sendBuffer));
	}

	virtual void OnDisconnected() override
	{
		cout << "Disconnected" << endl;
	}

	virtual int32 OnRecv(BYTE* buffer, int32 len) override
	{
		//echo
		cout << "OnRecv Len : " << len << endl;

		this_thread::sleep_for(1s);

		Send((BYTE*)sendBuffer, sizeof(sendBuffer));
		return len;
	}

	virtual void OnSend(int32 len) override
	{
		//echo
		cout << "OnSend Len : " << len << endl;
	}
};

int main()
{
	this_thread::sleep_for(1s);

	ClientServiceRef service = MakeShared<ClientService>(
		NetworkAddress(L"127.0.0.1", 7777), // network address
		MakeShared<IocpCore>(), // iocp core
		MakeShared<ServerSession>, // session (no parentheses needed here)
		1 // max count
	);

	ASSERT_CRASH(service->Start());

	for (int32 i = 0; i < 2; i++)
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