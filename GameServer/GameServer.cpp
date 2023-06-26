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

    //옵션을 해석하고 처리할 주체(level)?
    //소켓 코드이면 level로 SOL_SOCKET을 주로 사용
    //IPv4라면, IPPROTO_IP와 같이 지정
    bool enable = true;
    //KEEPALIVE
    ::setsockopt(serverSocket, SOL_SOCKET, SO_KEEPALIVE, (char*)&enable, sizeof(enable));
    
    //LINGER
    // onoff = 0이면 closeSocket시 바로 retur, 아니면 linger초 만큼 대기함.
    LINGER linger;
    linger.l_onoff = 1; //linger를 킨다.
    linger.l_linger = 5; //대기시간
    ::setsockopt(serverSocket, SOL_SOCKET, SO_LINGER, (char*)&linger, sizeof(linger));

    //Half close
    //SD_SEND : send 막는다
    //SD_RECEIVE : recv 막는다.
    //SD_BOTH : 둘 다 막는다.
    //정석적인 방법은 close로 바로 연결을 끊어버리는 것이 아니라, 아래와 같이
    //shutdown을 이용해서 선택적으로 닫는 방법을 쓰는 것이 좋다.
    ::shutdown(serverSocket, SD_SEND);

    // SO_SNDBUF : 송신 버퍼 크기
    // SO_RCVBUF : 수신 버퍼 크기
    int32 sendBufferSize;
    int32 optionLen = sizeof(sendBufferSize);
    ::getsockopt(serverSocket, SOL_SOCKET, SO_SNDBUF, (char*)&sendBufferSize, &optionLen);
    cout << "송신 버퍼 크기 : " << sendBufferSize << endl;

    int32 recvBufferSize;
    optionLen = sizeof(recvBufferSize);
    ::getsockopt(serverSocket, SOL_SOCKET, SO_SNDBUF, (char*)&recvBufferSize, &optionLen);
    cout << "수신 버퍼 크기 : " << recvBufferSize << endl;

    //SO_REUSEADDR
    //ip주소 및 port를 재사용
    {
        ::setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&enable, sizeof(enable));
    }

    //IPPROTO_TCP
    //TCP_NODELAY = Nalge 알고리즘 작동 여부
    //데이터가 충분히 크면 보내고, 그렇지 않으면 데이터가 충분히 쌓일때까지 대기
    // 장점 : 작은 패킷이 불필요하게 생성되는 일을 방지
    // 단점 : 반응 시간에서 손해를 본다.
    {
        //게임에서는 실시간 서비스를 위해 네이글 알고리즘을 끄는 선택을 한다.
        bool enable = true;
        ::setsockopt(serverSocket, IPPROTO_TCP, TCP_NODELAY, (char*)&enable, sizeof(enable));
    }
    ::WSACleanup(); //socket terminate
} 