#include "pch.h"
#include "ThreadManager.h"
#include "CoreTLS.h"
#include "CoreGlobal.h"

/**
	Thread Manager Class
*/

ThreadManager::ThreadManager()
{
	InitTLS();
}

ThreadManager::~ThreadManager()
{
	Join();
}

void ThreadManager::Launch(function<void(void)> callback)
{
	LockGuard guard(_lock); //lock guard for launch
	_threads.push_back(thread([=]
		{
			InitTLS(); //init thread local storage
			callback(); // do callback
			DestroyTLS(); // destroy when going out of scope
		}
	));
}

void ThreadManager::Join()
{
	for (thread& t : _threads)
	{
		if(t.joinable())
			t.join();
	}
	_threads.clear(); //clear vector 
}

void ThreadManager::InitTLS()
{
	static Atomic<uint32> SThreadId = 1; // id issuer
	LThreadId = SThreadId.fetch_add(1); // return previous value after doing operation
}

void ThreadManager::DestroyTLS()
{

}
