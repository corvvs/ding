#include "ping.h"
#include <limits.h>

// [オプション定義]
// 特殊なものはここでは定義していない.
// 目的のものがなければ関数 parse_longoption / parse_shortoption も見ること.

// verbose
#define OPTION_VERBOSE(f)        f('v', "verbose",        pref->verbose)
// dont resolve address in ip timestamp
#define OPTION_NUMERIC(f)        f('n', "numeric",        pref->dont_resolve_addr_in_ip_ts)
// bypass routing - ルーティングを無視する; このマシンと直接繋がっているノードにしかpingが届かなくなる
#define OPTION_IGNORE_ROUTING(f) f('r', "ignore-routing", pref->bypass_routing)
// show usage
#define OPTION_HELP(f)           f('?', "help",           pref->show_usage)

// count - 送信するping(ICMP Echo)の数
#define OPTION_COUNT(f)          f('c', "count",   pref->count, 0, ULONG_MAX)
// ICMP データサイズ - 送信するICMP Echoのデータサイズ; 16未満を指定するとRTTを計測しなくなる
#define OPTION_SIZE(f)           f('s', "size",    pref->data_size, 0, MAX_ICMP_DATASIZE)
// preload - 0より大きい値がセットされている場合, その数だけ最初に(waitを無視して)連続送信する
#define OPTION_PRELOAD(f)        f('l', "preload", pref->preload, 0, INT_MAX)
// セッションタイムアウト - セッション開始から指定時間経過するとセッションが終了する
#define OPTION_TIMEOUT(f)        f('w', "timeout", pref->session_timeout_s, 1, INT_MAX)
// 最終送信後タイムアウト - そのセッションの最後のping送信から指定時間経過するとセッションが終了する
#define OPTION_LINGER(f)         f('W', "linger",  pref->wait_after_final_request_s, 1, INT_MAX)
// TTL - 送信するIPデータグラムのTTL
#define OPTION_TTL(f)            f('m', "ttl",     pref->ttl, 1, 255)
// ToS - 送信するIPデータグラムのToS
#define OPTION_TOS(f)            f('T', "tos",     pref->tos, 0, 255)

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

// ロングオプション解析
static	int parse_longoption(t_arguments* args, bool by_root, t_preferences* pref, const char* arg) {
	// フラグ ロングオプションのラッパー
	#define PARSE_FLAG_LOPT(_, str, store)\
		if (ft_strcmp(long_opt, str) == 0) {\
			store = true;\
			PRECEDE_NEXT_ARG;\
			return 0;\
		}

	#define PARSE_ARGUMENT_LOPT(str, block) {\
		int tail = ft_starts_with(long_opt, str);\
		if (tail >= 0) {\
			if (long_opt[tail] == '=') {\
				long_opt += tail + 1;\
			} else {\
				PICK_NEXT_ARG;\
				long_opt = *args->argv;\
			}\
			block\
			PRECEDE_NEXT_ARG;\
			return 0;\
		}\
	}

	// 整数引数を取るロングオプションのラッパー
	#define PARSE_NUMBER_LOPT(_, str, store, min, max) PARSE_ARGUMENT_LOPT(str, {\
		unsigned long rv;\
		if (parse_number(long_opt, &rv, min, max)) {\
			return -1;\
		}\
		store = rv;\
	})

	// -- ここから本体 --
	(void)by_root;
	const char *long_opt = arg + 2;

	OPTION_VERBOSE(PARSE_FLAG_LOPT)
	OPTION_NUMERIC(PARSE_FLAG_LOPT)
	OPTION_IGNORE_ROUTING(PARSE_FLAG_LOPT)
	OPTION_HELP(PARSE_FLAG_LOPT)

	OPTION_COUNT(PARSE_NUMBER_LOPT)
	OPTION_SIZE(PARSE_NUMBER_LOPT)
	OPTION_PRELOAD(PARSE_NUMBER_LOPT)
	OPTION_TIMEOUT(PARSE_NUMBER_LOPT)
	OPTION_LINGER(PARSE_NUMBER_LOPT)
	OPTION_TTL(PARSE_NUMBER_LOPT)
	OPTION_TOS(PARSE_NUMBER_LOPT)

	PARSE_ARGUMENT_LOPT("ip-timestamp", {
		if (ft_strcmp(long_opt, "tsonly") == 0) {
			pref->ip_ts_type = IP_TST_TSONLY;
		} else if (ft_strcmp(long_opt, "tsaddr") == 0) {
			pref->ip_ts_type = IP_TST_TSADDR;
		} else {
			dprintf(STDERR_FILENO, "%s: unsupported timestamp type: %s\n", PROGRAM_NAME, long_opt);
			return -1;
		}
	})

	// 未知のロングオプション
	dprintf(STDERR_FILENO, "%s: unrecognized option '%s'\n",
		PROGRAM_NAME,
		arg);
	return -1;

	#undef PARSE_FLAG_LOPT
	#undef PARSE_NUMBER_LOPT
}


// ショートオプション解析
static	int parse_shortoption(t_arguments* args, bool by_root, t_preferences* pref, const char* arg) {
	// フラグ ショートオプションのラッパー
	#define PARSE_FLAG_SOPT(ch, _, store) case ch: {\
		store = true;\
		break;\
	}

	// 整数引数を取るショートオプションのラッパー
	#define PARSE_NUMBER_SOPT(ch, _, store, min, max) case ch: {\
		PICK_ONE_ARG;\
		unsigned long rv;\
		if (parse_number(arg, &rv, min, max)) {\
			return -1;\
		}\
		EXHAUST_ARG;\
		store = rv;\
		break;\
	}

	// -- ここから本体 --
	while (*++arg) {
		switch (*arg) {

			OPTION_VERBOSE(PARSE_FLAG_SOPT)
			OPTION_NUMERIC(PARSE_FLAG_SOPT)
			OPTION_IGNORE_ROUTING(PARSE_FLAG_SOPT)
			OPTION_HELP(PARSE_FLAG_SOPT)

			OPTION_COUNT(PARSE_NUMBER_SOPT)
			OPTION_SIZE(PARSE_NUMBER_SOPT)
			OPTION_PRELOAD(PARSE_NUMBER_SOPT)
			OPTION_TIMEOUT(PARSE_NUMBER_SOPT)
			OPTION_LINGER(PARSE_NUMBER_SOPT)
			OPTION_TTL(PARSE_NUMBER_SOPT)
			OPTION_TOS(PARSE_NUMBER_SOPT)

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
	return 0;

	#undef PARSE_FLAG_SOPT
	#undef PARSE_NUMBER_SOPT
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
		if (arg[0] != '-' || arg[1] == '\0') {
			// オプションではない
			break;
		}
		if (ft_strncmp(arg, "--", 2) == 0) {
			if (parse_longoption(args, by_root, pref, arg)) {
				return -1;
			}
		} else {
			if (parse_shortoption(args, by_root, pref, arg)) {
				return -1;
			}
		}
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
