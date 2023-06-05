#include "ping.h"

int	resolve_host(t_target* target) {
	address_info_t	hints;
	address_info_t*	res;
	ft_memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	int status = getaddrinfo(target->given_host, NULL, &hints, &res);
	DEBUGWARN("status: %d", status);
	if (status < 0) {
		printf("ping: unknown host\n");
		return -1;
	}
	DEBUGWARN("ai_family: %d", res->ai_family);
	DEBUGWARN("AF_INET: %d", AF_INET);
	if (res->ai_family != AF_INET) {
		DEBUGERR("res->ai_family is not AF_INET: %d", res->ai_family);
		return -1;
	}
	const void*	resolved = inet_ntop(
		res->ai_family,
		&((socket_address_in_t *)res->ai_addr)->sin_addr,
		target->resolved_host,
		sizeof(target->resolved_host)
	);
	if (resolved == NULL) {
		DEBUGERR("inet_ntop failed: %d(%s)", errno, strerror(errno));
		printf("ping: unknown host\n");
		return -1;
	}
	return 0;
}
