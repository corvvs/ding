#include "ping.h"

// ICMPソケットを作成する
int create_icmp_socket(const t_preferences* prefs) {
	int sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if (sock < 0) {
		perror("socket");
		exit(EXIT_FAILURE);
	}

	if (prefs->ttl > 0) {
		if (setsockopt(sock, IPPROTO_IP, IP_TTL, &prefs->ttl, sizeof(prefs->ttl)) < 0) {
			perror("setsockopt failed");
			exit(EXIT_FAILURE);
		}
	}

	return sock;
}
