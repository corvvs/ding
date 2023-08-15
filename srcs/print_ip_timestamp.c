#include "ping.h"
extern int	g_is_little_endian;

// NOTE: IPオプションが存在して, それがIPタイムスタンプとして解釈できるための必要条件
// 
// IPv4ヘッダの最小サイズは20oct(オクテット), 最大サイズは60oct
// (IHLの最大値が15なので, 15 * 4 = 60octが最大).
// 
// IPオプションは, もし存在するなら20oct以降から始まり, 60octまで続く.
// よってまず以下が必要:
// (a) IPヘッダのサイズが20octより大きい
//
// IPヘッダサイズから20octを引いたものをオプションの長さとする.
// IPタイムスタンプは, 固定部4octと可変部 (4oct or 8oct) * n からなる. ゆえに
// (b) オプションの長さが8oct以上である
// 
// オプションの先頭について,
// (c-1) IPタイムスタンプのタイプ(=68)に一致する
// (c-2) フラグの値が期待されるものに一致する
// 
// IPタイムスタンプの"レングス"は, IPタイムスタンプ全体のオクテット数をあらわす. 従って
// (d-1) レングスが4oct + 記録単位以上である
// (d-2) レングスがオプションの長さ以下である
// 
// 記録単位はアドレスを記録するか否かによって 8oct または 4oct になる. よって
// (d-3) レングスが 4 + 記録単位 * n (n >= 1) の形に書ける
// 
// IPタイムスタンプの"ポインタ"は, 使用済み記録領域の**次**のバイトを指す. 従って
// (e-1) ポインタが5以上である
// (e-2) ポインタがレングス + 1以下である
// (e-3) ポインタが 4 + 記録単位 * n + 1 (n >= 1) の形に書ける

static size_t	ts_unit_size(uint8_t ts_type) {
	return (ts_type == IPOPT_TS_TSONLY)
		? sizeof(uint32_t)
		: (sizeof(uint32_t) + sizeof(uint32_t));
}

// [a: IPヘッダにオプションがあるか?]
static bool	has_ip_option(const ip_header_t* ip_header) {
	const size_t	ip_header_len = ihl_to_octets(ip_header->IP_HEADER_HL);
	// (a) IPヘッダのサイズが20octより大きい
	return ip_header_len > MIN_IPV4_HEADER_SIZE;
}

// [b: オプションがIPタイムスタンプを載せられる程度に大きいか?]
static bool	has_enough_room_for_ip_ts(size_t ip_header_options_capacity) {
	// (b) オプションの長さが8oct以上である
	const size_t	least_timestamp_option_len = 8;
	return ip_header_options_capacity >= least_timestamp_option_len;
}

// [c: 含まれているオプションはIPタイムスタンプか?]
static bool	contains_ip_ts(const uint8_t* ip_header_options) {
	// (c-1) IPタイムスタンプのタイプ(=68)に一致する
	if (ip_header_options[IPOPT_OPTVAL] != IPOPT_TS) { return false; }

	// (c-2) フラグの値が期待されるものに一致する
	const uint8_t	type = ip_header_options[IPOPT_POS_OV_FLG];
	if (type != IPOPT_TS_TSONLY && type != IPOPT_TS_TSANDADDR) { return false; }

	return true;
}

// [d: オプション記載のレングスは正しそうか?]
static bool	is_valid_ip_ts_length(
	size_t ip_header_options_capacity,
	const uint8_t* ip_header_options
) {
	const uint8_t	type = ip_header_options[IPOPT_POS_OV_FLG];
	const size_t	ts_fixed_octet = 4;
	const size_t	ts_length = ip_header_options[IPOPT_OLEN];
	const size_t	unit_size = ts_unit_size(type);

	// (d-1) レングスが4oct + 記録単位以上である
	if (ts_length < ts_fixed_octet + unit_size) { return false; }

	// (d-2) レングスがオプションの長さ以下である
	if (ts_length > ip_header_options_capacity) { return false; }

	// (d-3) レングスが 4 + 記録単位 * n (n >= 1) の形に書ける
	//   <=> (レングス - 4) が 記録単位で割り切れる
	if ((ts_length - ts_fixed_octet) % unit_size != 0) { return false; }

	return true;
}

