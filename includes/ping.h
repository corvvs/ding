#ifndef PING_H
#define PING_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/ip_icmp.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <errno.h>
#include "libft.h"
#include "common.h"

#define ICMP_ECHO_REQUEST 8
#define ICMP_ECHO_REPLY 0

#ifdef __APPLE__

typedef struct icmp	icmp_header_t;
#define ICMP_HEADER_TYPE     icmp_type
#define ICMP_HEADER_CODE     icmp_code
#define ICMP_HEADER_CHECKSUM icmp_cksum
#define ICMP_HEADER_ECHO     icmp_hun.ih_idseq
#define ICMP_HEADER_ID       icd_id
#define ICMP_HEADER_SEQ      icd_seq

#else

typedef struct icmphdr	icmp_header_t;
#define ICMP_HEADER_TYPE     type
#define ICMP_HEADER_CODE     code
#define ICMP_HEADER_CHECKSUM checksum
#define ICMP_HEADER_ECHO     un.echo
#define ICMP_HEADER_ID       id
#define ICMP_HEADER_SEQ      sequence

#endif

#define ICMP_ECHO_DATAGRAM_SIZE 64
#define ICMP_ECHO_DATA_SIZE (ICMP_ECHO_DATAGRAM_SIZE - sizeof(icmp_header_t))

typedef struct s_options
{

} t_options;

typedef struct s_ping
{
	const char*	target;
	int			socket_fd;

	t_options options;
} t_ping;

#endif
