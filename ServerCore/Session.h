#pragma once
#include "IocpCore.h"
#include "IocpEvent.h"
#include "NetworkAddress.h"
#include "RecvBuffer.h"

class Service;

/**
	Session
*/

class Session : public IocpObject
{
	friend class Listener;
	friend class IocpCore;
	friend class Service;

	enum
	{
		BUFFER_SIZE = 0x10000, //64KB �⺻ ���� ũ��
	};

public:
	Session();
	virtual ~Session();

public:
	//�ܺο��� ����� �Լ� ����
	void Send(SendBufferRef sendBuffer);
	bool Connect();
	void Disconnect(const WCHAR* cause);

	shared_ptr<Service> GetServiceRef() { return _service.lock(); }
	void SetServiceRef(shared_ptr<Service> servRef) { _service = servRef; }


public:
	//���� ���� ����
	void SetNetAddress(NetworkAddress netAddr);
	NetworkAddress GetNetAddress() { return _netAddress; }
	SOCKET GetSocket() { return _socket; }
	//����Ȯ��
	bool isConnected() { return _connected; }
	//�ڱ� �ڽſ� ���� shared pointer ��ȯ
	SessionRef GetSessionRef() { return static_pointer_cast<Session>(shared_from_this()); }

private:
	//���۰���
	bool RegisterConnect(); // Client Service���� ���
	bool RegisterDisconnect();
	void RegisterRecv();
	//���� �ڵ忡���� �ӽ÷� event���� ���۸� �����ϰ� �����Ƿ�, sendEvent�� ���ڷ� �޾��ش�.
	void RegisterSend();

	void ProcessConnect();
	void ProcessDisconnect();
	void ProcessRecv(int32 numOfBytes);
	void ProcessSend(int32 numOfBytes);

	void HandleError(int32 errCode);

protected:
	//������ �ڵ忡�� override
	virtual void OnConnected() { }
	virtual int32 OnRecv(BYTE* buffer, int32 len) { return len; }
	virtual void OnSend(int32 len) { }
	virtual void OnDisconnected() { }

private:
	//�������̽� ����
	HANDLE GetHandle() override;
	void Dispatch(class IocpEvent* iocpEvent, int32 numOfBytes = 0) override;

private:
	weak_ptr<Service> _service; // ���� ��ȯ�� ���ϱ� ���ؼ� Service �� ���ؼ��� weak pointer�� ����
	SOCKET _socket = INVALID_SOCKET;
	NetworkAddress _netAddress = {};
	Atomic<bool> _connected = false;

private:
	USE_LOCK;

	//���� ����
	RecvBuffer _recvBuffer;

	//�۽� ����
	Queue<SendBufferRef> _sendQueue;
	Atomic<bool> _sendRegistered = false; //send�� ��ϵƴ��� ���������� Ȯ��

private:
	//iocp event ����
	ConnectEvent _connectEvent;
	DisconnectEvent _disconnectEvent;
	RecvEvent _recvEvent;
	SendEvent _sendEvent;
};

