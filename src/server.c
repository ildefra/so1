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

/* the server */
int main(int argc, char **argv) {
	int ds_sock, ds_sock_acc;
    struct sockaddr addr;
	int addrlen;
    int make_tcp_socket(u_short);
    
    ds_sock = make_tcp_socket(LISTEN_PORT);
    ds_sock_acc = accept(ds_sock, &addr, &addrlen);
    
	/* do something */
	
	close(ds_sock);
    close(ds_sock_acc);
    return EXIT_SUCCESS;
}

/* TODO: split more */
/* creates a TCP socket that listens to all addresses */
int make_tcp_socket(const u_short sin_port) {
    int ds_sock;
    struct sockaddr_in my_addr;
    
    ds_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (ds_sock < 0) {
        fprintf(stderr, "could not create socket: exiting");
        exit(EXIT_FAILURE);
    }
    
	my_addr.sin_family      = AF_INET;
	my_addr.sin_port        = htons(sin_port);
	my_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	
	bind(ds_sock, (struct sockaddr *) &my_addr, sizeof(my_addr));
    
    /* TODO: handle bind error as above */

    return ds_sock;
}
