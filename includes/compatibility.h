#ifndef COMPATIBILITY_H
#define COMPATIBILITY_H

#ifdef __APPLE__

typedef struct ip ip_header_t;
#define IP_HEADER_VER ip_v
#define IP_HEADER_HL ip_hl
#define IP_HEADER_TOS ip_tos
#define IP_HEADER_LEN ip_len
#define IP_HEADER_ID ip_id
#define IP_HEADER_OFF ip_off
#define IP_HEADER_TTL ip_ttl
#define IP_HEADER_PROT ip_p
#define IP_HEADER_SUM ip_sum
typedef struct in_addr address_in_t;
#define IP_HEADER_SRC ip_src
#define IP_HEADER_DST ip_dst

typedef struct icmp icmp_header_t;
#define ICMP_HEADER_TYPE icmp_type
#define ICMP_HEADER_CODE icmp_code
#define ICMP_HEADER_CHECKSUM icmp_cksum
#define ICMP_HEADER_ECHO icmp_hun.ih_idseq
#define ICMP_HEADER_ID icd_id
#define ICMP_HEADER_SEQ icd_seq

typedef struct icmp icmp_detailed_header_t;
#define ICMP_DHEADER_TYPE icmp_type
#define ICMP_DHEADER_CODE icmp_code
#define ICMP_DHEADER_CHECKSUM icmp_cksum
#define ICMP_DHEADER_ORIGINAL_IP icmp_dun.id_ip.idi_ip

#define U64T "%llu"

#else

typedef struct iphdr ip_header_t;
#define IP_HEADER_VER version
#define IP_HEADER_HL ihl
#define IP_HEADER_TOS tos
#define IP_HEADER_LEN tot_len
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
#define ICMP_DHEADER_ORIGINAL_IP icmp_dun.id_ip.idi_ip

#define U64T "%lu"

#endif

#endif
