#pragma once
#include "NetworkAddress.h"
#include "Listener.h"
#include "IocpCore.h"
#include <functional>

enum class ServiceType : uint8
{
	Server,
	Client
};


/**
	Service 
*/

//세션을 생성하는 함수를 정의
using SessionFactory = function<SessionRef(void)>;

class Service : public enable_shared_from_this<Service>
{
public:
	Service(ServiceType type, NetworkAddress address, IocpCoreRef core, SessionFactory factory, int32 maxSessionCount = 1);
	virtual ~Service();

	virtual bool Start() abstract;
	bool CanStart() { return _sessionFactory != nullptr; }

	virtual void CloseService();
	void SetSessionFactory(SessionFactory func) { _sessionFactory = func; }

	SessionRef CreateSession(); //session을 생성하면서 iocp core에 등록
	void AddSession(SessionRef session);
	void ReleaseSession(SessionRef session);
	int32 GetCurrentSessionCount() { return _sessionCount; }
	int32 GetMaxSessionCount() { return _maxSessionCount; }

public:
	ServiceType GetServicetype() { return _type; }
	NetworkAddress GetNetAddress() { return _netAddr; }
	IocpCoreRef& GetIocpCore() { return _iocpCore; }



protected:
	USE_LOCK;

	ServiceType _type;
	NetworkAddress _netAddr = {};
	IocpCoreRef _iocpCore;

	Set<SessionRef> _sessions;
	int32 _sessionCount = 0;
	int32 _maxSessionCount = 0;
	SessionFactory _sessionFactory;
};

/**
	Client Service
*/

class ClientService : public Service
{
public:
	ClientService(NetworkAddress targetAddr, IocpCoreRef core, SessionFactory sessionFactory, int32 maxSessionCount = 1);
	virtual ~ClientService() {};

	virtual bool Start() override;
};

/**
	Server Service
*/

class ServerService : public Service
{
public:
	ServerService(NetworkAddress addr, IocpCoreRef core, SessionFactory sessionFactory, int32 maxSessionCount = 1);
	virtual ~ServerService() {};

	virtual bool Start() override;
	virtual void CloseService() override;

private:
	ListenerRef _listener = nullptr;
};
