#pragma once
#include <thread>
#include <functional> 

/**
	Thread Manager Class
*/

class ThreadManager
{
public:
	ThreadManager();
	~ThreadManager();

	void Launch(function<void(void)> callback); //input and return type is void
	void Join();

	static void InitTLS();
	static void DestroyTLS();

private:
	Mutex _lock;
	vector<thread> _threads;
};

