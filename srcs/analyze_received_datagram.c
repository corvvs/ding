#include "ping.h"

extern int	g_is_little_endian;

// predicate が満たされない場合, エラーメッセージを出力して false を返す
#define VALIDATE_CONDITION(predicate, format, ...) {\
	if (!(predicate)) {\
		DEBUGERR("[predicate failed] %s: " format , #predicate, __VA_ARGS__);\
		return false;\
	}\
}

static bool	validate_received_len(size_t received_size) {
	// 少なくともIPヘッダを含むことができる程度のサイズを持つことを確認
	VALIDATE_CONDITION(received_size >= sizeof(ip_header_t), "%zu < %zu", received_size, sizeof(ip_header_t));
	return true;
}

// IPレベルのバリデーション
static bool	validate_ip_is_reliable_header(
	const t_ping* ping,
	size_t received_size,
	const ip_header_t* ip_header
) {
	// NOTE: やるべきこと
	// IPヘッダに記述されている情報が信頼できるかどうかを確認する.
	// 具体的には...
	//
	// 以下の一次情報が与えられる:
	// - received_size: 受信サイズ
	// - ip_header: IPヘッダ(= 受信バッファの先頭)
	// 
	// これらの一次情報を使い, 以下の二次情報が信頼できることを確かめる:
	// - IPヘッダの Version に記述されているIPプロトコルのバージョン
	//   - 4 である
	// - IPヘッダの IHL に記述されているIPヘッダサイズ
	//   - 最小値(20オクテット)以上である
	//   - 受信サイズ以下である
	// - IPヘッダの Total Length に記述されているIPデータグラムサイズ
	//   - 受信サイズと一致する
	//   - IPヘッダ + ICMPヘッダ を含むのに十分大きい
	// 
	// NOTE: IPヘッダチェックサムは検証しない
	// NOTE: このレベルのバリデーションはOSカーネルを信頼するならば不要なのかもしれない
	// (そして普通は信頼する)

	// IPプロトコルバージョンが4であることを確認
	VALIDATE_CONDITION(ip_header->IP_HEADER_VER == IP_SUPPORTED_VERSION, "header version: %u", ip_header->IP_HEADER_VER);

	const size_t	ip_header_len = ihl_to_octets(ip_header->IP_HEADER_HL);
	DEBUGOUT("ip_header_len: %zu", ip_header_len);
	// IHL(IPヘッダサイズ)が最小値以上であることを確認
	VALIDATE_CONDITION(ip_header_len >= sizeof(ip_header_t), "IHL: %u", ip_header->IP_HEADER_HL);

	// IPヘッダサイズが受信サイズに収まっていることを確認
	VALIDATE_CONDITION(ip_header_len <= received_size, "ip_header_len: %zu, received_size: %zu", ip_header_len, received_size);

	// NOTE: OSがIPヘッダの Total Length を勝手に変えてきた時に帳尻を合わせるためのオフセット
	// たとえば macOS においては, Total Length が IPヘッダサイズを含まない値に改変される.
	const size_t	header_totalsize_offset = ping->received_ipheader_modified
		? sizeof(ip_header_t)
		: 0;
	const size_t	ip_total_length = ip_header->IP_HEADER_TOTLEN;
	DEBUGOUT("header_totalsize_offset: %zu", header_totalsize_offset);
	DEBUGOUT("ip_total_length: %zu", ip_total_length);
	// 受信サイズ(一次情報) == IPヘッダのTotal Length(二次情報) であることを確認
	const size_t	tuned_total_length = ip_total_length + header_totalsize_offset;
	VALIDATE_CONDITION(received_size == tuned_total_length, "rs: %zu, tot_len: %zu", received_size, tuned_total_length);

	// Total Length がIPヘッダとICMPヘッダを含むのに十分大きい
	// NOTE: ただし Total Length 自体は変更されている可能性があるので, 受信サイズを使って確認する
	const size_t	least_datagram_len = ip_header_len + sizeof(icmp_header_t);
	VALIDATE_CONDITION(received_size >= least_datagram_len, "rs: %zu, least_len: %zu", received_size, least_datagram_len);
	return true;
}

static bool	validate_icmp_is_expected_type(
	const t_ping* ping,
	const icmp_header_t* icmp_header
) {
	(void)ping;

	DEBUGOUT("ICMP_HEADER_TYPE: %u", icmp_header->ICMP_HEADER_TYPE);
	switch (icmp_header->ICMP_HEADER_TYPE) {
		case ICMP_TYPE_ECHO_REPLY:
		case ICMP_TYPE_TIME_EXCEEDED:
			return true;
		default:
			return false;
	}
}

