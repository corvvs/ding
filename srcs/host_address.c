#include "ping.h"

address_info_t*	resolve_str_into_address(const char* host_str) {
	address_info_t	hints = {
		.ai_family = AF_INET, .ai_socktype = SOCK_STREAM,
	};
	address_info_t*	res;
	if (getaddrinfo(host_str, NULL, &hints, &res)) {
		return NULL;
	}
	DEBUGWARN("ai_family: %d", res->ai_family);
	DEBUGWARN("AF_INET: %d", AF_INET);
	if (res->ai_family != AF_INET) {
		freeaddrinfo(res);
		return NULL;
	}
	return res;
}

static int	address_to_str(const address_info_t* res, char* resolved_str, size_t resolved_max_len) {
	const void*	red = inet_ntop(
		res->ai_family,
		&((socket_address_in_t *)res->ai_addr)->sin_addr,
		resolved_str,
		resolved_max_len
	);
	if (red == NULL) {
		DEBUGERR("inet_ntop failed: %d(%s)", errno, strerror(errno));
		return -1;
	}
	return 0;
}

static int	resolve_host(const char* given_str, char* resolved_str, size_t resolved_max_len) {
	// ユーザ指定ホスト -> (getaddrinfo) -> アドレス構造体 -> (inet_ntop) -> IPアドレス文字列
	address_info_t*	res = resolve_str_into_address(given_str);
	if (res == NULL) {
		print_error_by_message("unknown host");
		return -1;
	}
	int rv = address_to_str(res, resolved_str, resolved_max_len);
	freeaddrinfo(res);
	if (rv) {
		print_error_by_message("unknown host");
		return -1;
	}
	DEBUGWARN("given:    %s", given_str);
	DEBUGWARN("resolved: %s", resolved_str);
	return 0;
}

bool	setup_target_from_host(const char* host, t_session* target) {
	// 与えられたホストをIPアドレス文字列に変換する
	*target = (t_session){};
	target->given_host = host;
	if (resolve_host(target->given_host, target->resolved_host, sizeof(target->resolved_host))) {
		return false;
	}
	// IPアドレス文字列をアドレス構造体に変換する
	socket_address_in_t* addr = &target->address_to;
	addr->sin_family = AF_INET;
	if (inet_pton(AF_INET, target->resolved_host, &addr->sin_addr) != 1) {
		DEBUGERR("inet_pton() failed: %d(%s)", errno, strerror(errno));
		return false;
	}
	target->addr_to_ip = addr->sin_addr.s_addr;
	target->effectively_resolved = ft_strcmp(target->given_host, target->resolved_host) != 0;
	return true;
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
	snprintf(buf, sizeof(buf), "%u.%u.%u.%u",
		(addr32 >> 0) & 0xff,
		(addr32 >> 8) & 0xff,
		(addr32 >> 16) & 0xff,
		(addr32 >> 24) & 0xff
	);
	return buf;
}

const char*	stringify_address(const address_in_t* addr) {
	return stringify_serialized_address(serialize_address(addr));
}

static const char*	resolve_ipaddr_to_host(const t_ping* ping, uint32_t addr) {
	if (ping->target.effectively_resolved && ping->target.addr_to_ip == addr) {
		return ping->target.given_host;
	}
	return NULL;
}

void	print_address_serialized(const t_ping* ping, uint32_t addr) {
	const bool		try_to_resolve_host = !ping->prefs.dont_resolve_addr_received;
	if (try_to_resolve_host) {
		const char*	hostname = resolve_ipaddr_to_host(ping, addr);
		if (hostname != NULL) {
			printf("%s (%s)", hostname, stringify_serialized_address(addr));
			return;
		}
	}
	printf("%s", stringify_serialized_address(addr));
}

void	print_address_struct(const t_ping* ping, const address_in_t* addr) {
	const bool		try_to_resolve_host = !ping->prefs.dont_resolve_addr_received;
	if (try_to_resolve_host) {
		const char*	hostname = resolve_ipaddr_to_host(ping, serialize_address(addr));
		if (hostname != NULL) {
			printf("%s (%s)", hostname, stringify_address((const void*)addr));
			return;
		}
	}
	printf("%s", stringify_address((const void*)addr));
}
