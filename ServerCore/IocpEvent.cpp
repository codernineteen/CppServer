#include "pch.h"
#include "IocpEvent.h"

IocpEvent::IocpEvent(EventType type) : eventType(type)
{
	Init();
}

void IocpEvent::Init()
{
//overlapped 클래스 초기화
	OVERLAPPED::hEvent = 0;
	OVERLAPPED::Internal = 0;
	OVERLAPPED::InternalHigh = 0;
	OVERLAPPED::Offset= 0;
	OVERLAPPED::OffsetHigh = 0;
}
