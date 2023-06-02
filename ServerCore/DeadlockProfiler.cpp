#include "pch.h"
#include "DeadlockProfiler.h"

void DeadlockProfiler::PushLock(const char* name)
{
	LockGuard guard(_lock);

	//find id or issue
	int32 lockId = 0;

	auto findIt = _nameToId.find(name);
	if(findIt == _nameToId.end())
	{
		lockId = static_cast<int32>(_nameToId.size());
		_nameToId[name] = lockId;
		_idToName[lockId] = name;
	}
	else
	{
		lockId = findIt->second;
	}

	//If there was a lock holding
	if (LLockStack.empty() == false)
	{
		 //If It is undiscovered case
		const int32 prevId = LLockStack.top();
		if (lockId != prevId)
		{
			//lockId is not same with prevId
			//check dead lock
			set<int32>& history = _lockHistory[prevId];
			if (history.find(lockId) != history.end())
			{
				history.insert(lockId);
				CheckCycle();
			}
		}
	}

	LLockStack.push(lockId);
}

void DeadlockProfiler::PopLock(const char* name)
{
	LockGuard guard(_lock);

	if (LLockStack.empty())
		CRASH("MUTLIPLE_UNLOCK");
	
	int32 lockId = _nameToId[name];
	if (LLockStack.top() != lockId)
		CRASH("INVALID_UNLOCK");



	LLockStack.pop();
}

void DeadlockProfiler::CheckCycle()
{
	const int32 lockCount = static_cast<int32>(_nameToId.size());
	_discoverOrder = vector<int32>(lockCount, -1);
	_discoverCount = 0;
	_finished = vector<bool>(lockCount, false);
	_parent = vector<int32>(lockCount, -1); 

	for (int32 lockId = 0; lockId < lockCount; lockId++)
		Dfs(lockId);


	//if dfs finished
	_finished.clear();
	_discoverOrder.clear();
	_parent.clear();
}

void DeadlockProfiler::Dfs(int32 here)
{
	if (_discoverOrder[here] != -1)
		return;

	_discoverOrder[here] = _discoverCount++;

	//visit all adjacent vertices
	auto findIt = _lockHistory.find(here);
	if (findIt == _lockHistory.end())
	{
		_finished[here] = true;
		return;
	}
	
	set<int32>& nextSet = findIt->second;
	for (int32 there : nextSet)
	{
		if (_discoverOrder[there] == -1)
		{
			_parent[there] = here;
			Dfs(there);
			continue;
		}

		//if it already visited
		if (_discoverOrder[there] > _discoverOrder[here])
			continue;

		// there is not finished yet, but we found reverse-direction
		if (_finished[there] == false)
		{
			printf("%s -> %s\n", _idToName[here], _idToName[there]);

			int32 now = there;
			while (true)
			{
				printf("%s -> %s\n", _idToName[_parent[now]], _idToName[now]);
				now = _parent[now];
				if (now == there)
					break;
			}

			CRASH("DEADLOCK_DETECTED");
		}
	}

	_finished[here] = true;
}
