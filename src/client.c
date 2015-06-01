/*
 * client - Client part of the OS1 assignment
 */

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define SERVERIP "127.0.0.1"
#define MY_PORT 3490
#define PORT_MAXCHARS 5
#define IPVER_CHARS 4
#define MAX_MSGLEN 1024

#define CONN_MSG "Connecting to %s address %s ...\n"

/*
Includes usage:
<arpa/inet.h>:inet_ntop only
<netdb.h>   : getaddrinfo
<stdio.h>   : error logging only
<stdlib.h>  : exit statuses only
<string.h>  : memset only
<sys/types.h>:man says some OS could require it
<unistd.h>  : closing sockets only
*/

void close_sock(int);

/* Client entry point  */
int
main(int __unused argc, char __unused **argv)
{
	int sockfd;
    int connect_to(const char* const, u_short);
    void run_client(int);
    
    sockfd = connect_to(SERVERIP, MY_PORT);
    printf("connected\n");
    run_client(sockfd);
	close_sock(sockfd);
    return EXIT_SUCCESS;
}

/* 
 * Gets all the addresses associated to the given ip and port and connects to
 * the first available one
 */
int
connect_to(const char* const ip, const u_short port)
{
    struct addrinfo *servinfo, *currinfo;
    int sockfd;
    void get_serverinfo(const char* const, u_short, struct addrinfo**);
    int do_connect(struct addrinfo*);
    
    get_serverinfo(ip, port, &servinfo);
    for(currinfo = servinfo; currinfo != NULL; currinfo = currinfo->ai_next) {
        sockfd = do_connect(currinfo);
        if (sockfd != -1) break;
    }
    if (currinfo == NULL) {
        fprintf(stderr, "Failed to connect\n");
        exit(EXIT_FAILURE);
    }
    freeaddrinfo(servinfo);
    return sockfd;
}

/*
 * Performs the actual connection logic: creates a socket and uses it to connect
 * to the specified address.
 * returns the file descriptor of the newly-created socket, or -1 if an error
 * occurred.
 */
int
do_connect(struct addrinfo *ainfo)
{
	int sockfd, connect_res;
	void print_connecting(const int, const struct sockaddr*);
    
    sockfd = socket(ainfo->ai_family, ainfo->ai_socktype, ainfo->ai_protocol);
    if (sockfd == -1) {
        perror("socket()");
        return -1;
    }
    print_connecting(ainfo->ai_family, ainfo->ai_addr);
    connect_res = connect(sockfd, ainfo->ai_addr, ainfo->ai_addrlen);
    if (connect_res == -1) {
        close_sock(sockfd);
        perror("connect()");
        return -1;
    }
    return sockfd;
}


/* TODO: split! */
/*
 * Just (!!) prints the "connecting to..." message, switching on the given
 * address family to get the right fields from ai_addr
 */
void
print_connecting(const int ai_family, const struct sockaddr *ai_addr)
{
    void *addr;
    char ipstr[INET6_ADDRSTRLEN];
    const char* afamily_tostring(int);
    
    switch (ai_family) {
        case AF_INET:
            addr = &(((struct sockaddr_in*) ai_addr)->sin_addr);
            break;
        case AF_INET6:
            addr = &(((struct sockaddr_in6*) ai_addr)->sin6_addr);
            break;
        default:
            fprintf(
                    stderr, "FATAL: Unrecognized address family %d", ai_family);
            exit(EXIT_FAILURE);
    }

    inet_ntop(ai_family, addr, ipstr, sizeof(ipstr));    
    printf(CONN_MSG, afamily_tostring(ai_family), ipstr);
}


/*
 * Returns a 4-character string representation of the given address family.
 * Returns "????" on unknown families.
 */
const char*
afamily_tostring(const int afamily)
{
    char *ipver = malloc(IPVER_CHARS + 1);
    switch (afamily) {
        case AF_INET:
            ipver = "IPv4";
            break;
        case AF_INET6:
            ipver = "IPV6";
            break;
        default:
            ipver = "????";
            break;
    }
    return ipver;
}


/* TODO: split */
/* TODO: implement actual commands */
/* Actual business logic of the client  */
void
run_client(const int sockfd)
{
    char buff[MAX_MSGLEN + 1];  /* +1 for the null terminator  */
    int len;
    
    printf("TRACE: inside run_client\n");
    while (1) {
        printf("\nPlease enter a command: ");
        scanf("%s", buff);
        send(sockfd, buff, MAX_MSGLEN, 0);
        
        len = recv(sockfd, buff, MAX_MSGLEN, 0);
        if (len == -1) {
            perror("recv()");
            exit(EXIT_FAILURE);
        }
        buff[len] = '\0';   /* null terminator must be added after reading  */
        printf("Server answered: %s (%d bytes)\n", buff, len);
    }
}


/* code clone in server.c - BEGIN */

void
get_serverinfo(const char* const ip, u_short port, struct addrinfo **servinfo)
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


void
close_sock(const int sockfd)
{
    int close_result;
    
    close_result = close(sockfd);
	if (close_result < 0) {
        fprintf(stderr, "Could not close socket: exiting");
        exit(EXIT_FAILURE);
    }
}

/* code clone in server.c - END */
