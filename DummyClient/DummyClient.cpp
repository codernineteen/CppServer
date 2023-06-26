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

    // [parameters]
    // af : address family(IPV4와 같은 주소 집합을 규정)
    // type : TCP(socket stream) | UDP(datagram)
    // protocol : TCP | UDP와 같은 통신 규약
    SOCKET clientSocket = ::socket(AF_INET, SOCK_DGRAM, 0);
    if (clientSocket == INVALID_SOCKET)
        return HandleError();

    SOCKADDR_IN serverAddr;
    ::memset(&serverAddr, 0, sizeof(serverAddr)); 
    serverAddr.sin_family = AF_INET;
    ::inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);
    serverAddr.sin_port = ::htons(7777); 

    //connected UDP
    //연결의 개념은 여전히 없지만, 즐겨찾기 처럼 보낼 대상을 저장해서 사용할 수 있다.
    ::connect(clientSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr));

    while (true)
    {
        char sendBuffer[100] = "hello world!";

        for(int32 i=0; i < 10; i++)
        {
            //sendto를 하는 시점에 나의ip주소 와 포트번호를 자동으로 설정함.
            //Unconnected UDP
            /*int32 resultCode = ::sendto(clientSocket, sendBuffer, sizeof(sendBuffer), 0, (SOCKADDR*)&serverAddr, sizeof(serverAddr));*/

            //connected UDP는 TCP처럼 그냥 send를 사용한다.
            int32 resultCode = ::send(clientSocket, sendBuffer, sizeof(sendBuffer), 0);

            if (resultCode == SOCKET_ERROR)
                return HandleError();
            
            cout << "Send data. Len : " << sizeof(sendBuffer) << endl;
        }


        //연결 과정없이 데이터를 보내는 것이 특징
        SOCKADDR_IN recvAddr;
        ::memset(&recvAddr, 0, sizeof(recvAddr));
        int32 addrLen = sizeof(recvAddr);

        char recvBuffer[1000];
        int32 recvLen = ::recvfrom(clientSocket, recvBuffer, sizeof(recvBuffer), 0, (SOCKADDR*)&recvAddr, &addrLen);

        if (recvLen <= 0)
            return HandleError();

        cout << "Recv data : " << recvBuffer << endl;

        this_thread::sleep_for(1s);
    }

 

    ::closesocket(clientSocket); //socket resource return 
    ::WSACleanup(); //socket terminate
}
