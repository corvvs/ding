#include "ping.h"

t_received_result	receive_reply(const t_ping* ping, t_acceptance* acceptance) {
	struct msghdr		msg_received;
	struct iovec		iov_received;
	ft_memset(&msg_received, 0, sizeof(msg_received));
	msg_received.msg_name = NULL;
	msg_received.msg_namelen = 0;
	msg_received.msg_iov = &iov_received;
	msg_received.msg_iovlen = 1;
	iov_received.iov_base = acceptance->recv_buffer;
	iov_received.iov_len = acceptance->recv_buffer_len;
	int rv = recvmsg(ping->socket_fd, &msg_received, 0);
	if (rv < 0) {
		if (errno == EAGAIN) {
			DEBUGOUT("recvmsg: timed out: %d", errno);
			return RR_TIMEOUT;
		}
		if (errno == EINTR) {
			DEBUGOUT("recvmsg: interrupted: %d", errno);
			return RR_INTERRUPTED;
		}
		DEBUGERR("recvmsg: failed: %d(%s)", errno, strerror(errno));
		return RR_ERROR;
	}
	if (rv == 0) {
		// ソケットが閉じた -> 異常事態なので終了
		DEBUGERR("socket has been closed UNEXPECTEDLY: %d(%s)", errno, strerror(errno));
		exit(-1);
	}
	acceptance->epoch_received = get_current_time();
	acceptance->received_len = rv;

	if (!assimilate(ping, acceptance)) {
		return RR_UNACCEPTABLE;
	}
	return RR_SUCCESS;
}

