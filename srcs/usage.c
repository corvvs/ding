#include "ping.h"

void	print_usage(void) {
	printf("Usage: %s [OPTION...] HOST ...\n", PROGRAM_NAME);
	printf("Send ICMP ECHO_REQUEST packets to network hosts.\n");
	printf("\n");
	printf("  -c, --count=NUMBER         stop after sending NUMBER packets\n");
	printf("  -n, --numeric              do not resolve host addresses\n");
	printf("  -r, --ignore-routing       send directly to a host on an attached network\n");
	printf("  -m, --ttl=N                specify N as time-to-live\n");
	printf("  -T, --tos=NUM              set type of service (TOS) to NUM\n");
	printf("  -v, --verbose              verbose output\n");
	printf("  -w, --timeout=N            stop after N seconds\n");
	printf("  -W, --linger=N             number of seconds to wait for response\n");
	printf("  -f                         flood ping (root only)\n");
	printf("      --ip-timestamp=FLAG    IP timestamp of type FLAG, which is one of\n");
	printf("                             \"tsonly\" and \"tsaddr\"\n");
	printf("  -l, --preload=NUMBER       send NUMBER packets as fast as possible before\n");
	printf("                             falling into normal mode of behavior (root only)\n");
	printf("  -p                         fill ICMP packet with given pattern (hex)\n");
	printf("  -s, --size=NUMBER          send NUMBER data octets\n");
	printf("\n");
	printf("  -?, --help                 give this help list\n");
	printf("\n");
	printf("Options marked with (root only) are available only to superuser.\n");
	printf("\n");
	printf("Report bugs to <yokawada@student.42tokyo.jp>.\n");
}
