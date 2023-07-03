#pragma once

/**
	IOCP Object
*/
//enable_shared_from_this�� ����ؼ� ������� weak ptr�� ������ �� �ְ� ���ش�.
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

	bool Register(IocpObjectRef iocpObject); //handle ���
	bool Dispatch(uint32 timeoutMS = INFINITE); //��Ŀ�����尡 iocp queue�� Ȯ���ϸ� ó��


private:
	HANDLE _iocpHandle;
};
