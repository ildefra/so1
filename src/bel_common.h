#ifndef BELCOMMON_H_INCLUDED
#define BELCOMMON_H_INCLUDED

#include <netdb.h>


#define COMM_PORT 7477

/*
 * Message lengths for client-server communication. Last byte will always be the
 * string terminator '\0', so the actual payload is always one byte shorter
 */
#define UNAME_MSGLEN 32
#define PWORD_MSGLEN 32
#define STD_MSGLEN 1024
#define ANSWER_MSGLEN 3

#define ANSWER_OK "OK"
#define ANSWER_KO "KO"

extern void bel_close_or_die(const int);
extern int bel_new_sock(const struct addrinfo);
extern void bel_getaddrinfo_or_die(
		const char* const, const u_short, struct addrinfo**);
extern void bel_print_address(const char* const, const struct sockaddr*);

extern void bel_recvall_or_die(const int, char*, const size_t);
extern void bel_sendall_or_die(const int, const char* const, const size_t);

#endif	/* BELCOMMON_H_INCLUDED */
