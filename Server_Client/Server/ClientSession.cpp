#include "stdafx.h"
#include "ClientSession.h"
#include "IOCPManager.h"
#include "SessionManager.h"
#include "CommonDecl.h"


ClientSession::ClientSession()
	:refCount(0),isConnected(0)
{
	ZeroMemory(&clientAddr, sizeof(SOCKADDR_IN));
	clientSock = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
}


ClientSession::~ClientSession()
{

}

bool ClientSession::ConnectionAccept()
{
	OverlappedIOContext* acceptContext = new OverlappedIOContext(this, PACKET_TYPE::CONNECT_ACCEPT);
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
			std::cout << "[ERROR] AcceptEx failed, error code " << GetLastError() << std::endl;
			return false;
		}
	}

	return true;
}

void ClientSession::Disconnect(PACKET_TYPE type)
{
	OverlappedIOContext* context = new OverlappedIOContext(this, type);
	bool retval = GIocpmanager->DisconnectEx(clientSock, (LPWSAOVERLAPPED)context, TF_REUSE_SOCKET);
	if (retval == false)
	{
		if (WSAGetLastError() != WSA_IO_PENDING)
		{
			DeleteIOContext(context);
			std::cout << "[ERROR] ClientSession::Disconnext(PACKET_TYPE type) failed, error code " << GetLastError() << std::endl;

		}
	}
}

bool ClientSession::ObjectCommunicateCompletion(PACKET_TYPE type)
{
	OverlappedIOContext* context = new OverlappedIOContext(this, type);
	DWORD recvBytes = 0;
	DWORD flags = 0;
	context->wsaBuf.len = BUFSIZE;
	context->wsaBuf.buf = new char[BUFSIZE];

	int retval = WSARecv(clientSock, &context->wsaBuf, 1, &recvBytes, &flags, (LPWSAOVERLAPPED)context, NULL);
	if (retval == SOCKET_ERROR)
	{
		if (WSAGetLastError() != WSA_IO_PENDING)
		{
			DeleteIOContext(context);
			std::cout << "[ERROR] ClientSession::ObjectCommunicationCompletion failed, error code " << GetLastError() << std::endl;
			return false;
		}
	}

	ObjectDataParser(context->wsaBuf.buf, context->type);

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
			std::cout << "[ERROR] SO_UPDATE_ACCEPT_CONTEXT failed, error code " << GetLastError() << std::endl;
			result = false;
			break;
		}

		retval = setsockopt(clientSock, IPPROTO_TCP, TCP_NODELAY, (const char*)&opt, sizeof(int));
		if (retval == SOCKET_ERROR)
		{
			std::cout << "[ERROR] TCP_NODELAY failed, error code " << GetLastError() << std::endl;
			result = false;
			break;
		}

		opt = 0;
		retval = setsockopt(clientSock, SOL_SOCKET, SO_RCVBUF, (const char*)&opt, sizeof(int));
		if (retval == SOCKET_ERROR)
		{
			std::cout << "[ERROR] SO_RCVBUF failed, error code " << GetLastError() << std::endl;
			result = false;
			break;
		}

		int addrlen = sizeof(SOCKADDR_IN);
		retval = getpeername(clientSock, (SOCKADDR*)&clientAddr, &addrlen);
		if(retval == SOCKET_ERROR)
		{
			std::cout << "[ERROR] getpeername failed, error code " << GetLastError() << std::endl;
			result = false;
			break;
		}

		HANDLE hClientIOCP = CreateIoCompletionPort((HANDLE)clientSock, GIocpmanager->GetIOCPHandle(), (ULONG_PTR)this, 0);
		if (hClientIOCP != GIocpmanager->GetIOCPHandle())
		{
			std::cout << "[ERROR] CreateIoCompletionPort failed, error code " << GetLastError() << std::endl;
			result = false;
			break;
		}
	} while (false);

	if (!result)
	{
		Disconnect(PACKET_TYPE::DISCONNECT_FORCED);
		return;
	}

	std::cout << "[CONSOLE] Client connected, IP:" << inet_ntoa(clientAddr.sin_addr) << ", Port:" << ntohs(clientAddr.sin_port) << std::endl;

}

// 180416 변경, PrintObjectData -> ObjectDataParser
// context 전송 확인을 위한 테스트 함수
void ClientSession::ObjectDataParser(char* data, PACKET_TYPE type)
{
	if (type == PACKET_TYPE::DISCONNECT_USER)
	{
		DisconnectedUserContext* duc = new DisconnectedUserContext();
		std::cout << "[PARSING] DisconnectedUserContext" << std::endl;
		std::cout << "[PARSING] userid : " << duc->userid << std::endl;
		delete duc;
		return;
	}

	if (type == PACKET_TYPE::DISCONNECT_FORCED)
	{
		DisconnectedForcedContext* dfc = new DisconnectedForcedContext();
		std::cout << "[PARSING] DisconnectedForcedContext" << std::endl;
		std::cout << "[PARSING] userid : " << dfc->userid << std::endl;
		delete dfc;
		return;
	}

	if (type == PACKET_TYPE::OBJECT_CREATE)
	{
		ObjectCreateContext* occ = new ObjectCreateContext();
		std::cout << "[PARSING] ObjectCreateContext" << std::endl;
		std::cout << "[PARSING] objectid : " << occ->objectid << std::endl;
		std::cout << "[PARSING] userid : " << occ->userid << std::endl;
		std::cout << "[PARSING] xpos : " << occ->xpos << std::endl;
		std::cout << "[PARSING] ypos : " << occ->ypos << std::endl;
		delete occ;
		return;
	}

	if (type == PACKET_TYPE::OBJECT_COLLISION)
	{
		ObjectCollisionContext* occ = new ObjectCollisionContext();
		std::cout << "[PARSING] ObjectCollisionContext" << std::endl;
		std::cout << "[PARSING] main objectid : " << occ->mainobjectid << std::endl;
		std::cout << "[PARSING] sub objectid : " << occ->subobjectid << std::endl;
		std::cout << "[PARSING] xpos : " << occ->xpos << std::endl;
		std::cout << "[PARSING] ypos : " << occ->ypos << std::endl;
		delete occ;
		return;
	}

	if (type == PACKET_TYPE::OBJECT_DELETE)
	{
		ObjectDeleteContext* odc = new ObjectDeleteContext();
		std::cout << "[PARSING] ObjectDeleteContext" << std::endl;
		std::cout << "[PARSING] objectid : " << odc->objectid << std::endl;
		delete odc;
		return;
	}

	std::cout << "[PARSING] ObjectDataParser failed, not a Object-Type Packet" << std::endl;
	return;
}

void ClientSession::SessionReset()
{
	ZeroMemory(&clientAddr, sizeof(SOCKADDR_IN));

	LINGER lingerOpt;
	lingerOpt.l_linger = 0;
	lingerOpt.l_onoff = 1;

	int retval = setsockopt(clientSock, SOL_SOCKET, SO_LINGER, (char*)&lingerOpt, sizeof(LINGER));
	if (retval == SOCKET_ERROR)
	{
		std::cout << "[ERROR] setsockopt linger option error, error code " << GetLastError() << std::endl;
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