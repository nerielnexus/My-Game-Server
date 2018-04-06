#include "stdafx.h"
#include "CommonDecl.h"

OverlappedIOContext::OverlappedIOContext(ClientSession* owner, PACKET_TYPE ptype)
	:owner(owner), type(ptype)
{
	ZeroMemory(&ovl, sizeof(OVERLAPPED));
	ZeroMemory(&wsaBuf, sizeof(WSABUF));
}

void DeleteIOContext(OverlappedIOContext* context)
{
	if (context == nullptr)
		return;

	//todo 만약 ReleaseRef 같은게 있다면 처리하자

	switch (context->type)
	{
	case PACKET_TYPE::CONNECT_ACCEPT:
	case PACKET_TYPE::DISCONNECT_USER:
	case PACKET_TYPE::DISCONNECT_FORCED:
	case PACKET_TYPE::OBJECT_CREATE:
	case PACKET_TYPE::OBJECT_MOVE:
	case PACKET_TYPE::OBJECT_COLLISION:
	case PACKET_TYPE::OBJECT_DELETE:
		delete context;
		break;

	default:
		//todo 이상한 패킷 타입의 경우 에러 처리
		break;
	}
}