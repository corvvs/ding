#include "ping.h"
extern int	g_is_little_endian;

static char*	resolve_ipaddr_to_host(uint32_t addr) {
	static char			hostname[NI_MAXHOST];
	const char*			addr_str = stringify_serialized_address(addr);
	socket_address_in_t	sa = { .sin_family = AF_INET };
	errno = 0;
	if (inet_pton(AF_INET, addr_str, &sa.sin_addr) != 1) {
		DEBUGWARN("failed to inet_pton for %s: %s", addr_str, strerror(errno));
		return NULL;
	}
	errno = 0;
	int rv = getnameinfo(
		(struct sockaddr*)&sa,
		sizeof(struct sockaddr_in),
		hostname,
		sizeof(hostname),
		NULL,
		0,
		NI_NAMEREQD
	);
	if (rv) {
		if (errno) {
			DEBUGWARN("failed to getnameinfo for %s: %s", addr_str, strerror(errno));
		}
		return NULL;
	}
	return hostname;
}

static void	print_address_within_timestamp(uint32_t addr, bool try_to_resolve_host) {
	if (try_to_resolve_host) {
		char*	hostname = resolve_ipaddr_to_host(addr);
		if (hostname != NULL) {
			printf("\t%s (%s)", hostname, stringify_address((const void*)&addr));
			return;
		}
	}
	printf("\t%s", stringify_address((const void*)&addr));
}

bool	validate_ip_timestamp_option(const t_acceptance* acceptance) {
	const size_t	ip_header_len = acceptance->ip_header->IP_HEADER_HL * 4;
	// IPヘッダオプションがない -> sayonara
	if (ip_header_len <= MIN_IPV4_HEADER_SIZE) { return false; }
	const size_t	ip_header_options_capacity = ip_header_len - MIN_IPV4_HEADER_SIZE;
	// IPヘッダのあまりサイズが4以下 -> sayonara
	if (ip_header_options_capacity <= 4) { return false; }
	const uint8_t*	ip_header_options = (void*)acceptance->ip_header + MIN_IPV4_HEADER_SIZE;
	// オプションの種類がIPタイムスタンプでない -> sayonara
	if (ip_header_options[IPOPT_OPTVAL] != IPOPT_TS) { return false; }
	const uint8_t	type = ip_header_options[IPOPT_POS_OV_FLG];
	// タイムスタンプ種別がサポートされたものではない -> sayonara
	if (type != IPOPT_TS_TSONLY && type != IPOPT_TS_TSANDADDR) { return false; }
	const size_t	unit_size = type == IPOPT_TS_TSONLY
		? sizeof(uint32_t)
		: (sizeof(uint32_t) + sizeof(uint32_t));
	const size_t	ip_header_options_len = ip_header_options[IPOPT_OLEN];
	// IPヘッダのサイズよりオプションのサイズがでかい -> sayonara
	if (ip_header_options_capacity < ip_header_options_len) { return false; }
	// オプションのサイズがタイムスタンプの入る余地がないほどちいさい -> sayonara
	if (ip_header_options_len <= IPOPT_MINOFF + unit_size) { return false; }
	// ポインタ
	const size_t	ip_header_options_pointer = ip_header_options[IPOPT_OFFSET];
	// ポインタが最小値よりも小さい -> sayonara
	if (ip_header_options_pointer <= IPOPT_MINOFF) { return false; }
	// ポインタがオプションのサイズよりも大きい -> sayonara
	// (データが満タンなら両者は一致し, 空きがあるならポインタが小さくなるはず)
	DEBUGOUT("ip_header_options_len: %zu", ip_header_options_len);
	DEBUGOUT("ip_header_options_pointer: %zu", ip_header_options_pointer);
	if (ip_header_options_len + 1 < ip_header_options_pointer) { return false; }
	return true;
}

// IPタイムスタンプを表示する
// ASSERT: IPタイムスタンプはオプションの先頭にある
void	print_ip_timestamp(
	const t_ping* ping,
	const t_acceptance* acceptance
) {
	if (!validate_ip_timestamp_option(acceptance)) { return; }

	const uint8_t*	ip_header_options = (void*)acceptance->ip_header + MIN_IPV4_HEADER_SIZE;
	const uint8_t	type = ip_header_options[IPOPT_POS_OV_FLG];
	const size_t	unit_size = type == IPOPT_TS_TSONLY
		? sizeof(uint32_t)
		: (sizeof(uint32_t) + sizeof(uint32_t));
	const size_t	ip_header_options_pointer = ip_header_options[IPOPT_OFFSET];
	
	// ここまで来てようやく表示に入る
	bool			is_first = true;
	const size_t	ts_len = ip_header_options_pointer - (IPOPT_MINOFF + 1);
	uint8_t			ts_buffer[ts_len];
	const bool		try_to_resolve_host = !ping->prefs.dont_resolve_addr_in_ip_ts;
	ft_memcpy(ts_buffer, ip_header_options + IPOPT_MINOFF, ts_len);
	for (size_t	i = 0; i < ts_len; i += unit_size) {
		if (is_first) { printf("TS:"); }
		size_t	j = i;
		// アドレスを表示
		if (type == IPOPT_TS_TSANDADDR) {
			// -n オプションが指定されている場合はホストを解決せず, アドレスのみを表示する.
			// そうでない場合はホストの解決を試み, 成功した場合はそれを表示する.
			uint32_t addr = *(uint32_t*)&ts_buffer[j];
			print_address_within_timestamp(addr, try_to_resolve_host);
			j += sizeof(uint32_t);
		}

		// タイムスタンプを表示
		uint32_t timestamp = *(uint32_t*)&ts_buffer[j];
		timestamp = SWAP_NEEDED(timestamp);
		printf("\t%u ms\n", timestamp);
		is_first = false;
	}
	printf("\n");
}
