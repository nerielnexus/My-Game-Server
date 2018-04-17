#include "stdafx.h"
#include "ClientSession.h"
#include "IOCPManager.h"
#include "SessionManager.h"
#include "CommonDecl.h"

OverlappedIOContext::OverlappedIOContext(ClientSession* owner, PACKET_TYPE ptype)
	:owner(owner), type(ptype)
{
	ZeroMemory(&ovl, sizeof(OVERLAPPED));
	ZeroMemory(&wsaBuf, sizeof(WSABUF));
	owner->AddRef();
}

ConnectionIOContext::ConnectionIOContext(ClientSession* client, PACKET_TYPE type)
	:OverlappedIOContext(client, type)
{

}

ObjectIOContext::ObjectIOContext(ClientSession* client, PACKET_TYPE type)
	: OverlappedIOContext(client, type)
{

}

DummyIOContext::DummyIOContext(ClientSession* client)
	: OverlappedIOContext(client, PACKET_TYPE::IO_DUMMY)
{

}

void DeleteIOContext(OverlappedIOContext* context)
{
	if (context == nullptr)
		return;

	//todo 만약 ReleaseRef 같은게 있다면 처리하자
	(context->owner)->ReleaseRef();

	switch (context->type)
	{
	case PACKET_TYPE::CONNECT_ACCEPT:
	case PACKET_TYPE::DISCONNECT_USER:
	case PACKET_TYPE::DISCONNECT_FORCED:
	case PACKET_TYPE::OBJECT_CREATE:
	case PACKET_TYPE::OBJECT_MOVE:
	case PACKET_TYPE::OBJECT_COLLISION:
	case PACKET_TYPE::OBJECT_DELETE:
		delete context;
		break;

	default:
		//todo 이상한 패킷 타입의 경우 에러 처리
		break;
	}
}

ClientSession::ClientSession()
	:refCount(0),isConnected(0)
{
	ZeroMemory(&clientAddr, sizeof(SOCKADDR_IN));
	clientSock = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
}

ClientSession::~ClientSession()
{

}

bool ClientSession::AcceptClient()
{
	ConnectionIOContext* acceptContext = new ConnectionIOContext(this, PACKET_TYPE::CONNECT_ACCEPT);
	DWORD bytes = 0;
	acceptContext->wsaBuf.len = 0;
	acceptContext->wsaBuf.buf = nullptr;

	if (FALSE == GIocpmanager->AcceptEx(
		*(GIocpmanager->GetListenSock()),
		clientSock,
		GIocpmanager->acceptBuffer,
		0,
		sizeof(SOCKADDR_IN) + 16,
		sizeof(SOCKADDR_IN) + 16,
		&bytes,
		(LPOVERLAPPED)acceptContext))
	{
		if (WSAGetLastError() != WSA_IO_PENDING)
		{
			DeleteIOContext(acceptContext);
			std::cout << "[ERROR] AcceptEx failed, "; ErrorMsg(GetLastError()); std::cout << " (" << GetLastError() << ")" << std::endl;
			return false;
		}
	}

	return true;
}

void ClientSession::DisconnectClient(PACKET_TYPE type)
{
	OverlappedIOContext* context = new OverlappedIOContext(this, type);
	bool retval = GIocpmanager->DisconnectEx(clientSock, (LPWSAOVERLAPPED)context, TF_REUSE_SOCKET);
	if (retval == false)
	{
		if (WSAGetLastError() != WSA_IO_PENDING)
		{
			DeleteIOContext(context);
			std::cout << "[ERROR] ClientSession::Disconnext(PACKET_TYPE type) failed, "; ErrorMsg(GetLastError()); std::cout << " (" << GetLastError() << ")" << std::endl;

		}
	}
}

bool ClientSession::DummyRecv()
{
	DummyIOContext* init = new DummyIOContext(this);
	init->wsaBuf.buf = nullptr;
	init->wsaBuf.len = 0;

	DWORD dwBytes = 0;
	DWORD dwFlags = 0;

	if (SOCKET_ERROR == WSARecv(clientSock, &init->wsaBuf, 1, &dwBytes, (LPDWORD)&dwFlags, (LPWSAOVERLAPPED)init, NULL))
	{
		if (WSAGetLastError() != WSA_IO_PENDING)
		{
			std::cout << "[ERROR] DummyRecv failed, "; ErrorMsg(GetLastError()); std::cout << " (" << GetLastError() << ")" << std::endl;
			return false;
		}
	}

	return true;
}

