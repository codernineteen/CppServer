#include "pch.h"
#include <iostream>

#include <WinSock2.h> //window 용 소켓 라이브러리
#include <MSWSock.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib") // <- 이거 빼먹으면 에러남


int HandleError()
{
    int32 errCode = ::WSAGetLastError();
    cout << "Socket error code is : " << errCode << endl;
    return 1;
}

int main()
{
    this_thread::sleep_for(1s);
	//win socket initailize -> 라이브러리 초기화
	WSAData wsaData;
    if (::WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
        return HandleError();

    SOCKET clientSocket = ::socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET)
        return HandleError();

    u_long on = 1;
    //non-blocking 소켓으로 설정
    if (::ioctlsocket(clientSocket, FIONBIO, &on) == INVALID_SOCKET)
        return HandleError();

    SOCKADDR_IN serverAddr;
    ::memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    ::inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);
    serverAddr.sin_port = ::htons(3000);

    //Connect
    while (true)
    {
        if (::connect(clientSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
        {
            if (::WSAGetLastError() == WSAEWOULDBLOCK) // connect 경우의 EWOULDBLOCK은 연결을 시도중인 것을 의미함
                continue;

            if (::WSAGetLastError() == WSAEISCONN) // 이미 연결된 상태라면
                break;

            //ERROR
            break;
        }
    }

    cout << "CONNECTED" << endl;
    char sendBuffer[100] = "Hello world!";

    while (true)
    {
        if (::send(clientSocket, sendBuffer, sizeof(sendBuffer), 0) == SOCKET_ERROR)
        {
            if (::WSAGetLastError() == WSAEWOULDBLOCK)
                continue;

            //Error
            break;
        }

        cout << "Sent data : " << sendBuffer << endl;

        while (true)
        {
            char recvBuffer[1000];
            int32 recvLen = ::recv(clientSocket, recvBuffer, sizeof(recvBuffer), 0);
            if (recvLen == SOCKET_ERROR)
            {
                if (::WSAGetLastError() == WSAEWOULDBLOCK)
                    continue;

                //Error
                break;
            }
            else if (recvLen == 0)
            {
                //연결 끊김
                break;
            }

            cout << "Recv data : " << recvBuffer << endl;
            break;
        }
        this_thread::sleep_for(1s);
    }
    

    ::closesocket(clientSocket); //socket resource return 
    ::WSACleanup(); //socket terminate
}
