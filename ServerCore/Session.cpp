#include "pch.h"
#include "Session.h"
#include "SocketUtils.h"
#include "Service.h"

Session::Session()
{
	_socket = SocketUtils::CreateSocket();
}

Session::~Session()
{
	SocketUtils::Close(_socket);
}

void Session::Send(BYTE* buffer, int32 len)
{
	// �̽�
	// 1. buffer ����
	// 2. sendEvent����, wsaSend ��ø ����

	SendEvent* sendEvent = xnew<SendEvent>();
	sendEvent->owner = shared_from_this();
	sendEvent->buffer.resize(len);
	::memcpy(sendEvent->buffer.data(), buffer, len);

	WRITE_LOCK;
	RegisterSend(sendEvent);
}

bool Session::Connect()
{
	//���� ���� ������ �ʿ��� �� connect�� �ʿ�
	return RegisterConnect();
}

void Session::Disconnect(const WCHAR* cause)
{
	if (_connected.exchange(false) == false)
		return;

	// �ӽ� ó��
	wcout << "Disconnect" << cause << endl;

	OnDisconnected(); //���߿� override
	GetServiceRef()->ReleaseSession(GetSessionRef());

	RegisterDisconnect();
}

void Session::SetNetAddress(NetworkAddress netAddr)
{
}

HANDLE Session::GetHandle()
{
	return reinterpret_cast<HANDLE>(_socket);
}

void Session::Dispatch(IocpEvent* iocpEvent, int32 numOfBytes)
{
	switch (iocpEvent->eventType)
	{
	case EventType::Connect:
		ProcessConnect();
		break;
	
	case EventType::Disconnect:
		ProcessDisconnect();
		break;

	case EventType::Recv:
		ProcessRecv(numOfBytes);
		break;

	case EventType::Send:
		ProcessSend(static_cast<SendEvent*>(iocpEvent), numOfBytes);
		break;

	default:
		break;
	}
}

bool Session::RegisterConnect()
{
	if (isConnected())
		return false;

	//service������ client���� �Ѵ�.
	if (GetServiceRef()->GetServicetype() != ServiceType::Client)
		return false;

	if (SocketUtils::SetReuseAddr(_socket, true) == false)
		return false;

	if (SocketUtils::BindAnyAddress(_socket, 0) == false)
		return false;

	_connectEvent.Init();
	_connectEvent.owner = shared_from_this();

	DWORD numOfBytes = 0;
	SOCKADDR_IN sockAddr = GetServiceRef()->GetNetAddress().GetSockAddr();
	if (false == SocketUtils::ConnectEx(_socket, reinterpret_cast<const SOCKADDR*>(&sockAddr), sizeof(sockAddr), nullptr, 0, &numOfBytes, &_connectEvent))
	{
		int32 errCode = ::WSAGetLastError();
		if (errCode != WSA_IO_PENDING)
		{
			_connectEvent.owner = nullptr;
			return false;
		}
	}

	return true;
}

bool Session::RegisterDisconnect()
{
	_disconnectEvent.Init();
	_disconnectEvent.owner = shared_from_this();

	//TF_REUSE_SOCKET flag�� ������ ������ ������ �ٽ� ������ �� �ֵ��� ���ش�.
	if (false == SocketUtils::DisconnectEx(_socket, &_disconnectEvent, TF_REUSE_SOCKET, 0))
	{
		int32 errCode = ::WSAGetLastError();
		if (errCode != WSA_IO_PENDING)
		{
			_disconnectEvent.owner = nullptr;
			return false;
		}
	}

	return true;
}

void Session::RegisterRecv()
{
	if (isConnected() == false)
		return;

	_recvEvent.owner = shared_from_this();

	WSABUF wsaBuf;
	wsaBuf.buf = reinterpret_cast<char*>(_recvBuffer);
	wsaBuf.len = len32(_recvBuffer);

	DWORD numOfBytes = 0;
	DWORD flags = 0;

	if (SOCKET_ERROR == ::WSARecv(_socket, &wsaBuf, 1, OUT &numOfBytes, OUT &flags, &_recvEvent, nullptr)) {
		int32 errCode = ::WSAGetLastError();

		if (errCode != WSA_IO_PENDING)
		{
			//pending�� �ƴϸ� ������ �ִ� ��Ȳ
			HandleError(errCode);
			_recvEvent.owner = nullptr; // �����ڿ� ���� ������ release (���� ī��Ʈ�� ���ҽ�Ű�� ���ؼ� �� ���ֱ�)
		}

		//pending ����
	}
}

void Session::RegisterSend(SendEvent* sendEvent)
{
	if (isConnected() == false)
		return;

	WSABUF wsaBuf;
	wsaBuf.buf = (char*)sendEvent->buffer.data();
	wsaBuf.len = (ULONG)sendEvent->buffer.size();

	DWORD numOfBytes = 0;
	DWORD flags = 0;
	if (SOCKET_ERROR == ::WSASend(_socket, &wsaBuf, 1, OUT & numOfBytes, flags, sendEvent, nullptr))
	{
		int32 errCode = ::WSAGetLastError();
		if (errCode != WSA_IO_PENDING)
		{
			//pending�� �ƴϸ� ������ �ִ� ��Ȳ
			HandleError(errCode);
			sendEvent->owner = nullptr; // release reference
			xdelete(sendEvent);
		}
	}
}

void Session::ProcessConnect()
{
	_connectEvent.owner = nullptr;
	_connected.store(true);

	//���񽺿� ���� ���
	GetServiceRef()->AddSession(GetSessionRef());

	//������ �ڵ忡�� override�� �Լ� ȣ��
	OnConnected();

	//���� ��� (recv ���� ���)
	RegisterRecv();
}

void Session::ProcessDisconnect()
{
	_disconnectEvent.owner = nullptr;
}

void Session::ProcessRecv(int32 numOfBytes)
{
	//���������� recv�� �����ϱ� ������ �����ڰ� �ʿ��������.
	_recvEvent.owner = nullptr; 
	
	if (numOfBytes == 0)
	{
		//������ ���� ��Ȳ
		Disconnect(L"Recv zero byte");
		return;
	}

	//������ override
	OnRecv(_recvBuffer, numOfBytes);

	//�ٽ� ���� ���
	RegisterRecv();
}

void Session::ProcessSend(SendEvent* sendEvent, int32 numOfBytes)
{
	sendEvent->owner = nullptr;
	xdelete(sendEvent);

	if (numOfBytes == 0)
	{
		Disconnect(L"Send Zero byte");
		return;
	}

	//������ �ڵ忡�� override
	OnSend(numOfBytes);
}

void Session::HandleError(int32 errCode)
{
	switch (errCode)
	{
	case WSAECONNRESET:
	case WSAECONNABORTED:
		Disconnect(L"HandleError");
	default:
		//TODO
		cout << "Handle Error : " << errCode << endl;
		break;
	}
}
