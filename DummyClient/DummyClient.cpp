#include "pch.h"
#include "ThreadManager.h"
#include <iostream>

#include <Service.h>
#include <Session.h>

char sendData[] = "Hello world!";

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
		SendBufferRef sendBuffer = MakeShared<SendBuffer>(4096);
		sendBuffer->CopyData(sendData, len16(sendData));
		Send(sendBuffer);
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

		SendBufferRef sendBuffer = MakeShared<SendBuffer>(4096);
		sendBuffer->CopyData(sendData, len16(sendData));
		Send(sendBuffer);
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
		5 // max count
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