bool ClientSession::InitializeRecv()
{
	//? 여기에 모든 패킷 타입에 대한 WSARecv 를 돌려야해?
	//? DisconnectedForced 는 강종이니까 받을 이유가 없겠지?
	//? 그렇다면 5개의 WSARecv 를 해야한다?
	//? 이건 미친짓이야

	if (!InitializeDisconUser())
		return false;

	if (!InitializeObjCreate())
		return false;

	if (!InitializeObjMove())
		return false;

	if (!InitiazlieObjColl())
		return false;

	if (!InitializeObjDelete())
		return false;

	return true;
}

bool ClientSession::InitializeDisconUser()
{
	DWORD dwBytes = 0;
	DWORD dwFlags = 0;

	ConnectionIOContext* conn = new ConnectionIOContext(this, PACKET_TYPE::DISCONNECT_USER);
	conn->wsaBuf.len = sizeof(DisconnectedUser);
	conn->wsaBuf.buf = new char[sizeof(DisconnectedUser)];
	if (SOCKET_ERROR == WSARecv(clientSock, &conn->wsaBuf, 1, &dwBytes, (LPDWORD)&dwFlags, (LPWSAOVERLAPPED)conn, NULL))
	{
		if (WSAGetLastError() != WSA_IO_PENDING)
		{
			std::cout << "[ERROR] InitializeDisconUser failed, "; ErrorMsg(GetLastError()); std::cout << " (" << GetLastError() << ")" << std::endl;
			return false;
		}
	}

	return true;
}

bool ClientSession::InitializeObjCreate()
{
	DWORD dwBytes = 0;
	DWORD dwFlags = 0;

	ObjectIOContext* createObj = new ObjectIOContext(this, PACKET_TYPE::OBJECT_CREATE);
	createObj->wsaBuf.len = sizeof(DisconnectedUser);
	createObj->wsaBuf.buf = new char[sizeof(DisconnectedUser)];
	if (SOCKET_ERROR == WSARecv(clientSock, &createObj->wsaBuf, 1, &dwBytes, (LPDWORD)&dwFlags, (LPWSAOVERLAPPED)createObj, NULL))
	{
		if (WSAGetLastError() != WSA_IO_PENDING)
		{
			std::cout << "[ERROR] InitializeRecv failed, "; ErrorMsg(GetLastError()); std::cout << " (" << GetLastError() << ")" << std::endl;
			return false;
		}
	}
	return true;
}

bool ClientSession::InitializeObjMove()
{
	DWORD dwBytes = 0;
	DWORD dwFlags = 0;

	ObjectIOContext* moveObj = new ObjectIOContext(this, PACKET_TYPE::OBJECT_MOVE);
	moveObj->wsaBuf.len = sizeof(DisconnectedUser);
	moveObj->wsaBuf.buf = new char[sizeof(DisconnectedUser)];
	if (SOCKET_ERROR == WSARecv(clientSock, &moveObj->wsaBuf, 1, &dwBytes, (LPDWORD)&dwFlags, (LPWSAOVERLAPPED)moveObj, NULL))
	{
		if (WSAGetLastError() != WSA_IO_PENDING)
		{
			std::cout << "[ERROR] InitializeRecv failed, "; ErrorMsg(GetLastError()); std::cout << " (" << GetLastError() << ")" << std::endl;
			return false;
		}
	}

	return true;
}

bool ClientSession::InitiazlieObjColl()
{
	DWORD dwBytes = 0;
	DWORD dwFlags = 0;

	ObjectIOContext* collisionObj = new ObjectIOContext(this, PACKET_TYPE::OBJECT_COLLISION);
	collisionObj->wsaBuf.len = sizeof(DisconnectedUser);
	collisionObj->wsaBuf.buf = new char[sizeof(DisconnectedUser)];
	if (SOCKET_ERROR == WSARecv(clientSock, &collisionObj->wsaBuf, 1, &dwBytes, (LPDWORD)&dwFlags, (LPWSAOVERLAPPED)collisionObj, NULL))
	{
		if (WSAGetLastError() != WSA_IO_PENDING)
		{
			std::cout << "[ERROR] InitializeRecv failed, "; ErrorMsg(GetLastError()); std::cout << " (" << GetLastError() << ")" << std::endl;
			return false;
		}
	}

	return true;
}

