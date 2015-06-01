/* bel_common - Functions shared by bel_client and bel_server  */

#include "bel_common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define PORT_MAXCHARS 5


/*
 * Returns a 4-character string representation of the given address family.
 * Returns "????" on unknown families.
 */
const char*
bel_afamily_tostring(const int afamily)
{
    char *ipver;
    const int ipver_chars = 4;
    
    ipver = malloc(ipver_chars + 1);
    switch (afamily) {
        case AF_INET:
            ipver = "IPv4";
            break;
        case AF_INET6:
            ipver = "IPv6";
            break;
        default:
            ipver = "????";
            break;
    }
    return ipver;
}


/*
 * Fills servinfo with the list of internet addresses associated with the given
 * ip and port
 */
void
bel_get_serverinfo(
        const char* const ip, const u_short port, struct addrinfo **servinfo)
{
    char port_str[PORT_MAXCHARS];
    struct addrinfo hints;
    int getaddrinfo_res;
    struct addrinfo make_hints(void);
    
    printf("TRACE: inside get_serverinfo\n");
    sprintf(port_str, "%d", port);
    hints = make_hints();
    if (ip == NULL) hints.ai_flags = AI_PASSIVE;
    getaddrinfo_res = getaddrinfo(ip, port_str, &hints, servinfo);
    if (getaddrinfo_res != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(getaddrinfo_res));
        exit(EXIT_FAILURE);
    }
}

struct addrinfo
make_hints(void)
{
    struct addrinfo hints;
    
    memset(&hints, 0, sizeof(hints));
    hints.ai_family     = AF_UNSPEC;
    hints.ai_socktype   = SOCK_STREAM;
    
    return hints;
}


/* Closes the given socket. Exits on error  */
void
bel_close_sock(const int sockfd)
{
    int close_result;
    
    close_result = close(sockfd);
	if (close_result < 0) {
        fprintf(stderr, "[FATAL] Could not close socket: exiting");
        exit(EXIT_FAILURE);
    }
}
