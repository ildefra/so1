#ifndef BELCOMMON_H_INCLUDED
#define BELCOMMON_H_INCLUDED

#include <netdb.h>

#define COMM_PORT 3490
#define MSG_MAXLEN 1024
#define UNAME_MAXLEN 32
#define PWORD_MAXLEN 32

extern void bel_getaddrinfo_or_die(
		const char* const, const u_short, struct addrinfo**);
extern void bel_print_address(const char* const, const struct sockaddr*);

extern int bel_new_sock(const struct addrinfo);
extern void bel_close_or_die(const int);

#endif	/* BELCOMMON_H_INCLUDED */