bool ClientSession::InitializeObjDelete()
{
	DWORD dwBytes = 0;
	DWORD dwFlags = 0;

	ObjectIOContext* deleteObj = new ObjectIOContext(this, PACKET_TYPE::OBJECT_DELETE);
	deleteObj->wsaBuf.len = sizeof(DisconnectedUser);
	deleteObj->wsaBuf.buf = new char[sizeof(DisconnectedUser)];
	if (SOCKET_ERROR == WSARecv(clientSock, &deleteObj->wsaBuf, 1, &dwBytes, (LPDWORD)&dwFlags, (LPWSAOVERLAPPED)deleteObj, NULL))
	{
		if (WSAGetLastError() != WSA_IO_PENDING)
		{
			std::cout << "[ERROR] InitializeRecv failed, "; ErrorMsg(GetLastError()); std::cout << " (" << GetLastError() << ")" << std::endl;
			return false;
		}
	}

	return true;
}

bool ClientSession::ObjectDataRecv(PACKET_TYPE type)
{
	ObjectIOContext* context = new ObjectIOContext(this, type);
	DWORD recvBytes = 0;
	DWORD flags = 0;

	switch (type)
	{
	case PACKET_TYPE::OBJECT_CREATE:
		context->wsaBuf.len = sizeof(ObjectCreate);
		break;
	case PACKET_TYPE::OBJECT_MOVE:
		context->wsaBuf.len = sizeof(ObjectMove);
		break;
	case PACKET_TYPE::OBJECT_COLLISION:
		context->wsaBuf.len = sizeof(ObjectCollision);
		break;
	case PACKET_TYPE::OBJECT_DELETE:
		context->wsaBuf.len = sizeof(ObjectDelete);
		break;
	}

	context->wsaBuf.buf = new char[context->wsaBuf.len];

	int retval = WSARecv(clientSock, &context->wsaBuf, 1, &recvBytes, &flags, (LPWSAOVERLAPPED)context, NULL);
	if (retval == SOCKET_ERROR)
	{
		if (WSAGetLastError() != WSA_IO_PENDING)
		{
			DeleteIOContext(context);
			std::cout << "[ERROR] ClientSession::ObjectCommunicationCompletion failed, "; ErrorMsg(GetLastError()); std::cout << " (" << GetLastError() << ")" << std::endl;
			return false;
		}
	}

	//std::cout << "[RECV] IP:" << inet_ntoa(clientAddr.sin_addr) << ", Port:" << ntohs(clientAddr.sin_port) << std::endl;
	ObjectDataParser(context->wsaBuf.buf, context->type);
	delete context;

	return true;
}

void ClientSession::AcceptCompletion()
{
	bool result = true;
	int retval = 0;
	int opt = 1;

	do
	{
		retval = setsockopt(clientSock, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, (char*)GIocpmanager->GetListenSock(), sizeof(SOCKET));
		if (retval == SOCKET_ERROR)
		{
			std::cout << "[ERROR] SO_UPDATE_ACCEPT_CONTEXT failed, "; ErrorMsg(GetLastError()); std::cout << " (" << GetLastError() << ")" << std::endl;
			result = false;
			break;
		}

		retval = setsockopt(clientSock, IPPROTO_TCP, TCP_NODELAY, (const char*)&opt, sizeof(int));
		if (retval == SOCKET_ERROR)
		{
			std::cout << "[ERROR] TCP_NODELAY failed, "; ErrorMsg(GetLastError()); std::cout << " (" << GetLastError() << ")" << std::endl;
			result = false;
			break;
		}

		opt = 0;
		retval = setsockopt(clientSock, SOL_SOCKET, SO_RCVBUF, (const char*)&opt, sizeof(int));
		if (retval == SOCKET_ERROR)
		{
			std::cout << "[ERROR] SO_RCVBUF failed, "; ErrorMsg(GetLastError()); std::cout << " (" << GetLastError() << ")" << std::endl;
			result = false;
			break;
		}

		int addrlen = sizeof(SOCKADDR_IN);
		retval = getpeername(clientSock, (SOCKADDR*)&clientAddr, &addrlen);
		if(retval == SOCKET_ERROR)
		{
			std::cout << "[ERROR] getpeername failed, "; ErrorMsg(GetLastError()); std::cout << " (" << GetLastError() << ")" << std::endl;
			result = false;
			break;
		}

		HANDLE hClientIOCP = CreateIoCompletionPort((HANDLE)clientSock, GIocpmanager->GetIOCPHandle(), (ULONG_PTR)this, 0);
		if (hClientIOCP != GIocpmanager->GetIOCPHandle())
		{
			std::cout << "[ERROR] CreateIoCompletionPort failed, "; ErrorMsg(GetLastError()); std::cout << " (" << GetLastError() << ")" << std::endl;
			result = false;
			break;
		}
	} while (false);

	if (!result)
	{
		DisconnectClient(PACKET_TYPE::DISCONNECT_FORCED);
		return;
	}

	if (DummyRecv() == false)
	{
		return;
	}

	//std::cout << "[CONSOLE] Client connected, IP:" << inet_ntoa(clientAddr.sin_addr) << ", Port:" << ntohs(clientAddr.sin_port) << std::endl;

}

