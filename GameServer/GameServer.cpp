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
        //실질적으로 이벤트가 발생했는지 검사
        int32 index = ::WSAWaitForMultipleEvents(wsaEvents.size(), &wsaEvents[0], FALSE, WSA_INFINITE, FALSE);

        if (index == WSA_WAIT_FAILED) //대기 실패 예외처리
            continue;

        index -= WSA_WAIT_EVENT_0; // 리턴 값에 붙어나온 정보를 빼줘서 정확한 인덱스 추출


        //이벤트 연동 시 여러 이벤트의 집합을 같이 등록했기 때문에, 정확히 어떤 이벤트로부터 발생했는지 알아내야한다.
        WSANETWORKEVENTS networkEvents; //아래 함수로부터 리턴된 네트워크 이벤트가 여기 저장됌
        if (::WSAEnumNetworkEvents(sessions[index].socket, wsaEvents[index], &networkEvents) == SOCKET_ERROR)
            continue;

        // listener 소켓 검사
        if (networkEvents.lNetworkEvents & FD_ACCEPT) //bit flag 검사
        {
            //에러 검사
            if (networkEvents.iErrorCode[FD_ACCEPT_BIT] != 0)
                continue;

            SOCKADDR_IN clientAddr;
            int32 addrLen = sizeof(clientAddr);
            SOCKET clientSocket = ::accept(listenSocket, (SOCKADDR*)&clientAddr, &addrLen);
            if (clientSocket != INVALID_SOCKET)
            {
                cout << "Client connected" << endl;

                //클라이언트 소켓 이벤트 등록
                WSAEVENT clientEvent = ::WSACreateEvent();
                wsaEvents.push_back(clientEvent);
                sessions.push_back({ clientSocket });
                if (::WSAEventSelect(clientSocket, clientEvent, FD_READ | FD_WRITE | FD_CLOSE) == SOCKET_ERROR)
                    return 0;
            }
        }

        //client session 소켓 체크
        if (networkEvents.lNetworkEvents & FD_READ || networkEvents.lNetworkEvents & FD_WRITE)
        {
            //에러 검사
            if ((networkEvents.lNetworkEvents & FD_READ) && (networkEvents.iErrorCode[FD_READ_BIT] != 0))
                continue;

            if ((networkEvents.lNetworkEvents & FD_WRITE) && (networkEvents.iErrorCode[FD_WRITE_BIT] != 0))
                continue;

            Session& s = sessions[index];

            // Read
            if (s.recvBytes == 0)
            {
                int32 recvLen = ::recv(s.socket, s.recvBuffer, BUFFSIZE, 0);
                if (recvLen == SOCKET_ERROR && ::WSAGetLastError() != WSAEWOULDBLOCK)
                {
                    // TODO : remove session
                    continue;
                }

                s.recvBytes = recvLen;
                cout << "Recv Data : " << s.recvBuffer << endl;
            }

            //Write
            if (s.recvBytes > s.sendBytes)
            {
                int32 sendLen = ::send(s.socket, &s.recvBuffer[s.sendBytes], s.recvBytes - s.sendBytes, 0);
                if (sendLen == SOCKET_ERROR && ::WSAGetLastError() != WSAEWOULDBLOCK)
                {
                    continue;
                }

                s.sendBytes += sendLen;
                if (s.recvBytes == s.sendBytes)
                {
                    s.recvBytes = 0;
                    s.sendBytes = 0;
                }

                cout << "Send Len : " << s.sendBytes << endl;
            }
        }

        //FD_CLOSE처리
        if (networkEvents.lNetworkEvents & FD_CLOSE)
        {
            //close처리
        }
        
    }



   
    ::WSACleanup(); //socket terminate
} 