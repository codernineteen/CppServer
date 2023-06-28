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

    //WSAEventSelect = (event select �Լ��� �ٽ��� �ȴ�.)

    vector<WSAEVENT> wsaEvents; //���� ������ŭ ����ؼ� ����
    vector<Session> sessions;
    sessions.reserve(100);

    WSAEVENT listenEvent = ::WSACreateEvent(); // �̺�Ʈ ����
    wsaEvents.push_back(listenEvent);
    sessions.push_back({ listenSocket }); //������ Ŭ���̾�Ʈ ���� ������ �ƴ�����, �����Ǵ� �̺�Ʈ�� �ε����� ��ġ���Ѽ� ���ϰ� ����ϱ� ����.
    //���ϰ� �̺�Ʈ ��ü�� ���� �̺�Ʈ�� ����
    if (::WSAEventSelect(listenSocket, listenEvent, FD_ACCEPT | FD_CLOSE) == SOCKET_ERROR)
    {
        return 0;
    }


    while (true)
    {
        //���������� �̺�Ʈ�� �߻��ߴ��� �˻�
        int32 index = ::WSAWaitForMultipleEvents(wsaEvents.size(), &wsaEvents[0], FALSE, WSA_INFINITE, FALSE);

        if (index == WSA_WAIT_FAILED) //��� ���� ����ó��
            continue;

        index -= WSA_WAIT_EVENT_0; // ���� ���� �پ�� ������ ���༭ ��Ȯ�� �ε��� ����


        //�̺�Ʈ ���� �� ���� �̺�Ʈ�� ������ ���� ����߱� ������, ��Ȯ�� � �̺�Ʈ�κ��� �߻��ߴ��� �˾Ƴ����Ѵ�.
        WSANETWORKEVENTS networkEvents; //�Ʒ� �Լ��κ��� ���ϵ� ��Ʈ��ũ �̺�Ʈ�� ���� �����
        if (::WSAEnumNetworkEvents(sessions[index].socket, wsaEvents[index], &networkEvents) == SOCKET_ERROR)
            continue;

        // listener ���� �˻�
        if (networkEvents.lNetworkEvents & FD_ACCEPT) //bit flag �˻�
        {
            //���� �˻�
            if (networkEvents.iErrorCode[FD_ACCEPT_BIT] != 0)
                continue;

            SOCKADDR_IN clientAddr;
            int32 addrLen = sizeof(clientAddr);
            SOCKET clientSocket = ::accept(listenSocket, (SOCKADDR*)&clientAddr, &addrLen);
            if (clientSocket != INVALID_SOCKET)
            {
                cout << "Client connected" << endl;

                //Ŭ���̾�Ʈ ���� �̺�Ʈ ���
                WSAEVENT clientEvent = ::WSACreateEvent();
                wsaEvents.push_back(clientEvent);
                sessions.push_back({ clientSocket });
                if (::WSAEventSelect(clientSocket, clientEvent, FD_READ | FD_WRITE | FD_CLOSE) == SOCKET_ERROR)
                    return 0;
            }
        }

        //client session ���� üũ
        if (networkEvents.lNetworkEvents & FD_READ || networkEvents.lNetworkEvents & FD_WRITE)
        {
            //���� �˻�
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

        //FD_CLOSEó��
        if (networkEvents.lNetworkEvents & FD_CLOSE)
        {
            //closeó��
        }
        
    }



   
    ::WSACleanup(); //socket terminate
} 