#pragma once

class ClientSession;

// 패킷의 대분류
enum PACKET_TYPE
{
	CONNECT_ACCEPT = 0,  // 첫 연결
	DISCONNECT_USER,   // 유저에 의한 접속종료
	DISCONNECT_FORCED, // 유저가 아닌 접속종료, 보통 서버에서 클라이언트에게 상대 클라이언트의 강제종료를 알릴 때 사용함
	OBJECT_CREATE,     // 오브젝트 생성
	OBJECT_MOVE,       // 오브젝트 이동, 이 패킷은 주기적인 전송을 통해 pulse 역할도 함
	OBJECT_COLLISION,  // 오브젝트 충돌
	OBJECT_DELETE,     // 오브젝트 삭제
	IO_DUMMY           // IO 초기화용 더미 소켓
};

#pragma region packet struct

// 이하의 구조체들을 1바이트 단위로 패킹한다
#pragma pack(push,1)

// 분류가 Connection - Connect 인 패킷의 WSABUF 내용
struct ConnectAccept
{
	unsigned int userid;
	short pattern1;
	short pattern2;
	short pattern3;
};

// 분류가 Connection - Disconnect_User 인 패킷의 WSABUF 내용
struct DisconnectedUser
{
	unsigned int userid;
};

// 분류가 Connection - Disconnect_Forced 인 패킷의 WSABUF 내용
struct DisconnectedForced
{
	unsigned int userid;
};

// 분류가 Object_Modify - Object_Create 인 패킷의 WSABUF 내용
struct ObjectCreate
{
	unsigned int objectid;
	unsigned int userid;
	float xpos;
	float ypos;
};

// 분류가 Object_Modify - Object_Move 인 패킷의 WSABUF 내용
// 이 패킷은 정상적인 연결 확인을 위한 Pulse 의 역할도 맡음
struct ObjectMove
{
	unsigned int objectid;
	float xpos;
	float ypos;
};

// 분류가 Object_Modify - Object_Collision 인 패킷의 WSABUF 내용
struct ObjectCollision
{
	unsigned int mainobjectid;
	unsigned int subobjectid;
	float xpos;
	float ypos;
};

// 분류가 Object_Modify - Object_Delete 인 패킷의 WSABUF 내용
struct ObjectDelete
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

struct ConnectionIOContext : public OverlappedIOContext
{
	ConnectionIOContext(ClientSession* client, PACKET_TYPE type);
};

struct ObjectIOContext : public OverlappedIOContext
{
	ObjectIOContext(ClientSession* client, PACKET_TYPE type);
};

struct DummyIOContext : public OverlappedIOContext
{
	DummyIOContext(ClientSession* client);
};

void DeleteIOContext(OverlappedIOContext* context);

class ClientSession
{
public:
	ClientSession();
	~ClientSession();

	bool AcceptClient();
	void DisconnectClient(PACKET_TYPE type);
	bool DummyRecv();

	bool InitializeRecv();
	bool InitializeDisconUser();
	bool InitializeObjCreate();
	bool InitializeObjMove();
	bool InitiazlieObjColl();
	bool InitializeObjDelete();

	bool ObjectDataRecv(PACKET_TYPE type);
	void ObjectDataParser(char* data, PACKET_TYPE type);
	void AcceptCompletion();
	void ResetClientSession();
	void AddRef();
	void ReleaseRef();

private:
	SOCKET      clientSock;
	SOCKADDR_IN clientAddr;
	volatile long refCount;
	volatile long isConnected;

	friend class SessionManager;
};

