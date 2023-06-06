#include "ping.h"

int	g_is_little_endian;

// 与えられた宛先から sockaddr_in を生成する
socket_address_in_t	retrieve_addr(const t_ping* ping) {
	socket_address_in_t addr = {};
	addr.sin_family = AF_INET;
	inet_pton(AF_INET, ping->target.resolved_host, &addr.sin_addr);
	return addr;
}

// ICMP送信用ソケットを作成する
int create_icmp_socket(void) {
	int sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if (sock < 0) {
		perror("socket");
		exit(EXIT_FAILURE);
	}
	return sock;
}

size_t	set_timestamp_for_data(uint8_t* data_buffer, size_t buffer_len) {
	if (buffer_len < sizeof(timeval_t)) { return 0; }
	timeval_t	tm;
	gettimeofday(&tm, NULL);
	ft_memcpy(data_buffer, &tm, sizeof(timeval_t));
	return sizeof(timeval_t);
}

void	print_msg_flags(const struct msghdr* msg) {
	// msg_flags の表示
	DEBUGINFO("msg_flags: %x",    msg->msg_flags);
	DEBUGINFO("MSG_EOR: %d",      !!(msg->msg_flags & MSG_EOR)); // レコードの終り
	DEBUGINFO("MSG_TRUNC: %d",    !!(msg->msg_flags & MSG_TRUNC)); // データグラムを受信しきれなかった
	DEBUGINFO("MSG_CTRUNC: %d",   !!(msg->msg_flags & MSG_CTRUNC)); // 補助データが受信しきれなかった
	DEBUGINFO("MSG_OOB: %d",      !!(msg->msg_flags & MSG_OOB)); // 対域外データを受信した
	DEBUGINFO("MSG_ERRQUEUE: %d", !!(msg->msg_flags & MSG_ERRQUEUE)); // ソケットのエラーキューからエラーを受信した
}

void	deploy_datagram(uint8_t* datagram_buffer, size_t datagram_len, uint16_t sequence) {
	// ASSERT(datagram_len >= sizeof(icmp_header_t));
	icmp_header_t*	icmp_hd = (icmp_header_t*)datagram_buffer;
	uint8_t*		icmp_dt = (void*)datagram_buffer + sizeof(icmp_header_t);
	const size_t	icmp_dt_size = datagram_len - sizeof(icmp_header_t);

	icmp_hd->ICMP_HEADER_TYPE = ICMP_ECHO_REQUEST;
	icmp_hd->ICMP_HEADER_CODE = 0;
	icmp_hd->ICMP_HEADER_ECHO.ICMP_HEADER_ID = getpid();
	icmp_hd->ICMP_HEADER_ECHO.ICMP_HEADER_SEQ = sequence;

	// エンディアン変換
	icmp_hd->ICMP_HEADER_TYPE = SWAP_NEEDED(icmp_hd->ICMP_HEADER_TYPE);
	icmp_hd->ICMP_HEADER_CODE = SWAP_NEEDED(icmp_hd->ICMP_HEADER_CODE);
	icmp_hd->ICMP_HEADER_ECHO.ICMP_HEADER_ID = SWAP_NEEDED(icmp_hd->ICMP_HEADER_ECHO.ICMP_HEADER_ID);
	icmp_hd->ICMP_HEADER_ECHO.ICMP_HEADER_SEQ = SWAP_NEEDED(icmp_hd->ICMP_HEADER_ECHO.ICMP_HEADER_SEQ);

	size_t	data_offset = 0;

	// 可能なら先頭部分を現在時刻で埋める
	data_offset += set_timestamp_for_data(icmp_dt, icmp_dt_size);

	// データフィールドをASCIIで埋める
	// TODO: 全体で共通化
	// TODO: パターンが与えられた場合はパターンを使う
	for (size_t i = data_offset, j = 0; i < ICMP_ECHO_DATA_SIZE; i++, j++) {
		icmp_dt[i] = j;
	}

	// 最後にチェックサム計算
	icmp_hd->ICMP_HEADER_CHECKSUM = derive_icmp_checksum(datagram_buffer, ICMP_ECHO_DATAGRAM_SIZE);
	DEBUGOUT("checksum: %d", icmp_hd->ICMP_HEADER_CHECKSUM);
}

// ICMP Echo Request を送信
int	send_ping(
	const t_ping* ping,
	const uint8_t* datagram_buffer,
	size_t datagram_len,
	const socket_address_in_t* addr
) {
	int rv = sendto(
		ping->socket_fd,
		datagram_buffer, datagram_len,
		0,
		(struct sockaddr *)addr, sizeof(socket_address_in_t)
	);
	DEBUGOUT("sendto rv: %d", rv);
	if (rv < 0) {
		DEBUGOUT("errno: %d (%s)", errno, strerror(errno));
	}
	return rv;
}

int	recv_ping(
	const t_ping* ping,
	struct msghdr* msg
) {

	int rv = recvmsg(
		ping->socket_fd,
		msg,
		0
	);
	if (rv < 0) {
		DEBUGOUT("errno: %d (%s)", errno, strerror(errno));
	}
	return rv;
}

sig_atomic_t volatile g_interrupted = 0;

void	sig_int(int signal) {
	(void)signal;
	g_interrupted = 1;
}

