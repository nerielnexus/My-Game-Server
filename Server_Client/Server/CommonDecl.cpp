#include "stdafx.h"
#include "CommonDecl.h"

void ErrorMsg(DWORD err)
{
	TCHAR* s = 0;
	FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER, 0, err, 0, (TCHAR*)&s, 0, 0);
	std::cout << s << std::endl;
}