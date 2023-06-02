#include "pch.h"
#include "Lock.h"
#include "DeadlockProfiler.h"

void Lock::WriteLock(const char* name)
{
#if _DEBUG
	GDeadlockProfiler->PushLock(name);
#endif

	// 1. When the same thread owns the write permission, always success
	const uint32 lockThreadId = (_lockFlag.load() & WRITE_THREAD_MASK) >> 16; // remove read flags and move 16bits backward
	if (LThreadId == lockThreadId)
	{
		_writeCount++;
		return;
	}

	// 2. When nobody owns(whilte writing) or share(while reading), Race and gain the ownership
	// The moment is EMPTY_FLAG


	//move thread id 16bits forward and wrap it with WRITE_THREAD_MASK
	const int64 beginTick = ::GetTickCount64();
	const uint32 desired = ((LThreadId << 16) & WRITE_THREAD_MASK);
	while(true)
	{
		for (uint32 spinCount = 0; spinCount < MAX_SPIN_COUNT; spinCount++)
		{
			//compare and swap operation
			//if  _lockFlag is same with expectation, swap it with desired value
			uint32 expected = EMPTY_FLAG;
			if (_lockFlag.compare_exchange_strong(OUT expected, desired)) 
			{
				_writeCount++; // For tracking recursive lock case
				return;
			}
		}
		//If this process takes too longer than expectation, crash it
		if (::GetTickCount64() - beginTick >= ACQUIRE_TIMEOUT_TICK)
		{
			CRASH("LOCK_TIMEOUT");
		}
		//After 5000 times try, yeild the authority to other threads
		this_thread::yield();
	}
	
}

void Lock::WriteUnlock(const char* name)
{
#if _DEBUG
	GDeadlockProfiler->PopLock(name);
#endif

	// Until we unlock all the read locks, it is impossible to unlock writer lock
	if ((_lockFlag.load() & READ_COUNT_MASK) != 0)
	{
		CRASH("INVALID_UNLOCK_ORDER")
	}


	const int32 lockCount = --_writeCount;
	if (lockCount == 0)
		_lockFlag.store(EMPTY_FLAG);
}

void Lock::ReadLock(const char* name)
{
#if _DEBUG
	GDeadlockProfiler->PushLock(name);
#endif

	// 1. when same threads owns
	const uint32 lockThreadId = (_lockFlag.load() & WRITE_THREAD_MASK) >> 16; // remove read flags and move 16bits backward
	if (LThreadId == lockThreadId)
	{
		_lockFlag.fetch_add(1);
		return;
	}

	// 2. when nobody owns , race and increments share count
	const int64 beginTick = ::GetTickCount64(); //start count tick
	while (true)
	{
		for (uint32 spinCount = 0; spinCount < MAX_SPIN_COUNT; spinCount++)
		{
			uint32 expected = (_lockFlag.load() & READ_COUNT_MASK);
			if (_lockFlag.compare_exchange_strong(OUT expected, expected + 1))
				return; // if the right case, just increment read count
		}

		if (::GetTickCount64() - beginTick >= ACQUIRE_TIMEOUT_TICK)
		{
			CRASH("LOCK_TIMEOUT");
		}

		this_thread::yield();
	}
}

void Lock::ReadUnlock(const char* name)
{
#if _DEBUG
	GDeadlockProfiler->PopLock(name);
#endif

	if ((_lockFlag.fetch_sub(1) & READ_COUNT_MASK) == 0)
	{
		CRASH("MULTIPLE_UNLOCK");
	}
}
