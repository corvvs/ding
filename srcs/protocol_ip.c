#include "ping.h"
extern int	g_is_little_endian;


// mem をIPヘッダとして, 必要ならエンディアン変換を行う
// ※ IPオプションは考慮しないので必要ならその都度変換すること
void	flip_endian_ip(void* mem) {
	if (!g_is_little_endian) { return; }

	ip_header_t*	ip_hd = (ip_header_t*)mem;
	ip_hd->IP_HEADER_TOS =  SWAP_NEEDED(ip_hd->IP_HEADER_TOS);
	ip_hd->IP_HEADER_LEN =  SWAP_NEEDED(ip_hd->IP_HEADER_LEN);
	ip_hd->IP_HEADER_ID =   SWAP_NEEDED(ip_hd->IP_HEADER_ID);
	ip_hd->IP_HEADER_OFF =  SWAP_NEEDED(ip_hd->IP_HEADER_OFF);
	ip_hd->IP_HEADER_TTL =  SWAP_NEEDED(ip_hd->IP_HEADER_TTL);
	ip_hd->IP_HEADER_PROT = SWAP_NEEDED(ip_hd->IP_HEADER_PROT);
	ip_hd->IP_HEADER_SUM =  SWAP_NEEDED(ip_hd->IP_HEADER_SUM);
	// NOTE: IPアドレスはエンディン変換の対象としないのが普通
}
