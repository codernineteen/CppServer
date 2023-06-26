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

int main() 
{
    //Win socket initialization
    WSAData wsaData;
    if (::WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        printf("WSAStartup failed with error");
        return 1;
    }

    //udp에서는 유일한 소켓 하나로 통신을 한다.
    SOCKET serverSocket = ::socket(AF_INET, SOCK_DGRAM, 0);
    if (serverSocket == INVALID_SOCKET)
    {
        return HandleError();
    }

    SOCKADDR_IN serverAddr; 
    ::memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET; 
    serverAddr.sin_addr.s_addr = ::htonl(INADDR_ANY);
    serverAddr.sin_port = ::htons(7777); 

    if (::bind(serverSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
    {
        return HandleError();
    }

    //udp 준비 끝
    while (true)
    {
        //연결 과정없이 데이터를 보내는 것이 특징
        SOCKADDR_IN clientAddr;
        ::memset(&clientAddr, 0, sizeof(clientAddr));
        int32 addrLen = sizeof(clientAddr);

        this_thread::sleep_for(1s);

        char recvBuffer[1000];
        int32 recvLen = ::recvfrom(serverSocket, recvBuffer, sizeof(recvBuffer), 0, (SOCKADDR*)&clientAddr, &addrLen) ;
        
        if (recvLen <= 0)
            return HandleError();

        cout << "Recv data : " << recvBuffer << endl;

        int32 errCode = ::sendto(serverSocket, recvBuffer, recvLen, 0, (SOCKADDR*)&clientAddr, addrLen);

        if (errCode == SOCKET_ERROR)
            return HandleError();

        cout << "Send data : " << recvBuffer << endl;
    }

    ::WSACleanup(); //socket terminate
} 