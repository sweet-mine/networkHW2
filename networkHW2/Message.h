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

// 메세지 헤더 생성 함수
uint32_t make_header(bool is_group, bool is_file, uint32_t msg_length) {
	// 길이는 30비트만 사용 가능 (최대 1,073,741,823)
	msg_length &= 0x3FFFFFFF;
	uint32_t header = 0;
	header |= (is_group ? 1U : 0U) << 31;
	header |= (is_file ? 1U : 0U) << 30;
	header |= msg_length;
	return header;
}

// 메세지 헤더 읽는 함수
void parse_header(uint32_t header, bool& is_group, bool& is_file, uint32_t& msg_length) {
	is_group = (header >> 31) & 0x1;
	is_file = (header >> 30) & 0x1;
	msg_length = header & 0x3FFFFFFF;
}