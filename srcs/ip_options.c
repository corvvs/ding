#include "ping.h"
extern int	g_is_little_endian;

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
	ft_memcpy(ts_buffer, ip_header_options + IPOPT_MINOFF, ts_len); // メモリアライメント違反を避けるためにコピー
	for (size_t	i = 0; i < ts_len; i += unit_size) {
		if (is_first) { printf("TS:"); }
		size_t	j = i;
		// アドレスを表示
		if (type == IPOPT_TS_TSANDADDR) {
			// -n オプションが指定されている場合はホストを解決せず, アドレスのみを表示する.
			// そうでない場合はホストの解決を試み, 成功した場合はそれを表示する.
			uint32_t addr = *(uint32_t*)&ts_buffer[j];
			printf("\t");
			print_address(ping, addr);
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
