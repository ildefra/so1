/* bel_server - server part of the OS1 assignment  */

#include "bel_common.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/* server entry point */
int main(int __unused argc, char __unused **argv) {
	int sockfd;
    int bind_to_port(u_short);
    void run_server(int);
    
    sockfd = bind_to_port(COMM_PORT);
    
    /* still not listening though, but we cannot print this after the fact */
    printf("[INFO] server listening on port %d\n", COMM_PORT);
    
    run_server(sockfd);
	bel_close_sock(sockfd);
    return EXIT_SUCCESS;
}

/* 
 * gets all the addresses associated to the given port and binds to the first
 * available one
 */
int bind_to_port(const u_short port) {
    struct addrinfo *servinfo, *currinfo;
    int sockfd;
    int do_bind(struct addrinfo*);

    bel_get_serverinfo(NULL, port, &servinfo);
    for(currinfo = servinfo; currinfo != NULL; currinfo = currinfo->ai_next) {
        sockfd = do_bind(currinfo);
        if (sockfd != -1) break;
    }
    if (currinfo == NULL) {
        fprintf(stderr, "[FATAL] failed to bind: exiting\n");
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
	int sockfd, bind_res;
    const char* const bind_msg  = "[INFO] binding to ";
    void set_reuseaddr(const int);
    
    sockfd = bel_open_sock(*ainfo);
    if (sockfd == -1) return -1;
    set_reuseaddr(sockfd);
    
    bel_print_address(bind_msg, ainfo->ai_addr);
    bind_res = bind(sockfd, ainfo->ai_addr, ainfo->ai_addrlen);
    if (bind_res == -1) {
        bel_close_sock(sockfd);
        perror("bind()");
        return -1;
    }
    return sockfd;
}

/*
 * Used before bind to force binding (use "man setsockopt" for details). If this
 * call fails something bad happened, so we exit the program
 */
void
set_reuseaddr(const int sockfd) {
    int yes = 1, setsockopt_res;
    
    setsockopt_res =
            setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
    if (setsockopt_res == -1) {
        perror("[FATAL] setsockopt()");
        exit(EXIT_FAILURE);
    }
}


/* main server loop */
void run_server(const int sockfd) {
    int sockfd_acc;
    void do_listen(int);
    int accept_incoming(int);
    void handle_client(int);

    printf("[TRACE] run_server: sockfd = '%d'\n", sockfd);
    do_listen(sockfd);
    while(1) {
        sockfd_acc = accept_incoming(sockfd);
        if (fork() == 0) {
            handle_client(sockfd_acc);
            bel_close_sock(sockfd_acc);
            exit(EXIT_SUCCESS);
        }
        bel_close_sock(sockfd_acc);
	}
}

/* performs the listen() syscall, and exits the program if it fails */
void do_listen(const int sockfd) {
    int listen_result;
    const int listen_backlog = 10;
    
    listen_result = listen(sockfd, listen_backlog);
	if (listen_result < 0) {
        perror("[FATAL] listen()");
        exit(EXIT_FAILURE);
    }
}

/*
 * blocks until an incoming request arrives on the given socket, and then
 * returns the file descriptor of the socket created to serve the request
 */
int accept_incoming(const int sockfd) {
    int addrlen, sockfd_acc;
    struct sockaddr_storage client_addr;

    const char* const conn_msg = "[INFO] incoming connection from ";
    
    addrlen = sizeof(client_addr);
    sockfd_acc = accept(sockfd, (struct sockaddr *) &client_addr, &addrlen);
    if (sockfd_acc < 0) {
        perror("accept()");
        exit(EXIT_FAILURE); /* TODO: is it right? */
    }
    bel_print_address(conn_msg, (struct sockaddr *) &client_addr);
    
    return sockfd_acc;
}


/* TODO: implement actual commands */
void handle_client(const int sockfd_acc) {
    char buff[MAX_MSGLEN + 1];  /* +1 for the null terminator */
    int len;
    
    while (1) {
        len = recv(sockfd_acc, buff, MAX_MSGLEN, 0);
        if (len == -1) {
            perror("[FATAL] recv()");
            exit(EXIT_FAILURE);
        }
        buff[len] = '\0';   /* null terminator must be added after reading */
        printf("client sent the following command: %s (%d bytes)\n", buff, len);
    }
}
