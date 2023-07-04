#pragma once

/**
	IOCP Object
*/
//enable_shared_from_this를 상속해서 재귀적인 weak ptr을 점유할 수 있게 해준다.
class IocpObject : public enable_shared_from_this<IocpObject>
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

	bool Register(IocpObjectRef iocpObject); //handle 등록
	bool Dispatch(uint32 timeoutMS = INFINITE); //워커쓰레드가 iocp queue를 확인하며 처리


private:
	HANDLE _iocpHandle;
};
