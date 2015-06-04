#ifndef BELCOMMON_H_INCLUDED
#define BELCOMMON_H_INCLUDED

#include <netdb.h>

#define COMM_PORT 3490
#define MAX_MSGLEN 1024

extern void bel_get_serverinfo(
		const char* const, const u_short, struct addrinfo**);

extern const char* bel_afamily_tostring(const int);
extern void* bel_get_inaddr(const struct sockaddr *sa);

extern int bel_open_sock(const struct addrinfo);
extern void bel_close_sock(const int);

#endif	/* BELCOMMON_H_INCLUDED */
