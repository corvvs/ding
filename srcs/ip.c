#include "ping.h"
extern int	g_is_little_endian;

// mem をIPヘッダとして, 必要ならエンディアン変換を行う
void	ip_convert_endian(void* mem) {
	if (!g_is_little_endian) { return; }

	ip_header_t*	ip_hd = (ip_header_t*)mem;
	ip_hd->tos = SWAP_NEEDED(ip_hd->tos);
	ip_hd->tot_len = SWAP_NEEDED(ip_hd->tot_len);
	ip_hd->id = SWAP_NEEDED(ip_hd->id);
	ip_hd->frag_off = SWAP_NEEDED(ip_hd->frag_off);
	ip_hd->ttl = SWAP_NEEDED(ip_hd->ttl);
	ip_hd->protocol = SWAP_NEEDED(ip_hd->protocol);
	ip_hd->check = SWAP_NEEDED(ip_hd->check);
	ip_hd->saddr = SWAP_NEEDED(ip_hd->saddr);
	ip_hd->saddr = SWAP_NEEDED(ip_hd->saddr);
}