// ICMPタイプに依存したIPレベルのバリデーション
static bool	validate_ip_icmp_type_specific(
	const t_ping* ping,
	const ip_header_t* ip_header,
	const icmp_header_t* icmp_header
) {
	if (ping->inaccessible_ipheader) { return true; }
	switch (icmp_header->ICMP_HEADER_TYPE) {
		case ICMP_TYPE_ECHO_REPLY: {
			// Reply の送信元 == Request の送信先であることを確認
			const socket_address_in_t* address_to = &ping->target.address_to;
			const uint32_t	address_request_to = address_to->sin_addr.s_addr;
			const uint32_t	address_reply_from = serialize_address(&ip_header->IP_HEADER_SRC);
			DEBUGOUT("address_request_to: %s", stringify_serialized_address(address_request_to));
			DEBUGOUT("address_reply_from: %s", stringify_serialized_address(address_reply_from));
			if (address_request_to != address_reply_from) {
				DEBUGWARN("address_request_to != address_reply_from: %u != %u", address_request_to, address_reply_from);
				// NOTE: エラーにはしない
			}
			break;
		}
		case ICMP_TYPE_TIME_EXCEEDED: {
			// ICMPデータグラムサイズが最低限情報(IPヘッダ+ICMPヘッダ)を含めるほど大きいことを確認
			const size_t	datagram_len = ip_header->IP_HEADER_TOTLEN;
			const size_t	minimum_header_len = sizeof(ip_header_t) + sizeof(icmp_header_t);
			const size_t	minimum_embedded_len = sizeof(ip_header_t) + sizeof(icmp_header_t);
			const size_t	least_datagram_len = minimum_header_len + minimum_embedded_len;
			VALIDATE_CONDITION(datagram_len >= least_datagram_len, "dg_len: %zu, least_dg_len: %zu", datagram_len, least_datagram_len);
			break;
		}
	}
	return true;
}

// ICMPタイプに依存したICMPレベルのバリデーション
static bool	validate_icmp_icmp_type_specific(
	const t_ping* ping,
	icmp_header_t* icmp_header,
	size_t icmp_datagram_size
) {

	if (icmp_header->ICMP_HEADER_TYPE == ICMP_TYPE_ECHO_REPLY) {
		// ヘッダのID値が期待と一致することを確認
		// NOTE:データグラムソケットを使っている場合, こちらが指定したIDをカーネルが書き換えてしまうので, このチェックは飛ばす
		if (!ping->inaccessible_ipheader) {
			uint16_t	received_id = icmp_header->ICMP_HEADER_ECHO.ICMP_HEADER_ID;
			received_id = SWAP_NEEDED(received_id);
			VALIDATE_CONDITION(received_id == ping->icmp_header_id, "received_id: %u, expected_id: %u", received_id, ping->icmp_header_id);
		}
	}

	// チェックサムが正しいことを確認
	if (!is_valid_icmp_checksum(ping, icmp_header, icmp_datagram_size)) {
		DEBUGWARN("%s", "checksum is bad");
		// NOTE: 違っていてもエラーにはしない
	}

	return true;
}

// 受信データグラムを解析し, 検証する.
// NOTE: 検証の過程でエンディアン変換も行う.
bool	analyze_received_datagram(const t_ping* ping, t_acceptance* acceptance) {
	// 受信サイズのバリデーション
	if (!validate_received_len(acceptance->received_len)) {
		return false;
	}
	ip_header_t*	ip_header = (ip_header_t*)acceptance->recv_buffer;

	// IPヘッダの信頼性チェック
	if (!ping->inaccessible_ipheader) {
		// NOTE: データグラムソケットを使っている場合IPヘッダを受信できないので, ここのチェックは飛ばす
		if (!ping->received_ipheader_modified) {
			flip_endian_ip(ip_header);
		}
		if (!validate_ip_is_reliable_header(ping, acceptance->received_len, ip_header)) {
			return false;
		}
	}

	const size_t	ip_header_len = ping->inaccessible_ipheader
		? 0
		: ihl_to_octets(ip_header->IP_HEADER_HL);
	icmp_header_t*	icmp_header = (icmp_header_t*)(acceptance->recv_buffer + ip_header_len);
	// ICMPタイプがサポート対象であることを確認
	if (!validate_icmp_is_expected_type(ping, icmp_header)) {
		return false;
	}
	// ICMPタイプに応じたIPレベルのチェック
	if (!validate_ip_icmp_type_specific(ping, ip_header, icmp_header)) {
		return false;
	}
	// ICMPタイプに応じたICMPレベルのチェック
	const size_t	icmp_datagram_size = acceptance->received_len - ip_header_len;
	if (!validate_icmp_icmp_type_specific(ping, icmp_header, icmp_datagram_size)) {
		return false;
	}
	flip_endian_icmp(icmp_header);
	acceptance->ip_header = ip_header;
	acceptance->icmp_datagram_size = icmp_datagram_size;
	acceptance->icmp_header = icmp_header;
	return true;
}
