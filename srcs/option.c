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
		DEBUGOUT("ptr_digit: %c", *ptr_digit);
		if (!ptr_digit) {
			// 基数外の文字が出現した
			break;
		}
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
	}
	return negative ? -ans : ans;
}

static int parse_number(
	const char* str,
	unsigned long* out,
	unsigned long max,
	unsigned long min
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

#define NEXT_ONE_ARG \
	parsed += 1;\
	argc -= 1;\
	argv += 1

#define PICK_ONE_ARG \
	if (argc < 2) {\
		dprintf(STDERR_FILENO, PROGRAM_NAME ": option requires an argument -- '%c'\n", *arg);\
		return -1;\
	}\
	NEXT_ONE_ARG

int	parse_option(int argc, char** argv, bool by_root, t_preferences* pref) {
	int parsed = 0;
	while (argc > 0) {
		const char*	arg = *argv;
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
				PICK_ONE_ARG;
				unsigned long rv;
				DEBUGOUT("argv: %s", *argv);
				if (parse_number(*argv, &rv, 255, 1)) {
					return -1;
				}
				pref->ttl = rv;
				NEXT_ONE_ARG;
				continue;
			}

			if (ft_strcmp(long_opt, "ip-timestamp") == 0) {
				// --ip-timestamp: IPヘッダにタイムスタンプオプションを入れる
				PICK_ONE_ARG;
				if (ft_strcmp(*argv, "tsonly") == 0) {
					pref->ip_ts_type = IP_TST_TSONLY;
				} else if (ft_strcmp(*argv, "tsaddr") == 0) {
					pref->ip_ts_type = IP_TST_TSADDR;
				} else {
					dprintf(STDERR_FILENO, "%s: unsupported timestamp type: %s\n", PROGRAM_NAME, *argv);
					return -1;
				}
				NEXT_ONE_ARG;
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
				case 'v':
					pref->verbose = true;
					break;

				// count
				case 'c': {
					PICK_ONE_ARG;
					unsigned long rv;
					if (parse_number(*argv, &rv, ULONG_MAX, 0)) {
						return -1;
					}
					pref->count = rv;
					break;
				}

				// ttl
				case 'm': {
					PICK_ONE_ARG;
					unsigned long rv;
					if (parse_number(*argv, &rv, 255, 1)) {
						return -1;
					}
					pref->ttl = rv;
					break;
				}

				// dont resolve address in ip timestamp
				case 'n': {
					pref->resolve_addr_in_ip_ts = false;
					break;
				}

				// preload
				case 'l': {
					PICK_ONE_ARG;
					unsigned long rv;
					if (parse_number(*argv, &rv, INT_MAX, 0)) {
						return -1;
					}
					pref->preload = rv;
					break;
				}

				// セッションタイムアウト
				case 'w': {
					PICK_ONE_ARG;
					unsigned long rv;
					if (parse_number(*argv, &rv, INT_MAX, 1)) {
						return -1;
					}
					pref->session_timeout_s = rv;
					break;
				}

				// 最終送信後タイムアウト
				case 'W': {
					PICK_ONE_ARG;
					unsigned long rv;
					if (parse_number(*argv, &rv, INT_MAX, 1)) {
						return -1;
					}
					pref->wait_after_final_request_s = rv;
					break;
				}

				case 's': {
					PICK_ONE_ARG;
					unsigned long rv;
					if (parse_number(*argv, &rv, MAX_ICMP_DATASIZE, 0)) {
						return -1;
					}
					pref->data_size = rv;
					break;
				}

				// データパターン
				case 'p': {
					PICK_ONE_ARG;
					if (parse_pattern(*argv, pref->data_pattern, MAX_DATA_PATTERN_LEN)) {
						return -1;
					}
					break;
				}

				// flood
				case 'f': {
					if (!by_root) {
						print_error_by_message("flood ping is not permitted");
						return -1;
					}
					pref->flood = true;
					break;
				}

				// source address
				case 'S': {
					PICK_ONE_ARG;
					pref->given_source_address = *argv;
					break;
				}

				// ToS
				case 'T': {
					PICK_ONE_ARG;
					unsigned long rv;
					if (parse_number(*argv, &rv, 255, 0)) {
						return -1;
					}
					pref->tos = rv;
					break;
				}

				// bypass routing
				case 'r': {
					pref->bypass_routing = true;
					break;
				}

				default:
					// 未知のオプション
					dprintf(STDERR_FILENO, "invalid option -- '%c'\n", *arg);
					return -1;
			}
		}
		NEXT_ONE_ARG;
	}
	return parsed;
}

t_preferences	default_preferences(void) {
	return (t_preferences){
		.verbose = false,
		.count = 0,
		.data_size = ICMP_ECHO_DEFAULT_DATAGRAM_SIZE,
		.ip_ts_type = IP_TST_NONE,
		.ttl = 64,
		.session_timeout_s = 0,
		.wait_after_final_request_s = 10,
		.tos = -1,
		.resolve_addr_in_ip_ts = true,
	};
}
