#include "ping.h"
#include <limits.h>

static int	ft_isspace(int ch) {
	return ch == ' ' || ('\t' <= ch && ch <= '\r');
}

static size_t	ft_strtoul(const char* str, char **endptr, int base) {
	if (!(base == 0 || (2 <= base && base <= 36))) {
		errno = EINVAL;
		return 0;
	}
	while (ft_isspace(*str)) {
		str += 1;
	}
	bool	negative = (*str == '-');
	if (*str == '-' || *str == '+') {
		str += 1;
	}
	unsigned long	ans = 0;
	unsigned long	actual_base = base;
	const bool		accept_0x = (base == 0 || base == 16);
	const bool		accept_0 = (base == 0);
	if (accept_0x && ft_strncmp(str, "0x", 2) == 0) {
		str += 2;
		actual_base = 16;
	} else if (accept_0 && ft_strncmp(str, "0", 1) == 0) {
		str += 1;
		actual_base = 8;
	} else if (actual_base == 0) {
		actual_base = 10;
	}
	DEBUGOUT("actual_base: %lu", actual_base);
	const char*	bases = "0123456789abcdefghijklmnopqrstuvwxyz";
	while (*str) {
		char* ptr_digit = ft_strchr(bases, ft_tolower(*str));
		if (ptr_digit == NULL) {
			// 基数外の文字が出現した
			break;
		}
		DEBUGOUT("ptr_digit: %c", *ptr_digit);
		unsigned long	digit = ptr_digit - bases;
		if (digit >= actual_base) {
			// 桁の数字が基数以上だった
			break;
		}
		// オーバーフローチェック
		unsigned long	lower = ULONG_MAX / actual_base;
		if (ans > lower || (ans == lower && digit > ULONG_MAX - ans * actual_base)) {
			ans = ULONG_MAX;
			errno = ERANGE;
			break;
		}
		ans = ans * actual_base + digit;
		str += 1;
	}
	if (endptr) {
		*endptr = (char*)str;
		DEBUGOUT("endptr: %p: %d", *endptr, **endptr);
	}
	return negative ? -ans : ans;
}

static int parse_number(
	const char* str,
	unsigned long* out,
	unsigned long min,
	unsigned long max
) {
	char*		err;
	unsigned long rv = ft_strtoul(str, &err, 0);
	if (*err) {
		dprintf(STDERR_FILENO, PROGRAM_NAME ": invalid value (`%s' near `%s')\n", str, err);
		return -1;
	}
	if (rv > max) {
		dprintf(STDERR_FILENO, PROGRAM_NAME ": option value too big: %s\n", str);
		return -1;
	}
	if (rv < min) {
		dprintf(STDERR_FILENO, PROGRAM_NAME ": option value too small: %s\n", str);
		return -1;
	}
	*out = rv;
	return 0;
}

// 16進数字(0-9, a-f, A-f)を整数値(0-15)に変換する.
// 変換できなければ -1 を返す.
int	chtox(char c) {
	c = ft_tolower(c);
	if ('0' <= c && c <= '9') {
		return c - '0';
	}
	if ('a' <= c && c <= 'f') {
		return c - 'a' + 10;
	}
	return -1;
}

int	parse_pattern(
	const char* str,
	char* buffer,
	size_t max_len
) {
	size_t i = 0, j = 0;
	for (; str[i];) {
		if (max_len <= j) {
			dprintf(STDERR_FILENO, PROGRAM_NAME ": pattern too long: %s\n", str);
			return -1;
		}
		int	x = chtox(str[i]);
		if (x < 0) {
			dprintf(STDERR_FILENO, PROGRAM_NAME ": error in pattern near %c\n", str[i]);
			return -1;
		}
		i += 1;
		uint8_t n = x;
		if (str[i]) {
			x = chtox(str[i]);
			if (x < 0) {
				dprintf(STDERR_FILENO, PROGRAM_NAME ": error in pattern near %c\n", str[i]);
				return -1;
			}
			i += 1;
			n = (n * 16) + x;
		}
		buffer[j++] = n;
	}
	buffer[j] = '\0';
	return 0;
}

void	proceed_arguments(t_arguments* args, int n) {
	args->argc -= n;
	args->argv += n;
}

// argv を1つ進める
#define PRECEDE_NEXT_ARG \
	parsed += 1;\
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
// ロングオプション用のラッパーはない

// フラグ ショートオプションのラッパー
#define PARSE_FLAG_SOPT(ch, store) case ch: {\
	store = true;\
	break;\
}

int	parse_option(t_arguments* args, bool by_root, t_preferences* pref) {
	int parsed = 0;
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
			// ロングオプションかもしれない
			const char *long_opt = arg + 2;

			if (ft_strcmp(long_opt, "ttl") == 0) {
				// --ttl: TTL設定(-m と等価)
				PICK_NEXT_ARG;
				unsigned long rv;
				if (parse_number(*args->argv, &rv, 255, 1)) {
					return -1;
				}
				pref->ttl = rv;
				PRECEDE_NEXT_ARG;
				continue;
			}

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
				continue;
			}

			// 未知のロングオプション
			dprintf(STDERR_FILENO, "%s: unrecognized option -- '%s'\n",
				PROGRAM_NAME,
				arg);
			return -1;
		}

		// arg はおそらくオプションなので解析する
		while (*++arg) {
			switch (*arg) {

				// verbose
				PARSE_FLAG_SOPT('v', pref->verbose)
				// dont resolve address in ip timestamp
				PARSE_FLAG_SOPT('n', pref->resolve_addr_in_ip_ts)
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
	return parsed;
}

t_preferences	default_preferences(void) {
	return (t_preferences){
		.data_size = ICMP_ECHO_DEFAULT_DATAGRAM_SIZE,
		.ip_ts_type = IP_TST_NONE,
		.ttl = 64,
		.wait_after_final_request_s = 10,
		.tos = -1,
		.resolve_addr_in_ip_ts = true,
	};
}
