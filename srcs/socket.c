#include "ping.h"

// 設定に従い各種オプションをセット
static int	apply_socket_options_by_prefs(const t_preferences* prefs, int sock) {
	// TTL設定
	if (prefs->ttl > 0) {
		if (setsockopt(sock, IPPROTO_IP, IP_TTL, &prefs->ttl, sizeof(prefs->ttl)) < 0) {
			perror("setsockopt failed");
			return -1;
		}
	}
	// ルーティングを無視する
	if (prefs->bypass_routing) {
		int one = 1; // boolean を有効にするには 1 を指定する
		if (setsockopt (sock, SOL_SOCKET, SO_DONTROUTE, (char *) &one, sizeof(one))) {
			perror("setsockopt failed");
			return -1;
		}
	}

	// ソースアドレス固定
	if (prefs->given_source_address) {
		// ソースアドレスをアドレス構造体に変換
		address_info_t* source_address = resolve_str_into_address(prefs->given_source_address);
		if (source_address == NULL) {
			perror("source addr failed");
			return -1;
		}
		// ソケットにソースアドレスを設定
		int	rv = bind(sock, source_address->ai_addr, source_address->ai_addrlen);
		freeaddrinfo(source_address);
		if (rv) {
			perror("bind failed");
			return -1;
		}
		DEBUGINFO("binded sock %d to %s", sock, prefs->given_source_address);
	}
	return 0;
}

// ICMPソケットを作成する
int create_icmp_socket(const t_preferences* prefs) {
	// ソケット生成
	int sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if (sock < 0) {
		perror("socket");
		return -1;
	}

	if (apply_socket_options_by_prefs(prefs, sock)) {
		return -1;
	}

	return sock;
}
