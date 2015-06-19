/* 
 * bel_client.c - Client part of the OS1 assignment
 *
 * General considerations:
 * - communication protocol is based on fixed-length messages
 * - every communication failure or server "KO" answer will shutdown the program
 */

#include "bel_common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define ARGC_OK 2

/* (file descriptor of) the socket used to communicate with server  */
static int sockfd;

/*
 * Explicitly closes the resources acquired by the current process. Called on
 * process exit
 */
void
cleanup(void)
{
    printf("[DEBUG] resource cleanup\n");
    if (sockfd != 0) bel_close_or_die(sockfd);
}

/* Client entry point  */
int
main(int argc, char **argv)
{
    void connect_to(const char* const, const u_short);
    void authenticate(void);
    void run_client(void);
    
    if (argc != ARGC_OK) {
        printf("usage: client <remote address>\n");
        exit(EXIT_FAILURE);
    }
    printf("[INFO] program started with pid = '%ld'\n", (long) getpid());
    atexit(cleanup);
    connect_to(argv[1], COMM_PORT);
    printf("connected to server\n");
    authenticate();
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
    struct addrinfo *servinfo = NULL, *currinfo = NULL;
    
    int do_connect(struct addrinfo*);
    
    bel_getaddrinfo_or_die(ip, port, &servinfo);
    for(currinfo = servinfo; currinfo != NULL; currinfo = currinfo->ai_next) {
        do_connect_res = do_connect(currinfo);
        if (do_connect_res != -1) break;
    }
    if (currinfo == NULL) {
        fprintf(stderr, "[FATAL] Failed to connect: exiting\n");
        exit(EXIT_FAILURE);
    }
    freeaddrinfo(servinfo);
}

/*
 * Performs the actual connection logic: creates a socket and uses it to connect
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


void
authenticate(void) {
    void send_credentials(void);
    int ok_from_server(void);
    
    send_credentials();
    if (!ok_from_server()) {
        printf("wrong username and/or password: exiting\n");
        exit(EXIT_SUCCESS);
    }
}

/*
 * Asks the user for his username and password and sends them to the server for
 * authentication.
 */
void
send_credentials(void)
{
    /* +2 for \n and \0  */
    char uname[UNAME_MSGLEN + 2];
    char pword[PWORD_MSGLEN + 2];
    
    printf("insert your username (max %d characters): ", UNAME_MSGLEN);
    fgets(uname, sizeof(uname), stdin);
    bel_sendall_or_die(sockfd, uname, UNAME_MSGLEN + 2);
    
    printf("insert your password (max %d characters): ", PWORD_MSGLEN);
    fgets(pword, sizeof(pword), stdin);
    bel_sendall_or_die(sockfd, pword, PWORD_MSGLEN + 2);
}

/*
 * Waits for an answer from the server. Returns 1 for a positive answer ("OK")
 * and 0 for a negative one (should be "KO", but does not check for it)
 */
int
ok_from_server(void)
{
    char answer[ANSWER_MSGLEN];
    
    bel_recvall_or_die(sockfd, answer, ANSWER_MSGLEN);
    return strcmp(answer, "OK") == 0;
}


/* Actual business logic of the client  */
void
run_client(void)
{
    char buf[STD_MSGLEN + 1];
    
    
    for (;;) {
        printf("\nPlease enter a command: ");
        scanf("%s", buf);
        
        /* TODO: implement actual commands */
        
        bel_sendall_or_die(sockfd, buf, STD_MSGLEN + 1);
        ok_from_server();
    }
}


/* Removes the last character of the given string if it is a newline  */
/*
void
chop_newline(char *str)
{
    size_t len;
    
    len = strlen(str);
    if (len > 0 && str[len-1] == '\n') str[len-1] = '\0';
}
*/
