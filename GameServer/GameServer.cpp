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

#include <WinSock2.h> //window �� ���� ���̺귯��
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

    //TCP ����
    SOCKET listenSocket = ::socket(AF_INET, SOCK_STREAM, 0);
    if (listenSocket == INVALID_SOCKET)
        return HandleError();


    u_long on = 1;
    //non-blocking �������� ����
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

    SOCKADDR_IN clientAddr;
    int32 addrLen = sizeof(clientAddr);

    // Accept
    while (true)
    {
        SOCKET clientSocket = ::accept(listenSocket, (SOCKADDR*)&clientAddr, &addrLen);
        if (clientSocket == INVALID_SOCKET)
        {
            //non blocking ������ ����� ����, ���� ����߾���� ��Ȳ�� ������� �ʾұ� ������
            //������� Ȯ���� �� ����. ���� �߰����� �˻簡 �ʿ��ϴ�.
            if (::WSAGetLastError() == WSAEWOULDBLOCK)
                continue; // �� ��Ȳ�̶�� accept�� ������� ���� ��Ȳ���κ��� �߻��� ���̶� ū ������ �ƴϴ�.

            //ERROR
            break;
        }

        cout << "Client Connected" << endl;

        // Recv
        while (true)
        {
            char recvBuffer[1000];
            int32 recvLen = ::recv(clientSocket, recvBuffer, sizeof(recvBuffer), 0);
            if (recvLen == SOCKET_ERROR)
            {
                //�����͸� �����ϴ� ���������� ����������, ����ŷ Ư������ ���� �۵� ���ε� ���� �����͸� �������� ������ ����ġ�� ���ؼ� ������ ���̰� �ùٸ��� ���� ��Ȳ�� ���� ���� �ִ�.
                if (::WSAGetLastError() == WSAEWOULDBLOCK)
                    continue;

                //Error
                break;
            }
            else if (recvLen == 0)
            {
                //���� ����
                break;
            }

            cout << "Recv Data Len" << recvLen << endl;

            //Send - send�� �ٷ� �����Ѵٴ� ������ ����.
            while (true)
            {
                if (::send(clientSocket, recvBuffer, recvLen, 0) == SOCKET_ERROR)
                {
                    if (::WSAGetLastError() == WSAEWOULDBLOCK)
                        continue;

                    //Error
                    break;
                }

                cout << "Sent data " << endl;
                break;
            }
        }
    }

   
    ::WSACleanup(); //socket terminate
} 