#pragma once

class ClientSession;

// ��Ŷ�� ��з�
enum PACKET_TYPE
{
	CONNECTION,
	OBJECT_MODIFY
};

// ��Ŷ ��з��� Connection �� ���, � �������� �����ϴ� ���� �з�
enum CONNECTION_EVENT
{
	CONNECT,
	DISCONNECT_USER,
	DISCONNECT_FORCED
};

// ��Ŷ ��з��� Object_Modify �� ���, � ������Ʈ �̺�Ʈ���� �����ϴ� ���� �з�
enum OBJECT_EVENT
{
	OBJECT_CREATE,
	OBJECT_MOVE,
	OBJECT_COLLISION,
	OBJECT_DELETE
};

struct OverlappedIOContext
{
	OverlappedIOContext(ClientSession* owner, PACKET_TYPE ptype)
		:owner(owner), type(ptype)
	{ }

	ClientSession*	owner;
	OVERLAPPED		ovl;
	PACKET_TYPE		type;
	WSABUF			wsaBuf;
};

class ClientSession
{
public:
	ClientSession();
	~ClientSession();
};

