#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>

#define LISTEN_PORT 2015

/*
include usage:
<stdio.h>   : error logging
<stdlib.h>  : exit status
<unistd.h>  : closing sockets
*/

/* TODO: split more */
/* the server */
int main(int argc, char **argv) {
	int ds_sock, ds_sock_acc;
    struct sockaddr addr;
	int addrlen;
    int make_tcp_socket(u_short);
    void close_sock(int);
    
    ds_sock = make_tcp_socket(LISTEN_PORT);
    ds_sock_acc = accept(ds_sock, &addr, &addrlen);
	if (ds_sock_acc < 0) {
        fprintf(stderr, "could not accept incoming connections: exiting");
        exit(EXIT_FAILURE);
    }
    
	/* do something */
	
    close_sock(ds_sock_acc);
	close_sock(ds_sock);
    return EXIT_SUCCESS;
}

/* TODO: sin_port is passed around too much */

/* creates a TCP socket that listens to all addresses */
int make_tcp_socket(const u_short sin_port) {
    int ds_sock;
    void safe_bind(int, u_short);
    
    ds_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (ds_sock < 0) {
        fprintf(stderr, "could not create socket: exiting");
        exit(EXIT_FAILURE);
    }
    
    safe_bind(ds_sock, sin_port);
    return ds_sock;
}

/* binds the given socket to the given port */
void safe_bind(const int ds_sock, const u_short sin_port) {
    int bind_result;
    struct sockaddr_in my_addr;
    struct sockaddr_in make_sockaddr(u_short);
    
	my_addr = make_sockaddr(sin_port);
	
	bind_result = bind(ds_sock, (struct sockaddr *) &my_addr, sizeof(my_addr));
	if (bind_result < 0) {
        fprintf(stderr, "could not bind socket to port %d: exiting", sin_port);
        exit(EXIT_FAILURE);
    }
}

/* creates an internet address structure for listening sockets to bind to */
struct sockaddr_in make_sockaddr(const u_short sin_port) {
    struct sockaddr_in my_addr;
    
    my_addr.sin_family      = AF_INET;
	my_addr.sin_port        = htons(sin_port);
	my_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    
    return my_addr;
}


void close_sock(const int ds_sock) {
    int close_result;
    
    close_result = close(ds_sock);
	if (close_result < 0) {
        fprintf(stderr, "could not close socket: exiting");
        exit(EXIT_FAILURE);
    }
}
