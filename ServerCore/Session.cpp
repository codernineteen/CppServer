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
	// 이슈
	// 1. buffer 관리
	// 2. sendEvent관리, wsaSend 중첩 여부

	SendEvent* sendEvent = xnew<SendEvent>();
	sendEvent->owner = shared_from_this();
	sendEvent->buffer.resize(len);
	::memcpy(sendEvent->buffer.data(), buffer, len);

	WRITE_LOCK;
	RegisterSend(sendEvent);
}

bool Session::Connect()
{
	//서버 끼리 연결이 필요할 때 connect가 필요
	return RegisterConnect();
}

void Session::Disconnect(const WCHAR* cause)
{
	if (_connected.exchange(false) == false)
		return;

	// 임시 처리
	wcout << "Disconnect" << cause << endl;

	OnDisconnected(); //나중에 override
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

	//service유형이 client여야 한다.
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

	//TF_REUSE_SOCKET flag는 연결이 끊어진 소켓을 다시 재사용할 수 있도록 해준다.
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
			//pending이 아니면 문제가 있는 상황
			HandleError(errCode);
			_recvEvent.owner = nullptr; // 소유자에 대한 참조를 release (참조 카운트를 감소시키기 위해서 꼭 해주기)
		}

		//pending 상태
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
			//pending이 아니면 문제가 있는 상황
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

	//서비스에 세션 등록
	GetServiceRef()->AddSession(GetSessionRef());

	//컨텐츠 코드에서 override한 함수 호출
	OnConnected();

	//수신 등록 (recv 최초 등록)
	RegisterRecv();
}

void Session::ProcessDisconnect()
{
	_disconnectEvent.owner = nullptr;
}

void Session::ProcessRecv(int32 numOfBytes)
{
	//실질적으로 recv를 진행하기 때문에 소유자가 필요없어진다.
	_recvEvent.owner = nullptr; 
	
	if (numOfBytes == 0)
	{
		//연결이 끊긴 상황
		Disconnect(L"Recv zero byte");
		return;
	}

	//컨텐츠 override
	OnRecv(_recvBuffer, numOfBytes);

	//다시 수신 등록
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

	//컨텐츠 코드에서 override
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
