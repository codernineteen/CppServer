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

int main() 
{
    //Win socket initialization
    WSAData wsaData;
    if (::WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        /* Tell the user that we could not find a usable */
        /* Winsock DLL.                                  */
        printf("WSAStartup failed with error");
        return 1;
    }

    //Listen socket initialization
    SOCKET listenSocket = ::socket(AF_INET, SOCK_STREAM, 0); //cross platform socket interface
    if (listenSocket == INVALID_SOCKET)
    {
        int32 errCode = ::WSAGetLastError();
        cout << "Socket error code is : " << errCode << endl;
        return 1;
    }

    // 서버 주소
    SOCKADDR_IN serverAddr;
    ::memset(&serverAddr, 0, sizeof(serverAddr)); 
    serverAddr.sin_family = AF_INET; 
    serverAddr.sin_addr.s_addr = ::htonl(INADDR_ANY); //알아서 주소를 고름(유동적으로 동작)
    serverAddr.sin_port = ::htons(7777); 

    // 포트 바인딩
    if (::bind(listenSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
    {
        int32 errCode = ::WSAGetLastError();
        cout << "Socket error code is : " << errCode << endl;
        return 1;
    }

    // start listen
    //backlog는 어떤 대기열이 있을 때 최대 대기자 수를 지정한다.
    if (::listen(listenSocket, 10))
    {
        int32 errCode = ::WSAGetLastError();
        cout << "Socket error code is : " << errCode << endl;
        return 1;
    }

    // ---- communication start ----
    while (true)
    {
        SOCKADDR_IN clientAddr; //클라이언트 주소
        ::memset(&clientAddr, 0, sizeof(clientAddr));
        int32 addrLen = sizeof(clientAddr);
        
        SOCKET clientSocket = ::accept(listenSocket, (SOCKADDR*)&clientAddr, &addrLen); //accpet함수는 성공 시 클라이언트 소켓을 반환해주고, 이 소켓을 통해서 통신을 한다.
        if(clientSocket == INVALID_SOCKET)
        { 
            int32 errCode = ::WSAGetLastError();
            cout << "Socket error code is : " << errCode << endl;
            return 1;
        }

        //client entered
        char ipAddress[16];
        ::inet_ntop(AF_INET, &clientAddr.sin_addr, ipAddress, sizeof(ipAddress));
        cout << "client connected : " << ipAddress << endl;
    }

    ::WSACleanup(); //socket terminate
} 