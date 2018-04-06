#pragma once

enum PACKET_TYPE;

class ClientSession
{
public:
	ClientSession();
	~ClientSession();

	bool ConnectionAccept();
	void Disconnect(PACKET_TYPE type);
	bool ObjectCommunicateCompletion(PACKET_TYPE type);
	void PrintObjectData(char* data, PACKET_TYPE type);
	void AcceptCompletion();
	void SessionReset();

private:
	SOCKET      clientSock;
	SOCKADDR_IN clientAddr;

	friend class SessionManager;
};

