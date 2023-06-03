#ifndef YOYO_COMMON_H
#define YOYO_COMMON_H

// [デバッグ用出力]

#include <unistd.h>
#include <stdio.h>

#define TX_BLD "\e[01m"
#define TX_GRY "\e[30m"
#define TX_RED "\e[31m"
#define TX_GRN "\e[32m"
#define TX_YLW "\e[33m"
#define TX_BLU "\e[34m"
#define TX_MAZ "\e[35m"
#define TX_CYN "\e[36m"
#define BG_BLK "\e[40m"
#define BG_RED "\e[41m"
#define BG_YLW "\e[43m"
#define BG_CYN "\e[46m"
#ifdef __MACH__
#define TX_RST "\e[39;49m\e[0m"
#else
#define TX_RST "\e[0m"
#endif

#define YOYO_DPRINTF(...)                    \
	{                                        \
		dprintf(STDERR_FILENO, __VA_ARGS__); \
	}

#ifdef DEBUG
#define DEBUGSTRN(format) YOYO_DPRINTF("%s[%s:%d %s] " format "%s", TX_GRY, __FILE__, __LINE__, __func__, TX_RST)
#define DEBUGSTR(format) YOYO_DPRINTF("%s[%s:%d %s] " format "%s\n", TX_GRY, __FILE__, __LINE__, __func__, TX_RST)
#define DEBUGOUT(format, ...) YOYO_DPRINTF("%s[D] [%s:%d %s] " format "%s\n", TX_GRY, __FILE__, __LINE__, __func__, __VA_ARGS__, TX_RST)
#define DEBUGINFO(format, ...) YOYO_DPRINTF("[I] [%s:%d %s] " format "\n", __FILE__, __LINE__, __func__, __VA_ARGS__)
#define DEBUGWARN(format, ...) YOYO_DPRINTF("%s[W] [%s:%d %s] " format "%s\n", TX_YLW, __FILE__, __LINE__, __func__, __VA_ARGS__, TX_RST)
#define DEBUGERR(format, ...) YOYO_DPRINTF("%s[E] [%s:%d %s] " format "%s\n", TX_RED, __FILE__, __LINE__, __func__, __VA_ARGS__, TX_RST)
#define DEBUGSAY(format, ...) YOYO_DPRINTF("[s] [%s:%d %s] " format "\n", __FILE__, __LINE__, __func__, __VA_ARGS__)
#else
#define DEBUGSTRN(format) ((void)0)
#define DEBUGSTR(format) ((void)0)
#define DEBUGOUT(format, ...) ((void)0)
#define DEBUGINFO(format, ...) ((void)0)
#define DEBUGWARN(format, ...) ((void)0)
#define DEBUGERR(format, ...) ((void)0)
#define DEBUGSAY(format, ...) ((void)0)
#endif
#define DEBUGFATAL(format, ...) YOYO_DPRINTF("%s[F] [%s:%d %s] " format "%s\n", BG_RED, __FILE__, __LINE__, __func__, __VA_ARGS__, TX_RST)

#endif
