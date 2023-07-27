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

int parse_number(
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
static int	chtox(char c) {
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
