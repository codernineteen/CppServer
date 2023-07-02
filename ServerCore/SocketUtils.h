#pragma once
#include "NetworkAddress.h"

/**
	Socket Utils
*/

class SocketUtils
{
public:
	static LPFN_CONNECTEX		ConnectEx; //ConnectEx의 포인터
	static LPFN_DISCONNECTEX	DisconnectEx; //ConnectEx의 포인터
	static LPFN_ACCEPTEX		AcceptEx; //ConnectEx의 포인터

public:
	static void Init();
	static void Clear();

	static bool BindWindowsFunction(SOCKET socket, GUID guid, LPVOID* fn);
	static SOCKET CreateSocket();

	//sock option setting
	static bool SetLinger(SOCKET socket, uint16 onoff, uint16 linger);
	static bool SetNoDelay(SOCKET socket, bool flag);
	static bool SetReuseAddr(SOCKET socket, bool flag);
	static bool SetRecvBufSize(SOCKET socket, int32 size);
	static bool SetSendBufSize(SOCKET socket, int32 size);
	static bool SetUpdateAcceptSocket(SOCKET socket, SOCKET listenSocket);


	static bool Bind(SOCKET socket, NetworkAddress netAddr);
	static bool BindAnyAddress(SOCKET socket, uint16 port);
	static bool Listen(SOCKET socket, int32 backlog = SOMAXCONN);
	static void Close(SOCKET& socket);
};


template<typename T>
static inline bool SetSockOpt(SOCKET socket, int32 level, int32 optName, T optVal)
{
	return SOCKET_ERROR != ::setsockopt(socket, level, optName, reinterpret_cast<const char*>(&optVal), sizeof(T));
}
