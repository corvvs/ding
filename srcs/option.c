#include "ping.h"
#include <limits.h>

void	proceed_arguments(t_arguments* args, int n) {
	args->argc -= n;
	args->argv += n;
}

// argv を1つ進める
#define PRECEDE_NEXT_ARG \
	proceed_arguments(args, 1)

// argv を1つ引数用として進める
#define PICK_NEXT_ARG \
	if (args->argc < 2) {\
		dprintf(STDERR_FILENO, PROGRAM_NAME ": option requires an argument -- '%c'\n", *arg);\
		return -1;\
	}\
	PRECEDE_NEXT_ARG

// arg を1文字進める
// 進められない場合はargvを1つ進める
#define PICK_ONE_ARG \
	if (*(arg + 1)) {\
		++arg;\
	} else {\
		PICK_NEXT_ARG;\
		arg = *args->argv;\
	}\

// argを最後まで進める
#define EXHAUST_ARG \
	arg = *arg ? (arg + ft_strlen(arg) - 1) : arg

// 整数引数を取るショートオプションのラッパー
#define PARSE_NUMBER_SOPT(ch, store, min, max) case ch: {\
	PICK_ONE_ARG;\
	unsigned long rv;\
	if (parse_number(arg, &rv, min, max)) {\
		return -1;\
	}\
	EXHAUST_ARG;\
	store = rv;\
	break;\
}

// 整数引数を取るロングオプションのラッパー
#define PARSE_NUMBER_LOPT(str, store, min, max)\
	if (ft_strcmp(long_opt, str) == 0) {\
		PICK_NEXT_ARG;\
		unsigned long rv;\
		if (parse_number(*args->argv, &rv, min, max)) {\
			return -1;\
		}\
		store = rv;\
		PRECEDE_NEXT_ARG;\
		return 0;\
	}

// フラグ ショートオプションのラッパー
#define PARSE_FLAG_SOPT(ch, store) case ch: {\
	store = true;\
	break;\
}

// フラグ ロングオプションのラッパー
#define PARSE_FLAG_LOPT(str, store)\
	if (ft_strcmp(long_opt, str) == 0) {\
		store = true;\
		PRECEDE_NEXT_ARG;\
		return 0;\
	}

static	int parse_longoption(t_arguments* args, bool by_root, t_preferences* pref, const char* arg) {
	(void)by_root;
	// ロングオプション解析
	const char *long_opt = arg + 2;

	PARSE_FLAG_LOPT("verbose", pref->verbose)
	PARSE_FLAG_LOPT("numeric", pref->dont_resolve_addr_in_ip_ts)
	PARSE_FLAG_LOPT("ignore-routing", pref->bypass_routing)
	PARSE_FLAG_LOPT("help", pref->show_usage)

	PARSE_NUMBER_LOPT("count", pref->count, 0, ULONG_MAX)
	PARSE_NUMBER_LOPT("size", pref->data_size, 0, MAX_ICMP_DATASIZE)
	PARSE_NUMBER_LOPT("preload", pref->preload, 0, INT_MAX)
	PARSE_NUMBER_LOPT("timeout", pref->session_timeout_s, 1, INT_MAX)
	PARSE_NUMBER_LOPT("linger", pref->wait_after_final_request_s, 1, INT_MAX)
	PARSE_NUMBER_LOPT("ttl", pref->ttl, 1, 255)
	PARSE_NUMBER_LOPT("tos", pref->tos, 0, 255)

	if (ft_strcmp(long_opt, "ip-timestamp") == 0) {
		// --ip-timestamp: IPヘッダにタイムスタンプオプションを入れる
		PICK_NEXT_ARG;
		if (ft_strcmp(*args->argv, "tsonly") == 0) {
			pref->ip_ts_type = IP_TST_TSONLY;
		} else if (ft_strcmp(*args->argv, "tsaddr") == 0) {
			pref->ip_ts_type = IP_TST_TSADDR;
		} else {
			dprintf(STDERR_FILENO, "%s: unsupported timestamp type: %s\n", PROGRAM_NAME, *args->argv);
			return -1;
		}
		PRECEDE_NEXT_ARG;
		return 0;
	}

	// 未知のロングオプション
	dprintf(STDERR_FILENO, "%s: unrecognized option -- '%s'\n",
		PROGRAM_NAME,
		arg);
	return -1;
}

