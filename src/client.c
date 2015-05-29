#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define MY_PORT 2015
#define MAX_MSGLEN 1024

/*
includes usage:
<arpa/inet.h>:inet_ntoa only
<stdio.h>   : error logging only
<stdlib.h>  : exit statuses only
<sys/types.h>:man says some OS could require it
<unistd.h>  : closing sockets only
*/

/* client entry point */
int main(int __unused argc, char __unused **argv) {
	int ds_sock;
    int make_tcp_socket();
    struct sockaddr_in make_clientsocket_address(u_short);
    void connect_to(int, struct sockaddr_in);
    void close_sock(int);
    void run_client(int);
    
    ds_sock = make_tcp_socket();
    connect_to(ds_sock, make_clientsocket_address(MY_PORT));
    run_client(ds_sock);
	close_sock(ds_sock);
    return EXIT_SUCCESS;
}

void connect_to(const int ds_sock, const struct sockaddr_in addr) {
    int connect_result;
    
    printf("connecting to %s ...\n", inet_ntoa(addr.sin_addr));
    connect_result = connect(ds_sock, (struct sockaddr *) &addr, sizeof(addr));
    if (connect_result < 0) {
        perror("connect()");
        exit(EXIT_FAILURE);
    }
    printf("connected\n");
}

/* creates an internet address structure for connecting to localhost on given port */
struct sockaddr_in make_clientsocket_address(const u_short sin_port) {
    struct sockaddr_in my_addr;
    
    my_addr.sin_family      = AF_INET;
	my_addr.sin_port        = htons(sin_port);
	my_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    
    return my_addr;
}

/* TODO: implement actual commands */
void run_client(const int ds_sock) {
    char buff[MAX_MSGLEN + 1];  /* +1 for the null terminator */
    int len;
    
    while (1) {
        printf("\nplease enter a command: ");
        scanf("%s", buff);
        send(ds_sock, buff, MAX_MSGLEN, 0);
        
        len = recv(ds_sock, buff, MAX_MSGLEN, 0);
        buff[len] = '\0';   /* null terminator must be added after reading */
        printf("server answered: %s (%d bytes)\n", buff, len);
    }
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
