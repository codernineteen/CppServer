#include "pch.h"
#include "IocpCore.h"
#include "IocpEvent.h"

/**
	IOCP Core class
*/

IocpCore::IocpCore()
{
	_iocpHandle = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL,0,0);
	ASSERT_CRASH(_iocpHandle != INVALID_HANDLE_VALUE);
}

IocpCore::~IocpCore()
{
	::CloseHandle(_iocpHandle);
}

bool IocpCore::Register(IocpObjectRef iocpObject)
{
	//새로운 handle을 관찰하도록 port에 등록
	return ::CreateIoCompletionPort(iocpObject->GetHandle(), _iocpHandle, /*key*/ 0, 0);
}

bool IocpCore::Dispatch(uint32 timeoutMS)
{
	DWORD numOfBytes = 0;
	ULONG_PTR key = 0;
	IocpEvent* iocpEvent = nullptr;

	if (::GetQueuedCompletionStatus(_iocpHandle, OUT & numOfBytes, OUT &key, OUT reinterpret_cast<LPOVERLAPPED*>(&iocpEvent), timeoutMS))
	{
		//성공했을 경우
		IocpObjectRef iocpObject = iocpEvent->owner;
		iocpObject->Dispatch(iocpEvent, numOfBytes);
	}
	else
	{
		int32 errCode = ::WSAGetLastError();
		//timeout이 무한 대기가 아닌 경우, 타임아웃 에러가 발생할 수 있으므로 해당 경우를 처리해준다.
		switch(errCode)
		{
		case WAIT_TIMEOUT:
			return false;
		default:
			//TODO
			IocpObjectRef iocpObject = iocpEvent->owner;
			iocpObject->Dispatch(iocpEvent, numOfBytes);
			break;
		}
	}

	return true;
}
