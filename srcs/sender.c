#include "ping.h"

extern int	g_is_little_endian;

static size_t	set_timestamp_for_data(uint8_t* data_buffer, size_t buffer_len) {
	if (buffer_len < sizeof(timeval_t)) { return 0; }
	timeval_t	tm;
	gettimeofday(&tm, NULL);
	ft_memcpy(data_buffer, &tm, sizeof(timeval_t));
	return sizeof(timeval_t);
}

static void	deploy_datagram(
	const t_ping* ping,
	uint8_t* datagram_buffer,
	size_t datagram_len,
	uint16_t sequence
) {
	// ASSERT(datagram_len >= sizeof(icmp_header_t));
	icmp_header_t*	icmp_hd = (icmp_header_t*)datagram_buffer;
	uint8_t*		icmp_dt = (void*)datagram_buffer + sizeof(icmp_header_t);
	const size_t	icmp_dt_size = datagram_len - sizeof(icmp_header_t);

	icmp_hd->ICMP_HEADER_TYPE = ICMP_ECHO_REQUEST;
	icmp_hd->ICMP_HEADER_CODE = 0;
	icmp_hd->ICMP_HEADER_ECHO.ICMP_HEADER_ID = ping->icmp_header_id;
	icmp_hd->ICMP_HEADER_ECHO.ICMP_HEADER_SEQ = sequence;

	// エンディアン変換
	icmp_hd->ICMP_HEADER_TYPE = SWAP_NEEDED(icmp_hd->ICMP_HEADER_TYPE);
	icmp_hd->ICMP_HEADER_CODE = SWAP_NEEDED(icmp_hd->ICMP_HEADER_CODE);
	icmp_hd->ICMP_HEADER_ECHO.ICMP_HEADER_ID = SWAP_NEEDED(icmp_hd->ICMP_HEADER_ECHO.ICMP_HEADER_ID);
	icmp_hd->ICMP_HEADER_ECHO.ICMP_HEADER_SEQ = SWAP_NEEDED(icmp_hd->ICMP_HEADER_ECHO.ICMP_HEADER_SEQ);

	size_t	data_offset = 0;

	// 可能なら先頭部分を現在時刻で埋める
	data_offset += set_timestamp_for_data(icmp_dt, icmp_dt_size);

	// データフィールドをASCIIで埋める
	// TODO: 全体で共通化
	// TODO: パターンが与えられた場合はパターンを使う
	for (size_t i = data_offset, j = 0; i < ICMP_ECHO_DATA_SIZE; i++, j++) {
		icmp_dt[i] = j;
	}

	// 最後にチェックサム計算
	icmp_hd->ICMP_HEADER_CHECKSUM = derive_icmp_checksum(datagram_buffer, ICMP_ECHO_DATAGRAM_SIZE);
	DEBUGOUT("checksum: %d", icmp_hd->ICMP_HEADER_CHECKSUM);
}

// ICMP Echo Request を送信
int	send_request(
	t_ping* ping,
	const socket_address_in_t* addr,
	uint16_t sequence
) {
	uint8_t datagram_buffer[ICMP_ECHO_DATAGRAM_SIZE] = {0};
	deploy_datagram(ping, datagram_buffer, sizeof(datagram_buffer), sequence);

	int rv = sendto(
		ping->socket_fd,
		datagram_buffer, ICMP_ECHO_DATAGRAM_SIZE,
		0,
		(struct sockaddr *)addr, sizeof(socket_address_in_t)
	);
	DEBUGOUT("sendto rv: %d", rv);
	if (rv < 0) {
		DEBUGOUT("errno: %d (%s)", errno, strerror(errno));
		return rv;
	}
	ping->stat_data.packets_sent += 1;
	return rv;
}
