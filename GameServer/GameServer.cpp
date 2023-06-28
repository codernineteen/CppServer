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

#include <WinSock2.h> //window �� ���� ���̺귯��
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

    //TCP ����
    SOCKET listenSocket = ::socket(AF_INET, SOCK_STREAM, 0);
    if (listenSocket == INVALID_SOCKET)
        return HandleError();


    u_long on = 1;
    //non-blocking �������� ����
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

    //WSAEventSelect = (event select �Լ��� �ٽ��� �ȴ�.)

    vector<WSAEVENT> wsaEvents; //���� ������ŭ ����ؼ� ����
    vector<Session> sessions;
    sessions.reserve(100);

    WSAEVENT listenEvent = ::WSACreateEvent(); // �̺�Ʈ ����
    wsaEvents.push_back(listenEvent);
    sessions.push_back({ listenSocket }); //������ Ŭ���̾�Ʈ ���� ������ �ƴ�����, �����Ǵ� �̺�Ʈ�� �ε����� ��ġ���Ѽ� ���ϰ� ����ϱ� ����.
    //���ϰ� �̺�Ʈ ��ü�� ���� �̺�Ʈ�� ����
    if (::WSAEventSelect(listenSocket, listenEvent, FD_ACCEPT | FD_CLOSE) == SOCKET_ERROR)
    {
        return 0;
    }


    while (true)
    {
        
    }



   
    ::WSACleanup(); //socket terminate
} 