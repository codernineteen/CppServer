#pragma once

/**
	Network Address
*/

//클라이언트의 주소 정보가 필요할 때, 함수를 부르는 대신 클래스에 주소를 매핑해서 쉽게 정보를 받아올 수 있도록 하기 위함.

class NetworkAddress
{
public:
	NetworkAddress() = default;
	NetworkAddress(SOCKADDR_IN sockAddr);
	NetworkAddress(wstring ip, uint16 port);

	SOCKADDR_IN& GetSockAddr() { return _sockAddr; }
	wstring GetIpAddress();
	uint16 GetPort() { return ::ntohs(_sockAddr.sin_port); }

public:
	static IN_ADDR ip2Address(const WCHAR* ip);


private:
	SOCKADDR_IN _sockAddr = {};
};

