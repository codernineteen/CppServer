#include "pch.h"
#include "CoreGlobal.h"

#include "ThreadManager.h"
#include "DeadlockProfiler.h"

ThreadManager* GThreadManager = nullptr;
DeadlockProfiler* GDeadlockProfiler = nullptr;

class CoreGlobal
{
public:
	CoreGlobal()
	{
		GThreadManager = new ThreadManager();
		GDeadlockProfiler = new DeadlockProfiler();
	}
	~CoreGlobal()
	{
		delete GThreadManager;
		delete GDeadlockProfiler;
	}
} GCoreGlobal;

