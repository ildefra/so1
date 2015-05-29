#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define MY_PORT 2015

/*
includes usage:
<arpa/inet.h>:inet_ntoa only
<stdio.h>   : error logging only
<stdlib.h>  : exit statuses only
<sys/types.h>:man says some OS could require it
<unistd.h>  : closing sockets only
*/

/* TODO: split more */
/* client entry point */
int main(int __unused argc, char __unused **argv) {
	int ds_sock;
    int make_tcp_socket();
    struct sockaddr_in make_clientsocket_address(u_short);
    void connect_to(int, struct sockaddr_in);
    void close_sock(int);
    
    ds_sock = make_tcp_socket();
    connect_to(ds_sock, make_clientsocket_address(MY_PORT));
    
    /* TODO: do stuff */
    
	close_sock(ds_sock);
    return EXIT_SUCCESS;
}

void connect_to(const int ds_sock, const struct sockaddr_in addr) {
    int connect_result;
    
    printf("connecting to %s\n", inet_ntoa(addr.sin_addr));
    connect_result = connect(ds_sock, (struct sockaddr *) &addr, sizeof(addr));
    if (connect_result < 0) {
        perror("connect()");
        exit(EXIT_FAILURE);
    }
}

/* creates an internet address structure for connecting to localhost on given port */
struct sockaddr_in make_clientsocket_address(const u_short sin_port) {
    struct sockaddr_in my_addr;
    
    my_addr.sin_family      = AF_INET;
	my_addr.sin_port        = htons(sin_port);
	my_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    
    return my_addr;
}


/* TODO: code clone in server.c */
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

/* TODO: code clone in server.c */
void close_sock(const int ds_sock) {
    int close_result;
    
    close_result = close(ds_sock);
	if (close_result < 0) {
        fprintf(stderr, "could not close socket: exiting");
        exit(EXIT_FAILURE);
    }
}
