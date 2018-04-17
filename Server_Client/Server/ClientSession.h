#pragma once

class ClientSession;

// ��Ŷ�� ��з�
enum PACKET_TYPE
{
	CONNECT_ACCEPT = 0,  // ù ����
	DISCONNECT_USER,   // ������ ���� ��������
	DISCONNECT_FORCED, // ������ �ƴ� ��������, ���� �������� Ŭ���̾�Ʈ���� ��� Ŭ���̾�Ʈ�� �������Ḧ �˸� �� �����
	OBJECT_CREATE,     // ������Ʈ ����
	OBJECT_MOVE,       // ������Ʈ �̵�, �� ��Ŷ�� �ֱ����� ������ ���� pulse ���ҵ� ��
	OBJECT_COLLISION,  // ������Ʈ �浹
	OBJECT_DELETE,     // ������Ʈ ����
	IO_DUMMY           // IO �ʱ�ȭ�� ���� ����
};

#pragma region packet struct

// ������ ����ü���� 1����Ʈ ������ ��ŷ�Ѵ�
#pragma pack(push,1)

// �з��� Connection - Connect �� ��Ŷ�� WSABUF ����
struct ConnectAccept
{
	unsigned int userid;
	short pattern1;
	short pattern2;
	short pattern3;
};

// �з��� Connection - Disconnect_User �� ��Ŷ�� WSABUF ����
struct DisconnectedUser
{
	unsigned int userid;
};

// �з��� Connection - Disconnect_Forced �� ��Ŷ�� WSABUF ����
struct DisconnectedForced
{
	unsigned int userid;
};

// �з��� Object_Modify - Object_Create �� ��Ŷ�� WSABUF ����
struct ObjectCreate
{
	unsigned int objectid;
	unsigned int userid;
	float xpos;
	float ypos;
};

// �з��� Object_Modify - Object_Move �� ��Ŷ�� WSABUF ����
// �� ��Ŷ�� �������� ���� Ȯ���� ���� Pulse �� ���ҵ� ����
struct ObjectMove
{
	unsigned int objectid;
	float xpos;
	float ypos;
};

// �з��� Object_Modify - Object_Collision �� ��Ŷ�� WSABUF ����
struct ObjectCollision
{
	unsigned int mainobjectid;
	unsigned int subobjectid;
	float xpos;
	float ypos;
};

// �з��� Object_Modify - Object_Delete �� ��Ŷ�� WSABUF ����
struct ObjectDelete
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

