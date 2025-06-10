#include "..\..\Common.h"

char* SERVERIP = (char*)"127.0.0.1";
#define SERVERPORT 9000
#define BUFSIZE    50

int main(int argc, char* argv[])
{
	int retval;

	// 명령행 인수가 있으면 IP 주소로 사용
	if (argc > 1) SERVERIP = argv[1];

	// 윈속 초기화
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	// connect() 호출에 사용할 변수
	struct sockaddr_in serveraddr;
	memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	inet_pton(AF_INET, SERVERIP, &serveraddr.sin_addr);
	serveraddr.sin_port = htons(SERVERPORT);

	// 소켓 생성
	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET) err_quit("socket()");

	// connect()
	retval = connect(sock, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR) err_quit("connect()");

	// 데이터 통신에 사용할 변수
	char buf[BUFSIZE];
	size_t val;
	char fileName[BUFSIZE];

	//파일 열고 내용 전송
	printf("파일 이름 입력해주세요 : ");
	fgets(fileName, BUFSIZE, stdin);
	fileName[strlen(fileName) - 1] = '\0';
	FILE* file = fopen(fileName, "rb");
	if (file) {
		//파일명 전송
		retval = send(sock, fileName, strlen(fileName) + 1, 0);
		if (retval == SOCKET_ERROR) {
			err_display("send()");
		}

		//파일 내용 전송
		val = fread(buf, 1, BUFSIZE, file);
		retval = send(sock, buf, val, 0);
		if (retval == SOCKET_ERROR) {
			err_display("send()");
		}
		printf("파일 전송 완료\n");
		printf("[TCP 클라이언트] %d바이트를 보냈습니다.\n", retval);
		fclose(file);
	}
	
	else {
		printf("정확한 파일명을 입력하세요.");
	}

	// 소켓 닫기
	closesocket(sock);

	// 윈속 종료
	WSACleanup();
	return 0;
}