#include "ping.h"

// IPヘッダにIPタイムスタンプオプションをセットする
static bool	set_sockopt_timestamp(const t_preferences* prefs, int sock) {
	// オプション領域となるバッファ
	uint8_t			options[MAX_IPOPTLEN] = {0};

	// オプションタイプ
	options[IPOPT_OPTVAL] = IPOPT_TS;

	// レングス
	const size_t	unit_octets = prefs->ip_ts_type == IP_TST_TSONLY
		? sizeof(uint32_t)
		: (sizeof(uint32_t) + sizeof(uint32_t));
	const size_t	ts_fixed_octet = 4;
	const size_t	max_memorable_units = (MAX_IPOPTLEN - ts_fixed_octet) / unit_octets;
	options[IPOPT_OLEN] = ts_fixed_octet + max_memorable_units * unit_octets;

	// ポインタ
	options[IPOPT_OFFSET] = IPOPT_MINOFF + 1;

	// フラグ(タイムスタンプタイプ)
	const int8_t	type = prefs->ip_ts_type == IP_TST_TSONLY
		? IPOPT_TS_TSONLY
		: IPOPT_TS_TSANDADDR;
	options[IPOPT_POS_OV_FLG] = type;

	// バッファの内容をIPオプションとしてセット
	if (setsockopt(sock, IPPROTO_IP, IP_OPTIONS, options, options[IPOPT_OLEN])) {
		print_special_error_by_errno("setsockopt(IP_OPTIONS) (ip timestamp)");
		return false;
	}
	return true;
}

// 各種オプションをセット
static bool	apply_socket_options(const t_preferences* prefs, int sock) {
	int one = 1; // NOTE: boolean を有効にするには 1 を指定する
	if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (char *)&one, sizeof(one))) {
		close(sock);
		print_special_error_by_errno("setsockopt(SO_BROADCAST)");
		return false;
	}

	// TTL設定
	if (prefs->ttl > 0) {
		if (setsockopt(sock, IPPROTO_IP, IP_TTL, &prefs->ttl, sizeof(prefs->ttl))) {
			print_special_error_by_errno("setsockopt(IP_TTL)");
			return false;
		}
	}

	// ToS設定
	if (prefs->tos >= 0) {
		if (setsockopt(sock, IPPROTO_IP, IP_TOS, &prefs->tos, sizeof(prefs->tos))) {
			print_special_error_by_errno("setsockopt(IP_TOS)");
			return false;
		}
	}

	// ルーティングを無視する
	if (prefs->bypass_routing) {
		if (setsockopt(sock, SOL_SOCKET, SO_DONTROUTE, &one, sizeof(one))) {
			print_special_error_by_errno("setsockopt(SO_DONTROUTE)");
			return false;
		}
	}

	// IPタイムスタンプオプションをセット
	if (prefs->ip_ts_type != IP_TST_NONE) {
		if (!set_sockopt_timestamp(prefs, sock)) {
			return false;
		}
	}

	// ソースアドレス固定
	if (prefs->given_source_address) {
		// ソースアドレスをアドレス構造体に変換
		address_info_t* source_address = resolve_str_into_address(prefs->given_source_address);
		if (source_address == NULL) {
			print_special_error_by_errno("source addr failed");
			return false;
		}
		// ソケットにソースアドレスを設定
		int	rv = bind(sock, source_address->ai_addr, source_address->ai_addrlen);
		freeaddrinfo(source_address);
		if (rv) {
			print_special_error_by_errno("bind");
			return false;
		}
		DEBUGINFO("binded sock %d to %s", sock, prefs->given_source_address);
	}
	return true;
}

static bool	accessible_ipheader(void) {
#ifdef __APPLE__
	return false;
#else
	return true;
#endif
}

// ICMPソケットを作成し, そのFDを返す.
// 失敗した場合は負の値を返す.
int create_icmp_socket(bool* inaccessible_ipheader, const t_preferences* prefs) {
	// ソケット生成
	errno = 0;
	int sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if (sock < 0) {
		if (errno != EPERM && errno != EACCES) {
			print_special_error_by_errno("socket");
			return -1;
		}
		// SOCK_RAW で失敗した場合は SOCK_DGRAM で試す
		DEBUGINFO("%s", "SOCK_RAW failed; try SOCK_DGRAM");
		sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_ICMP);
		if (sock < 0) {
			print_special_error_by_errno("socket");
			return -1;
		}
		*inaccessible_ipheader = accessible_ipheader();
	}

	if (!apply_socket_options(prefs, sock)) {
		close(sock);
		return -1;
	}

	return sock;
}
