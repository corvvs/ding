#ifndef PING_STRUCTURES_H
#define PING_STRUCTURES_H

#include "ping_libs.h"
#include "ping_types.h"
#include "ping_constants.h"

typedef enum e_received_result
{
	RR_SUCCESS,
	RR_TIMEOUT,
	RR_INTERRUPTED,
	RR_UNACCEPTABLE,
	RR_ERROR,
} t_received_result;

// 受信データを管理する構造体
typedef struct s_acceptance
{
	// 受信用バッファ
	uint8_t recv_buffer[RECV_BUFFER_LEN];
	// 受信用バッファのサイズ
	size_t recv_buffer_len;
	// 受信サイズ
	size_t received_len;
	// IPヘッダ
	ip_header_t *ip_header;
	// ICMPヘッダ
	icmp_header_t *icmp_header;
	// ICMPデータグラムサイズ
	size_t icmp_datagram_size;
	// 受信時刻
	timeval_t epoch_received;
	// 重複フラグ
	bool is_duplicate_sequence;
} t_acceptance;

// 統計情報の元データを管理する構造体
typedef struct s_stat_data
{
	// 送信済みパケット数
	size_t sent_icmps;
	// ICMP Echo Replyの数
	size_t received_echo_replies;
	// タイムスタンプつきで返ってきたICMP Echo Replyの数
	size_t received_echo_replies_with_ts;
	// 受信済みパケット数(Echo Reply以外も含む)
	size_t received_icmps;
	// ラウンドトリップタイム配列
	// NOTE: サイズは received_echo_replies_with_ts に等しい
	double *roundtrip_times;
	// ラウンドトリップタイム配列のキャパシティ
	size_t roundtrip_times_cap;
} t_stat_data;

typedef enum e_ip_timestamp_type
{
	IP_TST_NONE,
	IP_TST_TSONLY,
	IP_TST_TSADDR,
} t_ip_timestamp_type;

// 設定構造体
typedef struct s_preferences
{
	// verbose モード
	bool verbose;
	// request 送信数
	size_t count;
	// request TTL
	int ttl;
	// preload 送信数
	size_t preload;
	// ICMP Echo の送信間隔(秒)
	uint64_t echo_interval_s;
	// 最後の request 送信後の待機時間(秒)
	uint64_t wait_after_final_request_s;
	// pingセッションのタイムアウト時間(秒)
	uint64_t session_timeout_s;
	// ToS: Type of Service
	int tos;
	// データ部分のサイズ
	size_t data_size;
	// データパターン
	char data_pattern[MAX_DATA_PATTERN_LEN + 1];
	// IPタイムスタンプ種別
	t_ip_timestamp_type ip_ts_type;
	// 戻りパケット処理時にアドレスを解決するかどうか
	bool dont_resolve_addr_received;
	// flood
	bool flood;
	// ユーザ指定送信元アドレス
	char *given_source_address;
	// ルーティングを無視してパケットを送信する
	bool bypass_routing;
	// 受信したパケットを16進ダンプする
	bool hexdump_received;
	// 送信したパケットを16進ダンプする
	bool hexdump_sent;

	// これが有効な場合, pingを実行せずヘルプを表示して終了する
	bool show_usage;

	// これ以下は直接指定されず, 他の設定から誘導(算出)されるもの
	bool sending_timestamp;

	// パース時に消費した argv の要素数
	int parsed_arguments;
} t_preferences;

// ターゲット構造体
typedef struct s_session
{
	// 入力ホスト
	const char *given_host;
	// 入力ホストの解決後IPアドレス文字列
	char resolved_host[16];
	// IPアドレス構造体
	socket_address_in_t address_to;
	// IPアドレス
	uint32_t addr_to_ip;
	// given_host -> resolved_host が実質的な変換を伴ったかどうか
	bool effectively_resolved;
	// セッション開始時刻
	timeval_t start_time;
	// ICMP シーケンス
	uint16_t icmp_sequence;
	// 送受信統計
	t_stat_data stat_data;
	timeval_t last_request_sent;
	// session-fatal なエラーが起きたかどうか
	bool error_occurred;
	// 受信タイムアウトしたかどうか
	bool receiving_timed_out;
	// ICMP Sequenceの重複チェック用ビットフィールド
	uint8_t	sequence_field[SEQ_BITFIELD_LEN];
} t_session;

// マスター構造体
typedef struct s_ping
{
	// 宛先によらないパラメータ

	// 送信ソケット
	int socket;
	// pingのID
	uint16_t icmp_header_id;
	// 設定
	t_preferences prefs;
	// 受信パケットのIPヘッダにアクセスできない時に立つフラグ
	bool inaccessible_ipheader;
	// 受信パケットのIPヘッダがカーネルによって書き換えられる時に立つフラグ
	bool received_ipheader_modified;

	// 宛先に依存するパラメータ
	t_session target;
} t_ping;

#endif
