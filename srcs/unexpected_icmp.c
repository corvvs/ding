#include "ping.h"

static void	print_time_exceeded_line(const ip_header_t* ip_header, size_t original_icmp_whole_len) {
	dprintf(
		STDERR_FILENO,
		"%zu bytes from %s: Time to live exceeded\n",
		original_icmp_whole_len,
		stringify_address(&ip_header->IP_HEADER_SRC)
	);
}

void	print_unexpected_icmp(t_acceptance* acceptance) {
	const ip_header_t*	ip_header = acceptance->ip_header;
	icmp_header_t*		icmp_header = acceptance->icmp_header;
	const size_t		icmp_whole_len = acceptance->icmp_whole_len;
	switch (icmp_header->ICMP_HEADER_TYPE) {
		case ICMP_TIMXCEED: {
			icmp_detailed_header_t*	dicmp = (icmp_detailed_header_t*)icmp_header;
			ip_header_t*			original_ip = (ip_header_t*)&(dicmp->ICMP_DHEADER_ORIGINAL_IP);
			const size_t	original_ip_whole_len = icmp_whole_len - ((void*)original_ip - (void*)dicmp);
			const size_t	original_ip_header_len = original_ip->IP_HEADER_HL * 4;
			icmp_header_t*	original_icmp = (icmp_header_t*)((void*)original_ip + original_ip_header_len);

			const size_t	original_icmp_whole_len = original_ip_whole_len - original_ip_header_len;
			if (!is_valid_icmp_checksum(original_icmp, original_icmp_whole_len)) {
				dprintf(STDERR_FILENO, "warning: ICMP checksum is invalid\n");
			}

			flip_endian_ip(original_ip);
			flip_endian_icmp(original_icmp);

			print_time_exceeded_line(ip_header, original_icmp_whole_len);
		}
		default:
			break;
	}

}
