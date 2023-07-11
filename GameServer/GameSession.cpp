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

int32 GameSession::OnRecvPacket(BYTE* buffer, int32 len)
{
    // Echo
    PacketHeader header = *((PacketHeader*)buffer);

    cout << "Packet ID : " << header.id << ", Size : " << header.size << endl;

    return len;
}

void GameSession::OnSend(int32 len)
{
    // Echo
    //cout << "OnSend Len: " << len << endl;
}