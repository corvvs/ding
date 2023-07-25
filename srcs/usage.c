#include "ping.h"

void	print_usage(void) {
	printf("Usage: %s [OPTION...] HOST ...\n", PROGRAM_NAME);
	printf("Send ICMP ECHO_REQUEST packets to network hosts.\n");
	printf("\n");
	printf("  -c                         stop after sending NUMBER packets\n");
	printf("  -n                         do not resolve host addresses\n");
	printf("  -r                         send directly to a host on an attached network\n");
	printf("      --ttl=N                specify N as time-to-live\n");
	printf("  -T                         set type of service (TOS) to NUM\n");
	printf("  -v                         verbose output\n");
	printf("  -w                         stop after N seconds\n");
	printf("  -W                         number of seconds to wait for response\n");
	printf("  -f                         flood ping (root only)\n");
	printf("      --ip-timestamp=FLAG    IP timestamp of type FLAG, which is one of\n");
	printf("                             \"tsonly\" and \"tsaddr\"\n");
	printf("  -l                         send NUMBER packets as fast as possible before\n");
	printf("                             falling into normal mode of behavior (root only)\n");
	printf("  -p                         fill ICMP packet with given pattern (hex)\n");
	printf("  -s                         send NUMBER data octets\n");
	printf("\n");
	printf("  -?                         give this help list\n");
	printf("\n");
	printf("Options marked with (root only) are available only to superuser.\n");
	printf("\n");
	printf("Report bugs to <yokawada@student.42tokyo.jp>.\n");
}
