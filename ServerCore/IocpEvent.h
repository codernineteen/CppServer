#pragma once

class Session;

/**
	Iocp Event type
*/

enum class EventType : uint8
{
	Connect,
	Disconnect,
	Accept,
	Recv,
	Send
};

/**
	Iocp Event class
*/

class IocpEvent : public OVERLAPPED
{
public:
	IocpEvent(EventType type);

	void Init();

public:
	EventType eventType;
	IocpObjectRef owner;
};

/**
	Connect Event
*/

class ConnectEvent : public IocpEvent
{
public:
	ConnectEvent() : IocpEvent(EventType::Connect) {}
};

/**
	Disconnect Event
*/

class DisconnectEvent : public IocpEvent
{
public:
	DisconnectEvent() : IocpEvent(EventType::Disconnect) {}
};

/**
	Accept Event
*/

class AcceptEvent : public IocpEvent
{
public:
	AcceptEvent() : IocpEvent(EventType::Accept) { }

public:
	SessionRef session = nullptr;
};

/**
	Recv Event
*/

class RecvEvent : public IocpEvent
{
public:
	RecvEvent() : IocpEvent(EventType::Recv) {}
};

/**
	Send Event
*/

class SendEvent : public IocpEvent
{
public:
	SendEvent() : IocpEvent(EventType::Send) {}

	//ť�� �ɾ���Ҵ� �����͸� ������ ���ؼ� SendEvent�� vector�� �Ű��ش�.
	//WSASend�� �ϴ� ������ ����� sendBuffer�� �������� �ʵ��� ����� �ϴµ�,
	//queue���� sendBufferRef�� ������ �۾��� ref count�� �ٿ��� �Ҹ� ���ɼ��� �߱��� �� �ֱ� ������ event�� �Ű��ִ� ���̴�.
	Vector<SendBufferRef> sendBuffers;
};

