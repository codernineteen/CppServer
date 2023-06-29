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

    SOCKET clientSocket = ::socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET)
        return HandleError();

    u_long on = 1;
    //non-blocking �������� ����
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
            if (::WSAGetLastError() == WSAEWOULDBLOCK) // connect ����� EWOULDBLOCK�� ������ �õ����� ���� �ǹ���
                continue;

            if (::WSAGetLastError() == WSAEISCONN) // �̹� ����� ���¶��
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
        wsaBuf.buf = sendBuffer; //Ŀ���� ó���ϱ� ������ ����� �ǵ�� �ȵǴ� �κ�
        wsaBuf.len = sizeof(sendBuffer);

        DWORD sendLen = 0;
        DWORD flags = 0;

        if (::WSASend(clientSocket, &wsaBuf, 1, &sendLen, flags, &overlapped, nullptr) == SOCKET_ERROR)
        {
            if (::WSAGetLastError() == WSA_IO_PENDING)
            {
                //pending ���� - recvLen�� ���� ä������ ����
                ::WSAWaitForMultipleEvents(1, &wsaEvent, TRUE, WSA_INFINITE, FALSE); //�̺�Ʈ �������� ���
                ::WSAGetOverlappedResult(clientSocket, &overlapped, &sendLen, FALSE, &flags);
            }
            else
            {
                //���� ���� ó��
                break;
            }
        }
        cout << "Send data len : " << sendLen << endl;

        this_thread::sleep_for(1s);
    }
    

    ::closesocket(clientSocket); //socket resource return 
    ::WSACleanup(); //socket terminate
}
