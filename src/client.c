/*
 * server.c -- server part of the OS1 assignment
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
#define MAX_MSGLEN 1024

/*
includes usage:
<arpa/inet.h>:inet_ntop only
<netdb.h>   : getaddrinfo
<stdio.h>   : error logging only
<stdlib.h>  : exit statuses only
<string.h>  : memset only
<sys/types.h>:man says some OS could require it
<unistd.h>  : closing sockets only
*/

void close_sock(int);

/* client entry point */
int main(int __unused argc, char __unused **argv) {
	int sockfd;
    int connect_to(const char* const, u_short);
    void run_client(int);
    
    sockfd = connect_to(SERVERIP, MY_PORT);
    run_client(sockfd);
	close_sock(sockfd);
    return EXIT_SUCCESS;
}

/* TODO: split */
/* connects to the given ip and port */
int connect_to(const char* const ip, u_short port) {
    struct addrinfo *servinfo, *currinfo;
    int sockfd, connect_res;
    void getserverinfo(const char* const, u_short, struct addrinfo**);
    void print_connecting(const int, const struct sockaddr*);
    
    getserverinfo(ip, port, &servinfo);
    
    /* loop through all the results and connect to the first we can */
    for(currinfo = servinfo; currinfo != NULL; currinfo = currinfo->ai_next) {
        sockfd = socket(
                currinfo->ai_family, currinfo->ai_socktype, currinfo->ai_protocol);
        if (sockfd == -1) {
            perror("socket()");
            continue;
        }
        
        print_connecting(currinfo->ai_family, currinfo->ai_addr);
        connect_res = connect(sockfd, currinfo->ai_addr, currinfo->ai_addrlen);
        if (connect_res == -1) {
            close_sock(sockfd);
            perror("connect()");
            continue;
        }

        break;
    }

    if (currinfo == NULL) {
        fprintf(stderr, "failed to connect\n");
        exit(EXIT_FAILURE);
    }

    printf("connected\n");

    freeaddrinfo(servinfo); /* all done with this structure */
    
    return sockfd;
}

void getserverinfo(
        const char* const ip, u_short port, struct addrinfo **servinfo) {
    char port_str[PORT_MAXCHARS];
    struct addrinfo hints;
    int getaddrinfo_res;
    struct addrinfo make_tcp_hints(void);
    
    printf("TRACE: inside getserverinfo\n");
    sprintf(port_str, "%d", port);
    hints = make_tcp_hints();
    getaddrinfo_res = getaddrinfo(ip, port_str, &hints, servinfo);
    if (getaddrinfo_res != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(getaddrinfo_res));
        exit(EXIT_FAILURE);
    }
}

struct addrinfo make_tcp_hints(void) {
    struct addrinfo hints;
    
    memset(&hints, 0, sizeof(hints));
    hints.ai_family     = AF_UNSPEC;
    hints.ai_socktype   = SOCK_STREAM;
    
    return hints;
}

/* TODO: split! */
/*
 * just (!!) prints the "connecting to..." message, switching on the address
 * family to get the right fields
 */
void print_connecting(const int ai_family, const struct sockaddr *ai_addr) {
    void *addr;
    char *ipver;
    char ipstr[INET6_ADDRSTRLEN];

    switch (ai_family) {
        case AF_INET:
            addr = &(((struct sockaddr_in*) ai_addr)->sin_addr);
            ipver = "IPv4";
            break;
        case AF_INET6:
            addr = &(((struct sockaddr_in6*) ai_addr)->sin6_addr);
            ipver = "IPv6";
            break;
        default:
            fprintf(
                    stderr, "FATAL: unrecognized address family %d", ai_family);
            exit(EXIT_FAILURE);
    }

    inet_ntop(ai_family, addr, ipstr, sizeof(ipstr));    
    printf("connecting to %s address %s ...\n", ipver, ipstr);
}


/* TODO: implement actual commands */
void run_client(const int sockfd) {
    char buff[MAX_MSGLEN + 1];  /* +1 for the null terminator */
    int len;
    
    printf("TRACE: inside run_client\n");
    while (1) {
        printf("\nplease enter a command: ");
        scanf("%s", buff);
        send(sockfd, buff, MAX_MSGLEN, 0);
        
        len = recv(sockfd, buff, MAX_MSGLEN, 0);
        buff[len] = '\0';   /* null terminator must be added after reading */
        printf("server answered: %s (%d bytes)\n", buff, len);
    }
}


/* TODO: code clone in server.c */
void close_sock(const int sockfd) {
    int close_result;
    
    close_result = close(sockfd);
	if (close_result < 0) {
        fprintf(stderr, "could not close socket: exiting");
        exit(EXIT_FAILURE);
    }
}
