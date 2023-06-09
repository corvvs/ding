#include "ping.h"
extern int	g_is_little_endian;

// mem をIPヘッダとして, 必要ならエンディアン変換を行う
void	flip_endian_icmp(void* mem) {
	if (!g_is_little_endian) { return; }

	icmp_header_t*	hd = (icmp_header_t*)mem;
	hd->ICMP_HEADER_TYPE = SWAP_NEEDED(hd->ICMP_HEADER_TYPE);
	hd->ICMP_HEADER_CODE = SWAP_NEEDED(hd->ICMP_HEADER_CODE);
	hd->ICMP_HEADER_CHECKSUM = SWAP_NEEDED(hd->ICMP_HEADER_CHECKSUM);
	hd->ICMP_HEADER_ECHO.ICMP_HEADER_ID = SWAP_NEEDED(hd->ICMP_HEADER_ECHO.ICMP_HEADER_ID);
	hd->ICMP_HEADER_ECHO.ICMP_HEADER_SEQ = SWAP_NEEDED(hd->ICMP_HEADER_ECHO.ICMP_HEADER_SEQ);
}

// datagram に対して ICMP チェックサムを計算する
// WARNING: datagram はネットワークバイトオーダーになっている必要がある
uint16_t	derive_icmp_checksum(const void* datagram, size_t len) {
	uint32_t	sum = 0;
	uint16_t*	ptr = (uint16_t*)datagram;
	while (len > 0) {
		if (len >= sizeof(uint16_t)) {
			sum += *ptr;
			// DEBUGOUT("sum: %u %u", sum, *ptr);
			len -= sizeof(uint16_t);
			ptr += 1;
		} else {
			sum += *((uint8_t*)ptr) << 8;
			break;
		}
	}
	// 1の補数和にする
	sum = (sum >> 16) + (sum & 0xffff);
	sum = (sum >> 16) + sum;
	// 最後にもう1度1の補数を取る
	return (~sum) & 0xffff;
}

static size_t	set_timestamp_for_data(uint8_t* data_buffer, size_t buffer_len) {
	if (buffer_len < sizeof(timeval_t)) { return 0; }
	timeval_t	tm;
	gettimeofday(&tm, NULL);
	ft_memcpy(data_buffer, &tm, sizeof(timeval_t));
	return sizeof(timeval_t);
}

static void	fill_data_with_pattern(const t_ping* ping, uint8_t* icmp_dt, size_t data_offset) {
	if (*(ping->prefs.data_pattern)) {
		for (size_t i = data_offset, j = 0; i < ICMP_ECHO_DATA_SIZE; i++, j++) {
			if (!ping->prefs.data_pattern[j]) {
				j = 0;
			}
			icmp_dt[i] = ping->prefs.data_pattern[j];
		}
	} else {
		for (size_t i = data_offset, j = 0; i < ICMP_ECHO_DATA_SIZE; i++, j++) {
			icmp_dt[i] = j;
		}
	}
}

void	construct_icmp_datagram(
	const t_ping* ping,
	uint8_t* datagram_buffer,
	size_t datagram_len,
	uint16_t sequence
) {
	// ASSERT(datagram_len >= sizeof(icmp_header_t));
	icmp_header_t*	icmp_hd = (icmp_header_t*)datagram_buffer;
	uint8_t*		icmp_dt = (void*)datagram_buffer + sizeof(icmp_header_t);
	const size_t	icmp_dt_size = datagram_len - sizeof(icmp_header_t);
	*icmp_hd = (icmp_header_t) {
		.ICMP_HEADER_TYPE = ICMP_ECHO_REQUEST,
		.ICMP_HEADER_CODE = 0,
		.ICMP_HEADER_ECHO.ICMP_HEADER_ID = ping->icmp_header_id,
		.ICMP_HEADER_ECHO.ICMP_HEADER_SEQ = sequence,
	};
	flip_endian_icmp(icmp_hd);
	size_t	data_offset = 0;

	// データバッファを埋める
	data_offset += set_timestamp_for_data(icmp_dt, icmp_dt_size);
	fill_data_with_pattern(ping, icmp_dt, data_offset);

	// 最後にチェックサム計算
	icmp_hd->ICMP_HEADER_CHECKSUM = derive_icmp_checksum(datagram_buffer, ICMP_ECHO_DATAGRAM_SIZE);
}
