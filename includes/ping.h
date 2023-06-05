#ifndef PING_H
#define PING_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <signal.h>
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

typedef struct iphdr ip_header_t;
typedef struct timeval timeval_t;
typedef struct addrinfo	address_info_t;
typedef struct sockaddr_in	socket_address_in_t;

#define ICMP_ECHO_DATAGRAM_SIZE 64
#define ICMP_ECHO_DATA_SIZE (ICMP_ECHO_DATAGRAM_SIZE - sizeof(icmp_header_t))

// 統計情報の元データを管理する構造体
typedef struct s_stat_data {
	// 送信済みパケット数
	size_t	packets_sent;
	// 受信済みパケット数
	size_t	packets_receipt;
	// ラウンドトリップ数
	double*	rtts;
	// ラウンドトリップ数のキャパシティ
	size_t	rtts_cap;
}	t_stat_data;

// オプション構造体
typedef struct s_options
{

} t_options;

// ターゲット構造体
typedef struct s_target
{
	const char*	given_host;
	char		resolved_host[16];
} t_target;

// マスター構造体
typedef struct s_ping
{
	t_target	target;

	int			socket_fd;
	t_stat_data	stat_data;
	t_options options;
} t_ping;

// host.c
int	resolve_host(t_target* target);

// ip.c
void	ip_convert_endian(void* mem);

// icmp.c
void	icmp_convert_endian(void* mem);

// endian.c
bool		is_little_endian(void);
uint16_t	swap_2byte(uint16_t value);
uint32_t	swap_4byte(uint32_t value);
uint64_t	swap_8byte(uint64_t value);

// time.c
timeval_t	get_current_time(void);
double		get_current_epoch_ms(void);

// stats.c
timeval_t	mark_sent(t_ping* ping);
double	mark_receipt(t_ping* ping, const timeval_t* epoch_sent);
void	print_stats(const t_ping* ping);

// math.c
double	ft_square(double x);
double	ft_sqrt(double x);

// debug.c
void	debug_hexdump(const char* label, const void* mem, size_t len);
void	debug_ip_header(const void* mem);
void	debug_icmp_header(const void* mem);

# define SWAP_BYTE(value) (sizeof(value) < 2 ? (value) : sizeof(value) < 4 ? swap_2byte(value) : sizeof(value) < 8 ? swap_4byte(value) : swap_8byte(value))
// エンディアン変換が必要(=環境のエンディアンがネットワークバイトオーダーではない)ならエンディアン変換を行う.
// (返り値が uint64_t になることに注意)
// (使用するファイルで extern int	g_is_little_endian; すること)
# define SWAP_NEEDED(value) (g_is_little_endian ? SWAP_BYTE(value) : (value))

#endif
