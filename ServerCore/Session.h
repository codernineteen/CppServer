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
		BUFFER_SIZE = 0x10000, //64KB 기본 버퍼 크기
	};

public:
	Session();
	virtual ~Session();

public:
	//외부에서 사용할 함수 정의
	void Send(SendBufferRef sendBuffer);
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
	void RegisterSend();

	void ProcessConnect();
	void ProcessDisconnect();
	void ProcessRecv(int32 numOfBytes);
	void ProcessSend(int32 numOfBytes);

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

private:
	weak_ptr<Service> _service; // 참조 순환을 피하기 위해서 Service 에 대해서는 weak pointer로 참조
	SOCKET _socket = INVALID_SOCKET;
	NetworkAddress _netAddress = {};
	Atomic<bool> _connected = false;

private:
	USE_LOCK;

	//수신 관련
	RecvBuffer _recvBuffer;

	//송신 관련
	Queue<SendBufferRef> _sendQueue;
	Atomic<bool> _sendRegistered = false; //send가 등록됐는지 원자적으로 확인

private:
	//iocp event 재사용
	ConnectEvent _connectEvent;
	DisconnectEvent _disconnectEvent;
	RecvEvent _recvEvent;
	SendEvent _sendEvent;
};

