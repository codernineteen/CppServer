#pragma once

/**
	IOCP Object
*/

class IocpObject
{
public:
	virtual HANDLE GetHandle() abstract;
	virtual void Dispatch(class IocpEvent* iocpEvent, int32 numOfBytes = 0) abstract;
};

/**
	IOCP Core class
*/

class IocpCore
{
public:
	IocpCore();
	~IocpCore();

	HANDLE GetHandle() { return _iocpHandle;  }

	bool Register(class IocpObject* iocpObject); //handle 등록
	bool Dispatch(uint32 timeoutMS = INFINITE); //워커쓰레드가 iocp queue를 확인하며 처리


private:
	HANDLE _iocpHandle;
};

//임시로 iocp core객체를 헤더파일에서 관리
extern IocpCore GIocpCore;

