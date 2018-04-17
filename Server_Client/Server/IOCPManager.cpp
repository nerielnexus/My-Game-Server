#include "stdafx.h"
#include "IOCPManager.h"
#include "ClientSession.h"
#include "SessionManager.h"
#include "CommonDecl.h"

IOCPManager* GIocpmanager = nullptr;
char IOCPManager::acceptBuffer[64] = { 0, };
CRITICAL_SECTION cs;

IOCPManager::IOCPManager()
{
	listenSock			= NULL;
	hIocpHandle			= nullptr;
	iocpThreadCount		= 0;
	lpfnAcceptEx		= nullptr;
	lpfnDisconnectEx	= nullptr;
	InitializeCriticalSection(&cs);
}


IOCPManager::~IOCPManager()
{
}

bool IOCPManager::Initialize()
{
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		std::cout << "[ERROR] IOCPManager::Initialize() WSAStartup failed" << std::endl;
		return false;
	}

	listenSock = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (listenSock == INVALID_SOCKET)
	{
		std::cout << "[ERROR] IOCPManager::Initialize() WSASocket failed" << std::endl;
		return false;
	}

	int opt = 1;
	int retval = setsockopt(listenSock, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(int));
	if (retval != 0)
	{
		std::cout << "[ERROR] IOCPManager::Initialize() setsockopt failed, " << WSAGetLastError() << std::endl;
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
		std::cout << "[ERROR] IOCPManager::Initialize() bind failed, " << WSAGetLastError() << std::endl;
		return false;
	}

	SYSTEM_INFO sysinfo;
	GetSystemInfo(&sysinfo);
	iocpThreadCount = sysinfo.dwNumberOfProcessors * 2;

	hIocpHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if (hIocpHandle == nullptr)
	{
		std::cout << "[ERROR] IOCPManager::Initialize() CreateIoCompletionPort failed" << std::endl;
		return false;
	}

	HANDLE connectionHandle = CreateIoCompletionPort((HANDLE)listenSock, hIocpHandle, 0, 0);
	if (connectionHandle == nullptr)
	{
		std::cout << "[ERROR] IOCPManager::Initialize() CreateCompletionPort (create connection between IOCP & listenSock) failed" << std::endl;
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
			std::cout << "[ERROR] IOCPManager::Initialize() _beginthreadex failed" << std::endl;
			return false;
		}
	}

	GUID guidAcceptEx = WSAID_ACCEPTEX;
	GUID guidDisconnectEx = WSAID_DISCONNECTEX;
	DWORD bytes = 0;
	if (SOCKET_ERROR == WSAIoctl(listenSock, SIO_GET_EXTENSION_FUNCTION_POINTER, &guidAcceptEx, sizeof(GUID), &lpfnAcceptEx, sizeof(LPFN_ACCEPTEX), &bytes, nullptr, nullptr))
	{
		std::cout << "[ERROR] IOCPManager::Initialize() WSAIoctl (AcceptEx) failed, " << WSAGetLastError() << std::endl;
		return false;
	}

	if (SOCKET_ERROR == WSAIoctl(listenSock, SIO_GET_EXTENSION_FUNCTION_POINTER, &guidDisconnectEx, sizeof(GUID), &lpfnDisconnectEx, sizeof(LPFN_DISCONNECTEX), &bytes, nullptr, nullptr))
	{
		std::cout << "[ERROR] IOCPManager::Initialize() WSAIoctl (DisconnectEx) failed, " << WSAGetLastError() << std::endl;
		return false;
	}

	GSessionManager->InitializeSession();

	return true;
}

void IOCPManager::StartAcceptLoop()
{
	if (SOCKET_ERROR == listen(listenSock, SOMAXCONN))
	{
		std::cout << "[ERROR] IOCPManager::StartAcceptLoop() listen failed" << std::endl;
		return;
	}

	while (GSessionManager->AcceptSession())
	{
		Sleep(100);
	}
}

void IOCPManager::Finalize()
{
	CloseHandle(hIocpHandle);
	WSACleanup();
}

