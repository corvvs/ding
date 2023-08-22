#ifndef PING_TYPES_H
#define PING_TYPES_H

#include "ping_libs.h"

typedef struct timeval		timeval_t;
typedef struct addrinfo		address_info_t;
typedef struct sockaddr_in	socket_address_in_t;
typedef struct sockaddr		socket_address_t;

#ifdef __APPLE__

typedef struct ip ip_header_t;
#define IP_HEADER_VER ip_v
#define IP_HEADER_HL ip_hl
#define IP_HEADER_TOS ip_tos
#define IP_HEADER_TOTLEN ip_len
#define IP_HEADER_ID ip_id
#define IP_HEADER_OFF ip_off
#define IP_HEADER_TTL ip_ttl
#define IP_HEADER_PROT ip_p
#define IP_HEADER_SUM ip_sum
typedef struct in_addr address_in_t;
#define IP_HEADER_SRC ip_src
#define IP_HEADER_DST ip_dst

// macOS には struct icmphdr 相当の構造体定義がなさそうなので自前定義する
typedef struct ft_icmp_hdr {
	u_char  icmp_type;
	u_char  icmp_code;
	u_int16_t icmp_cksum;
	struct ft_ih_idseq {
		n_short icd_id;
		n_short icd_seq;
	} ih_idseq;
}	icmp_header_t;
#define ICMP_HEADER_TYPE icmp_type
#define ICMP_HEADER_CODE icmp_code
#define ICMP_HEADER_CHECKSUM icmp_cksum
#define ICMP_HEADER_ECHO ih_idseq
#define ICMP_HEADER_ID icd_id
#define ICMP_HEADER_SEQ icd_seq

typedef struct icmp icmp_detailed_header_t;
#define ICMP_DHEADER_TYPE icmp_type
#define ICMP_DHEADER_CODE icmp_code
#define ICMP_DHEADER_CHECKSUM icmp_cksum
#define ICMP_DHEADER_EMBEDDED_IP icmp_dun.id_ip.idi_ip

#define U64T "%llu"

#else

typedef struct iphdr ip_header_t;
#define IP_HEADER_VER version
#define IP_HEADER_HL ihl
#define IP_HEADER_TOS tos
#define IP_HEADER_TOTLEN tot_len
#define IP_HEADER_ID id
#define IP_HEADER_OFF frag_off
#define IP_HEADER_TTL ttl
#define IP_HEADER_PROT protocol
#define IP_HEADER_SUM check
typedef uint32_t address_in_t;
#define IP_HEADER_SRC saddr
#define IP_HEADER_DST daddr

typedef struct icmphdr icmp_header_t;
#define ICMP_HEADER_TYPE type
#define ICMP_HEADER_CODE code
#define ICMP_HEADER_CHECKSUM checksum
#define ICMP_HEADER_ECHO un.echo
#define ICMP_HEADER_ID id
#define ICMP_HEADER_SEQ sequence

typedef struct icmp icmp_detailed_header_t;
#define ICMP_DHEADER_TYPE icmp_type
#define ICMP_DHEADER_CODE icmp_code
#define ICMP_DHEADER_CHECKSUM icmp_cksum
#define ICMP_DHEADER_EMBEDDED_IP icmp_dun.id_ip.idi_ip

#define U64T "%lu"

#endif

#endif
