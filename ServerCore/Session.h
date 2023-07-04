#pragma once
#include "IocpCore.h"
#include "IocpEvent.h"
#include "NetworkAddress.h"

class Service;

/**
	Session
*/

class Session : public IocpObject
{
	friend class Listener;
	friend class IocpCore;
	friend class Service;

public:
	Session();
	virtual ~Session();

public:
	//외부에서 사용할 함수 정의
	void Send(BYTE* buffer, int32 len);
	bool Connect();
	void Disconnect(const WCHAR* cause);

	shared_ptr<Service> GetServiceRef() { return _service.lock(); }
	void SetServiceRef(shared_ptr<Service> servRef) { _service = servRef; }


public:
	//세션 정보 관련
	void SetNetAddress(NetworkAddress netAddr);
	NetworkAddress GetNetAddress() { return _netAddress; }
	SOCKET GetSocket() { return _socket; }
	//연결확인
	bool isConnected() { return _connected; }
	//자기 자신에 대한 shared pointer 반환
	SessionRef GetSessionRef() { return static_pointer_cast<Session>(shared_from_this()); }

private:
	//전송관련
	bool RegisterConnect(); // Client Service에서 사용
	bool RegisterDisconnect();
	void RegisterRecv();
	//현재 코드에서는 임시로 event내에 버퍼를 저장하고 있으므로, sendEvent를 인자로 받아준다.
	void RegisterSend(SendEvent* sendEvent);

	void ProcessConnect();
	void ProcessDisconnect();
	void ProcessRecv(int32 numOfBytes);
	void ProcessSend(SendEvent* sendEvent, int32 numOfBytes);

	void HandleError(int32 errCode);

protected:
	//컨텐츠 코드에서 override
	virtual void OnConnected() { }
	virtual int32 OnRecv(BYTE* buffer, int32 len) { return len; }
	virtual void OnSend(int32 len) { }
	virtual void OnDisconnected() { }

private:
	//인터페이스 구현
	HANDLE GetHandle() override;
	void Dispatch(class IocpEvent* iocpEvent, int32 numOfBytes = 0) override;

public:
	//임시 버퍼
	BYTE _recvBuffer[1000];

private:
	weak_ptr<Service> _service; // 참조 순환을 피하기 위해서 Service 에 대해서는 weak pointer로 참조
	SOCKET _socket = INVALID_SOCKET;
	NetworkAddress _netAddress = {};
	Atomic<bool> _connected = false;

private:
	USE_LOCK;

	//수신 관련

	//송신 관련

private:
	//iocp event 재사용
	ConnectEvent _connectEvent;
	DisconnectEvent _disconnectEvent;
	RecvEvent _recvEvent;
};

