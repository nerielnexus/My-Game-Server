#include "stdafx.h"
#include "CommonDecl.h"
#include "SessionManager.h"
#include "ClientSession.h"

SessionManager* GSessionManager = nullptr;

SessionManager::SessionManager()
	:currentClientCount(0)
{
	CList.clear();
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
	while (currentClientCount - returnClientCount < MAXCONN)
	{
		ClientSession* newClient = CList.back();
		CList.pop_back();

		currentClientCount++;

		if (newClient->ConnectionAccept() == false)
			return false;
	}
	return true;
}

void SessionManager::ReturnClientSession(ClientSession* client)
{
	client->SessionReset();
	CList.push_back(client);
	returnClientCount++;
}