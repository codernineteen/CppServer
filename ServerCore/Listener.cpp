#include "pch.h"
#include "Listener.h"
#include "SocketUtils.h"
#include "IocpEvent.h"
#include "Session.h"

Listener::~Listener()
{
	SocketUtils::Close(_socket);

	for (AcceptEvent* event : _acceptEvents)
	{
		// TODO

		xdelete(event);
	}
}

bool Listener::StartAccept(NetworkAddress netAddr)
{
	_socket = SocketUtils::CreateSocket();
	if (_socket == INVALID_SOCKET)
		return false;

	if (GIocpCore.Register(this) == false)
		return false;

	if (SocketUtils::SetReuseAddr(_socket, true) == false)
		return false;

	if (SocketUtils::SetLinger(_socket, 0, 0) == false)
		return false;

	if (SocketUtils::Bind(_socket, netAddr) == false)
		return false;

	if (SocketUtils::Listen(_socket) == false)
		return false;

	const int32  acceptCount = 1;
	for(int32 i = 0; i < acceptCount; i++)
	{
		//�̸� acceptEvent�� ����س��� worker thread�� ��� Ȥ�� ���߿� ó���� �� �ֵ��� ���ش�.
		AcceptEvent* acceptEvent = xnew<AcceptEvent>();
		_acceptEvents.push_back(acceptEvent);
		RegisterAccept(acceptEvent);
	}


	return false;
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
	//core�κ����� dispatch�帧�� �Ѱܹ޾Ƽ� ó��
	ASSERT_CRASH(iocpEvent->GetType() == EventType::Accept);

	AcceptEvent* acceptEvent = static_cast<AcceptEvent*>(iocpEvent);
	ProcessAccept(acceptEvent);
}

void Listener::RegisterAccept(AcceptEvent* acceptEvent)
{
	//accept event�� session������ �����ؼ�, dispatch�ÿ� � ������ �۾����� �ľ��� �� �ֵ��� ���ش�.
	Session* session = xnew<Session>();

	acceptEvent->Init();
	acceptEvent->SetSession(session);

	DWORD bytesReceived = 0; //accept byte �� ����� ��

	if (false == SocketUtils::AcceptEx(_socket, session->GetSocket(), session->_recvBuffer, 0, sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, OUT & bytesReceived, static_cast<LPOVERLAPPED>(acceptEvent)))
	{
		const int32 errCode = ::WSAGetLastError();
		if (errCode != WSA_IO_PENDING)
		{
			//pending�� �ƴҰ�� acceptEvent�� �ٽ� �ɾ��ش�.
			RegisterAccept(acceptEvent);
		}
	}
}

void Listener::ProcessAccept(AcceptEvent* acceptEvent)
{
	Session* session = acceptEvent->GetSession(); //���� ���� ����

	if (false == SocketUtils::SetUpdateAcceptSocket(session->GetSocket(), _socket))
	{
		//������ �ִ��� � �����ε� �ٽ� ����Լ��� ȣ������� ����ؼ� accept�� �� �� �ִ�.
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

//���� : ����ؼ� ������ �־ acceptEvent �� �����ϴ� ���� �� �� �ִµ�,
//���������� accept�� �ǰų� ������ ���� ����̵�, � ������ ������ �� �̻� �� acceptEvent�� ����� socket�� ���� ������ �����ص� ���� ������ ������ �ʴ´�.
