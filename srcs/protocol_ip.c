#include "ping.h"
extern int	g_is_little_endian;


// mem をIPヘッダとして, 必要ならエンディアン変換を行う
// ※ IPオプションは考慮しないので必要ならその都度変換すること
void	flip_endian_ip(ip_header_t* ip_hd) {
	if (!g_is_little_endian) { return; }

	ip_hd->IP_HEADER_TOS    = SWAP_NEEDED(ip_hd->IP_HEADER_TOS);
	ip_hd->IP_HEADER_TOTLEN = SWAP_NEEDED(ip_hd->IP_HEADER_TOTLEN);
	ip_hd->IP_HEADER_OFF    = SWAP_NEEDED(ip_hd->IP_HEADER_OFF);
	ip_hd->IP_HEADER_TTL    = SWAP_NEEDED(ip_hd->IP_HEADER_TTL);
	ip_hd->IP_HEADER_PROT   = SWAP_NEEDED(ip_hd->IP_HEADER_PROT);
	// NOTE: IPアドレスはエンディアン変換の対象としないのが普通
	// ip_hd->IP_HEADER_ID =   SWAP_NEEDED(ip_hd->IP_HEADER_ID);
	// ip_hd->IP_HEADER_SUM =  SWAP_NEEDED(ip_hd->IP_HEADER_SUM);
	// NOTE: ID, ヘッダーチェックサムをエンディアン変換すると
	// 特殊な状況でICMPチェックサムが合わなくなるので除外
}

size_t	ihl_to_octets(size_t ihl) {
	return ihl * OCTETS_IN_32BITS;
}
