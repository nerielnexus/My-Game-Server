#pragma once
#include <list>

class ClientSession;

class SessionManager
{
public:
	SessionManager();
	~SessionManager();

	void InitializeSession();
	bool AcceptSession();
	void ReturnClientSession(ClientSession* client);

private:
	typedef std::list<ClientSession*> ClientList;
	ClientList CList;

	uint64_t currentClientCount;
	uint64_t returnClientCount;
};

extern SessionManager* GSessionManager;