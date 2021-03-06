#include "stdafx.h"
#include "CommonDecl.h"

void ErrorMsg(DWORD err)
{
	TCHAR* s = 0;
	FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER, 0, err, 0, (TCHAR*)&s, 0, 0);
	std::cout << s << std::endl;
}

int _tmain(int argv, TCHAR* argc[])
{
	// Winsock Start - winsock.dll 로드
	WSADATA WSAData;
	if (WSAStartup(MAKEWORD(2, 2), &WSAData) != 0)
	{
		printf("Error - Can not load 'winsock.dll' file\n");
		return 1;
	}

	SOCKET listenSocket = WSASocket(PF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
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
		DWORD dwBytes = 0;

		ObjectCreate* occ = new ObjectCreate();
		occ->objectid = 111;
		occ->userid = 222;
		occ->xpos = 333.333;
		occ->ypos = 444.444;

		ObjectIOContext* ocovl = new ObjectIOContext(nullptr, PACKET_TYPE::OBJECT_CREATE);
		ocovl->wsaBuf.len = sizeof(ObjectCreate);
		ocovl->wsaBuf.buf = new char[ocovl->wsaBuf.len];
		CopyMemory(ocovl->wsaBuf.buf, occ, sizeof(ObjectCreate));

		if (SOCKET_ERROR == WSASend(listenSocket, &ocovl->wsaBuf, 1, (LPDWORD)&dwBytes, 0, (LPWSAOVERLAPPED)ocovl, NULL))
		{
			if (WSAGetLastError() != WSA_IO_PENDING)
			{
				std::cout << "[ERROR] WSASend failed, error code ";
				ErrorMsg(GetLastError());
				std::cout << std::endl;
			}
		}

		std::cout << "send" << std::endl;
		Sleep(1000);
	}
}