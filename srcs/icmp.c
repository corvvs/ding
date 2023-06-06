#include "ping.h"
extern int	g_is_little_endian;

// mem をIPヘッダとして, 必要ならエンディアン変換を行う
void	icmp_convert_endian(void* mem) {
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
