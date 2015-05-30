/*
 * server.c -- server part of the OS1 assignment
 */

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define MY_PORT 3490
#define LISTEN_BACKLOG 10
#define MAX_MSGLEN 1024

/*
includes usage:
<arpa/inet.h>:inet_ntoa only
<stdio.h>   : error logging only
<stdlib.h>  : exit statuses only
<string.h>  : memset only
<sys/types.h>:man says some OS could require it
<unistd.h>  : closing sockets only
*/

void close_sock(int);

/* server entry point */
int main(int __unused argc, char __unused **argv) {
	int ds_sock;
    int make_tcp_socket();
    void bind_to_port(int, u_short);
    void run_server(int);
    
    ds_sock = make_tcp_socket();
    bind_to_port(ds_sock, MY_PORT);
    run_server(ds_sock);
	close_sock(ds_sock);
    return EXIT_SUCCESS;
}

/* binds the given socket to the given port */
void bind_to_port(const int ds_sock, const u_short sin_port) {
    int bind_result;
    struct sockaddr_in srv_addr;
    struct sockaddr_in make_serversocket_address(u_short);
    
	srv_addr = make_serversocket_address(sin_port);
	
	bind_result =
            bind(ds_sock, (struct sockaddr *) &srv_addr, sizeof(srv_addr));
	if (bind_result < 0) {
        perror("bind()");
        exit(EXIT_FAILURE);
    }
    
    /* still not listening though, but we cannot print this after the fact */
    printf("server listening on port %d\n", sin_port);
}

/* creates an internet address structure for listening sockets to bind to */
struct sockaddr_in make_serversocket_address(const u_short sin_port) {
    struct sockaddr_in addr;
    
    addr.sin_family         = AF_INET;
	addr.sin_port           = htons(sin_port);
	addr.sin_addr.s_addr    = htonl(INADDR_ANY);
    
    return addr;
}

/* main server loop */
void run_server(const int ds_sock) {
    int ds_sock_acc;
    void do_listen(int);
    int accept_incoming(int);
    void handle_client(int);

    do_listen(ds_sock);
    while(1) {
        ds_sock_acc = accept_incoming(ds_sock);
        if (fork() == 0) {
            handle_client(ds_sock_acc);
            close_sock(ds_sock_acc);
            exit(EXIT_SUCCESS);
        } else close_sock(ds_sock_acc);
	}
}

/* listen() syscall, with error handling */
void do_listen(const int ds_sock) {
    int listen_result;
    
    listen_result = listen(ds_sock, LISTEN_BACKLOG);
	if (listen_result < 0) {
        perror("listen()");
        exit(EXIT_FAILURE);
    }
}

/*
 * blocks until an incoming request arrives on the given socket, and then
 * returns the file descriptor of the socket created to serve the request
 */
int accept_incoming(const int ds_sock) {
    int ds_sock_acc;
    struct sockaddr_in client_addr;
	int addrlen;

    addrlen = sizeof(client_addr);
    ds_sock_acc = accept(ds_sock, (struct sockaddr *) &client_addr, &addrlen);
    if (ds_sock_acc < 0) {
        perror("accept()");
        exit(EXIT_FAILURE); /* TODO: is it right? */
    }
    
    /* TODO: inet_ntoa is deprecated, use inet_ntop instead */
    printf("incoming connection from %s\n", inet_ntoa(client_addr.sin_addr));
    return ds_sock_acc;
}

/* TODO: implement actual commands */
void handle_client(const int ds_sock_acc) {
    char buff[MAX_MSGLEN + 1];  /* +1 for the null terminator */
    int len;
    
    while (1) {
        len = recv(ds_sock_acc, buff, MAX_MSGLEN, 0);
        buff[len] = '\0';   /* null terminator must be added after reading */
        printf("client sent the following command: %s\n", buff);
    }
}


/* TODO: code clone in client.c */
/* creates a TCP socket */
int make_tcp_socket() {
    int ds_sock;
    
    ds_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (ds_sock < 0) {
        perror("socket()");
        exit(EXIT_FAILURE);
    }
    return ds_sock;
}

/* TODO: code clone in client.c */
void close_sock(const int ds_sock) {
    int close_result;
    
    close_result = close(ds_sock);
	if (close_result < 0) {
        perror("close()");
        exit(EXIT_FAILURE);
    }
}
