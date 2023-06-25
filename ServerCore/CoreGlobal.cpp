#include "pch.h"
#include "CoreGlobal.h"

#include "ThreadManager.h"
#include "DeadlockProfiler.h"
#include "Memory.h"

ThreadManager* GThreadManager = nullptr;
Memory* GMemory = nullptr;
DeadlockProfiler* GDeadlockProfiler = nullptr;

class CoreGlobal
{
public:
	CoreGlobal()
	{
		GThreadManager = new ThreadManager();
		GMemory = new Memory();
		GDeadlockProfiler = new DeadlockProfiler();
	}
	~CoreGlobal()
	{
		delete GThreadManager;
		delete GMemory;
		delete GDeadlockProfiler;
	}
} GCoreGlobal;

