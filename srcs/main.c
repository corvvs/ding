#include "ping.h"

// システムのエンディアンがリトルエンディアンかどうか
int	g_is_little_endian;
// NOTE: エンディアン変換時に参照する

static t_preferences	determine_preference(char **argv) {
	t_preferences	pref = {};
	if (!make_preference(argv, &pref)) {
		exit(STATUS_OPERAND_FAILED);
	}
	return pref;
}

int main(int argc, char **argv) {
	(void)argc;

	// 最初にシステムのエンディアンを求める
	g_is_little_endian = is_little_endian();

	if (*argv == NULL) {
		exit(STATUS_GENERIC_FAILED);
	}
	// NOTE: プログラム名は使用しないので飛ばす
	argv += 1;

	const t_preferences	pref = determine_preference(argv);
	if (pref.show_usage) {
		print_usage();
		exit(STATUS_SUCCEEDED);
	}

	argv += pref.parsed_arguments;
	const int status = ping_run(&pref, argv);
	DEBUGWARN("finished: status: %d", status);
	return status;
}
