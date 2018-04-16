#include "stdafx.h"
#include "CommonDecl.h"
#include "SessionManager.h"
#include "ClientSession.h"

SessionManager* GSessionManager = nullptr;

SessionManager::SessionManager()
	:currentClientCount(0), returnClientCount(0)
{
	CList.clear();
	InitializeCriticalSection(&cs);
}


SessionManager::~SessionManager()
{
	CList.clear();
}

void SessionManager::InitializeSession()
{
	for (int i = 0; i < MAXCONN; i++)
	{
		ClientSession* client = new ClientSession();
		CList.push_back(client);
	}
}

bool SessionManager::AcceptSession()
{
	EnterCriticalSection(&cs);
	while (currentClientCount - returnClientCount < MAXCONN)
	{
		ClientSession* newClient = CList.back();
		CList.pop_back();

		++currentClientCount;
		newClient->AddRef();

		if (newClient->ConnectionAccept() == false)
			return false;
	}
	LeaveCriticalSection(&cs);
	return true;
}

void SessionManager::ReturnClientSession(ClientSession* client)
{
	EnterCriticalSection(&cs);
	client->SessionReset();
	CList.push_back(client);
	++returnClientCount;
	LeaveCriticalSection(&cs);
}