// 180416 변경, PrintObjectData -> ObjectDataParser
// context 전송 확인을 위한 테스트 함수
void ClientSession::ObjectDataParser(char* data, PACKET_TYPE type)
{
	if (type == PACKET_TYPE::DISCONNECT_USER)
	{
		DisconnectedUser* duc = new DisconnectedUser();
		CopyMemory(duc, data, sizeof(DisconnectedUser));
		std::cout << "[PARSING] DisconnectedUser" << std::endl;
		std::cout << "[PARSING] userid : " << duc->userid << std::endl;
		delete duc;
		return;
	}

	if (type == PACKET_TYPE::DISCONNECT_FORCED)
	{
		DisconnectedForced* dfc = new DisconnectedForced();
		CopyMemory(dfc, data, sizeof(DisconnectedForced));
		std::cout << "[PARSING] DisconnectedForced" << std::endl;
		std::cout << "[PARSING] userid : " << dfc->userid << std::endl;
		delete dfc;
		return;
	}

	if (type == PACKET_TYPE::OBJECT_CREATE)
	{
		ObjectCreate* occ = new ObjectCreate();
		CopyMemory(occ, data, sizeof(ObjectCreate));
		std::cout << "[PARSING] ObjectCreate" << std::endl;
		std::cout << "[PARSING] objectid : " << occ->objectid << std::endl;
		std::cout << "[PARSING] userid : " << occ->userid << std::endl;
		std::cout << "[PARSING] xpos : " << occ->xpos << std::endl;
		std::cout << "[PARSING] ypos : " << occ->ypos << std::endl;
		delete occ;
		return;
	}

	if (type == PACKET_TYPE::OBJECT_COLLISION)
	{
		ObjectCollision* occ = new ObjectCollision();
		CopyMemory(occ, data, sizeof(ObjectCollision));
		std::cout << "[PARSING] ObjectCollision" << std::endl;
		std::cout << "[PARSING] main objectid : " << occ->mainobjectid << std::endl;
		std::cout << "[PARSING] sub objectid : " << occ->subobjectid << std::endl;
		std::cout << "[PARSING] xpos : " << occ->xpos << std::endl;
		std::cout << "[PARSING] ypos : " << occ->ypos << std::endl;
		delete occ;
		return;
	}

	if (type == PACKET_TYPE::OBJECT_DELETE)
	{
		ObjectDelete* odc = new ObjectDelete();
		CopyMemory(odc, data, sizeof(ObjectDelete));
		std::cout << "[PARSING] ObjectDelete" << std::endl;
		std::cout << "[PARSING] objectid : " << odc->objectid << std::endl;
		delete odc;
		return;
	}

	std::cout << "[PARSING] ObjectDataParser failed, not a Object-Type Packet" << std::endl;
	return;
}

void ClientSession::ResetClientSession()
{
	ZeroMemory(&clientAddr, sizeof(SOCKADDR_IN));

	LINGER lingerOpt;
	lingerOpt.l_linger = 0;
	lingerOpt.l_onoff = 1;

	int retval = setsockopt(clientSock, SOL_SOCKET, SO_LINGER, (char*)&lingerOpt, sizeof(LINGER));
	if (retval == SOCKET_ERROR)
	{
		std::cout << "[ERROR] setsockopt linger option error, "; ErrorMsg(GetLastError()); std::cout << " (" << GetLastError() << ")" << std::endl;
	}
	closesocket(clientSock);
	clientSock = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
}

void ClientSession::AddRef()
{
	if (InterlockedIncrement(&refCount) <= 0)
	{
		std::cout << "[ERROR] AddRef failed" << std::endl;
		return;
	}
}

void ClientSession::ReleaseRef()
{
	long retval = InterlockedDecrement(&refCount);
	if (retval <= 0)
	{
		std::cout << "[ERROR] ReleaseRef failed" << std::endl;
		return;
	}

	if (retval == 0)
	{
		std::cout << "[CONSOLE] Client returned" << std::endl;
		GSessionManager->ReturnClientSession(this);
	}
}