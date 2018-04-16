#pragma once

#include "stdafx.h"

#define BUFSIZE 512
#define SERVERPORT 9000
#define MAXCONN 10000
#define SERVERIP "127.0.0.1"

#include "stdafx.h"

class ClientSession;

// 패킷의 대분류
enum PACKET_TYPE
{
	CONNECT_ACCEPT,    // 첫 연결
	DISCONNECT_USER,   // 유저에 의한 접속종료
	DISCONNECT_FORCED, // 유저가 아닌 접속종료, 보통 서버에서 클라이언트에게 상대 클라이언트의 강제종료를 알릴 때 사용함
	OBJECT_CREATE,     // 오브젝트 생성
	OBJECT_MOVE,       // 오브젝트 이동, 이 패킷은 주기적인 전송을 통해 pulse 역할도 함
	OBJECT_COLLISION,  // 오브젝트 충돌
	OBJECT_DELETE      // 오브젝트 삭제
};

#pragma region packet struct

// 이하의 구조체들을 1바이트 단위로 패킹한다
#pragma pack(push,1)

// 분류가 Connection - Connect 인 패킷의 WSABUF 내용
struct ConnectAcceptContext
{
	unsigned int userid;
	short pattern1;
	short pattern2;
	short pattern3;
};

// 분류가 Connection - Disconnect_User 인 패킷의 WSABUF 내용
struct DisconnectedUserContext
{
	unsigned int userid;
};

// 분류가 Connection - Disconnect_Forced 인 패킷의 WSABUF 내용
struct DisconnectedForcedContext
{
	unsigned int userid;
};

// 분류가 Object_Modify - Object_Create 인 패킷의 WSABUF 내용
struct ObjectCreateContext
{
	unsigned int objectid;
	unsigned int userid;
	float xpos;
	float ypos;
};

// 분류가 Object_Modify - Object_Move 인 패킷의 WSABUF 내용
// 이 패킷은 정상적인 연결 확인을 위한 Pulse 의 역할도 맡음
struct ObjectMoveContext
{
	unsigned int objectid;
	float xpos;
	float ypos;
};

// 분류가 Object_Modify - Object_Collision 인 패킷의 WSABUF 내용
struct ObjectCollisionContext
{
	unsigned int mainobjectid;
	unsigned int subobjectid;
	float xpos;
	float ypos;
};

// 분류가 Object_Modify - Object_Delete 인 패킷의 WSABUF 내용
struct ObjectDeleteContext
{
	unsigned int objectid;
};

#pragma pack(pop)

#pragma endregion

// 기본적인 패킷의 구조
struct OverlappedIOContext
{
	OverlappedIOContext(ClientSession* owner, PACKET_TYPE ptype);

	OVERLAPPED		ovl;
	ClientSession*	owner;
	PACKET_TYPE		type;
	WSABUF			wsaBuf; // 이하의 structs 들을 담을 배열이 있는 구조체
};