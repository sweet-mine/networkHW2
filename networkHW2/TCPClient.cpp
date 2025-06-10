#include "Common.h"
#include "Message.h"

char* SERVERIP = (char*)"127.0.0.1";
#define SERVERPORT 9000
#define BUFSIZE    512

using namespace std;
int main(int argc, char* argv[])
{
	int retval;

	// 명령행 인수가 있으면 IP 주소로 사용
	if (argc > 1) SERVERIP = argv[1];

	// 윈속 초기화
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	// 소켓 생성
	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET) err_quit("socket()");

	// connect()
	struct sockaddr_in serveraddr;
	memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	inet_pton(AF_INET, SERVERIP, &serveraddr.sin_addr);
	serveraddr.sin_port = htons(SERVERPORT);
	retval = connect(sock, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR) err_quit("connect()");

	// 데이터 통신에 사용할 변수
	string buf;
	char c_buf[BUFSIZE];
	

	// id 송수신
	while (true) {
		string id;
		bool flag = false;
		cout << "사용할 ID 입력 (공백 금지, 20바이트 이내)\n";
		getline(cin, id);
		id = cp949_to_utf8(id);
		printf("\n");
		if (id.empty() || id.size() > 20 || id.find(' ') != std::string::npos) {
			cout << "공백이 포함됐거나 20바이트를 초과했습니다.\n";
			continue;
		}

		retval = send(sock, id.c_str(), 20, 0);
		if (retval == SOCKET_ERROR) err_quit("send()");
		retval = recv(sock, (char*)&flag, 1, 0);
		if (retval == SOCKET_ERROR) err_quit("recv()");
		
		if (flag == false) {
			cout << "중복 ID입니다. 다른 ID로 바꿔주세요.\n";
			exit(1);
		}
		else break;
	}

	// 서버와 데이터 통신
	while (1) {
		// 데이터 입력
		cout << "\n보낼 데이터 : ";
		getline(cin, buf);
		buf = cp949_to_utf8(buf);

		// 데이터 보내기
		retval = send(sock, buf.c_str(), buf.size(), 0);
		if (retval == SOCKET_ERROR) {
			err_display("send()");
			break;
		}
		if (retval == 0) continue;
		printf("[TCP 클라이언트] %d바이트를 보냈습니다.\n", retval);

		// 데이터 받기
		retval = recv(sock, c_buf, 33, 0);
		if (retval == SOCKET_ERROR) {
			err_display("recv()");
			break;
		}
		
		// 화면 출력
		buf = string(c_buf, retval);
		buf = utf8_to_cp949(buf);
		cout << "받은 데이터 : " << buf;
	}

	// 소켓 닫기
	closesocket(sock);

	// 윈속 종료
	WSACleanup();
	return 0;
}