#ifndef BELCOMMON_H_INCLUDED
#define BELCOMMON_H_INCLUDED

#include <netdb.h>

#define COMM_PORT 3490
#define MAX_MSGLEN 1024

extern void bel_get_serverinfo(
		const char* const, const u_short, struct addrinfo**);
extern void bel_print_address(const char* const, const struct sockaddr*);

extern int bel_open_sock(const struct addrinfo);
extern void bel_close_sock(const int);

#endif	/* BELCOMMON_H_INCLUDED */
