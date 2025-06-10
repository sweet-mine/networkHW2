#pragma once
#include "Common.h"

// 메세지 타입 구조체
struct MSGTYPE { // 메세지 형식
	std::string dst; // 수신자 id
	std::string src; // 송신자 id
	char* data;
};

// 메세지 생성 후 반환
MSGTYPE* create_msg(std::string dst, std::string src, char* data) {
	MSGTYPE* msg = new MSGTYPE;
	msg->dst = dst;
	msg->src = src;
	msg->data = data;
	return msg;
};