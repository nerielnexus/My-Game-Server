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
	void ObjectDataParser(char* data, PACKET_TYPE type);
	void AcceptCompletion();
	void SessionReset();
	void AddRef();
	void ReleaseRef();

private:
	SOCKET      clientSock;
	SOCKADDR_IN clientAddr;
	volatile long refCount;
	volatile long isConnected;

	friend class SessionManager;
};

