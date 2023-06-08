#include "ping.h"

static int	recv_ping(const t_ping* ping, struct msghdr* msg) {

	int rv = recvmsg(
		ping->socket_fd,
		msg,
		0
	);
	if (rv < 0) {
		DEBUGOUT("EAGAIN: %d", EAGAIN);
		DEBUGOUT("errno: %d (%s)", errno, strerror(errno));
	}
	return rv;
}

int	receive_reply(const t_ping* ping, t_acceptance* acceptance) {
	struct msghdr		msg_receipt;
	struct iovec		iov_receipt;
	// socket_address_in_t	sa;
	ft_memset(&msg_receipt, 0, sizeof(msg_receipt));
	msg_receipt.msg_name = NULL;
	msg_receipt.msg_namelen = 0;
	msg_receipt.msg_iov = &iov_receipt;
	msg_receipt.msg_iovlen = 1;
	iov_receipt.iov_base = acceptance->recv_buffer;
	iov_receipt.iov_len = acceptance->recv_buffer_len;
	int rv = recv_ping(ping, &msg_receipt);
	if (rv < 0) {
		DEBUGERR("recv_ping failed: %d(%s)", errno, strerror(errno));
		return rv;
	}
	if (rv == 0) {
		DEBUGERR("socket has been closed UNEXPECTEDLY: %d(%s)", errno, strerror(errno));
		return rv;
	}
	acceptance->epoch_receipt = get_current_time();
	acceptance->receipt_len = rv;
	return rv;
}

