#pragma once

struct OverlappedIOContext;
class ClientSession;

class IOCPManager
{
public:
	IOCPManager();
	~IOCPManager();

	// 서버 시작 전 초기화하는 작업
	bool Initialize();
	
	// 클라이언트 accpet 루프
	void StartAcceptLoop();

	// 서버 종료 전 정리 작업
	void Finalize();

	/*
	// AcceptEx 의 매개변수
	BOOL AcceptEx(SOCKET sListenSocket,         // 서버 소켓
	              SOCKET sAcceptSocket,         // 클라이언트 소켓, bind/connect 되면 안됨
	              PVOID lpOutputBuffer,         // local 주소, remote 주소, 연결시 받을 첫 데이터 블록의 내용을 저장할 버퍼
	              DWORD dwReceiceDataLength,    // 버퍼에 저장되는 데이터 블록의 크기 
	              DWORD dwLocalAddressLength,   // local 주소의 길이
	              DWORD dwRemoteAddressLength,  // remote 주소의 길이
	              LPDWORD lpdwBytesReceived,    // 완료시 전달할 overlapped 구조체의 주소
	              LPOVERLAPPED lpOverlapped     // Overlapped 구조체의 포인터, NULL은 안됨
	              );
	*/
	BOOL AcceptEx(SOCKET sListenSocket, SOCKET sAcceptSocket, PVOID lpOutputBuffer, DWORD dwReceiceDataLength,
		DWORD dwLocalAddressLength, DWORD dwRemoteAddressLength, LPDWORD lpdwBytesReceived, LPOVERLAPPED lpOverlapped);
	/*
	// DisconnectEx 의 매개변수
	BOOL DisconnectEx(SOCKET hSocket,              // 연결되어 있는 클라이언트 소켓
	                  LPOVERLAPPED lpOverlapped,   // OVERLAPPED 구조체의 포인터, NULL 이면 안됨
	                  DWORD dwFlags,               // 소켓 사용에 대한 플래그
	                  DWORD reserved               // 무조건 0을 전달
	                  );

	// 특이사항
	// dwFlags : TF_REUSE_SOCKET 을 전달해서 AcceptEx, ConnectEx 에서 사용한 소켓을 재사용할 수 있다
	// reserved : 0이 아닌 다른 값을 전달하면 WSAEINVAL 오류를 반환함
	*/
	BOOL DisconnectEx(SOCKET hSocket, LPOVERLAPPED lpOverlapped, DWORD dwFlags);

	HANDLE	GetIOCPHandle() const;
	int		GetIOCPThreadCount() const;
	SOCKET*	GetListenSock();

	static char acceptBuffer[64];

private:
	// 패킷에 대한 completion 처리 함수
	static bool CompletionProcess(ClientSession* client, OverlappedIOContext* context, DWORD dwReceived);

	static unsigned int WINAPI IocpThread(LPVOID lpParam);

private:
	SOCKET	listenSock;

	int		iocpThreadCount;
	HANDLE	hIocpHandle;

	LPFN_ACCEPTEX		lpfnAcceptEx;
	LPFN_DISCONNECTEX	lpfnDisconnectEx;
};

extern IOCPManager* GIocpmanager;