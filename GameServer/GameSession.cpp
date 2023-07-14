#include "pch.h"
#include "GameSession.h"
#include "GameSessionManager.h"
#include "ServerPacketHandler.h"


void GameSession::OnDisconnected()
{
    GameSessionManager::GetInstance().Remove(std::static_pointer_cast<GameSession>(shared_from_this()));
}

void GameSession::OnConnected()
{
    GameSessionManager::GetInstance().Add(std::static_pointer_cast<GameSession>(shared_from_this()));
}

void GameSession::OnRecvPacket(BYTE* buffer, int32 len)
{
    // Echo
    ServerPacketHandler::HandlePacket(buffer, len);
}

void GameSession::OnSend(int32 len)
{
    // Echo
    //cout << "OnSend Len: " << len << endl;
}