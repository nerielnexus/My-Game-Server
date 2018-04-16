#pragma once

#include "stdafx.h"

#define BUFSIZE 512
#define SERVERPORT 9000
#define MAXCONN 10000
#define SERVERIP "127.0.0.1"

#include "stdafx.h"

class ClientSession;

// ��Ŷ�� ��з�
enum PACKET_TYPE
{
	CONNECT_ACCEPT,    // ù ����
	DISCONNECT_USER,   // ������ ���� ��������
	DISCONNECT_FORCED, // ������ �ƴ� ��������, ���� �������� Ŭ���̾�Ʈ���� ��� Ŭ���̾�Ʈ�� �������Ḧ �˸� �� �����
	OBJECT_CREATE,     // ������Ʈ ����
	OBJECT_MOVE,       // ������Ʈ �̵�, �� ��Ŷ�� �ֱ����� ������ ���� pulse ���ҵ� ��
	OBJECT_COLLISION,  // ������Ʈ �浹
	OBJECT_DELETE      // ������Ʈ ����
};

#pragma region packet struct

// ������ ����ü���� 1����Ʈ ������ ��ŷ�Ѵ�
#pragma pack(push,1)

// �з��� Connection - Connect �� ��Ŷ�� WSABUF ����
struct ConnectAcceptContext
{
	unsigned int userid;
	short pattern1;
	short pattern2;
	short pattern3;
};

// �з��� Connection - Disconnect_User �� ��Ŷ�� WSABUF ����
struct DisconnectedUserContext
{
	unsigned int userid;
};

// �з��� Connection - Disconnect_Forced �� ��Ŷ�� WSABUF ����
struct DisconnectedForcedContext
{
	unsigned int userid;
};

// �з��� Object_Modify - Object_Create �� ��Ŷ�� WSABUF ����
struct ObjectCreateContext
{
	unsigned int objectid;
	unsigned int userid;
	float xpos;
	float ypos;
};

// �з��� Object_Modify - Object_Move �� ��Ŷ�� WSABUF ����
// �� ��Ŷ�� �������� ���� Ȯ���� ���� Pulse �� ���ҵ� ����
struct ObjectMoveContext
{
	unsigned int objectid;
	float xpos;
	float ypos;
};

// �з��� Object_Modify - Object_Collision �� ��Ŷ�� WSABUF ����
struct ObjectCollisionContext
{
	unsigned int mainobjectid;
	unsigned int subobjectid;
	float xpos;
	float ypos;
};

// �з��� Object_Modify - Object_Delete �� ��Ŷ�� WSABUF ����
struct ObjectDeleteContext
{
	unsigned int objectid;
};

#pragma pack(pop)

#pragma endregion

// �⺻���� ��Ŷ�� ����
struct OverlappedIOContext
{
	OverlappedIOContext(ClientSession* owner, PACKET_TYPE ptype);

	OVERLAPPED		ovl;
	ClientSession*	owner;
	PACKET_TYPE		type;
	WSABUF			wsaBuf; // ������ structs ���� ���� �迭�� �ִ� ����ü
};