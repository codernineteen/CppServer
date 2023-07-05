#include "pch.h"
#include "Session.h"
#include "SocketUtils.h"
#include "Service.h"

Session::Session() : _recvBuffer(BUFFER_SIZE)
{
	_socket = SocketUtils::CreateSocket();
}

Session::~Session()
{
	SocketUtils::Close(_socket);
}

void Session::Send(SendBufferRef sendBuffer)
{
	// ���� Register Send�� �ɸ��� ���� �����̸�, Send ��û
	WRITE_LOCK;

	_sendQueue.push(sendBuffer);

	//exchange�� ������ ���� ��ȯ
	if (_sendRegistered.exchange(true) == false)
		RegisterSend();
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
		ProcessSend(numOfBytes);
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
	if (false == SocketUtils::ConnectEx(_socket, reinterpret_cast<SOCKADDR*>(&sockAddr), sizeof(sockAddr), nullptr, 0, &numOfBytes, &_connectEvent))
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

	_recvEvent.Init();
	_recvEvent.owner = shared_from_this();

	WSABUF wsaBuf;
	wsaBuf.buf = reinterpret_cast<char*>(_recvBuffer.WritePos());
	wsaBuf.len = _recvBuffer.FreeSize();

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

void Session::RegisterSend()
{
	if (isConnected() == false)
		return;

	_sendEvent.Init();
	_sendEvent.owner = shared_from_this();

	//���� �����͸� sendEvent�� ���
	{
		WRITE_LOCK;

		int32 writeSize = 0;
		while (_sendQueue.empty() == false)
		{
			SendBufferRef sendBuffer = _sendQueue.front();

			writeSize += sendBuffer->WriteSize();

			_sendQueue.pop();
			_sendEvent.sendBuffers.push_back(sendBuffer);
		}
	}

	//scatter & gather ��� ���� : ����� �ִ� �����͸� ���ļ� ������
	Vector<WSABUF> wsaBufs;
	wsaBufs.reserve(_sendEvent.sendBuffers.size());

	for (SendBufferRef& buf : _sendEvent.sendBuffers)
	{
		WSABUF wsaBuf;
		wsaBuf.buf = reinterpret_cast<char*>(buf->Buffer());
		wsaBuf.len = static_cast<ULONG>(buf->WriteSize());
		wsaBufs.push_back(wsaBuf);
	}

	DWORD numOfBytes = 0;
	DWORD flags = 0;
	if (SOCKET_ERROR == ::WSASend(_socket, wsaBufs.data(), static_cast<DWORD>(wsaBufs.size()), OUT & numOfBytes, flags, &_sendEvent, nullptr))
	{
		int32 errCode = ::WSAGetLastError();
		if (errCode != WSA_IO_PENDING)
		{
			//pending�� �ƴϸ� ������ �ִ� ��Ȳ
			HandleError(errCode);
			_sendEvent.owner = nullptr; // release reference
			_sendEvent.sendBuffers.clear(); // release reference
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

	if (_recvBuffer.OnWrite(numOfBytes) == false)
	{
		Disconnect(L"On Write overflow");
		return;
	}

	int32 dataSize = _recvBuffer.DataSize(); //���� ������ ũ��
	int32 processLen = OnRecv(_recvBuffer.ReadPos(), dataSize);
	if (processLen < 0 || dataSize < processLen || _recvBuffer.OnRead(processLen) == false)
	{
		Disconnect(L"On read overflow");
		return;
	}

	//��ġ ����
	_recvBuffer.Clean();

	//�ٽ� ���� ���
	RegisterRecv();
}

void Session::ProcessSend(int32 numOfBytes)
{
	//event�� ���õ� refs release
	_sendEvent.owner = nullptr;
	_sendEvent.sendBuffers.clear(); //

	if (numOfBytes == 0)
	{
		Disconnect(L"Send Zero byte");
		return;
	}

	//������ �ڵ忡�� override
	OnSend(numOfBytes);
	
	WRITE_LOCK;
	if (_sendQueue.empty())
		_sendRegistered.store(false);
	else
		RegisterSend(); //ó���� �������ϱ� ���� �ٽ� ť�� ã���ϱ�, send���
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
