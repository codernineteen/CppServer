#include "pch.h"
#include <iostream>

#include <WinSock2.h> //window �� ���� ���̺귯��
#include <MSWSock.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib") // <- �̰� �������� ������


int HandleError()
{
    int32 errCode = ::WSAGetLastError();
    cout << "Socket error code is : " << errCode << endl;
    return 1;
}

int main()
{
	//win socket initailize -> ���̺귯�� �ʱ�ȭ
	WSAData wsaData;
    if (::WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
        return HandleError();

    // [parameters]
    // af : address family(IPV4�� ���� �ּ� ������ ����)
    // type : TCP(socket stream) | UDP(datagram)
    // protocol : TCP | UDP�� ���� ��� �Ծ�
    SOCKET clientSocket = ::socket(AF_INET, SOCK_DGRAM, 0);
    if (clientSocket == INVALID_SOCKET)
        return HandleError();

    SOCKADDR_IN serverAddr;
    ::memset(&serverAddr, 0, sizeof(serverAddr)); 
    serverAddr.sin_family = AF_INET;
    ::inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);
    serverAddr.sin_port = ::htons(7777); 

    //connected UDP
    //������ ������ ������ ������, ���ã�� ó�� ���� ����� �����ؼ� ����� �� �ִ�.
    ::connect(clientSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr));

    while (true)
    {
        char sendBuffer[100] = "hello world!";

        for(int32 i=0; i < 10; i++)
        {
            //sendto�� �ϴ� ������ ����ip�ּ� �� ��Ʈ��ȣ�� �ڵ����� ������.
            //Unconnected UDP
            /*int32 resultCode = ::sendto(clientSocket, sendBuffer, sizeof(sendBuffer), 0, (SOCKADDR*)&serverAddr, sizeof(serverAddr));*/

            //connected UDP�� TCPó�� �׳� send�� ����Ѵ�.
            int32 resultCode = ::send(clientSocket, sendBuffer, sizeof(sendBuffer), 0);

            if (resultCode == SOCKET_ERROR)
                return HandleError();
            
            cout << "Send data. Len : " << sizeof(sendBuffer) << endl;
        }


        //���� �������� �����͸� ������ ���� Ư¡
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
