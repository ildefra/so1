/* bel_client - Client part of the OS1 assignment  */

#include "bel_common.h"
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>

/* Client entry point  */
int
main(int __unused argc, char __unused **argv)
{
	int sockfd;
    int connect_to(const char* const, u_short);
    void run_client(int);
    
    const char* const server_ip = "127.0.0.1";
    
    sockfd = connect_to(server_ip, COMM_PORT);
    printf("Connected\n");
    run_client(sockfd);
	bel_close_sock(sockfd);
    return EXIT_SUCCESS;
}

/* 
 * Gets all the addresses associated to the given ip and port and connects to
 * the first available one
 */
int
connect_to(const char* const ip, const u_short port)
{
    struct addrinfo *servinfo, *currinfo;
    int sockfd;
    int do_connect(struct addrinfo*);
    
    bel_get_serverinfo(ip, port, &servinfo);
    for(currinfo = servinfo; currinfo != NULL; currinfo = currinfo->ai_next) {
        sockfd = do_connect(currinfo);
        if (sockfd != -1) break;
    }
    if (currinfo == NULL) {
        fprintf(stderr, "[FATAL] Failed to connect\n");
        exit(EXIT_FAILURE);
    }
    freeaddrinfo(servinfo);
    return sockfd;
}

/*
 * Performs the actual connection logic: creates a socket and uses it to connect
 * to the specified address.
 * returns the file descriptor of the newly-created socket, or -1 if an error
 * occurred.
 */
int
do_connect(struct addrinfo *ainfo)
{
	int sockfd, connect_res;
	void print_connecting(const int, const struct sockaddr*);
    
    sockfd = socket(ainfo->ai_family, ainfo->ai_socktype, ainfo->ai_protocol);
    if (sockfd == -1) {
        perror("[WARN] socket()");
        return -1;
    }
    print_connecting(ainfo->ai_family, ainfo->ai_addr);
    connect_res = connect(sockfd, ainfo->ai_addr, ainfo->ai_addrlen);
    if (connect_res == -1) {
        bel_close_sock(sockfd);
        perror("[WARN] connect()");
        return -1;
    }
    return sockfd;
}


/* TODO: split! */
/*
 * Just (!!) prints the "connecting to..." message, switching on the given
 * address family to get the right fields from ai_addr
 */
void
print_connecting(const int ai_family, const struct sockaddr *ai_addr)
{
    void *addr;
    char ipstr[INET6_ADDRSTRLEN];
    
    const char* const err_msg   = "[FATAL] Unrecognized address family %d";
    const char* const conn_msg  = "Connecting to %s address %s ...\n";
    
    switch (ai_family) {
        case AF_INET:
            addr = &(((struct sockaddr_in*) ai_addr)->sin_addr);
            break;
        case AF_INET6:
            addr = &(((struct sockaddr_in6*) ai_addr)->sin6_addr);
            break;
        default:
            fprintf(stderr, err_msg, ai_family);
            exit(EXIT_FAILURE);
    }

    inet_ntop(ai_family, addr, ipstr, sizeof(ipstr));    
    printf(conn_msg, bel_afamily_tostring(ai_family), ipstr);
}


/* TODO: split */
/* TODO: implement actual commands */
/* Actual business logic of the client  */
void
run_client(const int sockfd)
{
    char buff[MAX_MSGLEN + 1];  /* +1 for the null terminator  */
    int len;
    
    printf("[TRACE] run_client: sockfd = '%d'\n", sockfd);
    while (1) {
        printf("\nPlease enter a command: ");
        scanf("%s", buff);
        send(sockfd, buff, MAX_MSGLEN, 0);
        
        len = recv(sockfd, buff, MAX_MSGLEN, 0);
        if (len == -1) {
            perror("recv()");
            exit(EXIT_FAILURE);
        }
        buff[len] = '\0';   /* null terminator must be added after reading  */
        printf("Server answered: %s (%d bytes)\n", buff, len);
    }
}
