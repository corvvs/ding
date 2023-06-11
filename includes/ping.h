#ifndef PING_H
#define PING_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
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

typedef struct timeval		timeval_t;
typedef struct addrinfo		address_info_t;
typedef struct sockaddr_in	socket_address_in_t;

#include "compatibility.h"

#define ICMP_ECHO_DATAGRAM_SIZE 64
#define ICMP_ECHO_DATA_SIZE (ICMP_ECHO_DATAGRAM_SIZE - sizeof(icmp_header_t))

typedef enum e_received_result {
	RR_SUCCESS,
	RR_TIMEOUT,
	RR_INTERRUPTED,
	RR_ERROR,
}	t_received_result;

#define	RECV_BUFFER_LEN 4096
// 受信データを管理する構造体
typedef struct s_acceptance {
	// 受信用バッファ
	uint8_t			recv_buffer[RECV_BUFFER_LEN];
	// 受信用バッファのサイズ
	size_t			recv_buffer_len;
	// 受信サイズ
	size_t			received_len;
	// IPヘッダ
	ip_header_t*	ip_header;
	// ICMPヘッダ
	icmp_header_t*	icmp_header;
	// ICMP全体サイズ
	size_t			icmp_whole_len;
	// 受信時刻
	timeval_t		epoch_received;
}	t_acceptance;

// 統計情報の元データを管理する構造体
typedef struct s_stat_data {
	// 送信済みパケット数
	size_t	packets_sent;
	// 受信済みパケット数
	size_t	packets_received;
	// ラウンドトリップ数
	double*	rtts;
	// ラウンドトリップ数のキャパシティ
	size_t	rtts_cap;
}	t_stat_data;

typedef struct s_preferences
{
	// verbose モード
	bool		verbose;
	// request 送信数
	size_t		count;
	// request TTL
	int			ttl;
	// preload 送信数
	size_t		preload;
	// 最後の request 送信後の待機時間(秒)
	uint64_t	wait_after_final_request_s;
	// pingセッションのタイムアウト時間(秒)
	uint64_t	session_timeout_s;
} t_preferences;

// ターゲット構造体
typedef struct s_target
{
	const char*				given_host;
	char					resolved_host[16];
	socket_address_in_t		addr_to;
} t_target;

// マスター構造体
typedef struct s_ping
{
	t_target		target;

	uint16_t		icmp_header_id;
	int				socket_fd;
	t_stat_data		stat_data;
	timeval_t		start_time;
	t_preferences	prefs;
} t_ping;

// option.c
int	parse_option(int argc, char** argv, t_preferences* pref);
t_preferences	default_preferences(void);

// address.c
int	retrieve_target(const char* host, t_target* target);
uint32_t	serialize_address(const address_in_t* addr);
const char*	stringify_serialized_address(uint32_t addr32);
const char*	stringify_address(const address_in_t* addr);

// socket.c
int create_icmp_socket(const t_preferences* prefs);

// ping_pong.c
int	ping_pong(t_ping* ping);

// sender.c
int	send_request(
	t_ping* ping,
	const socket_address_in_t* addr,
	uint16_t sequence
);

// receiver.c
t_received_result	receive_reply(const t_ping* ping, t_acceptance* acceptance);

// ip.c
void	ip_convert_endian(void* mem);

// icmp.c
void		icmp_convert_endian(void* mem);
uint16_t	derive_icmp_checksum(const void* datagram, size_t len);

// unexpected_icmp.c
void	print_unexpected_icmp(t_acceptance* acceptance);

// endian.c
bool		is_little_endian(void);
uint16_t	swap_2byte(uint16_t value);
uint32_t	swap_4byte(uint32_t value);
uint64_t	swap_8byte(uint64_t value);

// validator.c
int	check_acceptance(t_ping* ping, t_acceptance* acceptance, const socket_address_in_t* addr_to);

// time.c
timeval_t	get_current_time(void);
timeval_t	add_times(const timeval_t* a, const timeval_t* b);
timeval_t	sub_times(const timeval_t* a, const timeval_t* b);
double		get_ms(const timeval_t* a);

// stats.c
double	mark_received(t_ping* ping, const t_acceptance* acceptance);
void	print_stats(const t_ping* ping);

// math.c
double	ft_square(double x);
double	ft_sqrt(double x);

// debug.c
void	debug_hexdump(const char* label, const void* mem, size_t len);
void	debug_msg_flags(const struct msghdr* msg);
void	debug_ip_header(const void* mem);
void	debug_icmp_header(const void* mem);

# define SWAP_BYTE(value) (sizeof(value) < 2 ? (value) : sizeof(value) < 4 ? swap_2byte(value) : sizeof(value) < 8 ? swap_4byte(value) : swap_8byte(value))
// エンディアン変換が必要(=環境のエンディアンがネットワークバイトオーダーではない)ならエンディアン変換を行う.
// (返り値が uint64_t になることに注意)
// (使用するファイルで extern int	g_is_little_endian; すること)
# define SWAP_NEEDED(value) (g_is_little_endian ? SWAP_BYTE(value) : (value))

#endif
