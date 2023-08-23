#ifndef PING_CONSTANTS_H
#define PING_CONSTANTS_H

#include "ping_libs.h"
#include "ping_types.h"

#define PROGRAM_NAME "ping"
#define ICMP_TYPE_ECHO_REQUEST 0x8
#define ICMP_TYPE_ECHO_REPLY 0x0
#define ICMP_TYPE_TIME_EXCEEDED 0xb

#define MIN_IHL 5
#define MAX_IHL 15
#define IP_SUPPORTED_VERSION 4

#define OCTETS_IN_32BITS 4
#define MIN_IPV4_HEADER_SIZE (MIN_IHL * OCTETS_IN_32BITS)
#define MAX_IPV4_HEADER_SIZE (MAX_IHL * OCTETS_IN_32BITS)
#ifndef MAX_IPOPTLEN
#define MAX_IPOPTLEN (MAX_IPV4_HEADER_SIZE - MIN_IPV4_HEADER_SIZE)
#endif
#ifndef IPOPT_POS_OV_FLG
#define IPOPT_POS_OV_FLG 3
#endif

#define STATUS_SUCCEEDED 0
#define STATUS_GENERIC_FAILED 1
#define STATUS_OPERAND_FAILED 64

// 正直この値の意味が今ひとつわかっていないのだが inetutils の定義を踏襲しておく
#define MAX_ICMP_SIZE 76

#define MAX_IPV4_DATAGRAM_SIZE ((1 << 16) - 1)

// ICMP データグラムのデータ部分の最大サイズ
// = `-s` オプションの最大サイズ
#define MAX_ICMP_DATASIZE (MAX_IPV4_DATAGRAM_SIZE - MAX_IPV4_HEADER_SIZE - MAX_ICMP_SIZE)

#define ICMP_ECHO_DEFAULT_DATAGRAM_SIZE (64 - 8)

// `-p` オプションで指定できるデータパターンの最大長
#define MAX_DATA_PATTERN_LEN 16

// デフォルトのping(Echo)送信間隔
#define PING_DEFAULT_INTERVAL_S 1
// flooding時のping(Echo)送信間隔
#define TV_PING_FLOOD_INTERVAL \
	(timeval_t) { .tv_sec = 0, .tv_usec = 10000 }
// 間隔をあけない(間隔ゼロ)時, ゼロの代わりに使う値
#define TV_NEARLY_ZERO \
	(timeval_t) { .tv_sec = 0, .tv_usec = 1000 }

#define RECV_BUFFER_LEN MAX_IPV4_DATAGRAM_SIZE

#define SEQ_BITFIELD_LEN ((1 << 16) / 8)

#endif
