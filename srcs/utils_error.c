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

// glibc error の代替
void	exit_with_error(int status, int error_no, const char* message) {
	dprintf(STDERR_FILENO, "%s: %s\n", message, strerror(error_no));
	exit(status);
}
