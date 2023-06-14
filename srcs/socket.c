#include "ping.h"

// ICMPソケットを作成する
int create_icmp_socket(const t_preferences* prefs) {
	int sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if (sock < 0) {
		perror("socket");
		exit(EXIT_FAILURE);
	}

	// TTL設定
	if (prefs->ttl > 0) {
		if (setsockopt(sock, IPPROTO_IP, IP_TTL, &prefs->ttl, sizeof(prefs->ttl)) < 0) {
			perror("setsockopt failed");
			exit(EXIT_FAILURE);
		}
	}

	// ソースアドレス固定
	if (prefs->given_source_address) {
		// ソースアドレスをアドレス構造体に変換
		address_info_t* source_address = resolve_str_into_address(prefs->given_source_address);
		if (source_address == NULL) {
			perror("source addr failed");
			exit(EXIT_FAILURE);
		}
		// ソケットにソースアドレスを設定
		int	rv = bind(sock, source_address->ai_addr, source_address->ai_addrlen);
		freeaddrinfo(source_address);
		if (rv) {
			perror("bind failed");
			exit(EXIT_FAILURE);
		}
		DEBUGINFO("binded sock %d to %s", sock, prefs->given_source_address);
	}

	return sock;
}
