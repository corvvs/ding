#include "ping.h"

#define MAX_PACKET_SIZE (1u << 16)

int	g_is_little_endian;

static size_t       set_ip_header(void* buffer, const size_t max_buffer_len) {
    (void)max_buffer_len;
    ip_header_t*    ip_header = (ip_header_t*)buffer;
    *ip_header = (ip_header_t){
        .IP_HEADER_VER = 4,
        .IP_HEADER_HL = 5,
        .IP_HEADER_TOS = 0,
        .IP_HEADER_TOTLEN = sizeof(ip_header_t) + sizeof(icmp_header_t),
        .IP_HEADER_ID = htons(54321),
        .IP_HEADER_OFF = 0,
        .IP_HEADER_TTL = 255,
        .IP_HEADER_PROT = IPPROTO_ICMP,
    };
    return sizeof(ip_header_t);
}

// ICMPエコーリプライの作成
static size_t        create_packet(void* buffer, const size_t max_buffer_len, uint16_t icmp_id_val) {
    (void)max_buffer_len;
    size_t len = 0;
    len += set_ip_header(buffer, max_buffer_len);

    icmp_header_t *icmp = (icmp_header_t *)(buffer + len);

    size_t icmp_len = sizeof(icmp_header_t);
    icmp->ICMP_HEADER_TYPE = ICMP_ECHOREPLY;
    icmp->ICMP_HEADER_CODE = 0;
    icmp->ICMP_HEADER_CHECKSUM = 0;
    uint16_t    seq = 1;
    icmp->ICMP_HEADER_ECHO.ICMP_HEADER_ID = SWAP_NEEDED(icmp_id_val);  // 任意のID
    icmp->ICMP_HEADER_ECHO.ICMP_HEADER_SEQ = SWAP_NEEDED(seq);  // シーケンス番号
    icmp->ICMP_HEADER_CHECKSUM = derive_icmp_checksum(icmp, icmp_len);
    len += icmp_len;

    return len;
}

static int  create_socket(void) {
    int sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sockfd < 0) {
        perror("socket");
        exit(1);
    }

    int on = 1;
    // IPヘッダの自動補完を無効化
    if (setsockopt(sockfd, IPPROTO_IP, IP_HDRINCL, &on, sizeof(on)) < 0) {
        perror("setsockopt");
        exit(1);
    }

    return sockfd;
}

static void send_packet(int sockfd, const char* dest_host, const socket_address_in_t* dest_addr, uint16_t icmp_id_val) {
    unsigned char packet[MAX_PACKET_SIZE] = {};

    const size_t total_len = create_packet(packet, MAX_PACKET_SIZE, icmp_id_val);

    if (sendto(sockfd, packet, total_len, 0, (socket_address_t *)dest_addr, sizeof(socket_address_in_t)) <= 0) {
        perror("sendto");
        exit(1);
    }

    printf("ICMP Echo Reply sent to %s\n", dest_host);
}

static socket_address_in_t create_dest_addr(const char* dest_host) {
    socket_address_in_t dest_addr = {
        .sin_family = AF_INET,
        .sin_port = htons(0),  // ICMPはトランスポート層のプロトコルではないのでポートは不要
    };
    inet_pton(AF_INET, dest_host, &(dest_addr.sin_addr));
    return dest_addr;
}

int main(int argc, char *argv[]) {
	g_is_little_endian = is_little_endian();

    if (argc < 2) {
        printf("Usage: %s <destination IP> <ID>\n", argv[0]);
        exit(1);
    }

    int send_count = 10;
    int sleep_after_sending = 1;

    const char* dest_host = argv[1];
    uint16_t icmp_id_val = atoi(argv[2]);
    int sockfd = create_socket();
    socket_address_in_t dest_addr = create_dest_addr(dest_host);

    for (; send_count > 0; send_count--) {
        send_packet(sockfd, dest_host, &dest_addr, icmp_id_val);
        sleep(sleep_after_sending);
    }

    close(sockfd);
    return 0;
}
