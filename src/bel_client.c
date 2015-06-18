/* bel_client - Client part of the OS1 assignment  */

#include "bel_common.h"
#include <stdio.h>
#include <stdlib.h>

#define ARGC_OK 2

/* (file descriptor of) the socket used to communicate with server  */
static int sockfd;

/* closes the socket. Called on program exit  */
void
cleanup(void)
{
    printf("[DEBUG] resource cleanup\n");
    bel_close_or_die(sockfd);
}

/* client entry point  */
int
main(int argc, char **argv)
{
    void connect_to(const char* const, const u_short);
    void run_client();
    
    if (argc != ARGC_OK) {
        printf("usage: client <remote address>\n");
        exit(EXIT_FAILURE);
    }
    connect_to(argv[1], COMM_PORT);
    printf("connected to server\n");
    run_client();
    return EXIT_SUCCESS;
}

/*
 * Gets all the addresses associated to the given ip and port and connects to
 * the first available one
 */
void
connect_to(const char* const ip, const u_short port)
{
    int do_connect_res;
    struct addrinfo *servinfo, *currinfo;
    int do_connect(struct addrinfo*);
    
    bel_getaddrinfo_or_die(ip, port, &servinfo);
    for(currinfo = servinfo; currinfo != NULL; currinfo = currinfo->ai_next) {
        do_connect_res = do_connect(currinfo);
        if (do_connect_res != -1) break;
    }
    atexit(cleanup);
    if (currinfo == NULL) {
        fprintf(stderr, "[FATAL] Failed to connect: exiting\n");
        exit(EXIT_FAILURE);
    }
    freeaddrinfo(servinfo);
}

/*
 * performs the actual connection logic: creates a socket and uses it to connect
 * to the specified address.
 * Saves the file descriptor of the newly-created socket into sockfd. Returns
 * the file descriptor, or -1 on error.
 */
int
do_connect(struct addrinfo *ainfo)
{
	int connect_res;
	const char* const conn_msg  = "[INFO] connecting to ";
    
    sockfd = bel_new_sock(*ainfo);
    if (sockfd == -1) return -1;
    
    bel_print_address(conn_msg, ainfo->ai_addr);
    connect_res = connect(sockfd, ainfo->ai_addr, ainfo->ai_addrlen);
    if (connect_res == -1) {
        perror("[WARN] connect()");
        bel_close_or_die(sockfd);
        return -1;
    }
    return sockfd;
}


/* TODO: split */
/* TODO: implement actual commands */
/* Actual business logic of the client  */
void
run_client()
{
    char buff[MSG_MAXLEN + 1];
    ssize_t len;
    
    void sendall_or_die(const int, const char* const, const size_t);
    
    /* authenticate(); */
    for (;;) {
        printf("\nPlease enter a command: ");
        scanf("%s", buff);
        sendall_or_die(sockfd, buff, MSG_MAXLEN + 1);
        
        len = recv(sockfd, buff, MSG_MAXLEN, 0);
        if (len == -1) {
            perror("[FATAL] recv()");
            exit(EXIT_FAILURE);
        }
        buff[len] = '\0';   /* null terminator must be added after reading  */
        printf("Server answered: '%s' (%zd bytes)\n", buff, len);
    }
}

/*
 * sends <len> bytes of data from <buf> to the socket <sockfd>. Exits the
 * process on failure or disconnection
 */
void
sendall_or_die(const int sockfd, const char* const buf, const size_t len)
{
    size_t bytes_left;
    ssize_t bytes_sent;

    printf("[DEBUG] sending data to socket '%d'\n", sockfd);
    for(bytes_left = len; bytes_left > 0; bytes_left -= bytes_sent) {
        bytes_sent = send(sockfd, buf + len - bytes_left, bytes_left, 0);
        if (bytes_sent == -1 || bytes_sent == 0) {
            perror("[ERROR] send()");
            exit(EXIT_FAILURE);
        }
    }
}


/* removes the last character of the given string if it is a newline */
/*
void
chop_newline(char *str)
{
    int len;
    
    len = strlen(str);
    if (len > 0 && str[len-1] == '\n') str[len-1] = '\0';
}
*/
