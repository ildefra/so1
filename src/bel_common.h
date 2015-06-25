#ifndef BELCOMMON_H_INCLUDED
#define BELCOMMON_H_INCLUDED

#include <netdb.h>


#define COMM_PORT 7477


/*
 * Message lengths for client-server communication. Last byte will always be
 * the string terminator '\0', so the actual payload is always one byte shorter
 */
 
#define UNAME_MSGLEN 32
#define PWORD_MSGLEN 32

#define CMD_MSGLEN 7
#define CMD_READ	"READ"
#define CMD_SEND	"SEND"
#define CMD_DELETE	"DELETE"

#define ID_MSGLEN 7

#define ANSWER_MSGLEN 3
#define ANSWER_OK "OK"
#define ANSWER_KO "KO"

#define TXT_MSGLEN 256

/*
 * This one must be big enough to contain the full message list, so we set it
 * to over nine thousand
 */
#define LIST_MSGLEN 9001


typedef void (*Action)();


/*
 * Closes the given file (a socket is a file). Does nothing on invalid
 * descriptors. Exits on error
 */
extern void bel_close_or_die(const int);

/*
 * Creates a new socket and returns its file descriptor, or -1 in case of error
 */
extern int bel_new_sock(const struct addrinfo);

/*
 * Fills servinfo with the list of internet addresses associated with the given
 * ip, protocol and port. Accepted values for protocol are AF_INET (ipv4),
 * AF_INET6 (ipv6) and AF_UNSPEC (both).
 * Exits program on error
 */
extern void bel_getaddrinfo_or_die(
		const char* const, const int, const u_short, struct addrinfo**);

/*
 * Prints a string representation of the given socket address, prepending the
 * given prefix
 */
extern void bel_print_address(const char* const, const struct sockaddr*);


/*
 * Reads <len> bytes of data to <buf> from the socket <sockfd>.
 * Exits the process on failure or disconnection
 */
extern void
bel_recvall_or_die(const int sockfd, char* buf, const size_t len);

/*
 * Sends <len> bytes of data from <buf> to the socket <sockfd>.
 * Exits the process on failure or disconnection
 */
extern void
bel_sendall_or_die(const int sockfd, const char* const buf, const size_t len);


/*
 * Concatenates the two given strings on a newly-allocated block of memory.
 * It is responsibility of the caller to free the memory when it is no longer
 * needed.
 * Returns NULL if malloc() fails
 */
extern char*
bel_concat(const char* const, const char* const);


/* Removes the last character of the given string if it is a newline  */
extern void
bel_chop_newline(char*);

#endif	/* BELCOMMON_H_INCLUDED */
