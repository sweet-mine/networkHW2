#include "..\..\Common.h"

char* SERVERIP = (char*)"127.0.0.1";
#define SERVERPORT 9000
#define BUFSIZE    50

int main(int argc, char* argv[])
{
	int retval;

	// ����� �μ��� ������ IP �ּҷ� ���
	if (argc > 1) SERVERIP = argv[1];

	// ���� �ʱ�ȭ
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	// connect() ȣ�⿡ ����� ����
	struct sockaddr_in serveraddr;
	memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	inet_pton(AF_INET, SERVERIP, &serveraddr.sin_addr);
	serveraddr.sin_port = htons(SERVERPORT);

	// ���� ����
	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET) err_quit("socket()");

	// connect()
	retval = connect(sock, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR) err_quit("connect()");

	// ������ ��ſ� ����� ����
	char buf[BUFSIZE];
	size_t val;
	char fileName[BUFSIZE];

	//���� ���� ���� ����
	printf("���� �̸� �Է����ּ��� : ");
	fgets(fileName, BUFSIZE, stdin);
	fileName[strlen(fileName) - 1] = '\0';
	FILE* file = fopen(fileName, "rb");
	if (file) {
		//���ϸ� ����
		retval = send(sock, fileName, strlen(fileName) + 1, 0);
		if (retval == SOCKET_ERROR) {
			err_display("send()");
		}

		//���� ���� ����
		val = fread(buf, 1, BUFSIZE, file);
		retval = send(sock, buf, val, 0);
		if (retval == SOCKET_ERROR) {
			err_display("send()");
		}
		printf("���� ���� �Ϸ�\n");
		printf("[TCP Ŭ���̾�Ʈ] %d����Ʈ�� ���½��ϴ�.\n", retval);
		fclose(file);
	}
	
	else {
		printf("��Ȯ�� ���ϸ��� �Է��ϼ���.");
	}

	// ���� �ݱ�
	closesocket(sock);

	// ���� ����
	WSACleanup();
	return 0;
}