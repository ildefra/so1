#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define MY_PORT 2015
#define LISTEN_BACKLOG 10

/*
includes usage:
<stdio.h>   : error logging only
<stdlib.h>  : exit statuses only
<sys/types.h>:man says some OS could require it
<unistd.h>  : closing sockets only
*/

void close_sock(int);

/* the server */
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
    struct sockaddr_in my_addr;
    struct sockaddr_in make_socket_address(u_short);
    
	my_addr = make_socket_address(sin_port);
	
	bind_result = bind(ds_sock, (struct sockaddr *) &my_addr, sizeof(my_addr));
	if (bind_result < 0) {
        perror("bind()");
        exit(EXIT_FAILURE);
    }
    printf("server listening on port %d\n", sin_port);
}

/* creates an internet address structure for listening sockets to bind to */
struct sockaddr_in make_socket_address(const u_short sin_port) {
    struct sockaddr_in my_addr;
    
    my_addr.sin_family      = AF_INET;
	my_addr.sin_port        = htons(sin_port);
	my_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    
    return my_addr;
}


void run_server(const int ds_sock) {
    int ds_sock_acc;
    struct sockaddr addr;
	int addrlen;
    
    listen(ds_sock, LISTEN_BACKLOG);
    ds_sock_acc = accept(ds_sock, &addr, &addrlen);
	if (ds_sock_acc < 0) {
        perror("accept()");
        exit(EXIT_FAILURE);
    }
    
	/* do something */
	
    close_sock(ds_sock_acc);
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
