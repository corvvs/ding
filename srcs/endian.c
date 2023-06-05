#include "ping.h"

typedef union u_endian_field {
	uint32_t	u;
	uint8_t		c[4];
}	t_endian_field;

// 現在の環境がリトルエンディアンかどうかを返す
bool	is_little_endian(void) {
	t_endian_field	f;
	f.u = 0xff;
	return f.c[0] == 0xff;
}

uint16_t swap_2byte(uint16_t value) {
	value = ((value <<  8) & 0xff00) | ((value >>  8) & 0x00ff);
	return value;
}

uint32_t swap_4byte(uint32_t value) {
	value = ((value <<  8) & 0xff00ff00) | ((value >>  8) & 0x00ff00ff);
	value = ((value >> 16) & 0x0000ffff) | ((value << 16) & 0xffff0000);
	return value;
}

uint64_t swap_8byte(uint64_t value) {
	value = ((value <<  8) & 0xff00ff00ff00ff00) | ((value >>  8) & 0x00ff00ff00ff00ff);
	value = ((value >> 16) & 0x0000ffff0000ffff) | ((value << 16) & 0xffff0000ffff0000);
	value = ((value >> 32) & 0x00000000ffffffff) | ((value << 32) & 0xffffffff00000000);
	return value;
}
