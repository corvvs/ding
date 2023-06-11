#include "ping.h"
#include <limits.h>

int	ft_isspace(int ch) {
	return ch == ' ' || ('\t' <= ch && ch <= '\r');
}

size_t	ft_strtoul(const char* str, char **endptr, int base) {
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

int parse_number(const char* str, unsigned long* out, unsigned long max, unsigned long min) {
	char*		err;
	unsigned long rv = ft_strtoul(str, &err, 0);
	if (*err) {
		DEBUGERR("invalid value (`%s' near `%s')", str, err);
		return -1;
	}
	if (rv > max) {
		DEBUGERR("option value too big: %s", str);
		return -1;
	}
	if (rv < min) {
		DEBUGERR("option value too small: %s", str);
		return -1;
	}
	*out = rv;
	return 0;
}

#define PICK_ONE_ARG \
	if (argc < 2) {\
		DEBUGERR("ping: option requires an argument -- '%c'", *arg);\
		return -1;\
	}\
	parsed += 1;\
	argc -= 1;\
	argv += 1

int	parse_option(int argc, char** argv, t_preferences* pref) {
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
		// arg はおそらくオプションなので解析する
		DEBUGOUT("parsing: %s", arg);
		while (*++arg) {
			DEBUGOUT("*arg = %c(%d)", *arg, *arg);
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

				// deadline
				case 'W': {
					PICK_ONE_ARG;
					unsigned long rv;
					if (parse_number(*argv, &rv, INT_MAX, 1)) {
						return -1;
					}
					pref->wait_after_final_request_s = rv;
					break;
				}

				default:
					// 未知のオプション
					DEBUGERR("ping: invalid option -- '%c'", *arg);
					return -1;
			}
		}
		parsed += 1;
		argc -= 1;
		argv += 1;
	}
	return parsed;
}

t_preferences	default_preferences(void) {
	return (t_preferences){
		.verbose = false,
		.count = 0,
		.ttl = 64,
		.wait_after_final_request_s = 10,
	};
}
