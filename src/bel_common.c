/* bel_common - Functions shared by bel_client and bel_server  */

#include "bel_common.h"
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define PORT_MAXCHARS 5


/*
 * Closes the given file (a socket is a file). Does nothing on invalid
 * descriptors. Exits on error
 */
void
bel_close_or_die(const int fd)
{
    int close_result;
    
    if (fd < 0) return;
    printf("[DEBUG] closing file with fd = '%d'\n", fd);
    close_result = close(fd);
	if (close_result == -1) {
        perror("[ERROR] close()");
        exit(EXIT_FAILURE);
    }
}


/*
 * Creates a new socket and returns its file descriptor, or -1 in case of error
 */
int
bel_new_sock(const struct addrinfo ainfo)
{
    int sockfd;
    
    sockfd = socket(ainfo.ai_family, ainfo.ai_socktype, ainfo.ai_protocol);
    if (sockfd == -1) perror("[WARN] socket()");
    printf("[DEBUG] created socket with fd = '%d'\n", sockfd);
    
    return sockfd;
}


/*
 * Fills servinfo with the list of internet addresses associated with the given
 * ip and port. Exits program on error!
 */
void
bel_getaddrinfo_or_die(
        const char* const ip, const u_short port, struct addrinfo **servinfo)
{
    char port_str[PORT_MAXCHARS];
    struct addrinfo hints;
    int getaddrinfo_res;
    const char* const err_msg = "[FATAL] getaddrinfo(): %s\n";
    
    struct addrinfo make_hints(void);
    
    printf("[TRACE] inside bel_getaddrinfo_or_die\n");
    sprintf(port_str, "%d", port);
    hints = make_hints();
    if (ip == NULL) hints.ai_flags = AI_PASSIVE;
    getaddrinfo_res = getaddrinfo(ip, port_str, &hints, servinfo);
    if (getaddrinfo_res != 0) {
        fprintf(stderr, err_msg, gai_strerror(getaddrinfo_res));
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


/*
 * prints a string representation of the given socket address, prepending the
 * given prefix
 */
void
bel_print_address(const char* const prefix, const struct sockaddr *sa)
{
    char ipstr[INET6_ADDRSTRLEN];
    char* full_template;
    const char* const addr_template = "%s address %s\n";
    
    void* get_inaddr(const struct sockaddr *sa);
    char* concat(const char* const s1, const char* const s2);
    const char* afamily_tostring(const int);
    
    inet_ntop(sa->sa_family, get_inaddr(sa), ipstr, sizeof(ipstr));
    full_template = concat(prefix, addr_template);
    printf(full_template, afamily_tostring(sa->sa_family), ipstr);
    free(full_template);
}

/*
 * Gets the internet address from the given sockaddr, switching between IPv4 and
 * IPv6 depending on the address family (IPv4 or IPv6). Returns NULL on error
 */
void*
get_inaddr(const struct sockaddr *sa)
{
    void *in_addr;
    const char* const err_msg = "[ERROR] unrecognized address family '%d'\n";
    
    switch (sa->sa_family) {
        case AF_INET:
            in_addr = &(((struct sockaddr_in*) sa)->sin_addr);
            break;
        case AF_INET6:
            in_addr = &(((struct sockaddr_in6*) sa)->sin6_addr);
            break;
        default:
            fprintf(stderr, err_msg, sa->sa_family);
            in_addr = NULL;
    }
    return in_addr;
}

/*
 * concatenates the two given strings on a newly-allocated block of memory. It
 * is responsibility of the caller to free the memory when it is no longer
 * needed. Returns NULL if malloc() fails
 */
char*
concat(const char* const s1, const char* const s2)
{
    size_t len1, len2;
    char *result;
    
    len1 = strlen(s1);
    len2 = strlen(s2);
    result = malloc(len1 + len2 + 1);  /* +1 for the zero terminator  */
    if (result == NULL) return NULL;
    
    memcpy(result, s1, len1);
    
    /* +1 in order to also copy the zero terminator  */
    memcpy(result + len1, s2, len2 + 1);
    
    return result;
}

/*
 * Returns a 4-character string representation of the given address family.
 * Returns "????" on unknown families.
 */
const char*
afamily_tostring(const int afamily)
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
 * reads <len> bytes of data to <buf> from the socket <sockfd>. Exits the
 * process on failure or disconnection
 */
void
bel_recvall_or_die(const int sockfd, char *buf, const size_t len)
{
    size_t bytes_left;
    ssize_t bytes_read;
    const char* const trace_msg = "[TRACE] %zd bytes read, %zu remaining\n";
    
    ssize_t do_recv_or_die(const int, char*, const size_t);
    
    printf("[DEBUG] reading %zu bytes of data from socket '%d'\n", len, sockfd);
    bytes_left = len;
    while (bytes_left > 0) {
        bytes_read = do_recv_or_die(sockfd, buf + len - bytes_left, bytes_left);
        bytes_left -= bytes_read;
        printf(trace_msg, bytes_read, bytes_left);
    }
    buf[bytes_read] = '\0';   /* null terminator must be added after reading */
    printf("[DEBUG] message received: '%s'\n", buf);
}

/*
 * performs the recv() syscall, and exits the program on failure or
 * disconnection
 */
ssize_t
do_recv_or_die(const int sockfd, char *buf, const size_t len) {
    ssize_t bytes_read;
    
    bytes_read = recv(sockfd, buf, len, 0);
    if (bytes_read == -1) {
        perror("[ERROR] recv()");
        bel_close_or_die(sockfd);
        exit(EXIT_FAILURE);
    }
    if (bytes_read == 0) {
        printf("[DEBUG] socket '%d': connection reset by peer\n", sockfd);
        bel_close_or_die(sockfd);
        exit(EXIT_SUCCESS);        
    }
    return bytes_read;
}


/*
 * sends <len> bytes of data from <buf> to the socket <sockfd>. Exits the
 * process on failure or disconnection
 */
void
bel_sendall_or_die(const int sockfd, const char* const buf, const size_t len)
{
    size_t bytes_left;
    ssize_t bytes_sent;
    const char* const trace_msg = "[TRACE] %zd bytes sent, %zu remaining\n";

    ssize_t do_send_or_die(const int, const char* const, const size_t);
    
    printf("[DEBUG] sending %zu bytes of data to socket '%d'\n", len, sockfd);
    bytes_left = len;
    while (bytes_left > 0) {
        bytes_sent = do_send_or_die(sockfd, buf + len - bytes_left, bytes_left);
        bytes_left -= bytes_sent;
        printf(trace_msg, bytes_sent, bytes_left);
    }
}

/*
 * performs the send() syscall, and exits the program on failure or
 * disconnection
 */
ssize_t
do_send_or_die(const int sockfd, const char* const buf, const size_t len) {
    ssize_t bytes_sent;
    
    void bel_close_sock(int);
    
    bytes_sent = send(sockfd, buf, len, 0);
    if (bytes_sent == -1) {
        perror("[ERROR] send()");
        bel_close_or_die(sockfd);
        exit(EXIT_FAILURE);
    }
    if (bytes_sent == 0) {
        printf("[DEBUG] socket '%d': connection reset by peer\n", sockfd);
        bel_close_or_die(sockfd);
        exit(EXIT_SUCCESS);        
    }
    return bytes_sent;
}
