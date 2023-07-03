#pragma once
#include "IocpCore.h"
#include "NetworkAddress.h"

class AcceptEvent; //forward declaration

/**
	Listener Class
*/

class Listener : public IocpObject
{
public:
	Listener() = default;
	~Listener();

public:
	//외부에서 사용할 함수
	bool StartAccept(NetworkAddress netAddr); //listen 시점 결정
	void CloseSocket();

public:
	//인터페이스 구현
	HANDLE GetHandle() override;
	void Dispatch(class IocpEvent* iocpEvent, int32 numOfBytes = 0) override;

private:
	//수신 관련
	void RegisterAccept(AcceptEvent* acceptEvent);
	void ProcessAccept(AcceptEvent* acceptEvent);

protected:
	SOCKET _socket = INVALID_SOCKET;
	Vector<AcceptEvent*> _acceptEvents;

};

