#include "ping.h"

t_receipt_result	receive_reply(const t_ping* ping, t_acceptance* acceptance) {
	struct msghdr		msg_receipt;
	struct iovec		iov_receipt;
	ft_memset(&msg_receipt, 0, sizeof(msg_receipt));
	msg_receipt.msg_name = NULL;
	msg_receipt.msg_namelen = 0;
	msg_receipt.msg_iov = &iov_receipt;
	msg_receipt.msg_iovlen = 1;
	iov_receipt.iov_base = acceptance->recv_buffer;
	iov_receipt.iov_len = acceptance->recv_buffer_len;
	int rv = recvmsg(ping->socket_fd, &msg_receipt, 0);
	if (rv < 0) {
		if (errno == EAGAIN) {
			DEBUGOUT("timed out: %d", errno);
			return RR_TIMEOUT;
		}
		DEBUGERR("recv_ping failed: %d(%s)", errno, strerror(errno));
		return RR_ERROR;
	}
	if (rv == 0) {
		// ソケットが閉じた -> 異常事態なので終了
		DEBUGERR("socket has been closed UNEXPECTEDLY: %d(%s)", errno, strerror(errno));
		exit(-1);
	}
	acceptance->epoch_receipt = get_current_time();
	acceptance->receipt_len = rv;
	return RR_SUCCESS;
}

