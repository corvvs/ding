#include "ping.h"

// ICMP送信用ソケットを作成する
int create_icmp_socket(void) {
	int sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if (sock < 0) {
		perror("socket");
		exit(EXIT_FAILURE);
	}

	// 受信タイムアウト設定
	timeval_t	recv_timeout = {
		.tv_sec = 1,
		.tv_usec = 0,
	};
	if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &recv_timeout, sizeof(recv_timeout)) < 0) {
		perror("setsockopt failed");
		exit(EXIT_FAILURE);
	}
	return sock;
}
