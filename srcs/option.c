#include "ping.h"

int	parse_option(int argc, char** argv, t_preferences* pref) {
	int parsed = 0;
	while (argc > 0) {
		const char*	arg = *argv;
		if (arg == NULL) {
			// ありえないはずだが・・・
			DEBUGERR("%s", "argv has an NULL");
			return -1;
		}
		if (*arg != '-') {
			// オプションではない
			break;
		}
		// arg はおそらくオプションなので解析する
		DEBUGOUT("parsing: %s", arg);
		while (*++arg) {
			DEBUGOUT("*arg = %c(%d)", *arg, *arg);
			switch (*arg) {
				case 'v':
					pref->verbose = true;
					break;
				default:
					// 未知のオプション
					DEBUGERR("ping: invalid option -- '%c'", *arg);
					return -1;
			}
		}
		parsed += 1;
		argc -= 1;
		argv += 1;
	}
	return parsed;
}
