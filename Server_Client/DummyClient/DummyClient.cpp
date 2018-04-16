#include "stdafx.h"
#include "CommonDecl.h"

int _tmain(int argv, TCHAR* argc[])
{
	// Winsock Start - winsock.dll 로드
	WSADATA WSAData;
	if (WSAStartup(MAKEWORD(2, 2), &WSAData) != 0)
	{
		printf("Error - Can not load 'winsock.dll' file\n");
		return 1;
	}

	SOCKET listenSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (listenSocket == INVALID_SOCKET)
	{
		printf("Error - Invalid socket\n");
		return 1;
	}

	// 서버정보 객체설정
	SOCKADDR_IN serverAddr;
	memset(&serverAddr, 0, sizeof(SOCKADDR_IN));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(SERVERPORT);
	serverAddr.sin_addr.S_un.S_addr = inet_addr(SERVERIP);

	// 2. 연결요청
	if (connect(listenSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
	{
		printf("Error - Fail to connect\n");
		// 4. 소켓종료
		closesocket(listenSocket);
		// Winsock End
		WSACleanup();
		return 1;
	}
	else
	{
		printf("Server Connected\n");
	}

	while (true)
	{
		ObjectCreateContext* occ = new ObjectCreateContext();
		occ->objectid = 111;
		occ->userid = 222;
		occ->xpos = 333.333;
		occ->ypos = 444.444;
		OverlappedIOContext* ocovl = new OverlappedIOContext(nullptr, PACKET_TYPE::OBJECT_CREATE);
		ocovl->wsaBuf.len = sizeof(ObjectCreateContext);
		ocovl->wsaBuf.buf = new char[ocovl->wsaBuf.len];
		CopyMemory(ocovl->wsaBuf.buf, occ, sizeof(ObjectCreateContext));
		if (SOCKET_ERROR == WSASend(listenSocket, &ocovl->wsaBuf, ocovl->wsaBuf.len, NULL, 0, (LPWSAOVERLAPPED)&ocovl, NULL))
		{
			if (WSAGetLastError() != WSA_IO_PENDING)
			{
				std::cout << "[ERROR] WSASend failed, error code " << GetLastError() << std::endl;
			}
		}

		Sleep(1000);
	}
}