int	parse_option(t_arguments* args, bool by_root, t_preferences* pref) {
	while (args->argc > 0) {
		const char*	arg = *args->argv;
		DEBUGOUT("argc: %d, arg: %s", args->argc, arg);
		if (arg == NULL) {
			// ありえないはずだが・・・
			DEBUGERR("%s", "argv has an NULL");
			return -1;
		}
		if (*arg != '-') {
			// オプションではない
			break;
		}
		if (ft_strncmp(arg, "--", 2) == 0) {
			if (parse_longoption(args, by_root, pref, arg)) {
				return -1;
			}
			continue;
		}

		// ショートオプション解析
		while (*++arg) {
			switch (*arg) {

				// verbose
				PARSE_FLAG_SOPT('v', pref->verbose)
				// dont resolve address in ip timestamp
				PARSE_FLAG_SOPT('n', pref->dont_resolve_addr_in_ip_ts)
				// bypass routing - ルーティングを無視する; このマシンと直接繋がっているノードにしかpingが届かなくなる
				PARSE_FLAG_SOPT('r', pref->bypass_routing)
				// show usage
				PARSE_FLAG_SOPT('?', pref->show_usage)

				// count - 送信するping(ICMP Echo)の数
				PARSE_NUMBER_SOPT('c', pref->count, 0, ULONG_MAX)
				// ICMP データサイズ - 送信するICMP Echoのデータサイズ; 16未満を指定するとRTTを計測しなくなる
				PARSE_NUMBER_SOPT('s', pref->data_size, 0, MAX_ICMP_DATASIZE)
				// preload - 0より大きい値がセットされている場合, その数だけ最初に(waitを無視して)連続送信する
				PARSE_NUMBER_SOPT('l', pref->preload, 0, INT_MAX)
				// セッションタイムアウト - セッション開始から指定時間経過するとセッションが終了する
				PARSE_NUMBER_SOPT('w', pref->session_timeout_s, 1, INT_MAX)
				// 最終送信後タイムアウト - そのセッションの最後のping送信から指定時間経過するとセッションが終了する
				PARSE_NUMBER_SOPT('W', pref->wait_after_final_request_s, 1, INT_MAX)
				// TTL - 送信するIPデータグラムのTTL
				PARSE_NUMBER_SOPT('m', pref->ttl, 1, 255)
				// ToS - 送信するIPデータグラムのToS
				PARSE_NUMBER_SOPT('T', pref->tos, 0, 255)

				// データパターン - 送信するICMP Echoのデータ部分を埋めるパターン
				case 'p': {
					PICK_ONE_ARG;
					if (parse_pattern(arg, pref->data_pattern, MAX_DATA_PATTERN_LEN)) {
						return -1;
					}
					EXHAUST_ARG;
					break;
				}

				// ソースアドレス - 使用するソケットを指定したアドレスにbindする
				case 'S': {
					PICK_ONE_ARG;
					pref->given_source_address = (char*)arg;
					EXHAUST_ARG;
					break;
				}

				// flood - floodモードで実行する
				case 'f': {
					if (!by_root) {
						print_error_by_message("flood ping is not permitted");
						return -1;
					}
					pref->flood = true;
					break;
				}

				default:
					// 未知のオプション
					dprintf(STDERR_FILENO, "invalid option -- '%c'\n", *arg);
					return -1;
			}
		}
		PRECEDE_NEXT_ARG;
	}
	return 0;
}

t_preferences	default_preferences(void) {
	return (t_preferences){
		.data_size = ICMP_ECHO_DEFAULT_DATAGRAM_SIZE,
		.ip_ts_type = IP_TST_NONE,
		.ttl = 64,
		.wait_after_final_request_s = 10,
		.tos = -1,
	};
}
