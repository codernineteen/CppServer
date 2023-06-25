#include "pch.h"
#include <iostream>

#include <WinSock2.h> //window �� ���� ���̺귯��
#include <MSWSock.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib") // <- �̰� �������� ������

int main()
{
	//win socket initailize -> ���̺귯�� �ʱ�ȭ
	WSAData wsaData;
    if (::WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        /* Tell the user that we could not find a usable */
        /* Winsock DLL.                                  */
        printf("WSAStartup failed with error");
        return 1;
    }

    // [parameters]
    // af : address family(IPV4�� ���� �ּ� ������ ����)
    // type : TCP(socket stream) | UDP(datagram)
    // protocol : TCP | UDP�� ���� ��� �Ծ�
    SOCKET clientSocket = ::socket(AF_INET, SOCK_STREAM, 0); //cross platform socket interface
    if (clientSocket == INVALID_SOCKET)
    {
       int32 errCode = ::WSAGetLastError();
       cout << "Socket error code is : " << errCode << endl;
       return 1;
    }

    SOCKADDR_IN serverAddr; //IPv4�� ����� �� �ּ� Ÿ��
    ::memset(&serverAddr, 0, sizeof(serverAddr)); //�ּ� ���� �޸� �ʱ�ȭ
    serverAddr.sin_family = AF_INET; //�ּ� ü�� field
    ::inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr); //���� ȣ��Ʈ �ּ� - 4����Ʈ ����
    serverAddr.sin_port = ::htons(7777); //��Ʈ ���ε�(����� ��Ʈ��ȣ�� �����ϰ� ������ ��ȣ�� ���� - ���� ip�� ���ؼ� �����ĺ��� ����)
    //htons : Host to network short

    //ipv4�� ������ �ּ�ü�踦 �ٽ� �Ϲ����� ���� �ּ� Ÿ������ ĳ�����ؼ� ���ڷ� �ѱ��.
    if (::connect(clientSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)))
    {
        int32 errCode = ::WSAGetLastError();
        cout << "Socket error code is : " << errCode << endl;
        return 1;
    }

    // --- ���� �����ϸ� ���� ����! ---
    cout << "Connected to server" << endl;
    while (true)
    {
        this_thread::sleep_for(1s);
    }

 

    ::closesocket(clientSocket); //socket resource return 
    ::WSACleanup(); //socket terminate
}
