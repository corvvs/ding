#include "ping.h"

// システムのエンディアンがリトルエンディアンかどうか
int	g_is_little_endian;
// NOTE: エンディアン変換時に参照する

int main(int argc, char **argv) {
	(void)argc;
	if (*argv == NULL) {
		return STATUS_GENERIC_FAILED;
	}

	// 最初にシステムのエンディアンを求める
	g_is_little_endian = is_little_endian();

	argv += 1;
	// NOTE: プログラム名は使用しないので飛ばす

	t_preferences	pref = {};
	int				n_parsed_args = set_preference(argv, &pref);
	if (n_parsed_args < 0) {
		return STATUS_OPERAND_FAILED;
	}
	if (pref.show_usage) {
		print_usage();
		return STATUS_SUCCEEDED;
	}

	argv += n_parsed_args;

	const int status = ping_run(&pref, argv);
	DEBUGWARN("finished: status: %d", status);
	return status;
}
