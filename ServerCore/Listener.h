#pragma once
#include "IocpCore.h"
#include "NetworkAddress.h"

class AcceptEvent; //forward declaration
class ServerService;

/**
	Listener Class
*/

class Listener : public IocpObject
{
public:
	Listener() = default;
	~Listener();

public:
	//�ܺο��� ����� �Լ�
	bool StartAccept(ServerServiceRef service); //listen ���� ����
	void CloseSocket();

public:
	//�������̽� ����
	HANDLE GetHandle() override;
	void Dispatch(class IocpEvent* iocpEvent, int32 numOfBytes = 0) override;

private:
	//���� ����
	void RegisterAccept(AcceptEvent* acceptEvent);
	void ProcessAccept(AcceptEvent* acceptEvent);

protected:
	SOCKET _socket = INVALID_SOCKET;
	Vector<AcceptEvent*> _acceptEvents;
	ServerServiceRef _service;

};

