#pragma once

/*
	Game Session Manager
*/

class GameSession;
using GameSessionRef = shared_ptr<GameSession>;


class GameSessionManager
{
public:
	static GameSessionManager& GetInstance()
	{
		static GameSessionManager instance;
		return instance;
	}

	void Add(GameSessionRef session);
	void Remove(GameSessionRef session);
	void Broadcast(SendBufferRef sendBuffer);

private:
	GameSessionManager() {} // Private constructor to enforce singleton pattern
	~GameSessionManager() {} // Private destructor
	GameSessionManager(const GameSessionManager&) = delete; // Delete copy constructor
	GameSessionManager& operator=(const GameSessionManager&) = delete; // Delete assignment operator

private:
	USE_LOCK;
	Set<GameSessionRef> _sessions;
};