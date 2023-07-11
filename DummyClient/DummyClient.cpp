#include "pch.h"
#include "ThreadManager.h"
#include <iostream>

#include <Service.h>
#include <Session.h>
#include <BufferReader.h>

char sendData[] = "Hello world!";

class ServerSession : public PacketSession
{
public:
	~ServerSession()
	{
		cout << "client destructed" << endl;
	}


	virtual void OnConnected() override
	{
		//cout << "Connected to Server" << endl;
		/*SendBufferRef sendBuffer = GSendBufferManager->Open(4096);
		::memcpy(sendBuffer->Buffer(), sendData, sizeof(sendData));
		sendBuffer->Close(sizeof(sendData));


		Send(sendBuffer);*/
	}

	virtual void OnDisconnected() override
	{
		//cout << "Disconnected" << endl;
	}

	virtual int32 OnRecvPacket(BYTE* buffer, int32 len) override
	{
		BufferReader br(buffer, len);
		PacketHeader header;
		br >> header;

		uint64 id;
		uint32 hp;
		uint16 attack;
		br >> id >> hp >> attack;

		cout << "id : " << id << "hp : " << hp << "attack : " << attack << endl;

		char recvBuf[4096];
		br.Read(recvBuf, header.size - sizeof(PacketHeader) - 8 - 4 - 2);
		::memcpy(recvBuf, &buffer[4], header.size - sizeof(PacketHeader));
		cout << recvBuf << endl;

		return len;
	}

	virtual void OnSend(int32 len) override
	{
		//echo
		//cout << "OnSend Len : " << len << endl;
	}
};

int main()
{
	this_thread::sleep_for(1s);

	ClientServiceRef service = MakeShared<ClientService>(
		NetworkAddress(L"127.0.0.1", 7777), // network address
		MakeShared<IocpCore>(), // iocp core
		MakeShared<ServerSession>, // session (no parentheses needed here)
		1000 // max count
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