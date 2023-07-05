#include "pch.h"
#include "GameSession.h"
#include "GameSessionManager.h"


void GameSession::OnDisconnected()
{
    GameSessionManager::GetInstance().Remove(std::static_pointer_cast<GameSession>(shared_from_this()));
}

void GameSession::OnConnected()
{
    GameSessionManager::GetInstance().Add(std::static_pointer_cast<GameSession>(shared_from_this()));
}

int32 GameSession::OnRecv(BYTE* buffer, int32 len)
{
    // Echo
    cout << "OnRecv Len: " << len << endl;

    SendBufferRef sendBuffer = std::make_shared<SendBuffer>(4096);
    sendBuffer->CopyData(buffer, len);

    GameSessionManager::GetInstance().Broadcast(sendBuffer);

    return len;
}

void GameSession::OnSend(int32 len)
{
    // Echo
    cout << "OnSend Len: " << len << endl;
}