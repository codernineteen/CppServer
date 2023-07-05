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

	//큐에 걸어놓았던 데이터를 보내기 위해서 SendEvent의 vector로 옮겨준다.
	//WSASend를 하는 순간에 절대로 sendBuffer를 없어지지 않도록 해줘야 하는데,
	//queue에서 sendBufferRef를 꺼내는 작업이 ref count를 줄여서 소멸 가능성을 야기할 수 있기 때문에 event로 옮겨주는 것이다.
	Vector<SendBufferRef> sendBuffers;
};

