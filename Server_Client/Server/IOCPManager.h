#pragma once
class IOCPManager
{
public:
	IOCPManager();
	~IOCPManager();

	bool Initialize();

	static unsigned int WINAPI IocpThread(LPVOID lpParam);

private:
	SOCKET	listenSock;

	int		iocpThreadCount;
	HANDLE	hIocpHandle;
};

extern IOCPManager* GIocpmanager;