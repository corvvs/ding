#include "ping.h"

void	print_unexpected_icmp(t_acceptance* acceptance) {
	const ip_header_t*	ip_header = acceptance->ip_header;
	icmp_header_t*		icmp_header = acceptance->icmp_header;
	const size_t		icmp_whole_len = acceptance->icmp_whole_len;
	switch (icmp_header->ICMP_HEADER_TYPE) {
		case ICMP_ECHOREPLY:
			break;
		case ICMP_TIMXCEED: {
			// TODO: Check Code == 0
			icmp_detailed_header_t*	dicmp = (icmp_detailed_header_t*)icmp_header;
			ip_header_t*			original_ip = (ip_header_t*)&(dicmp->ICMP_DHEADER_ORIGINAL_IP);
			const size_t	original_ip_whole_len = icmp_whole_len - ((void*)original_ip - (void*)dicmp);
			const size_t	original_ip_header_len = original_ip->IP_HEADER_HL * 4;
			icmp_header_t*	original_icmp = (icmp_header_t*)((void*)original_ip + original_ip_header_len);
			flip_endian_ip(original_ip);
			flip_endian_icmp(original_icmp);
			// TODO: チェックサム
			debug_ip_header(original_ip);
			debug_icmp_header(original_icmp);
			const size_t	original_icmp_whole_len = original_ip_whole_len - original_ip_header_len;

			dprintf(
				STDERR_FILENO,
				"%zu bytes from %s: Time to live exeeded\n",
				original_icmp_whole_len,
				stringify_address(&ip_header->IP_HEADER_SRC)
			);
		}
		default:
			break;
	}

}
