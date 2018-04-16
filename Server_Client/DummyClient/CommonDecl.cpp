#include "stdafx.h"
#include "CommonDecl.h"

OverlappedIOContext::OverlappedIOContext(ClientSession* owner, PACKET_TYPE ptype)
	:owner(owner), type(ptype)
{
	ZeroMemory(&ovl, sizeof(OVERLAPPED));
	ZeroMemory(&wsaBuf, sizeof(WSABUF));
}