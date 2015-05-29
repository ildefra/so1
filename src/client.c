#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define MY_PORT 2015

/*
includes usage:
<stdio.h>   : error logging only
<stdlib.h>  : exit statuses only
<sys/types.h>:man says some OS could require it
<unistd.h>  : closing sockets only
*/

/* TODO: split more */
/* the client */
int main(int __unused argc, char __unused **argv) {
	int ds_sock, connect_result;
    struct sockaddr_in my_addr;
    int make_tcp_socket();
    void close_sock(int);
    
    ds_sock = make_tcp_socket();
    
	my_addr.sin_family      = AF_INET;
	my_addr.sin_port        = htons(MY_PORT);
    my_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    
    connect_result = connect(ds_sock, (struct sockaddr *) &my_addr, sizeof(my_addr));
    if (connect_result < 0) {
        perror("connect()");
        exit(EXIT_FAILURE);
    }
    
    /* TODO: connect to server and do stuff */
    
	close_sock(ds_sock);
    return EXIT_SUCCESS;
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
