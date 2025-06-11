/*
File Name : Message.h
Author: 이시행
Purpose: 메세지 타입과 헤더 처리 함수 선언 및 정의
Create date : 2025-06-12
*/

#pragma once
#include "Common.h"

// 메세지 타입 구조체
#pragma pack(push, 1) // 구조체 정렬 설정
struct MSGTYPE { // 메세지 형식
	char dst[21]; // 수신자 id
	char src[21]; // 송신자 id
	char data[BUFSIZE + 1];
};
#pragma pack(pop) // 구조체 정렬 원래대로 되돌리기

// 메세지 생성 후 반환
MSGTYPE* create_msg(std::string dst, std::string src, std::string data) {
    MSGTYPE* msg = new MSGTYPE;
	strcpy(msg->dst, dst.c_str());
	strcpy(msg->src, src.c_str());
	strcpy(msg->data, data.c_str());
	return msg;
};

// 메시지 헤더 생성 함수
uint32_t make_header(bool is_group, uint32_t msg_length) {
    msg_length &= 0x7FFFFFFF;
    uint32_t header = 0;
    header |= (uint32_t(is_group) << 31);
    header |= msg_length;
    return header;
}

// 메시지 헤더 파싱 함수
void parse_header(uint32_t header, bool& is_group, uint32_t& msg_length)
{
    is_group = (header >> 31) & 0x1;
    msg_length = header & 0x7FFFFFFF;
}

std::string cp949_to_utf8(const std::string& cp949_str) {
    // CP949 → wide
    int wlen = MultiByteToWideChar(949, 0, cp949_str.c_str(), -1, NULL, 0);
    std::wstring wide_str(wlen, L'\0');
    MultiByteToWideChar(949, 0, cp949_str.c_str(), -1, &wide_str[0], wlen);

    // wide → UTF-8
    int utf8len = WideCharToMultiByte(CP_UTF8, 0, wide_str.c_str(), -1, NULL, 0, NULL, NULL);
    std::string utf8_str(utf8len, '\0');
    WideCharToMultiByte(CP_UTF8, 0, wide_str.c_str(), -1, &utf8_str[0], utf8len, NULL, NULL);

    // 마지막 null 문자 제거
    //utf8_str.pop_back();
    return utf8_str;
}

std::string utf8_to_cp949(const std::string& utf8_str) {
    // UTF-8 → wide
    int wlen = MultiByteToWideChar(CP_UTF8, 0, utf8_str.c_str(), -1, NULL, 0);
    std::wstring wide_str(wlen, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, utf8_str.c_str(), -1, &wide_str[0], wlen);

    // wide → CP949
    int cp949len = WideCharToMultiByte(949, 0, wide_str.c_str(), -1, NULL, 0, NULL, NULL);
    std::string cp949_str(cp949len, '\0');
    WideCharToMultiByte(949, 0, wide_str.c_str(), -1, &cp949_str[0], cp949len, NULL, NULL);

    // 마지막 null 문자 제거
    //cp949_str.pop_back();
    return cp949_str;
}