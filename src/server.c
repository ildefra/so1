/*
 * server - server part of the OS1 assignment
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

#define MY_PORT 3490
#define PORT_MAXCHARS 5
#define MAX_MSGLEN 1024

/*
includes usage:
<arpa/inet.h>:inet_ntoa only
<netdb.h>   : getaddrinfo
<stdio.h>   : error logging only
<stdlib.h>  : exit statuses only
<string.h>  : memset only
<sys/types.h>:man says some OS could require it
<unistd.h>  : closing sockets only
*/

void close_sock(int);

/* server entry point */
int main(int __unused argc, char __unused **argv) {
	int sockfd;
    int bind_to_port(u_short);
    void run_server(int);
    
    sockfd = bind_to_port(MY_PORT);
    
    /* still not listening though, but we cannot print this after the fact */
    printf("server listening on port %d\n", MY_PORT);
    
    run_server(sockfd);
	close_sock(sockfd);
    return EXIT_SUCCESS;
}

/* 
 * gets all the addresses associated to the given port and binds to the first
 * available one
 */
int bind_to_port(const u_short port) {
    struct addrinfo *servinfo, *currinfo;
    int sockfd;
    void get_serverinfo(const char* const, u_short, struct addrinfo**);
    int do_bind(struct addrinfo*);

    get_serverinfo(NULL, port, &servinfo);
    for(currinfo = servinfo; currinfo != NULL; currinfo = currinfo->ai_next) {
        sockfd = do_bind(currinfo);
        if (sockfd != -1) break;
    }
    if (currinfo == NULL) {
        fprintf(stderr, "failed to bind\n");
        exit(EXIT_FAILURE);
    }
    freeaddrinfo(servinfo);
    return sockfd;
}

/*
 * performs the actual binding logic: creates a socket and uses it to bind to
 * the specified address.
 * returns the file descriptor of the newly-created socket, or -1 if an error
 * occurred.
 */
int do_bind(struct addrinfo *ainfo)
{
	int sockfd, yes = 1, setsockopt_res, bind_res;
    
    sockfd = socket(ainfo->ai_family, ainfo->ai_socktype, ainfo->ai_protocol);
    if (sockfd == -1) {
        perror("socket()");
        return -1;
    }
    
    setsockopt_res =
            setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
    if (setsockopt_res) {
        perror("setsockopt()");
        exit(EXIT_FAILURE);
    }
    
    bind_res = bind(sockfd, ainfo->ai_addr, ainfo->ai_addrlen);
    if (bind_res == -1) {
        close_sock(sockfd);
        perror("bind()");
        return -1;
    }
    return sockfd;
}


/* main server loop */
void run_server(const int sockfd) {
    int sockfd_acc;
    void do_listen(int);
    int accept_incoming(int);
    void handle_client(int);

    printf("TRACE: inside run_server\n");
    do_listen(sockfd);
    while(1) {
        sockfd_acc = accept_incoming(sockfd);
        if (fork() == 0) {
            handle_client(sockfd_acc);
            close_sock(sockfd_acc);
            exit(EXIT_SUCCESS);
        } else close_sock(sockfd_acc);
	}
}

/* listen() syscall, with error handling */
void do_listen(const int sockfd) {
    int listen_result;
    const int listen_backlog = 10;
    
    listen_result = listen(sockfd, listen_backlog);
	if (listen_result < 0) {
        perror("listen()");
        exit(EXIT_FAILURE);
    }
}

/*
 * blocks until an incoming request arrives on the given socket, and then
 * returns the file descriptor of the socket created to serve the request
 */
int accept_incoming(const int sockfd) {
    int sockfd_acc;
    struct sockaddr_storage client_addr;
	int addrlen;

    addrlen = sizeof(client_addr);
    sockfd_acc = accept(sockfd, (struct sockaddr *) &client_addr, &addrlen);
    if (sockfd_acc < 0) {
        perror("accept()");
        exit(EXIT_FAILURE); /* TODO: is it right? */
    }
    
    /* TODO: inet_ntoa is deprecated, use inet_ntop instead */
    printf("incoming connection from %s\n", inet_ntoa(client_addr.sin_addr));
    return sockfd_acc;
}


/* TODO: implement actual commands */
void handle_client(const int sockfd_acc) {
    char buff[MAX_MSGLEN + 1];  /* +1 for the null terminator */
    int len;
    
    while (1) {
        len = recv(sockfd_acc, buff, MAX_MSGLEN, 0);
        if (len == -1) {
            perror("recv()");
            exit(EXIT_FAILURE);
        }
        buff[len] = '\0';   /* null terminator must be added after reading */
        printf("client sent the following command: %s (%d bytes)\n", buff, len);
    }
}


/* code clone in client.c - BEGIN */

void get_serverinfo(
        const char* const ip, u_short port, struct addrinfo **servinfo) {
    char port_str[PORT_MAXCHARS];
    struct addrinfo hints;
    int getaddrinfo_res;
    struct addrinfo make_hints(void);
    
    printf("[TRACE] get_serverinfo\n");
    sprintf(port_str, "%d", port);
    hints = make_hints();
    if (ip == NULL) hints.ai_flags = AI_PASSIVE;
    getaddrinfo_res = getaddrinfo(ip, port_str, &hints, servinfo);
    if (getaddrinfo_res != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(getaddrinfo_res));
        exit(EXIT_FAILURE);
    }
}

struct addrinfo make_hints(void) {
    struct addrinfo hints;
    
    memset(&hints, 0, sizeof(hints));
    hints.ai_family     = AF_UNSPEC;
    hints.ai_socktype   = SOCK_STREAM;
    
    return hints;
}


void close_sock(const int sockfd) {
    int close_result;
    
    close_result = close(sockfd);
	if (close_result < 0) {
        fprintf(stderr, "could not close socket: exiting");
        exit(EXIT_FAILURE);
    }
}

/* code clone in client.c - END */