unsigned int WINAPI IOCPManager::IocpThread(LPVOID lpParam)
{
	//? lpParam 을 통해 들어온 dwThreadId 를 처리할 방법이 필요해보인다
	DWORD dwThreadId = reinterpret_cast<DWORD>(lpParam);
	HANDLE hCompletionPort = GIocpmanager->GetIOCPHandle();

	while (true)
	{
		DWORD dwReceivedByte = 0;
		ULONG_PTR completionKey = 0;
		OverlappedIOContext* context = nullptr;

		int retval = GetQueuedCompletionStatus(hCompletionPort, &dwReceivedByte, (PULONG_PTR)&completionKey, (LPOVERLAPPED*)&context, INFINITE);
		ClientSession* client = context ? context->owner : nullptr;
		
		if (retval == 0 || dwReceivedByte == 0)
		{
			if (GetLastError() == WAIT_TIMEOUT)
				continue;

			if (context == nullptr)
			{
				std::cout << "[ERROR] GQCS failed (LPOVERVALLED* is nullptr), "; ErrorMsg(GetLastError()); std::cout << " (" << GetLastError() << ")" << std::endl;
				system("pause");
			}

			if(context->type == PACKET_TYPE::OBJECT_CREATE ||
				context->type == PACKET_TYPE::OBJECT_MOVE ||
				context->type == PACKET_TYPE::OBJECT_COLLISION ||
				context->type == PACKET_TYPE::OBJECT_MOVE)
			{
				//x disconnect_user, disconnect_forced 가 아니면 비정상적인 종료로 봐야할까?
				//x 비정상적인 종료로 취급해야하는 전송량 0의 패킷은 어디까지?
				//! Connection 이 아닌 패킷 유형들이 에러로 들어온다면 소켓 삭제를 해주는게 좋을듯하다
				//todo 비정상적인 클라이언트 종료에 대한 처리
				//todo 상대 클라이언트에게 비정상적 종료를 통지
				//todo 서버 데이터 정리, 매칭 정리 등 필요한 작업을 진행

				client->DisconnectClient(PACKET_TYPE::DISCONNECT_FORCED);
				DeleteIOContext(context);
				std::cout << "[CONSOLE] object packet deleted" << std::endl;
				continue;
			}
		}

		if (client == nullptr)
		{
			std::cout << "[ERROR] client is nullptr" << std::endl;
			return -1;
		}

		//todo 잘 들어온 패킷에 대한 처리를 하자.

		bool completionStatus = false;
		switch (context->type)
		{
		case PACKET_TYPE::CONNECT_ACCEPT:
			//todo 각 packet type 에 대한 처리를 하는 함수 넣기
			client->AcceptCompletion();
			completionStatus = true;
			break;

		case PACKET_TYPE::DISCONNECT_USER:
			completionStatus = true;
			break;

		case PACKET_TYPE::DISCONNECT_FORCED:
			completionStatus = true;
			break;

		case PACKET_TYPE::OBJECT_CREATE:
		case PACKET_TYPE::OBJECT_MOVE:
		case PACKET_TYPE::OBJECT_COLLISION:
		case PACKET_TYPE::OBJECT_DELETE:
			//x 오브젝트 패킷을 처리하고, 그 결과값 (bool) 을 completionStatus 에 넣는 구문 작성하기
			completionStatus = ObjectCompletion(client, static_cast<ObjectIOContext*>(context), dwReceivedByte);
			break;

		case PACKET_TYPE::IO_DUMMY:
			completionStatus = DummyCompletion(client, static_cast<DummyIOContext*>(context), dwReceivedByte);
			break;

		default:
			std::cout << "[ERROR] Unknown Packet Type (" << context->type << ")" << std::endl;
			//todo 잘못된 패킷 타입을 받았을 때 해당 오류를 처리할 구문
			break;
		}

		if (!completionStatus)
		{
			//todo Object 패킷에 문제가 생겨 completionStatus == false 인 경우 disconnectex 해주자
			client->DisconnectClient(PACKET_TYPE::DISCONNECT_FORCED);
		}

		//x 다 사용한 overlapped io context 를 해제하자
		DeleteIOContext(context);
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

HANDLE IOCPManager::GetIOCPHandle()
{
	return this->hIocpHandle;
}

int IOCPManager::GetIOCPThreadCount()
{
	return this->iocpThreadCount;
}

SOCKET* IOCPManager::GetListenSock()
{
	return &(this->listenSock);
}

bool IOCPManager::ObjectCompletion(ClientSession* client, ObjectIOContext* context, DWORD dwReceived)
{
	return client->ObjectDataRecv(context->type);
}

bool IOCPManager::DummyCompletion(ClientSession* client, DummyIOContext* context, DWORD dwReceived)
{
	return client->InitializeRecv();
}