// [e: オプション記載のポインタは正しそうか?]
// NOTE: ここでの"ポインタ"はIPv4用語. Cのpointerではない.
static bool	is_valid_ip_ts_pointer(
	const uint8_t* ip_header_options
) {
	const uint8_t	type = ip_header_options[IPOPT_POS_OV_FLG];
	const size_t	ts_pointer = ip_header_options[IPOPT_OFFSET];

	// (e-1) ポインタが5以上である
	if (ts_pointer <= IPOPT_MINOFF) { return false; }

	// (e-2) ポインタがレングス + 1以下である
	// NOTE: データが満タンなら両者は一致し, 空きがあるならポインタが小さくなるはず
	const size_t	ts_length = ip_header_options[IPOPT_OLEN];
	DEBUGOUT("ts_length: %zu", ts_length);
	DEBUGOUT("ts_pointer: %zu", ts_pointer);
	if (ts_pointer > ts_length + 1) { return false; }

	// (e-3) ポインタが 4 + 記録単位 * n + 1 (n >= 1) の形に書ける
	//   <=> (ポインタ - 5) が 記録単位で割り切れる
	const size_t	unit_size = ts_unit_size(type);
	if ((ts_pointer - IPOPT_MINOFF - 1) % unit_size != 0) { return false; }

	return true;
}

bool	has_valid_ip_timestamp(const ip_header_t* ip_header) {
	if (!has_ip_option(ip_header)) { return false; }
	const size_t	ip_header_len = ihl_to_octets(ip_header->IP_HEADER_HL);
	const size_t	ip_header_options_capacity = ip_header_len - MIN_IPV4_HEADER_SIZE;

	if (!has_enough_room_for_ip_ts(ip_header_options_capacity)) { return false; }
	const uint8_t*	ip_header_options = (void*)ip_header + MIN_IPV4_HEADER_SIZE;
	// NOTE: これ以降は, IPタイムスタンプの4つの固定要素へのアクセスが保証される

	if (!contains_ip_ts(ip_header_options)) { return false; }

	if (!is_valid_ip_ts_length(ip_header_options_capacity, ip_header_options)) { return false; }

	if (!is_valid_ip_ts_pointer(ip_header_options)) { return false; }

	return true;
}

static void	print_address(const t_ping* ping, uint32_t addr) {
	printf("\t");
	print_address_serialized(ping, addr);
}

static void	print_timestamp(uint32_t timestamp) {
	printf("\t");
	printf("%u ms\n", (uint32_t)SWAP_NEEDED(timestamp));
}

// IPタイムスタンプを表示する
// ASSUMPTION: IPタイムスタンプはオプションの先頭にある
// ASSUMPTION: IPヘッダの固定部分(20oct)の内容は既に検証済みである
void	print_ip_timestamp(
	const t_ping* ping,
	const t_acceptance* acceptance
) {
	const ip_header_t*	ip_header = acceptance->ip_header;
	if (!has_valid_ip_timestamp(ip_header)) { return; }
	// これ以降, オプション内のレングス/ポインタの値は reliable と考えてよい.

	const uint8_t*	ip_header_options = (void*)acceptance->ip_header + MIN_IPV4_HEADER_SIZE;
	const uint8_t	ts_type = ip_header_options[IPOPT_POS_OV_FLG];
	const size_t	ts_pointer = ip_header_options[IPOPT_OFFSET];

	const size_t	ts_data_len = (ts_pointer - (IPOPT_MINOFF + 1)) / sizeof(uint32_t);
	uint32_t		ts_buffer[MAX_IPV4_HEADER_SIZE];

	// NOTE: メモリアライメント違反を避けるためにコピー
	ft_memcpy(ts_buffer, ip_header_options + IPOPT_MINOFF, ts_data_len * sizeof(uint32_t));

	for (size_t i = 0; i < ts_data_len; i += 1) {
		if (i == 0) { printf("TS:"); }
		// -n オプションが指定されている場合はホストを解決せず, アドレスのみを表示する.
		// そうでない場合はホストの解決を試み, 成功した場合はそれを表示する.

		if (ts_type == IPOPT_TS_TSANDADDR) {
			print_address(ping, ts_buffer[i]);
			i += 1;
		}

		print_timestamp(ts_buffer[i]);
	}
	printf("\n");
}
