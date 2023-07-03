#include "pch.h"
#include "Listener.h"
#include "SocketUtils.h"
#include "IocpEvent.h"
#include "Session.h"
#include "Service.h"

Listener::~Listener()
{
	SocketUtils::Close(_socket);

	for (AcceptEvent* event : _acceptEvents)
	{
		// TODO

		xdelete(event);
	}
}

bool Listener::StartAccept(ServerServiceRef service)
{
	_service = service;
	if (_service == nullptr)
		return false;

	_socket = SocketUtils::CreateSocket();
	if (_socket == INVALID_SOCKET)
		return false;

	if (_service->GetIocpCore()->Register(shared_from_this()) == false)
		return false;

	if (SocketUtils::SetReuseAddr(_socket, true) == false)
		return false;

	if (SocketUtils::SetLinger(_socket, 0, 0) == false)
		return false;

	if (SocketUtils::Bind(_socket, _service->GetNetAddress()) == false)
		return false;

	if (SocketUtils::Listen(_socket) == false)
		return false;

	const int32 acceptCount = _service->GetMaxSessionCount();
	for (int32 i = 0; i < acceptCount; i++)
	{
		//미리 acceptEvent를 등록해놔서 worker thread가 즉시 혹은 나중에 처리할 수 있도록 해준다.
		AcceptEvent* acceptEvent = xnew<AcceptEvent>();
		acceptEvent->owner = shared_from_this(); // listener 스스로를 owner로 지정
		_acceptEvents.push_back(acceptEvent);
		RegisterAccept(acceptEvent);
	}

	return true;
}

void Listener::CloseSocket()
{
	SocketUtils::Close(_socket);
}

HANDLE Listener::GetHandle()
{
	return reinterpret_cast<HANDLE>(_socket);
}

void Listener::Dispatch(IocpEvent* iocpEvent, int32 numOfBytes) 
{
	//core로부터의 dispatch흐름을 넘겨받아서 처리
	ASSERT_CRASH(iocpEvent->eventType == EventType::Accept);

	AcceptEvent* acceptEvent = static_cast<AcceptEvent*>(iocpEvent);
	ProcessAccept(acceptEvent);
}

void Listener::RegisterAccept(AcceptEvent* acceptEvent)
{
	//accept event에 session정보를 연동해서, dispatch시에 어떤 세션의 작업인지 파악할 수 있도록 해준다.
	SessionRef session = _service->CreateSession(); //Register IOCP

	acceptEvent->Init();
	acceptEvent->session = session;

	DWORD bytesReceived = 0; //accept byte 를 뱉어줄 곳

	if (false == SocketUtils::AcceptEx(_socket, session->GetSocket(), session->_recvBuffer, 0, sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, OUT & bytesReceived, static_cast<LPOVERLAPPED>(acceptEvent)))
	{
		const int32 errCode = ::WSAGetLastError();
		if (errCode != WSA_IO_PENDING)
		{
			//pending이 아닐경우 acceptEvent를 다시 걸어준다.
			RegisterAccept(acceptEvent);
		}
	}
}

void Listener::ProcessAccept(AcceptEvent* acceptEvent)
{
	SessionRef session = acceptEvent->session; //세션 정보 복원

	if (false == SocketUtils::SetUpdateAcceptSocket(session->GetSocket(), _socket))
	{
		//문제가 있더라도 어떤 식으로든 다시 등록함수를 호출해줘야 계속해서 accept를 할 수 있다.
		RegisterAccept(acceptEvent);
		return;
	}

	SOCKADDR_IN sockAddr;
	int32 sizeOfSockAddr = sizeof(sockAddr);
	if (SOCKET_ERROR == ::getpeername(session->GetSocket(), OUT reinterpret_cast<SOCKADDR*>(&sockAddr), &sizeOfSockAddr))
	{
		RegisterAccept(acceptEvent);
		return;
	}
	
	session->SetNetAddress(NetworkAddress(sockAddr));

	cout << "Client Connected" << endl;

	//TODO 

	RegisterAccept(acceptEvent);
}

//참고 : 계속해서 문제가 있어도 acceptEvent 를 재사용하는 것을 볼 수 있는데,
//정상적으로 accept가 되거나 문제가 생긴 경우이든, 어떤 과정이 끝나면 더 이상 그 acceptEvent를 사용할 socket이 없기 때문에 재사용해도 전혀 문제가 생기지 않는다.
