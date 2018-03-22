#include "stdafx.h"
#include "IOCPManager.h"
#include "ClientSession.h"

IOCPManager* GIocpmanager = nullptr;

IOCPManager::IOCPManager()
{
	listenSock			= NULL;
	hIocpHandle			= nullptr;
	iocpThreadCount		= 0;
	lpfnAcceptEx		= nullptr;
	lpfnDisconnectEx	= nullptr;
}


IOCPManager::~IOCPManager()
{
}

BOOL IOCPManager::Initialize()
{
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		std::cout << "[ERROR] WSAStartup failed" << std::endl;
		return false;
	}

	listenSock = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (listenSock == INVALID_SOCKET)
	{
		std::cout << "[ERROR] WSASocket failed" << std::endl;
		return false;
	}

	int opt = 1;
	int retval = setsockopt(listenSock, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(int));
	if (retval != 0)
	{
		std::cout << "[ERROR] setsockopt failed, " << WSAGetLastError() << std::endl;
		return false;
	}

	SOCKADDR_IN serverAddr;
	ZeroMemory(&serverAddr, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(LISTENPORT);
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	retval = bind(listenSock, (SOCKADDR*)&serverAddr, sizeof(serverAddr));
	if (retval != 0)
	{
		std::cout << "[ERROR] bind failed, " << WSAGetLastError() << std::endl;
		return false;
	}

	SYSTEM_INFO sysinfo;
	GetSystemInfo(&sysinfo);
	iocpThreadCount = sysinfo.dwNumberOfProcessors * 2;

	hIocpHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if (hIocpHandle == nullptr)
	{
		std::cout << "[ERROR] CreateIoCompletionPort failed" << std::endl;
		return false;
	}

	HANDLE connectionHandle = CreateIoCompletionPort((HANDLE)listenSock, hIocpHandle, 0, 0);
	if (connectionHandle == nullptr)
	{
		std::cout << "[ERROR] CreateCompletionPort (create connection between IOCP & listenSock) failed" << std::endl;
		return false;
	}

	for (int i = 0; i < iocpThreadCount; i++)
	{
		DWORD dwThreadId;
		HANDLE hThread = (HANDLE)_beginthreadex(NULL,
												0,
												IocpThread,
												(LPVOID)(i + 1),
												0,
												(unsigned int*)&dwThreadId
												);

		if (hThread == NULL)
		{
			std::cout << "[ERROR] _beginthreadex failed" << std::endl;
			return false;
		}
	}

	GUID guidAcceptEx = WSAID_ACCEPTEX;
	GUID guidDisconnectEx = WSAID_DISCONNECTEX;
	DWORD bytes = 0;
	if (SOCKET_ERROR == WSAIoctl(listenSock, SIO_GET_EXTENSION_FUNCTION_POINTER, &guidAcceptEx, sizeof(GUID), &lpfnAcceptEx, sizeof(LPFN_ACCEPTEX), &bytes, NULL, NULL))
	{
		std::cout << "[ERROR] WSAIoctl (AcceptEx) failed, " << WSAGetLastError() << std::endl;
		return false;
	}

	if (SOCKET_ERROR == WSAIoctl(listenSock, SIO_GET_EXTENSION_FUNCTION_POINTER, &guidDisconnectEx, sizeof(GUID), &lpfnDisconnectEx, sizeof(LPFN_DISCONNECTEX), &bytes, NULL, NULL))
	{
		std::cout << "[ERROR] WSAIoctl (AcceptEx) failed, " << WSAGetLastError() << std::endl;
		return false;
	}

	return true;
}

VOID IOCPManager::StartAcceptLoop()
{
	if (SOCKET_ERROR == listen(listenSock, SOMAXCONN))
	{
		std::cout << "[ERROR] listen failed" << std::endl;
		return;
	}

	while (true)
	{
		//todo 클라이언트 소켓 accept 과정을 넣자
		break;
	}
}

VOID IOCPManager::Finalize()
{
	CloseHandle(hIocpHandle);
	WSACleanup();
}

unsigned int WINAPI IOCPManager::IocpThread(LPVOID lpParam)
{
	DWORD dwReceivedByte = 0;
	//? lpParam 을 통해 들어온 dwThreadId 를 처리할 방법이 필요해보인다
	HANDLE hCompletionPort = GIocpmanager->GetIOCPHandle();
	ULONG_PTR completionKey;
	OverlappedIOContext* context = nullptr;

	while (true)
	{
		int retval = GetQueuedCompletionStatus(hCompletionPort, &dwReceivedByte, (PULONG_PTR)&completionKey, (LPOVERLAPPED*)&context, INFINITE);
		
		if (retval == 0 || dwReceivedByte == 0)
		{
			if (GetLastError() == WAIT_TIMEOUT)
				continue;

			if (context->type == PACKET_TYPE::CONNECTION || context->type == PACKET_TYPE::OBJECT_MODIFY)
			{
				//todo 에러에 대한 처리를 하자.
			}
		}

		//todo 잘 들어온 패킷에 대한 처리를 하자.
	}

	return 0;
}

BOOL IOCPManager::AcceptEx(SOCKET sListenSocket, SOCKET sAcceptSocket, PVOID lpOutputBuffer, DWORD dwReceiceDataLength,
	DWORD dwLocalAddressLength, DWORD dwRemoteAddressLength, LPDWORD lpdwBytesReceived, LPOVERLAPPED lpOverlapped)
{
	return IOCPManager::lpfnAcceptEx(sListenSocket, sAcceptSocket, lpOutputBuffer, dwReceiceDataLength, dwLocalAddressLength, dwRemoteAddressLength, lpdwBytesReceived, lpOverlapped);
}

BOOL IOCPManager::DisconnectEx(SOCKET hSocket, LPOVERLAPPED lpOverlapped, DWORD dwFlags)
{
	return IOCPManager::lpfnDisconnectEx(hSocket, lpOverlapped, dwFlags, 0);
}

HANDLE IOCPManager::GetIOCPHandle() const
{
	return this->hIocpHandle;
}

int IOCPManager::GetIOCPThreadCount() const
{
	return this->iocpThreadCount;
}

SOCKET IOCPManager::GetListenSock() const
{
	return this->listenSock;
}