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
    int32 sendBytes = 0;
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

    //WSAEventSelect = (event select 함수가 핵심이 된다.)

    vector<WSAEVENT> wsaEvents; //세션 개수만큼 비례해서 생성
    vector<Session> sessions;
    sessions.reserve(100);

    WSAEVENT listenEvent = ::WSACreateEvent(); // 이벤트 생성
    wsaEvents.push_back(listenEvent);
    sessions.push_back({ listenSocket }); //엄밀히 클라이언트 관련 세션은 아니지만, 대응되는 이벤트와 인덱스를 매치시켜서 편리하게 사용하기 위함.
    //소켓과 이벤트 객체를 관련 이벤트로 연동
    if (::WSAEventSelect(listenSocket, listenEvent, FD_ACCEPT | FD_CLOSE) == SOCKET_ERROR)
    {
        return 0;
    }


    while (true)
    {
        
    }



   
    ::WSACleanup(); //socket terminate
} 