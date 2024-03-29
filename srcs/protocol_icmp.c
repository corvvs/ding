#include "ping.h"
extern int	g_is_little_endian;

// mem をIPヘッダとして, 必要ならエンディアン変換を行う
void	flip_endian_icmp(icmp_header_t* icmp_hd) {
	if (!g_is_little_endian) { return; }

	icmp_hd->ICMP_HEADER_TYPE = SWAP_NEEDED(icmp_hd->ICMP_HEADER_TYPE);
	icmp_hd->ICMP_HEADER_CODE = SWAP_NEEDED(icmp_hd->ICMP_HEADER_CODE);
	icmp_hd->ICMP_HEADER_CHECKSUM = SWAP_NEEDED(icmp_hd->ICMP_HEADER_CHECKSUM);
	if (icmp_hd->ICMP_HEADER_TYPE == ICMP_ECHO || icmp_hd->ICMP_HEADER_TYPE == ICMP_TYPE_ECHO_REPLY) {
		icmp_hd->ICMP_HEADER_ECHO.ICMP_HEADER_ID = SWAP_NEEDED(icmp_hd->ICMP_HEADER_ECHO.ICMP_HEADER_ID);
		icmp_hd->ICMP_HEADER_ECHO.ICMP_HEADER_SEQ = SWAP_NEEDED(icmp_hd->ICMP_HEADER_ECHO.ICMP_HEADER_SEQ);
	}
}

// datagram に対して ICMP チェックサムを計算する
// WARNING: datagram はネットワークバイトオーダーになっている必要がある
uint16_t	derive_icmp_checksum(const void* datagram, size_t len) {
	uint32_t	sum = 0;
	uint16_t*	ptr = (uint16_t*)datagram;
	while (len > 0) {
		if (len >= sizeof(uint16_t)) {
			sum += *ptr;
			len -= sizeof(uint16_t);
			ptr += 1;
		} else {
			// len が奇数だった場合の処理
			// 「右に 0x00 を補う」という情報もあるが, それだとチェックサムエラーになる
			// (一見正常にsendできるが, tcpdump するとエラーと表示される)
			sum += *((uint8_t*)ptr);
			break;
		}
	}
	// 1の補数和にする
	sum = (sum >> 16) + (sum & 0xffff);
	sum = (sum >> 16) + sum;
	// 最後にもう1度1の補数を取る
	uint16_t	ans = ~sum;
	return ans;
}

// ICMPデータグラムのチェックサムが正しいかどうか検証する
// チェックサム計算の際, 一時的にデータを変更するので const がつかないが, 最終的には変更なしで返す
// 注意: icmp はネットワークバイトオーダーであること
bool	is_valid_icmp_checksum(const t_ping* ping, icmp_header_t* icmp_header, size_t icmp_datagram_size) {
	uint16_t		received_checksum = icmp_header->ICMP_HEADER_CHECKSUM;
	ip_header_t*	embedded_ip = NULL;
	if (icmp_header->ICMP_HEADER_TYPE == ICMP_TYPE_TIME_EXCEEDED) {
		icmp_detailed_header_t*	dicmp = (icmp_detailed_header_t*)icmp_header;
		embedded_ip = (ip_header_t*)&(dicmp->ICMP_DHEADER_EMBEDDED_IP);
	}

	// チェックサムを0にして再計算する
	// debug_hexdump("before cksum", icmp_header, icmp_datagram_size);
	icmp_header->ICMP_HEADER_CHECKSUM = 0;
	// NOTE: mac では, ICMPペイロード内にIPデータグラムが埋め込まれている場合
	// そちらのIPヘッダもエンディアン変換されているので,
	// そのままチェックサムを計算すると合わない
	// -> オリジナルIPヘッダのエンディアンを元に戻して計算する
	const bool	flip_embedded_ip = embedded_ip && ping->received_ipheader_modified;
	if (flip_embedded_ip) {
		flip_endian_ip(embedded_ip);
	}
	uint16_t	derived_checksum = derive_icmp_checksum(icmp_header, icmp_datagram_size);
	DEBUGOUT("checksum: received: %u(%04x) derived: %u(%04x) difference: %u(%04x) icmp_datagram_size: %zu",
		received_checksum, received_checksum,
		derived_checksum, derived_checksum,
		(received_checksum - derived_checksum),
		(received_checksum - derived_checksum),
		icmp_datagram_size
	);
	// チェックサムを元に戻す
	icmp_header->ICMP_HEADER_CHECKSUM = received_checksum;
	if (flip_embedded_ip) {
		flip_endian_ip(embedded_ip);
	}
	return derived_checksum == received_checksum;
}

static size_t	set_timestamp_for_data(uint8_t* data_buffer, size_t buffer_len) {
	if (buffer_len < sizeof(timeval_t)) { return 0; }
	timeval_t	tm;
	gettimeofday(&tm, NULL);
	ft_memcpy(data_buffer, &tm, sizeof(timeval_t));
	return sizeof(timeval_t);
}

static void	fill_data_with_pattern(
	const t_ping* ping,
	uint8_t* icmp_dt,
	size_t data_offset,
	size_t buffer_len
) {
	if (*(ping->prefs.data_pattern)) {
		for (size_t i = data_offset, j = 0; i < buffer_len; i++, j++) {
			if (!ping->prefs.data_pattern[j]) {
				j = 0;
			}
			icmp_dt[i] = ping->prefs.data_pattern[j];
		}
	} else {
		for (size_t i = data_offset, j = 0; i < buffer_len; i++, j++) {
			icmp_dt[i] = j;
		}
	}
}

// バッファ datagram_buffer にヘッダを含めたICMPデータグラム全体をコピーする
void	construct_icmp_datagram(
	const t_ping* ping,
	uint8_t* datagram_buffer,
	size_t icmp_datagram_len,
	uint16_t sequence
) {
	// ASSERT(datagram_len >= sizeof(icmp_header_t));
	icmp_header_t*	icmp_hd = (icmp_header_t*)datagram_buffer;
	uint8_t*		icmp_dt = (void*)datagram_buffer + sizeof(icmp_header_t);
	const size_t	icmp_dt_size = icmp_datagram_len - sizeof(icmp_header_t);
	*icmp_hd = (icmp_header_t) {
		.ICMP_HEADER_TYPE = ICMP_TYPE_ECHO_REQUEST,
		.ICMP_HEADER_CODE = 0,
		.ICMP_HEADER_ECHO.ICMP_HEADER_ID = ping->icmp_header_id,
		.ICMP_HEADER_ECHO.ICMP_HEADER_SEQ = sequence,
	};
	flip_endian_icmp(icmp_hd);
	size_t	data_offset = 0;

	// データバッファを埋める
	data_offset += set_timestamp_for_data(icmp_dt, icmp_dt_size);
	fill_data_with_pattern(ping, icmp_dt, data_offset, icmp_dt_size);

	// 最後にチェックサム計算
	icmp_hd->ICMP_HEADER_CHECKSUM = derive_icmp_checksum(datagram_buffer, icmp_datagram_len);
}
