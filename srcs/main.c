#include "ping.h"

int	g_is_little_endian;

// 与えられた宛先から sockaddr_in を生成する
struct sockaddr_in	retrieve_addr(const t_ping* ping) {
	struct sockaddr_in addr = {};
	addr.sin_family = AF_INET;
	inet_pton(AF_INET, ping->target, &addr.sin_addr);
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

// ICMP チェックサムを計算する
uint16_t	derive_icmp_checksum(const void* datagram, size_t len) {
	// TODO: エンディアンの考慮
	uint32_t	sum = 0;
	uint16_t*	ptr = (uint16_t*)datagram;
	while (len > 0) {
		if (len >= sizeof(uint16_t)) {
			sum += *ptr;
			// DEBUGOUT("sum: %u %u", sum, *ptr);
			len -= sizeof(uint16_t);
			ptr += 1;
		} else {
			sum += *((uint8_t*)ptr) << 8;
			break;
		}
	}
	// 1の補数和にする
	sum = (sum >> 16) + (sum & 0xffff);
	sum = (sum >> 16) + sum;
	// 最後にもう1度1の補数を取る
	return (~sum) & 0xffff;
}

size_t	set_timestamp_for_data(uint8_t* data_buffer, size_t buffer_len) {
	if (buffer_len < sizeof(struct timeval)) { return 0; }
	struct timeval	tm;
	gettimeofday(&tm, NULL);
	ft_memcpy(data_buffer, &tm, sizeof(struct timeval));
	DEBUGOUT("tv_sec: %ld", tm.tv_sec);
	DEBUGOUT("tv_usec: %ld", tm.tv_usec);
	DEBUGOUT("set: %ld %lx", *(long*)data_buffer, *(long*)data_buffer);
	DEBUGOUT("set: %ld %lx", *(long*)(data_buffer + sizeof(long)), *(long*)(data_buffer + sizeof(long)));
	DEBUGOUT("sizeof(struct timeval): %zu", sizeof(struct timeval));
	return sizeof(struct timeval);
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

void	deploy_datagram(uint8_t* datagram_buffer, size_t datagram_len) {
	// ASSERT(datagram_len >= sizeof(icmp_header_t));
	icmp_header_t*	icmp_hd = (icmp_header_t*)datagram_buffer;
	uint8_t*		icmp_dt = (void*)datagram_buffer + sizeof(icmp_header_t);
	const size_t	icmp_dt_size = datagram_len - sizeof(icmp_header_t);

	icmp_hd->ICMP_HEADER_TYPE = ICMP_ECHO_REQUEST;
	icmp_hd->ICMP_HEADER_CODE = 0;
	icmp_hd->ICMP_HEADER_ECHO.ICMP_HEADER_ID = getpid();
	icmp_hd->ICMP_HEADER_ECHO.ICMP_HEADER_SEQ = 0;

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


// 1つの宛先に対して ping セッションを実行する
int	run_ping_session(const t_ping* ping, const struct sockaddr_in* addr) {

	// [ICMPヘッダを準備する]
	uint8_t datagram_buffer[ICMP_ECHO_DATAGRAM_SIZE] = {0};
	deploy_datagram(datagram_buffer, sizeof(datagram_buffer));
	const size_t datagram_payload_len = ICMP_ECHO_DATAGRAM_SIZE - sizeof(icmp_header_t);

	// 送信前出力
	printf("PING %s (%s): %zu data bytes\n",
		ping->target, inet_ntoa(addr->sin_addr), datagram_payload_len);


	// 送信!!
	{
		int rv = sendto(
			ping->socket_fd,
			datagram_buffer, ICMP_ECHO_DATAGRAM_SIZE,
			0,
			(struct sockaddr *)addr, sizeof(struct sockaddr_in)
		);
		DEBUGOUT("sendto rv: %d", rv);
		if (rv < 0) {
			DEBUGOUT("errno: %d (%s)", errno, strerror(errno));
			return rv;
		}
	}

	// ECHO応答の受信を待機する
	{
		uint8_t*	recv_buffer = malloc(4096);
		ft_memset(recv_buffer, 0, 4096);
		uint8_t		ans_data[4096] = {};
		struct msghdr msg;
		struct iovec iov;
		struct sockaddr_in sa;
		ft_memset(&msg, 0, sizeof(msg));
		msg.msg_name = &sa;
		msg.msg_namelen = sizeof(sa);
		msg.msg_iov = &iov;
		msg.msg_iovlen = 1;
		msg.msg_control = ans_data;
		msg.msg_controllen = sizeof(ans_data);
		iov.iov_base = recv_buffer;
		iov.iov_len = 4096;

		int rv = recvmsg(
			ping->socket_fd,
			&msg,
			0
		);
		if (rv < 0) {
			DEBUGOUT("errno: %d (%s)", errno, strerror(errno));
			free(recv_buffer);
			return rv;
		} 

		DEBUGOUT("recvmsg rv: %d", rv);
		printf("Received ICMP packet from %s\n", inet_ntoa(sa.sin_addr));

		debug_hexdump("recv_buffer", recv_buffer, rv);
		// debug_hexdump("msg_control", msg.msg_control, rv);

		print_msg_flags(&msg);

		// TODO: 受信サイズ >= IPヘッダのサイズ であることを確認する

		ip_convert_endiandd(recv_buffer);

		debug_ip_header(recv_buffer);

		const ip_header_t*	ip_hd = (ip_header_t*)recv_buffer;
		// TODO: 受信サイズ == トータルサイズ であることを確認する
		if (rv != ip_hd->tot_len) {
			DEBUGERR("size doesn't match: rv: %d, tot_len: %u", rv, ip_hd->tot_len);
			return -1;
		}
		const size_t	ip_header_len = ip_hd->ihl * 4;
		DEBUGOUT("ip_header_len: %zu", ip_header_len);
		// CHECK: ip_header_len >= sizeof(ip_header_t)
		const size_t	icmp_len = (size_t)rv  - ip_header_len;
		DEBUGOUT("icmp_len: %zu", icmp_len);
		icmp_header_t*	icmp_header = (icmp_header_t*)(recv_buffer + ip_header_len);
		icmp_convert_endian(icmp_header);

		debug_icmp_header(icmp_header);

		// TODO:
		// 受信したものが「先立って自分が送ったICMP Echo Request」に対応する ICMP Echo Reply であることを確認する.
		// 具体的には:
		// - IP
		//   - Reply の送信元が, Request の送信先であること
		// - ICMP
		//   - Type = 0 であること
		//   - Reply のID, Sequenceが, Request のID, Sequence と一致すること
		//   - チェックサムがgoodであること


		// 受信時出力
		// printf ("%d bytes from %s: icmp_seq=%u", datalen,
		// 	inet_ntoa (*(struct in_addr *) &from->sin_addr.s_addr),
		// 	ntohs (icmp->icmp_seq));
		// printf (" ttl=%d", ip->ip_ttl);
		// if (timing)
		// 	printf (" time=%.3f ms", triptime);
		printf("%zu bytes from %s: icmp_seq=%u ttl=%u time=%.3f ms\n",
			icmp_len,
			inet_ntoa(sa.sin_addr),
			icmp_header->ICMP_HEADER_ECHO.ICMP_HEADER_SEQ,
			ip_hd->ttl,
			0.0
		);


		free(recv_buffer);
	}



	return 0;
}

int main(int argc, char **argv) {
	(void)argc;
	(void)argv;
	t_ping ping = {
		.target = "8.8.8.8",
	};

	g_is_little_endian = is_little_endian();
	DEBUGWARN("g_is_little_endian: %d", g_is_little_endian);

	// [ソケット作成
	// ソケットは全宛先で使い回すので最初に生成する
	ping.socket_fd = create_icmp_socket();

	{
		// [アドレス変換]
		struct sockaddr_in	addr = retrieve_addr(&ping);

		// [エコー送信]
		run_ping_session(&ping, &addr);

		// [宛先単位の後処理]
	}

	// [全体の後処理]
	close(ping.socket_fd);
}
