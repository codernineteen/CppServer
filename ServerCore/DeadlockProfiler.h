#pragma once
#include <stack>
#include <vector>
#include <map>

/**
	Deadlock profiler
*/

class DeadlockProfiler
{
public:
	void PushLock(const char* name);
	void PopLock(const char* name);
	void CheckCycle();

private:
	void Dfs(int32 index);

private:
	unordered_map<const char*, int32> _nameToId;
	unordered_map<int32, const char*> _idToName;
	map<int32, set<int32>> _lockHistory; 

	Mutex _lock;

private:
	vector<int32> _discoverOrder; // recorde order of vertices
	int32 _discoverCount;
	vector<bool> _finished;
	vector<int32> _parent;
};

