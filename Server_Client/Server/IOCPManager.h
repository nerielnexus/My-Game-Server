#pragma once

struct OverlappedIOContext;
class ClientSession;

class IOCPManager
{
public:
	IOCPManager();
	~IOCPManager();

	// ���� ���� �� �ʱ�ȭ�ϴ� �۾�
	bool Initialize();
	
	// Ŭ���̾�Ʈ accpet ����
	void StartAcceptLoop();

	// ���� ���� �� ���� �۾�
	void Finalize();

	/*
	// AcceptEx �� �Ű�����
	BOOL AcceptEx(SOCKET sListenSocket,         // ���� ����
	              SOCKET sAcceptSocket,         // Ŭ���̾�Ʈ ����, bind/connect �Ǹ� �ȵ�
	              PVOID lpOutputBuffer,         // local �ּ�, remote �ּ�, ����� ���� ù ������ ����� ������ ������ ����
	              DWORD dwReceiceDataLength,    // ���ۿ� ����Ǵ� ������ ����� ũ�� 
	              DWORD dwLocalAddressLength,   // local �ּ��� ����
	              DWORD dwRemoteAddressLength,  // remote �ּ��� ����
	              LPDWORD lpdwBytesReceived,    // �Ϸ�� ������ overlapped ����ü�� �ּ�
	              LPOVERLAPPED lpOverlapped     // Overlapped ����ü�� ������, NULL�� �ȵ�
	              );
	*/
	BOOL AcceptEx(SOCKET sListenSocket, SOCKET sAcceptSocket, PVOID lpOutputBuffer, DWORD dwReceiceDataLength,
		DWORD dwLocalAddressLength, DWORD dwRemoteAddressLength, LPDWORD lpdwBytesReceived, LPOVERLAPPED lpOverlapped);
	/*
	// DisconnectEx �� �Ű�����
	BOOL DisconnectEx(SOCKET hSocket,              // ����Ǿ� �ִ� Ŭ���̾�Ʈ ����
	                  LPOVERLAPPED lpOverlapped,   // OVERLAPPED ����ü�� ������, NULL �̸� �ȵ�
	                  DWORD dwFlags,               // ���� ��뿡 ���� �÷���
	                  DWORD reserved               // ������ 0�� ����
	                  );

	// Ư�̻���
	// dwFlags : TF_REUSE_SOCKET �� �����ؼ� AcceptEx, ConnectEx ���� ����� ������ ������ �� �ִ�
	// reserved : 0�� �ƴ� �ٸ� ���� �����ϸ� WSAEINVAL ������ ��ȯ��
	*/
	BOOL DisconnectEx(SOCKET hSocket, LPOVERLAPPED lpOverlapped, DWORD dwFlags);

	HANDLE	GetIOCPHandle() const;
	int		GetIOCPThreadCount() const;
	SOCKET*	GetListenSock();

	static char acceptBuffer[64];

private:
	// ��Ŷ�� ���� completion ó�� �Լ�
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