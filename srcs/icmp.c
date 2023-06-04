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
