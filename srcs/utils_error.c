#include "ping.h"

void	print_error_by_message(const char* message) {
	dprintf(STDERR_FILENO, "%s: %s\n", PROGRAM_NAME, message);
}

void	print_error_by_errno(void) {
	dprintf(STDERR_FILENO, "%s: %s\n", PROGRAM_NAME, strerror(errno));
}

void	print_special_error_by_errno(const char* name) {
	dprintf(STDERR_FILENO, "%s: %s\n", name, strerror(errno));
}
