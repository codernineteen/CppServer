#pragma once
#include "Session.h"

class GameSession : public Session
{
public:
	~GameSession()
	{
		cout << "game sesssion destructed" << endl;
	}
	
	virtual void OnDisconnected() override;
	virtual void OnConnected() override;
	virtual int32 OnRecv(BYTE* buffer, int32 len) override;
	virtual void OnSend(int32 len) override;
};

