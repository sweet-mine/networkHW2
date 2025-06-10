#include "Common.h"
#include "Message.h"
#include <thread>
#include <atomic>

char* SERVERIP = (char*)"127.0.0.1";
#define SERVERPORT 9000
#define BUFSIZE    512

using namespace std;

atomic<bool> running(true);

void recv_thread(SOCKET sock) {
    bool is_group;
    bool is_file;
	uint32_t msg_length;
    uint32_t header;
    char c_buf[BUFSIZE];
    while (running) {

        // 헤더 수신
		int retval = recv(sock, (char*)&header, 32, 0);
        if (retval == SOCKET_ERROR) {
            err_display("recv()");
            break;
        }
        else if (retval == 0) {
            cout << "\n[서버 종료됨]\n";
            break;
        }

        // 헤더 읽기
        parse_header(header, is_group, is_file, msg_length);

        // 메시지 수신
        retval = recv(sock, c_buf, msg_length, 0);
        if (retval == SOCKET_ERROR) {
            err_display("recv()");
            break;
        }

        string buf(c_buf, retval);
        buf = utf8_to_cp949(buf);
        cout << "\n받은 데이터: " << buf << endl;
        //cout.flush();
    }
    running = false;
}

void send_thread(SOCKET sock) {
    bool is_group;
    bool is_file;
    uint32_t msg_length;
    string buf;
    while (running) {
		cin >> is_group >> is_file;
        getline(cin, buf);
        if (buf.empty()) continue;
        buf = cp949_to_utf8(buf);
        msg_length = buf.size();

        // 헤더 생성
        uint32_t header = make_header(is_group, is_file, msg_length);

        // 헤더 전송
		int retval = send(sock, (char*)&header, 32, 0);
        if (retval == SOCKET_ERROR) {
            err_display("send()");
            break;
        }

        // 메세지 전송
        retval = send(sock, buf.c_str(), msg_length, 0);
        if (retval == SOCKET_ERROR) {
            err_display("send()");
            break;
        }

        if (retval > 0)
            printf("[송신] %d바이트를 보냈습니다.\n", retval);
    }
    running = false;
}

int main(int argc, char* argv[]) {
    int retval;

    if (argc > 1) SERVERIP = argv[1];

    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
        return 1;

    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) err_quit("socket()");

    struct sockaddr_in serveraddr;
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    inet_pton(AF_INET, SERVERIP, &serveraddr.sin_addr);
    serveraddr.sin_port = htons(SERVERPORT);

    retval = connect(sock, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
    if (retval == SOCKET_ERROR) err_quit("connect()");

    string id;
    bool flag = false;

    while (true) {
        cout << "사용할 ID 입력 (공백 금지, 20바이트 이내)\n";
        getline(cin, id);
        id = cp949_to_utf8(id);
        printf("\n");
        if (id.empty() || id.size() > 20 || id.find(' ') != string::npos) {
            cout << "공백이 포함됐거나 20바이트를 초과했습니다.\n";
            continue;
        }
        else break;
    }

    retval = send(sock, id.c_str(), 20, 0);
    if (retval == SOCKET_ERROR) err_quit("send()");

    retval = recv(sock, (char*)&flag, 1, 0);
    if (retval == SOCKET_ERROR) err_quit("recv()");
    if (!flag) {
        cout << "중복 ID입니다. 다른 ID로 요청하세요.\n";
        closesocket(sock);
        WSACleanup();
        return 0;
    }

    thread t_recv(recv_thread, sock);
    thread t_send(send_thread, sock);

    t_send.join();
    t_recv.join();

    closesocket(sock);
    WSACleanup();
    return 0;
}
