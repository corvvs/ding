#include "ping.h"

static int	resolve_host(const char* given_str, char* resolved_str, size_t resolved_max_len) {
	address_info_t	hints = {
		.ai_family = AF_INET, .ai_socktype = SOCK_STREAM,
	};
	address_info_t*	res;
	if (getaddrinfo(given_str, NULL, &hints, &res)) {
		printf("ping: unknown host\n");
		return -1;
	}
	DEBUGWARN("ai_family: %d", res->ai_family);
	DEBUGWARN("AF_INET: %d", AF_INET);
	if (res->ai_family != AF_INET) {
		DEBUGERR("res->ai_family is not AF_INET: %d", res->ai_family);
		freeaddrinfo(res);
		return -1;
	}
	const void*	red = inet_ntop(
		res->ai_family,
		&((socket_address_in_t *)res->ai_addr)->sin_addr,
		resolved_str,
		resolved_max_len
	);
	freeaddrinfo(res);
	if (red == NULL) {
		DEBUGERR("inet_ntop failed: %d(%s)", errno, strerror(errno));
		printf("ping: unknown host\n");
		return -1;
	}
	DEBUGWARN("given:    %s", given_str);
	DEBUGWARN("resolved: %s", resolved_str);
	return 0;
}

int	setup_target_from_host(const char* host, t_target* target) {
	// 与えられたホストをIPアドレス文字列に変換する
	target->given_host = host;
	if (resolve_host(target->given_host, target->resolved_host, sizeof(target->resolved_host))) {
		return -1;
	}
	// IPアドレス文字列をアドレス構造体に変換する
	socket_address_in_t* addr = &target->addr_to;
	addr->sin_family = AF_INET;
	if (inet_pton(AF_INET, target->resolved_host, &addr->sin_addr) != 1) {
		DEBUGERR("inet_pton() failed: %d(%s)", errno, strerror(errno));
		return -1;
	}
	return 0;
}

uint32_t	serialize_address(const address_in_t* addr) {
#ifdef __APPLE__
	return addr->s_addr;
#else
	return *addr;
#endif
}

const char*	stringify_serialized_address(uint32_t addr32) {
	static char	buf[16];
	snprintf(buf, 16, "%u.%u.%u.%u",
		(addr32 >> 24) & 0xff,
		(addr32 >> 16) & 0xff,
		(addr32 >> 8) & 0xff,
		(addr32 >> 0) & 0xff
	);
	return buf;
}

const char*	stringify_address(const address_in_t* addr) {
	return stringify_serialized_address(serialize_address(addr));
}