// 1つの宛先に対して ping セッションを実行する
int	run_ping_session(t_ping* ping, const socket_address_in_t* addr_to) {
	const size_t datagram_payload_len = ICMP_ECHO_DATAGRAM_SIZE - sizeof(icmp_header_t);
	// 送信前出力
	printf("PING %s (%s): %zu data bytes\n",
		ping->target.given_host, ping->target.resolved_host, datagram_payload_len);

	g_interrupted = 0;
	signal(SIGINT, sig_int);

	// [ICMPヘッダを準備する]
	for (uint16_t	sequence = 0; g_interrupted == 0; sequence += 1) {

		uint8_t datagram_buffer[ICMP_ECHO_DATAGRAM_SIZE] = {0};
		deploy_datagram(datagram_buffer, sizeof(datagram_buffer), sequence);

		// 送信!!
		if (send_ping(ping, datagram_buffer, sizeof(datagram_buffer), addr_to) < 0) {
			break;
		}
		// ECHO応答の受信を待機する
		uint8_t	recv_buffer[4096];
		struct msghdr		msg;
		struct iovec		iov;
		// socket_address_in_t	sa;
		size_t				recv_size = 0;
		ft_memset(&msg, 0, sizeof(msg));
		msg.msg_name = NULL;
		msg.msg_namelen = 0;
		msg.msg_iov = &iov;
		msg.msg_iovlen = 1;
		iov.iov_base = recv_buffer;
		iov.iov_len = 4096;
		int rv = recv_ping(ping, &msg);
		if (rv < 0) {
			break;
		}
		const timeval_t epoch_receipt = mark_sent(ping);
		recv_size = rv;
		debug_hexdump("recv_buffer", recv_buffer, recv_size);
		// まず 受信サイズ >= IPヘッダ(拡張なし)のサイズ であることを確認する
		if (recv_size < sizeof(ip_header_t)) {
			DEBUGERR("recv_size < sizeof(ip_header_t): %zu < %zu", recv_size, sizeof(ip_header_t));
			break;
		}

		ip_convert_endian(recv_buffer);

		// print_msg_flags(&msg);
		debug_ip_header(recv_buffer);

		const ip_header_t*	receipt_ip_header = (ip_header_t*)recv_buffer;
		// TODO: 受信サイズ == トータルサイズ であることを確認する
		if (recv_size != receipt_ip_header->tot_len) {
			DEBUGERR("size doesn't match: rv: %zu, tot_len: %u", recv_size, receipt_ip_header->tot_len);
			break;
		}
		const size_t	ip_header_len = receipt_ip_header->ihl * 4;
		DEBUGOUT("ip_header_len: %zu", ip_header_len);
		// CHECK: ip_header_len >= sizeof(ip_header_t)
		if (recv_size < ip_header_len) {
			DEBUGERR("recv_size < ip_header_len: %zu < %zu", recv_size, ip_header_len);
			break;
		}
		const size_t	icmp_len = recv_size  - ip_header_len;
		DEBUGOUT("icmp_len: %zu", icmp_len);
		if (icmp_len < sizeof(icmp_header_t)) {
			DEBUGERR("icmp_len < sizeof(icmp_header_t): %zu < %zu", icmp_len, sizeof(icmp_header_t));
			break;
		}
		void*			receipt_icmp = recv_buffer + ip_header_len;
		icmp_header_t*	receipt_icmp_header = (icmp_header_t*)receipt_icmp;

		if (validate_receipt_data(addr_to, receipt_ip_header, receipt_icmp, icmp_len)) {
			break;
		}
		icmp_convert_endian(receipt_icmp_header);
		// debug_icmp_header(receipt_icmp_header);

		const double triptime = mark_receipt(ping, receipt_icmp, &epoch_receipt);
		// 受信時出力
		printf("%zu bytes from %s: icmp_seq=%u ttl=%u time=%.3f ms\n",
			icmp_len,
			ping->target.resolved_host,
			receipt_icmp_header->ICMP_HEADER_ECHO.ICMP_HEADER_SEQ,
			receipt_ip_header->ttl,
			triptime
		);

		sleep(1);
	}
	// 統計情報を表示する
	print_stats(ping);


	return 0;
}

int main(int argc, char **argv) {
	if (argc < 1) {
		return 1;
	}

	// NOTE: プログラム名は使用しない
	argc -= 1;
	argv += 1;

	// TODO: オプションの解析

	if (argc < 1) {
		printf("ping: missing host operand\n");
		return 64; // なぜ 64 なのか...
	}

	t_ping ping = {
		.target = {
			.given_host = *argv,
		}
	};

	g_is_little_endian = is_little_endian();
	DEBUGWARN("g_is_little_endian: %d", g_is_little_endian);

	// [ソケット作成]
	// ソケットは全宛先で使い回すので最初に生成する
	ping.socket_fd = create_icmp_socket();

	{
		if (resolve_host(&ping.target)) {
			return -1;
		}
		DEBUGWARN("given:    %s", ping.target.given_host);
		DEBUGWARN("resolved: %s", ping.target.resolved_host);

		// [アドレス変換]
		socket_address_in_t	addr = retrieve_addr(&ping);

		// [エコー送信]
		run_ping_session(&ping, &addr);

		// [宛先単位の後処理]
		free(ping.stat_data.rtts);
		ping.stat_data = (t_stat_data){};
	}

	// [全体の後処理]
	close(ping.socket_fd);
}
