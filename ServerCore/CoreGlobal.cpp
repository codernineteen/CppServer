#include "pch.h"
#include "CoreGlobal.h"

#include "ThreadManager.h"
#include "DeadlockProfiler.h"
#include "Memory.h"
#include "SocketUtils.h"
#include "SendBuffer.h"

ThreadManager* GThreadManager = nullptr;
Memory* GMemory = nullptr;
DeadlockProfiler* GDeadlockProfiler = nullptr;
SendBufferManager* GSendBufferManager = nullptr;


class CoreGlobal
{
public:
	CoreGlobal()
	{
		GThreadManager = new ThreadManager();
		GMemory = new Memory();
		GDeadlockProfiler = new DeadlockProfiler();
		GSendBufferManager = new SendBufferManager();
		SocketUtils::Init();
	}
	~CoreGlobal()
	{
		delete GThreadManager;
		delete GMemory;
		delete GDeadlockProfiler;
		SocketUtils::Clear();
	}
} GCoreGlobal;

