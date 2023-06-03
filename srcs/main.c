#include "ping.h"

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
			DEBUGOUT("sum: %u %u", sum, *ptr);
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

void	set_timestamp_for_data(uint8_t* data_buffer, size_t buffer_len) {
	if (buffer_len < sizeof(struct timeval)) { return; }
	struct timeval	tm;
	gettimeofday(&tm, NULL);
	DEBUGOUT("tv_sec: %ld", tm.tv_sec);
	ft_memcpy(data_buffer, &tm, sizeof(struct timeval));
}

// 1つの宛先に対して ping セッションを実行する
int	run_ping_session(const t_ping* ping, const struct sockaddr_in* addr) {
	(void)ping;
	(void)addr;

	// [ICMPヘッダを準備する]
	uint8_t datagram_buffer[ICMP_ECHO_DATAGRAM_SIZE] = {0};
	icmp_header_t*	icmp_hd = (icmp_header_t*)datagram_buffer;
	uint8_t*		icmp_dt = (void*)datagram_buffer + sizeof(icmp_header_t);
	const size_t	icmp_dt_size = ICMP_ECHO_DATAGRAM_SIZE - sizeof(icmp_header_t);

	icmp_hd->ICMP_HEADER_TYPE = ICMP_ECHO_REQUEST;
	icmp_hd->ICMP_HEADER_CODE = 0;
	icmp_hd->ICMP_HEADER_ECHO.ICMP_HEADER_ID = 0;
	icmp_hd->ICMP_HEADER_ECHO.ICMP_HEADER_SEQ = 0;

	// データフィールドをASCIIで埋める
	// TODO: 全体で共通化
	// TODO: パターンが与えられた場合はパターンを使う
	for (size_t i = 0; i < ICMP_ECHO_DATA_SIZE; i++) {
		icmp_dt[i] = i;
	}

	// 可能なら先頭部分を現在時刻で埋める
	set_timestamp_for_data(icmp_dt, icmp_dt_size);

	// 最後にチェックサム計算
	icmp_hd->ICMP_HEADER_CHECKSUM = derive_icmp_checksum(datagram_buffer, ICMP_ECHO_DATAGRAM_SIZE);
	DEBUGOUT("checksum: %d", icmp_hd->ICMP_HEADER_CHECKSUM);

	// 送信!!
	int rv = sendto(
		ping->socket_fd,
		datagram_buffer, ICMP_ECHO_DATAGRAM_SIZE,
		0,
		(struct sockaddr *)addr, sizeof(struct sockaddr_in)
	);
	DEBUGOUT("rv: %d", rv);
	DEBUGOUT("errno: %d", errno);

	return 0;
}

int main(int argc, char **argv) {
	(void)argc;
	(void)argv;
	t_ping ping = {
		.target = "8.8.8.8",
	};

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
