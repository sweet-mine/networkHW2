#define NOMINMAX
#include "Common.h"
#include "Message.h"

char SERVERIP[16];
#define SERVERPORT 9000

using namespace std;

atomic<bool> running(true);
string id;
vector<string> chat_log;

void save_chat_log() {
    ofstream out("chat_log.txt");
    if (!out.is_open()) {
        cerr << "로그 파일 저장 실패.\n";
        return;
    }

    for (const string& line : chat_log) {
        out << line << "\n";
    }
    cout << "[시스템] 채팅 로그가 chat_log.txt에 저장되었습니다.\n";
}


void recv_thread(SOCKET sock) {
    bool is_group;
	uint32_t msg_length;
    uint32_t header;
    MSGTYPE msg;
    while (running) {

        // 헤더 수신
		int retval = recv(sock, (char*)&header, 4, 0);
        if (retval == SOCKET_ERROR) {
            err_display("recv()");
            break;
        }
        else if (retval == 0) {
            cout << "\n[서버 종료됨]\n";
            break;
        }

        // 헤더 읽기
        parse_header(header, is_group, msg_length);

        // 메시지 수신
        retval = recv(sock, (char*)&msg, msg_length, 0);
        if (retval == SOCKET_ERROR) {
            err_display("recv()");
            break;
        }

		string dst(msg.dst);
        string src(msg.src);
        string data(msg.data);
        dst = utf8_to_cp949(dst);
		src = utf8_to_cp949(src);
		data = utf8_to_cp949(data);
       
        string formatted;

        if (is_group) {
            formatted = "[그룹 메세지] " + src + " : " + data;
        }
        else {
            formatted = "[개인 메세지] " + src + " -> " + dst + " : " + data;
        }
        
        cout << "\n" << formatted << "\n";
        chat_log.push_back(formatted);
    }
    running = false;
}

void send_thread(SOCKET sock) {
    int is_group;
    uint32_t msg_length;
    string buf;
    MSGTYPE* msg;
    string receiver; // 수신자 ID
    string sender = id; // 송신자 ID
    while (running) {
        cout << "메세지 전송 방식을 선택하세요(Group : 1, Private : 0) : ";
		cin >> is_group;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        if(is_group != 0 && is_group != 1) {
            cout << "잘못된 입력입니다. 다시 시도하세요.\n";
            continue;
		}
        if (is_group == false) {
            cout << "수신자 ID 입력하세요(공백 금지, 20바이트 이내) : ";
            getline(cin, receiver);
            if (receiver.empty() || receiver.size() > 20 || receiver.find(' ') != string::npos) {
                cout << "ID에 공백이 포함됐거나 20바이트를 초과했습니다.\n";
                continue;
            }
        }
        receiver = cp949_to_utf8(receiver);
        cout << "전송할 메세지를 입력하세요 : ";
        getline(cin, buf);
		printf("메세지 입력 완료\n");
        if (buf.empty()) continue;
       
        buf = cp949_to_utf8(buf);
		receiver = cp949_to_utf8(receiver);
		sender = cp949_to_utf8(sender);

        msg_length = buf.size() + 42;
        // 헤더 생성
        uint32_t header = make_header(is_group, msg_length);

        // 메세지 생성
        msg = create_msg(receiver, sender, buf);
        if (msg == nullptr) {
            cout << "메세지 생성 실패\n";
            continue;
        }
        // 헤더 전송
		int retval = send(sock, (char*)&header, 4, 0);
        if (retval == SOCKET_ERROR) {
            err_display("send()");
            break;
        }

        // 메세지 전송
        retval = send(sock, (char*)msg, msg_length, 0);
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
	atexit(save_chat_log);

    cout << "서버 IP를 입력하세요 : ";
    cin >> SERVERIP;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

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
    bool flag = false;

    // ID 송신 및 중복 확인
    while (true) {
        cout << "사용할 ID 입력 (공백 금지, 20바이트 이내)\n";
        getline(cin, id);
        printf("\n");
        if (id.empty() || id.size() > 20 || id.find(' ') != string::npos) {
            cout << "공백이 포함됐거나 20바이트를 초과했습니다.\n";
            continue;
        }
        else break;
    }
    id = cp949_to_utf8(id);

    retval = send(sock, id.c_str(), 21, 0);
    if (retval == SOCKET_ERROR) err_quit("send()");

    retval = recv(sock, (char*)&flag, 1, 0);
    if (retval == SOCKET_ERROR) err_quit("recv()");
    if (!flag) {
        cout << "중복 ID입니다. 다른 ID로 요청하세요.\n";
        closesocket(sock);
        WSACleanup();
        return 0;
    }

    // 현재 유저 리스트 출력
    int usersize;
    char myid[21];
	printf("*****현재 접속 유저 목록 *****\n");
    retval = recv(sock, (char*)&usersize, sizeof(int), 0);
    if (retval == SOCKET_ERROR) err_quit("recv()");
    for (int i = 0; i < usersize; i++) {
        retval = recv(sock, myid, 21, 0);
        if (retval == SOCKET_ERROR) err_quit("recv()");
        string idStr(myid);
        idStr = utf8_to_cp949(idStr);
        printf("%s\n", idStr.c_str());
    }
    
    // 채팅 복원
    int chatsize;
    MSGTYPE msg;
    
	printf("*****복원된 채팅 목록 *****\n");
    retval = recv(sock, (char*)&chatsize, sizeof(int), 0);
    if (retval == SOCKET_ERROR) err_quit("recv()");
    for (int i = 0; i < chatsize; i++) {
        retval = recv(sock, (char*)&msg, sizeof(MSGTYPE), 0);
        if (retval == SOCKET_ERROR) err_quit("recv()");
        string src(msg.src);
        string data(msg.data);
        src = utf8_to_cp949(src);
        data = utf8_to_cp949(data);
        cout << "[그룹 메세지] " << src << " : " << data << "\n";
    }


    thread t_recv(recv_thread, sock);
    thread t_send(send_thread, sock);

    t_send.join();
    t_recv.join();

    closesocket(sock);
    WSACleanup();
    return 0;
}
