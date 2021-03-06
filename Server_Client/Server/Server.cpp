// Server.cpp: 콘솔 응용 프로그램의 진입점을 정의합니다.
//

#include "stdafx.h"
#include "ClientSession.h"
#include "SessionManager.h"
#include "IOCPManager.h"


int _tmain(int argc, _TCHAR* argv[])
{
	GIocpmanager = new IOCPManager;
	GSessionManager = new SessionManager;

	if (GIocpmanager->Initialize() == false)
		return -1;

	GIocpmanager->StartAcceptLoop();
	GIocpmanager->Finalize();

	delete GIocpmanager;
	delete GSessionManager;

	return 0;
}