/* bel_common - Functions shared by bel_client and bel_server  */

#include "bel_common.h"
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


/* Maximum number of characters (digits) a port can have  */
#define PORT_MAXCHARS 5


static struct addrinfo make_hints(const int);

static void* get_inaddr(const struct sockaddr*);
static const char* afamily_tostring(const int);

static ssize_t do_recv_or_die(const int, char*, const size_t);
static ssize_t do_send_or_die(const int, const char* const, const size_t);


void
bel_close_or_die(const int fd)
{
    int close_result = 0;
    
    if (fd < 0) return;
    printf("[DEBUG] closing file with fd = '%d'\n", fd);
    close_result = close(fd);
	if (close_result == -1) {
        perror("[ERROR] close()");
        exit(EXIT_FAILURE);
    }
}


int
bel_new_sock(const struct addrinfo ainfo)
{
    int sockfd;
    
    sockfd = socket(ainfo.ai_family, ainfo.ai_socktype, ainfo.ai_protocol);
    if (sockfd == -1) perror("[WARN] socket()");
    printf("[DEBUG] created socket with fd = '%d'\n", sockfd);
    
    return sockfd;
}


void
bel_getaddrinfo_or_die(
        const char* const ip, const int protocol, const u_short port,
        struct addrinfo **servinfo)
{
    char port_str[PORT_MAXCHARS] = "";
    struct addrinfo hints = {0};
    int getaddrinfo_res = 0;
    const char* const err_msg = "[FATAL] getaddrinfo(): %s\n";
    
    printf("[TRACE] inside bel_getaddrinfo_or_die\n");
    sprintf(port_str, "%d", port);
    hints = make_hints(protocol);
    if (ip == NULL) hints.ai_flags = AI_PASSIVE;
    getaddrinfo_res = getaddrinfo(ip, port_str, &hints, servinfo);
    if (getaddrinfo_res != 0) {
        fprintf(stderr, err_msg, gai_strerror(getaddrinfo_res));
        exit(EXIT_FAILURE);
    }
}

static struct addrinfo
make_hints(const int protocol)
{
    struct addrinfo hints = {0};
    
    hints.ai_family     = protocol;
    hints.ai_socktype   = SOCK_STREAM;
    return hints;
}


void
bel_print_address(const char* const prefix, const struct sockaddr *sa)
{
    char ipstr[INET6_ADDRSTRLEN] = "";
    char *full_template = NULL;
    const char* const addr_template = "%s address %s\n";
    
    inet_ntop(sa->sa_family, get_inaddr(sa), ipstr, sizeof(ipstr));
    full_template = bel_concat(prefix, addr_template);
    printf(full_template, afamily_tostring(sa->sa_family), ipstr);
    free(full_template);
}

/*
 * Gets the internet address from the given sockaddr, switching between IPv4
 * and IPv6 depending on the address family (IPv4 or IPv6).
 * Returns NULL on error
 */
static void*
get_inaddr(const struct sockaddr *sa)
{
    void *in_addr = NULL;
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
        break;
    }
    return in_addr;
}

/*
 * Returns a 4-character string representation of the given address family.
 * Returns "????" on unknown families.
 */
static const char*
afamily_tostring(const int afamily)
{
    switch (afamily) {
    case AF_INET:   return "IPv4";
    case AF_INET6:  return "IPv6";
    default:        return "????";
    }
}


void
bel_recvall_or_die(const int sockfd, char *buf, const size_t len)
{
    char *ptr = NULL;
    size_t  bytes_left = 0;
    ssize_t bytes_read = 0;
    
    printf("[TRACE] bel_recvall_or_die - buf = '%s', len = '%lu'\n",
            buf, (unsigned long) len);
    ptr = buf;
    bytes_left = len;
    while (bytes_left > 0) {
        bytes_read =
                do_recv_or_die(sockfd, ptr + len - bytes_left, bytes_left);
        bytes_left -= bytes_read;
    }
    buf[len - 1] = '\0';    /* must add the null terminator after reading  */
    printf("[DEBUG] message received: '%s'\n", buf);
}

/*
 * Performs the recv() system call, and exits the program on failure or
 * disconnection
 */
static ssize_t
do_recv_or_die(const int sockfd, char *buf, const size_t len) {
    ssize_t bytes_read = 0;
    
    printf("[TRACE] do_recv_or_die - buf = '%s', len = '%lu'\n",
            buf, (unsigned long) len);
    bytes_read = recv(sockfd, buf, len, 0);
    printf("[TRACE] recv() syscall returned '%ld'\n", (long) bytes_read);
    if (bytes_read == -1) {
        perror("[ERROR] recv()");
        exit(EXIT_FAILURE);
    }
    if (bytes_read == 0) {
        printf("[DEBUG] socket '%d': connection reset by peer\n", sockfd);
        exit(EXIT_SUCCESS);
    }
    return bytes_read;
}


void
bel_sendall_or_die(const int sockfd, const char* const buf, const size_t len)
{
    size_t  bytes_left = 0;
    ssize_t bytes_sent = 0;

    printf("[TRACE] bel_sendall_or_die - buf = '%s', len = '%lu'\n",
            buf, (unsigned long) len);    
    bytes_left = len;
    while (bytes_left > 0) {
        bytes_sent =
                do_send_or_die(sockfd, buf + len - bytes_left, bytes_left);
        bytes_left -= bytes_sent;
    }
}

/*
 * Performs the send() system call, and exits the program on failure or
 * disconnection.
 * The MSG_NOSIGNAL flag is used during the call, in order to make send()
 * return a 'Broken Pipe' error instead of a SIGPIPE, which would crash the
 * application if unhandled (it exits anyway, but at least an error message is
 * shown)
 */
static ssize_t
do_send_or_die(const int sockfd, const char* const buf, const size_t len) {
    ssize_t bytes_sent = 0;
    
    printf("[TRACE] do_send_or_die - buf = '%s', len = '%lu'\n",
            buf, (unsigned long) len);
    bytes_sent = send(sockfd, buf, len, MSG_NOSIGNAL);
    printf("[TRACE] send() syscall returned '%ld'\n", (long) bytes_sent);
    if (bytes_sent == -1) {
        perror("[ERROR] send()");
        exit(EXIT_FAILURE);
    }
    if (bytes_sent == 0) {
        printf("[DEBUG] socket '%d': connection reset by peer\n", sockfd);
        exit(EXIT_SUCCESS);
    }
    return bytes_sent;
}


char*
bel_concat(const char* const s1, const char* const s2)
{
    size_t len1, len2;
    char *result = NULL;
    
    len1 = strlen(s1);
    len2 = strlen(s2);
    result = malloc(len1 + len2 + 1);  /* +1 for the zero terminator  */
    if (result == NULL) return NULL;
    
    memcpy(result, s1, len1);
    
    /* +1 in order to also copy the zero terminator  */
    memcpy(result + len1, s2, len2 + 1);
    
    return result;
}


void
bel_chop_newline(char *str)
{
    size_t len;
    
    len = strlen(str);
    if (len > 0 && str[len-1] == '\n') str[len-1] = '\0';
}
