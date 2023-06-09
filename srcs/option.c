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
				case 'c':
					if (argc < 2) {
						DEBUGERR("ping: option requires an argument -- '%c'", *arg);
						return -1;
					}
					parsed += 1;
					argc -= 1;
					argv += 1;
					const char*	number_str = *argv;
					char*		err;
					pref->count = ft_strtoul(number_str, &err, 0);
					DEBUGOUT("count = %lu", pref->count);
					if (*err) {
						DEBUGERR("invalid value (`%s' near `%s')", number_str, err);
						return -1;
					}
					break;

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
