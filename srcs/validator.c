#include "ping.h"

extern int	g_is_little_endian;

// 生の受信データについてのバリデーション
static int	validate_raw_data(size_t received_size) {
	if (received_size < sizeof(ip_header_t)) {
		DEBUGERR("received_size < sizeof(ip_header_t): %zu < %zu", received_size, sizeof(ip_header_t));
		return -1;
	}
	return 0;
}

// IPレベルのバリデーション
static int	validate_ip_preliminary(
	const t_ping* ping,
	size_t received_size,
	const ip_header_t* received_ip_header
) {

	// CHECK: バージョンが4であることを確認する
	if (received_ip_header->IP_HEADER_VER != IPV4_VER) {
		DEBUGERR("received_ip_header->IP_HEADER_VER != 4: %u != 4", received_ip_header->IP_HEADER_VER);
		return -1;
	}

	// CHECK: 受信サイズ == トータルサイズ であることを確認する
	// OSがIPヘッダの Total Length を勝手に変えてきた時に帳尻を合わせるためのオフセット
	const size_t header_totalsize_offset = ping->received_ipheader_modified ? sizeof(ip_header_t) : 0;
	if (received_size != received_ip_header->IP_HEADER_LEN + header_totalsize_offset) {
		DEBUGERR("size doesn't match: rv: %zu(%zx), tot_len: %u", received_size, received_size, received_ip_header->IP_HEADER_LEN);
		return -1;
	}
	const size_t	ip_header_len = received_ip_header->IP_HEADER_HL * 4;
	DEBUGOUT("ip_header_len: %zu", ip_header_len);
	// CHECK: ip_header_len >= sizeof(ip_header_t)
	if (received_size < ip_header_len) {
		DEBUGERR("received_size < ip_header_len: %zu < %zu", received_size, ip_header_len);
		return -1;
	}

	// CHECK: IPヘッダ以降の部分がICMPヘッダを保持するのに十分なサイズを持つ
	const size_t		icmp_whole_len = received_size  - ip_header_len;
	DEBUGOUT("icmp_whole_len: %zu", icmp_whole_len);
	if (icmp_whole_len < sizeof(icmp_header_t)) {
		DEBUGERR("icmp_whole_len < sizeof(icmp_header_t): %zu < %zu", icmp_whole_len, sizeof(icmp_header_t));
		return -1;
	}
	return 0;
}

// IPレベルのバリデーション
static int	validate_ip_detailed(
	const t_ping* ping,
	const ip_header_t* received_ip_header,
	const icmp_header_t* received_icmp_header,
	const socket_address_in_t* address_to
) {
	if (ping->inaccessible_ipheader) { return 0; }
	// CHECK: IP: Reply の送信元 == Request の送信先
	switch (received_icmp_header->ICMP_HEADER_TYPE) {
		case ICMP_TYPE_ECHO_REPLY: {
			const uint32_t	address_request_to = address_to->sin_addr.s_addr;
			const uint32_t	address_reply_from = serialize_address(&received_ip_header->IP_HEADER_SRC);
			DEBUGOUT("address_request_to: %s", stringify_serialized_address(address_request_to));
			DEBUGOUT("address_reply_from: %s", stringify_address(&received_ip_header->IP_HEADER_SRC));
			if (address_request_to != address_reply_from) {
				DEBUGWARN("address_request_to != address_reply_from: %u != %u", address_request_to, address_reply_from);
			}
			break;
		}
		case ICMP_TYPE_TIME_EXCEEDED: {
			const size_t	datagram_len = received_ip_header->IP_HEADER_LEN;
			const size_t	minimum_header_len = sizeof(ip_header_t) + sizeof(icmp_header_t);
			const size_t	minimum_embedded_len = sizeof(ip_header_t) + sizeof(icmp_header_t);
			if (datagram_len < minimum_header_len + minimum_embedded_len) {
				DEBUGERR("datagram length is too short: %zu < %zu", datagram_len, minimum_header_len + minimum_embedded_len);
				return -1;
			}
			break;
		}
	}
	return 0;
}

static int	validate_icmp_preliminary(
	const t_ping* ping,
	t_acceptance* acceptance
) {
	icmp_header_t*	received_icmp_header = acceptance->icmp_header;

	switch (received_icmp_header->ICMP_HEADER_TYPE) {
		case ICMP_TYPE_ECHO_REPLY:
		case ICMP_TYPE_TIME_EXCEEDED:
			return 0;
		default:
			// 特定のTypeについてはフォローアップを行う
			print_unexpected_icmp(ping, acceptance);
			return -1;
	}
}


static int	validate_icmp_detailed(
	const t_ping* ping,
	icmp_header_t* received_icmp_header,
	size_t icmp_whole_len
) {

	if (received_icmp_header->ICMP_HEADER_TYPE == ICMP_TYPE_ECHO_REPLY) {
		// CHECK: is ID correct?
		// データグラムソケットを使っている場合, こちらが指定したIDをカーネルが書き換えてしまうので, このチェックは飛ばす
		if (!ping->inaccessible_ipheader) {
			uint16_t	received_id = received_icmp_header->ICMP_HEADER_ECHO.ICMP_HEADER_ID;
			received_id = SWAP_NEEDED(received_id);
			if (received_id != ping->icmp_header_id) {
				DEBUGERR("received_id != my_id: %u != %u", received_id, ping->icmp_header_id);
				return -1;
			}
		}
	}

	// CHECK: checksum is good?
	// TODO:
	// 受信したものが Time Exceeded だった場合,
	// ICMPペイロード内のオリジナルIPヘッダもエンディアン変換されているので,
	// そのままチェックサムを計算すると合わない
	// -> オリジナルIPヘッダのエンディアンを元に戻して計算する
	if (!is_valid_icmp_checksum(ping, received_icmp_header, icmp_whole_len)) {
		DEBUGWARN("%s", "checksum is bad");
	}

	return 0;
}

bool	analyze_received_datagram(const t_ping* ping, t_acceptance* acceptance) {
	const socket_address_in_t* address_to = &ping->target.address_to;
	if (validate_raw_data(acceptance->received_len)) {
		return false;
	}
	if (!ping->inaccessible_ipheader) {
		// NOTE: データグラムソケットを使っている場合IPヘッダを受信できないので, ここのチェックは飛ばす
		if (!ping->received_ipheader_modified) {
			flip_endian_ip(acceptance->recv_buffer);
		}
		if (validate_ip_preliminary(ping, acceptance->received_len, (ip_header_t*)acceptance->recv_buffer)) {
			return false;
		}
	}
	acceptance->ip_header = (ip_header_t*)acceptance->recv_buffer;
	const size_t	ip_header_len = ping->inaccessible_ipheader
		? 0
		: acceptance->ip_header->IP_HEADER_HL * 4;
	acceptance->icmp_whole_len = acceptance->received_len  - ip_header_len;
	acceptance->icmp_header = (icmp_header_t*)(acceptance->recv_buffer + ip_header_len);
	if (validate_icmp_preliminary(ping, acceptance)) {
		return false;
	}
	if (validate_ip_detailed(ping, acceptance->ip_header, acceptance->icmp_header, address_to)) {
		return false;
	}
	if (validate_icmp_detailed(ping, acceptance->icmp_header, acceptance->icmp_whole_len)) {
		return false;
	}
	flip_endian_icmp(acceptance->icmp_header);
	return true;
}
