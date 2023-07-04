#include "pch.h"
#include "Service.h"
#include "Listener.h"
#include "Session.h"

Service::Service(ServiceType type, NetworkAddress address, IocpCoreRef core, SessionFactory factory, int32 maxSessionCount)
	:_type(type), _netAddr(address), _iocpCore(core), _sessionFactory(factory), _maxSessionCount(maxSessionCount)
{

}

Service::~Service()
{

}

void Service::CloseService()
{
	//TODO
}

SessionRef Service::CreateSession()
{
	SessionRef session = _sessionFactory();
	//서비스와 session을 연동
	session->SetServiceRef(shared_from_this());
	if (_iocpCore->Register(session) == false)
		return nullptr;

	return session;
}

void Service::AddSession(SessionRef session)
{
	WRITE_LOCK;
	_sessionCount++;
	_sessions.insert(session);
}

void Service::ReleaseSession(SessionRef session)
{
	WRITE_LOCK;
	ASSERT_CRASH(_sessions.erase(session) != 0);
	_sessionCount--;
}


/**
	Client Service
*/


ClientService::ClientService(NetworkAddress targetAddr, IocpCoreRef core, SessionFactory factory, int32 maxSessionCount)
	:Service(ServiceType::Client, targetAddr, core, factory, maxSessionCount)
{

}

bool ClientService::Start()
{
	if (CanStart() == false)
		return false;

	const int32 maxSessionCount = GetMaxSessionCount();
	for (int32 i = 0; i < maxSessionCount; i++)
	{
		SessionRef session = CreateSession();
		if (session->Connect() == false)
			return false;
	}

	return true;
}


/**
	Server Service
*/

ServerService::ServerService(NetworkAddress sAddr, IocpCoreRef core, SessionFactory factory, int32 maxSessionCount)
	:Service(ServiceType::Server, sAddr, core, factory, maxSessionCount)
{
}

bool ServerService::Start()
{
	if (CanStart() == false)
		return false;

	_listener = MakeShared<Listener>();
	if (_listener == nullptr)
		return false;

	ServerServiceRef service = static_pointer_cast<ServerService>(shared_from_this());
	if (_listener->StartAccept(service) == false)
		return false;

	return true;
}

void ServerService::CloseService()
{
	Service::CloseService();
}
