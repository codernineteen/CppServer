#pragma once
#include "IocpCore.h"
#include "IocpEvent.h"
#include "NetworkAddress.h"

/**
	Session
*/

class Session : public IocpObject
{
public:
	Session();
	virtual ~Session();

public:
	//세션 정보 관련
	void SetNetAddress(NetworkAddress netAddr);
	NetworkAddress GetNetAddress() { return _netAddress; }
	SOCKET GetSocket() { return _socket; }

public:
	//인터페이스 구현
	HANDLE GetHandle() override;
	void Dispatch(class IocpEvent* iocpEvent, int32 numOfBytes = 0) override;

public:
	//임시 버퍼
	char _recvBuffer[1000];

private:
	SOCKET _socket = INVALID_SOCKET;
	NetworkAddress _netAddress = {};
	Atomic<bool> _connected = false;
};

