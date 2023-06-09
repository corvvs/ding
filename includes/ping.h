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
#include <sys/types.h>
#include <signal.h>
#include <errno.h>
#include "libft.h"
#include "common.h"

#define PROGRAM_NAME		"ping"
#define ICMP_ECHO_REQUEST	8
#define ICMP_ECHO_REPLY		0

typedef struct timeval		timeval_t;
typedef struct addrinfo		address_info_t;
typedef struct sockaddr_in	socket_address_in_t;

#include "compatibility.h"

#define ICMP_ECHO_DATAGRAM_SIZE 64
#define ICMP_ECHO_DATA_SIZE (ICMP_ECHO_DATAGRAM_SIZE - sizeof(icmp_header_t))

#define TV_PING_DEFAULT_INTERVAL	(timeval_t){ .tv_sec = 1, .tv_usec = 0 }
#define TV_PING_FLOOD_INTERVAL		(timeval_t){ .tv_sec = 0, .tv_usec = 10000 }
#define TV_NEARLY_ZERO				(timeval_t){ .tv_sec = 0, .tv_usec = 1000 }

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

#define MAX_DATA_PATTERN_LEN 16

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
	// データパターン
	char		data_pattern[MAX_DATA_PATTERN_LEN + 1];
	// flood
	bool		flood;
	// ユーザ指定送信元アドレス
	char*		given_source_address;
} t_preferences;

// ターゲット構造体
typedef struct s_target
{
	// 入力ホスト
	const char*			given_host;
	// 入力ホストの解決後IPアドレス文字列
	char				resolved_host[16];
	// IPアドレス構造体
	socket_address_in_t	addr_to;
	// 送受信統計
	t_stat_data			stat_data;
} t_target;

// マスター構造体
typedef struct s_ping
{
	// 宛先によらないパラメータ

	// 送信ソケット
	int				socket_fd;
	// pingのID
	uint16_t		icmp_header_id;
	// ping開始時刻
	timeval_t		start_time;
	// 設定
	t_preferences	prefs;

	// 宛先に依存するパラメータ
	t_target		target;
} t_ping;

// option.c
int				parse_option(int argc, char** argv, bool by_root, t_preferences* pref);
t_preferences	default_preferences(void);

// host_address.c
address_info_t*	resolve_str_into_address(const char* host_str);
int				setup_target_from_host(const char* host, t_target* target);
uint32_t		serialize_address(const address_in_t* addr);
const char*		stringify_serialized_address(uint32_t addr32);
const char*		stringify_address(const address_in_t* addr);

// socket.c
int create_icmp_socket(const t_preferences* prefs);

// ping_pong.c
int	ping_pong(t_ping* ping);

// ping_sender.c
int	send_request(t_ping* ping, uint16_t sequence);

// pong_receiver.c
t_received_result	receive_reply(const t_ping* ping, t_acceptance* acceptance);

// protocol_ip.c
void		flip_endian_ip(void* mem);

// protocol_icmp.c
void		flip_endian_icmp(void* mem);
uint16_t	derive_icmp_checksum(const void* datagram, size_t len);
void		construct_icmp_datagram(
	const t_ping* ping,
	uint8_t* datagram_buffer,
	size_t datagram_len,
	uint16_t sequence
);

// unexpected_icmp.c
void	print_unexpected_icmp(t_acceptance* acceptance);

// validator.c
int	check_acceptance(t_ping* ping, t_acceptance* acceptance);

// stats.c
double	mark_received(t_ping* ping, const t_acceptance* acceptance);
void	print_stats(const t_ping* ping);

// utils_math.c
double	ft_square(double x);
double	ft_sqrt(double x);

// utils_endian.c
bool		is_little_endian(void);
uint16_t	swap_2byte(uint16_t value);
uint32_t	swap_4byte(uint32_t value);
uint64_t	swap_8byte(uint64_t value);

// utils_time.c
timeval_t	get_current_time(void);
timeval_t	add_times(const timeval_t* a, const timeval_t* b);
timeval_t	sub_times(const timeval_t* a, const timeval_t* b);
double		get_ms(const timeval_t* a);
double		diff_times(const timeval_t* a, const timeval_t* b);

// utils_error.c
void	print_error_by_message(const char* message);
void	print_error_by_errno(void);
void	print_special_error_by_errno(const char* name);

// utils_debug.c
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
