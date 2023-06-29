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

    WSAEVENT wsaEvent = ::WSACreateEvent();
    WSAOVERLAPPED overlapped = {};
    overlapped.hEvent = wsaEvent;


    while (true)
    {
        WSABUF wsaBuf;
        wsaBuf.buf = sendBuffer; //커널이 처리하기 때문에 절대로 건들면 안되는 부분
        wsaBuf.len = sizeof(sendBuffer);

        DWORD sendLen = 0;
        DWORD flags = 0;

        if (::WSASend(clientSocket, &wsaBuf, 1, &sendLen, flags, &overlapped, nullptr) == SOCKET_ERROR)
        {
            if (::WSAGetLastError() == WSA_IO_PENDING)
            {
                //pending 상태 - recvLen이 아직 채워지지 않음
                ::WSAWaitForMultipleEvents(1, &wsaEvent, TRUE, WSA_INFINITE, FALSE); //이벤트 형식으로 대기
                ::WSAGetOverlappedResult(clientSocket, &overlapped, &sendLen, FALSE, &flags);
            }
            else
            {
                //에러 사후 처리
                break;
            }
        }
        cout << "Send data len : " << sendLen << endl;

        this_thread::sleep_for(1s);
    }
    

    ::closesocket(clientSocket); //socket resource return 
    ::WSACleanup(); //socket terminate
}
