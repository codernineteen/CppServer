#include "pch.h"
#include <iostream>

#include <WinSock2.h> //window 용 소켓 라이브러리
#include <MSWSock.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib") // <- 이거 빼먹으면 에러남

int main()
{
	//win socket initailize -> 라이브러리 초기화
	WSAData wsaData;
    if (::WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        /* Tell the user that we could not find a usable */
        /* Winsock DLL.                                  */
        printf("WSAStartup failed with error");
        return 1;
    }

    // [parameters]
    // af : address family(IPV4와 같은 주소 집합을 규정)
    // type : TCP(socket stream) | UDP(datagram)
    // protocol : TCP | UDP와 같은 통신 규약
    SOCKET clientSocket = ::socket(AF_INET, SOCK_STREAM, 0); //cross platform socket interface
    if (clientSocket == INVALID_SOCKET)
    {
       int32 errCode = ::WSAGetLastError();
       cout << "Socket error code is : " << errCode << endl;
       return 1;
    }

    SOCKADDR_IN serverAddr; //IPv4를 사용할 때 주소 타입
    ::memset(&serverAddr, 0, sizeof(serverAddr)); //주소 구조 메모리 초기화
    serverAddr.sin_family = AF_INET; //주소 체계 field
    ::inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr); //로컬 호스트 주소 - 4바이트 정수
    serverAddr.sin_port = ::htons(7777); //포트 바인딩(예약된 포트번호를 제외하고 임의의 번호를 지정 - 같은 ip에 대해서 고유식별자 역할)
    //htons : Host to network short

    //ipv4로 생성한 주소체계를 다시 일반적인 소켓 주소 타입으로 캐스팅해서 인자로 넘긴다.
    if (::connect(clientSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)))
    {
        int32 errCode = ::WSAGetLastError();
        cout << "Socket error code is : " << errCode << endl;
        return 1;
    }

    // --- 여기 도달하면 연결 성공! ---
    cout << "Connected to server" << endl;
    while (true)
    {
        this_thread::sleep_for(1s);
    }

 

    ::closesocket(clientSocket); //socket resource return 
    ::WSACleanup(); //socket terminate
}
