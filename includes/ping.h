#ifndef PING_H
#define PING_H

#include "ping_libs.h"
#include "ping_types.h"
#include "ping_constants.h"
#include "ping_structures.h"

// ping_run.c
int				ping_run(const t_preferences* prefs, char** hosts);

// ping_loop.c
void			ping_loop(t_ping* ping);

// ping_loop_send_ping.c
void			ping_loop_send_ping(t_ping* ping);

// ping_loop_wait_pong.c
void			ping_loop_wait_pong(t_ping* ping, const timeval_t* timeout);

// make_preference.c
bool			make_preference(char** argv, t_preferences* pref_ptr);

// option_aux.c
int				parse_number(const char* str, unsigned long* out, unsigned long min, unsigned long max);
int				parse_pattern(const char* str, char* buffer, size_t max_len);

// print_usage.c
void			print_usage(void);

// host_address.c
address_info_t*	resolve_str_into_address(const char* host_str);
bool			setup_target_from_host(const char* host, t_session* target);
uint32_t		serialize_address(const address_in_t* addr);
const char*		stringify_serialized_address(uint32_t addr32);
const char*		stringify_address(const address_in_t* addr);
void			print_address_serialized(const t_ping* ping, uint32_t addr);
void			print_address_struct(const t_ping* ping, const address_in_t* addr);

// socket.c
int create_icmp_socket(bool* inaccessible_ipheader, const t_preferences* prefs);

// ping_session.c
int	ping_session(t_ping* ping);

// ping_sender.c
int	send_request(t_ping* ping, uint16_t sequence);

// pong_receiver.c
t_received_result	receive_reply(const t_ping* ping, t_acceptance* acceptance);

// protocol_ip.c
void		flip_endian_ip(ip_header_t* header);
size_t		ihl_to_octets(size_t ihl);

// protocol_icmp.c
void		flip_endian_icmp(icmp_header_t* header);
uint16_t	derive_icmp_checksum(const void* datagram, size_t len);
bool		is_valid_icmp_checksum(const t_ping* ping, icmp_header_t* icmp_header, size_t icmp_datagram_size);
void		construct_icmp_datagram(
	const t_ping* ping,
	uint8_t* datagram_buffer,
	size_t datagram_len,
	uint16_t sequence
);

// print_echo_reply.c
void	print_echo_reply(const t_ping* ping, const t_acceptance* acceptance, double triptime);

// print_time_exceeded.c
void	print_time_exceeded(const t_ping* ping, const t_acceptance* acceptance);

// print_ip_timestamp.c
void		print_ip_timestamp(const t_ping* ping, const t_acceptance* acceptance);

// analyze_received_datagram.c
bool	analyze_received_datagram(const t_ping* ping, t_acceptance* acceptance);

// stats.c
double	record_received(t_ping* ping, const t_acceptance* acceptance);
void	record_echo_reply(t_ping* ping, t_acceptance* acceptance);
void	print_stats(const t_ping* ping);

// utils_math.c
double	ft_square(double x);
double	ft_sqrt(double x);

// utils_time.c
timeval_t	get_current_time(void);
timeval_t	add_times(const timeval_t* a, const timeval_t* b);
timeval_t	sub_times(const timeval_t* a, const timeval_t* b);
double		get_ms(const timeval_t* a);
double		diff_times(const timeval_t* a, const timeval_t* b);

// utils_error.c
void	print_error_by_message(const char* message);
void	print_error_by_errno(void);
void	print_special_error_by_errno(const char* name);
void	exit_with_error(int status, int error_no, const char* message);


// utils_debug.c
void	debug_hexdump(const char* label, const void* mem, size_t len);
void	debug_msg_flags(const struct msghdr* msg);
void	debug_ip_header(const void* mem);
void	debug_icmp_header(const void* mem);

#include "utils_endian.h"

#endif
