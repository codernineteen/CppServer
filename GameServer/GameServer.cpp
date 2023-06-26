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

    //udp������ ������ ���� �ϳ��� ����� �Ѵ�.
    SOCKET serverSocket = ::socket(AF_INET, SOCK_DGRAM, 0);
    if (serverSocket == INVALID_SOCKET)
    {
        return HandleError();
    }

    //�ɼ��� �ؼ��ϰ� ó���� ��ü(level)?
    //���� �ڵ��̸� level�� SOL_SOCKET�� �ַ� ���
    //IPv4���, IPPROTO_IP�� ���� ����
    bool enable = true;
    //KEEPALIVE
    ::setsockopt(serverSocket, SOL_SOCKET, SO_KEEPALIVE, (char*)&enable, sizeof(enable));
    
    //LINGER
    // onoff = 0�̸� closeSocket�� �ٷ� retur, �ƴϸ� linger�� ��ŭ �����.
    LINGER linger;
    linger.l_onoff = 1; //linger�� Ų��.
    linger.l_linger = 5; //���ð�
    ::setsockopt(serverSocket, SOL_SOCKET, SO_LINGER, (char*)&linger, sizeof(linger));

    //Half close
    //SD_SEND : send ���´�
    //SD_RECEIVE : recv ���´�.
    //SD_BOTH : �� �� ���´�.
    //�������� ����� close�� �ٷ� ������ ��������� ���� �ƴ϶�, �Ʒ��� ����
    //shutdown�� �̿��ؼ� ���������� �ݴ� ����� ���� ���� ����.
    ::shutdown(serverSocket, SD_SEND);

    // SO_SNDBUF : �۽� ���� ũ��
    // SO_RCVBUF : ���� ���� ũ��
    int32 sendBufferSize;
    int32 optionLen = sizeof(sendBufferSize);
    ::getsockopt(serverSocket, SOL_SOCKET, SO_SNDBUF, (char*)&sendBufferSize, &optionLen);
    cout << "�۽� ���� ũ�� : " << sendBufferSize << endl;

    int32 recvBufferSize;
    optionLen = sizeof(recvBufferSize);
    ::getsockopt(serverSocket, SOL_SOCKET, SO_SNDBUF, (char*)&recvBufferSize, &optionLen);
    cout << "���� ���� ũ�� : " << recvBufferSize << endl;

    //SO_REUSEADDR
    //ip�ּ� �� port�� ����
    {
        ::setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&enable, sizeof(enable));
    }

    //IPPROTO_TCP
    //TCP_NODELAY = Nalge �˰��� �۵� ����
    //�����Ͱ� ����� ũ�� ������, �׷��� ������ �����Ͱ� ����� ���϶����� ���
    // ���� : ���� ��Ŷ�� ���ʿ��ϰ� �����Ǵ� ���� ����
    // ���� : ���� �ð����� ���ظ� ����.
    {
        //���ӿ����� �ǽð� ���񽺸� ���� ���̱� �˰����� ���� ������ �Ѵ�.
        bool enable = true;
        ::setsockopt(serverSocket, IPPROTO_TCP, TCP_NODELAY, (char*)&enable, sizeof(enable));
    }
    ::WSACleanup(); //socket terminate
} 