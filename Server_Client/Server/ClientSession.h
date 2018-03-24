#pragma once

class ClientSession;

// 패킷의 대분류
enum PACKET_TYPE
{
	CONNECTION,
	OBJECT_MODIFY
};

// 패킷 대분류가 Connection 일 경우, 어떤 연결인지 구분하는 하위 분류
enum CONNECTION_EVENT
{
	CONNECT,
	DISCONNECT_USER,
	DISCONNECT_FORCED
};

// 패킷 대분류가 Object_Modify 일 경우, 어떤 오브젝트 이벤트인지 구분하는 하위 분류
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

struct ConnectionPacket
{
	CONNECTION_EVENT	conEvent;		// 4byte
	unsigned int		userid;			// 4byte
	short				side;			// 2byte, padding 2byte
	short				pattern1;
	short				pattern2;
	short				pattern3;
};										// total size = 4*2 + (2+2)*4 = 24byte

struct ObjectPacket
{
	OBJECT_EVENT	objEvent;			// 4byte
	unsigned int	mainObj;
	unsigned int	subObj;
	float			xpos;				// 4byte
	float			ypos;
};										// total size = 4*5 = 20byte

class ClientSession
{
public:
	ClientSession();
	~ClientSession();
};

