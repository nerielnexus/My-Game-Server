#include "stdafx.h"
#include "CommonDecl.h"

OverlappedIOContext::OverlappedIOContext(ClientSession* owner, PACKET_TYPE ptype)
	:owner(owner), type(ptype)
{
	ZeroMemory(&ovl, sizeof(OVERLAPPED));
	ZeroMemory(&wsaBuf, sizeof(WSABUF));
}

ConnectionIOContext::ConnectionIOContext(ClientSession* client, PACKET_TYPE type)
	:OverlappedIOContext(client, type)
{

}

ObjectIOContext::ObjectIOContext(ClientSession* client, PACKET_TYPE type)
	: OverlappedIOContext(client, type)
{

}

DummyIOContext::DummyIOContext(ClientSession* client)
	: OverlappedIOContext(client, PACKET_TYPE::IO_DUMMY)
{

}