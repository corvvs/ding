#include "ping.h"

// 設定に従い各種オプションをセット
static int	apply_socket_options_by_prefs(const t_preferences* prefs, int sock) {
	// TTL設定
	if (prefs->ttl > 0) {
		if (setsockopt(sock, IPPROTO_IP, IP_TTL, &prefs->ttl, sizeof(prefs->ttl)) < 0) {
			print_special_error_by_errno("setsockopt(IP_TTL)");
			return -1;
		}
	}

	// ToS設定
	if (prefs->tos >= 0) {
		if (setsockopt(sock, IPPROTO_IP, IP_TOS, &prefs->tos, sizeof(prefs->tos)) < 0) {
			print_special_error_by_errno("setsockopt(IP_TOS)");
			return -1;
		}
		DEBUGWARN("tos set OK; %d", prefs->tos);
	}

	// ルーティングを無視する
	if (prefs->bypass_routing) {
		int one = 1; // boolean を有効にするには 1 を指定する
		if (setsockopt (sock, SOL_SOCKET, SO_DONTROUTE, (char *)&one, sizeof(one))) {
			print_special_error_by_errno("setsockopt(SO_DONTROUTE)");
			return -1;
		}
	}

	// IPタイムスタンプオプションをセット
	if (prefs->ip_ts_type != IP_TST_NONE) {
		uint8_t options[MAX_IPOPTLEN] = {0};
		const size_t unit_octets = prefs->ip_ts_type == IP_TST_TSONLY
			? sizeof(uint32_t)
			: (sizeof(uint32_t) + sizeof(uint32_t));
		options[IPOPT_OPTVAL] = IPOPT_TS;
		options[IPOPT_OLEN] = (MAX_IPOPTLEN - 4) / unit_octets * unit_octets + 4;
		options[IPOPT_OFFSET] = IPOPT_MINOFF + 1;
		const int8_t type = prefs->ip_ts_type == IP_TST_TSONLY
			? IPOPT_TS_TSONLY
			: IPOPT_TS_TSANDADDR;
		options[IPOPT_POS_OV_FLG] = type;
		if (setsockopt (sock, IPPROTO_IP, IP_OPTIONS, options, options[IPOPT_OLEN])) {
			print_special_error_by_errno("setsockopt");
			return -1;
		}
	}

	// ソースアドレス固定
	if (prefs->given_source_address) {
		// ソースアドレスをアドレス構造体に変換
		address_info_t* source_address = resolve_str_into_address(prefs->given_source_address);
		if (source_address == NULL) {
			print_special_error_by_errno("source addr failed");
			return -1;
		}
		// ソケットにソースアドレスを設定
		int	rv = bind(sock, source_address->ai_addr, source_address->ai_addrlen);
		freeaddrinfo(source_address);
		if (rv) {
			print_special_error_by_errno("bind");
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
		print_special_error_by_errno("socket");
		return -1;
	}

	if (apply_socket_options_by_prefs(prefs, sock)) {
		return -1;
	}

	return sock;
}
