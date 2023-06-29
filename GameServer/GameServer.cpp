// GameServer.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include "pch.h"
#include <iostream>
#include <thread> 
#include <atomic>
#include <mutex>
#include <windows.h>
#include <future>
#include "ThreadManager.h"

#include <WinSock2.h> //window 용 소켓 라이브러리
#include <MSWSock.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

int HandleError() 
{
    int32 errCode = ::WSAGetLastError();
    cout << "Socket error code is : " << errCode << endl;
    return 1;
}

const int32 BUFFSIZE = 1000;

struct Session
{
    SOCKET socket = INVALID_SOCKET;
    char recvBuffer[BUFFSIZE];
    int32 recvBytes = 0;
    WSAOVERLAPPED overlapped = {};
};

int main()
{
    //Win socket initialization
    WSAData wsaData;
    if (::WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        printf("WSAStartup failed with error");
        return 1;
    }

    //TCP 소켓
    SOCKET listenSocket = ::socket(AF_INET, SOCK_STREAM, 0);
    if (listenSocket == INVALID_SOCKET)
        return HandleError();


    u_long on = 1;
    //non-blocking 소켓으로 설정
    if (::ioctlsocket(listenSocket, FIONBIO, &on) == INVALID_SOCKET)
        return HandleError();

    SOCKADDR_IN serverAddr;
    ::memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = ::htonl(INADDR_ANY);
    serverAddr.sin_port = ::htons(3000);

    if (::bind(listenSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
        return HandleError();

    if (::listen(listenSocket, SOMAXCONN) == SOCKET_ERROR)
        return HandleError();

    cout << "Accept" << endl;

    //overlapped model(event based)
    while (true)
    {
        SOCKADDR_IN clientAddr;
        int32 addrLen = sizeof(clientAddr);

        SOCKET clientSocket;
        while (true)
        {
            clientSocket = ::accept(listenSocket, (SOCKADDR*)&clientAddr, &addrLen);

            if (clientSocket != INVALID_SOCKET)
                break;

            if (::WSAGetLastError() == EWOULDBLOCK)
                continue;

            return 0; // error
        }

        Session session = Session{ clientSocket };
        WSAEVENT wsaEvent = ::WSACreateEvent();
        session.overlapped.hEvent = wsaEvent;

        cout << "Client Connected" << endl;

        while (true)
        {
            WSABUF wsaBuf;
            wsaBuf.buf = session.recvBuffer; //커널이 처리하기 때문에 절대로 건들면 안되는 부분
            wsaBuf.len = BUFFSIZE;

            DWORD recvLen = 0;
            DWORD flags = 0;

            if (::WSARecv(clientSocket, &wsaBuf, 1, &recvLen, &flags, &session.overlapped, nullptr) == SOCKET_ERROR)
            {
                if (::WSAGetLastError() == WSA_IO_PENDING)
                {
                    //pending 상태 - recvLen이 아직 채워지지 않음
                    ::WSAWaitForMultipleEvents(1, &wsaEvent, TRUE, WSA_INFINITE, FALSE); //이벤트 형식으로 대기
                    ::WSAGetOverlappedResult(session.socket, &session.overlapped, &recvLen, FALSE, &flags);
                }
                else
                {
                    //에러 사후 처리
                    break;
                }
            }

            cout << "data recv len = " << recvLen << endl;
        }

        ::closesocket(session.socket);
        ::WSACloseEvent(wsaEvent);

    }

    


   
    ::WSACleanup(); //socket terminate
} 