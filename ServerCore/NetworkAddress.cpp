#include "pch.h"
#include "NetworkAddress.h"

NetworkAddress::NetworkAddress(SOCKADDR_IN sockAddr) : _sockAddr(sockAddr)
{
}

NetworkAddress::NetworkAddress(wstring ip, uint16 port)
{
	::memset(&_sockAddr, 0, sizeof(_sockAddr));
	_sockAddr.sin_family = AF_INET;
	_sockAddr.sin_addr = ip2Address(ip.c_str());
	_sockAddr.sin_port = ::htons(port);
}

wstring NetworkAddress::GetIpAddress()
{
	WCHAR buffer[100]; //char�� �ƴ϶� wchar�̱� ������ ���� ������ ���� 2����Ʈ
	::InetNtopW(AF_INET, &_sockAddr.sin_addr, buffer, len16(buffer));
	return buffer;
}

IN_ADDR NetworkAddress::ip2Address(const WCHAR* ip)
{
	IN_ADDR address;
	::InetPtonW(AF_INET, ip, &address);
	return address;
}
