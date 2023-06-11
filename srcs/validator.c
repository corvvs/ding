#include "ping.h"

extern int	g_is_little_endian;

#define SET_VR(current, new)	current = (new > current) ? new : current

// バリデーションの結果は4種類:
// - VR_ACCEPTED: エラー表示なし, 受信カウントを増やす
// - (VR_WARNING:  エラー表示あり, 受信カウントを増やす -> バリデーション時にエラーを出した後, VR_ACCEPTED に変更する)
// - VR_IGNORED:  エラー表示なし, 受信カウントを増やさない
// - (VR_REJECTED: エラー表示あり, 受信カウントを増やさない -> バリデーション時にエラーを出した後, VR_IGNORED に変更する)

// 生の受信データについてのバリデーション
static int	validate_received_raw_data(size_t recv_size) {
	if (recv_size < sizeof(ip_header_t)) {
		DEBUGERR("recv_size < sizeof(ip_header_t): %zu < %zu", recv_size, sizeof(ip_header_t));
		return -1;
	}
	return 0;
}

// IPレベルのバリデーション
static int	validate_received_ip_preliminary(
	size_t recv_size,
	const ip_header_t* received_ip_header
) {

	// CHECK: バージョンが4であることを確認する
	if (received_ip_header->IP_HEADER_VER != 4) {
		DEBUGERR("received_ip_header->IP_HEADER_VER != 4: %u != 4", received_ip_header->IP_HEADER_VER);
		return -1;
	}

	// CHECK: 受信サイズ == トータルサイズ であることを確認する
	if (recv_size != received_ip_header->IP_HEADER_LEN) {
		DEBUGERR("size doesn't match: rv: %zu, tot_len: %u", recv_size, received_ip_header->IP_HEADER_LEN);
		return -1;
	}
	const size_t	ip_header_len = received_ip_header->IP_HEADER_HL * 4;
	DEBUGOUT("ip_header_len: %zu", ip_header_len);
	// CHECK: ip_header_len >= sizeof(ip_header_t)
	if (recv_size < ip_header_len) {
		DEBUGERR("recv_size < ip_header_len: %zu < %zu", recv_size, ip_header_len);
		return -1;
	}

	// CHECK: IPヘッダ以降の部分がICMPヘッダを保持するのに十分なサイズを持つ
	const size_t		icmp_whole_len = recv_size  - ip_header_len;
	DEBUGOUT("icmp_whole_len: %zu", icmp_whole_len);
	if (icmp_whole_len < sizeof(icmp_header_t)) {
		DEBUGERR("icmp_whole_len < sizeof(icmp_header_t): %zu < %zu", icmp_whole_len, sizeof(icmp_header_t));
		return -1;
	}
	return 0;
}


// IPレベルのバリデーション
static int	validate_received_ip_echo_reply(
	const ip_header_t* received_ip_header,
	const socket_address_in_t* addr_to
) {

	// CHECK: IP: Reply の送信元 == Request の送信先
	const uint32_t	address_request_to = SWAP_NEEDED(addr_to->sin_addr.s_addr);
	const uint32_t	address_reply_from = serialize_address(&received_ip_header->IP_HEADER_SRC);
	DEBUGOUT("address_request_to: %s", stringify_serialized_address(address_request_to));
	DEBUGOUT("address_reply_from: %s", stringify_address(&received_ip_header->IP_HEADER_SRC));
	if (address_request_to != address_reply_from) {
		DEBUGWARN("address_request_to != address_reply_from: %u != %u", address_request_to, address_reply_from);
	}
	return 0;
}

static int	validate_received_icmp_preliminary(
	const t_ping* ping,
	t_acceptance* acceptance
) {
	(void)ping;
	icmp_header_t*	received_icmp_header = acceptance->icmp_header;

	// CHECK: Type == 0?
	if (received_icmp_header->ICMP_HEADER_TYPE != 0) {
		DEBUGOUT("received_icmp_header->ICMP_HEADER_TYPE != 0: %u != 0", received_icmp_header->ICMP_HEADER_TYPE);

		// 特定のTypeについてはフォローアップを行う
		print_unexpected_icmp(acceptance);
		return -1;
	}

	return 0;
}


static int	validate_received_icmp_echo_reply(
	const t_ping* ping,
	void* received_icmp,
	size_t icmp_whole_len
) {
	icmp_header_t*	received_icmp_header = (icmp_header_t*)received_icmp;

	// CHECK: is ID correct?
	uint16_t	received_id = received_icmp_header->ICMP_HEADER_ECHO.ICMP_HEADER_ID;
	uint16_t	my_id = SWAP_NEEDED(ping->icmp_header_id);
	if (received_id != my_id) {
		DEBUGERR("received_id != my_id: %u != %u", received_id, my_id);
		return -1;
	}

	// CHECK: checksum is good?
	uint16_t	received_checksum = received_icmp_header->ICMP_HEADER_CHECKSUM;
	received_icmp_header->ICMP_HEADER_CHECKSUM = 0;
	DEBUGOUT("icmp_whole_len: %zu", icmp_whole_len);
	uint16_t	derived_checksum = derive_icmp_checksum(received_icmp, icmp_whole_len);
	DEBUGOUT("checksum: received: %u derived: %u", received_checksum, derived_checksum);
	if (received_checksum != derived_checksum) {
		DEBUGWARN("%s", "checksum is bad");
	}

	// CHECK: is icmp_whole_len long enough?
	if (icmp_whole_len < sizeof(timeval_t)) {
		DEBUGERR("icmp_whole_len < sizeof(timeval_t): %zu < %zu", icmp_whole_len, sizeof(timeval_t));
		return -1;
	}

	return 0;
}

int	check_acceptance(t_ping* ping, t_acceptance* acceptance, const socket_address_in_t* addr_to) {
	// debug_hexdump("recv_buffer", acceptance->recv_buffer, acceptance->received_len);
	if (validate_received_raw_data(acceptance->received_len)) {
		return 1;
	}
	ip_convert_endian(acceptance->recv_buffer);
	// debug_ip_header(acceptance->recv_buffer);
	if (validate_received_ip_preliminary(acceptance->received_len, (ip_header_t*)acceptance->recv_buffer)) {
		return 1;
	}
	acceptance->ip_header = (ip_header_t*)acceptance->recv_buffer;
	const size_t		ip_header_len = acceptance->ip_header->IP_HEADER_HL * 4;
	acceptance->icmp_whole_len = acceptance->received_len  - ip_header_len;
	acceptance->icmp_header = (icmp_header_t*)(acceptance->recv_buffer + ip_header_len);
	if (validate_received_icmp_preliminary(ping, acceptance)) {
		return 1;
	}
	if (validate_received_ip_echo_reply((ip_header_t*)acceptance->recv_buffer, addr_to)) {
		return 1;
	}
	if (validate_received_icmp_echo_reply(ping, acceptance->icmp_header, acceptance->icmp_whole_len)) {
		return 1;
	}
	icmp_convert_endian(acceptance->icmp_header);
	// debug_icmp_header(acceptance->icmp_header);
	return 0;
}
