#include "stdafx.h"
#include "ClientSession.h"
#include "IOCPManager.h"
#include "CommonDecl.h"


ClientSession::ClientSession()
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
	DWORD flags = 0;
	acceptContext->wsaBuf.len = 0;
	acceptContext->wsaBuf.buf = nullptr;

	bool acceptexRetVal = GIocpmanager->AcceptEx(GIocpmanager->GetListenSock(), clientSock, GIocpmanager->acceptBuffer,
		0, sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, &bytes, (LPOVERLAPPED)acceptContext);
	if (acceptexRetVal == false)
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
		DeleteIOContext(context);
		std::cout << "[ERROR] ClientSession::Disconnext(PACKET_TYPE type) failed, error code " << GetLastError() << std::endl;
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

	PrintObjectData(context->wsaBuf.buf, context->type);

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

void ClientSession::PrintObjectData(char* data, PACKET_TYPE type)
{

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