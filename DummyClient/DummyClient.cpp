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

    SOCKET clientSocket = ::socket(AF_INET, SOCK_DGRAM, 0);
    if (clientSocket == INVALID_SOCKET)
        return HandleError();
 

    ::closesocket(clientSocket); //socket resource return 
    ::WSACleanup(); //socket terminate
}
