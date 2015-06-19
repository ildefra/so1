/* 
 * bel_server.c - Server part of the OS1 assignment
 *
 * General considerations:
 * - communication protocol is based on fixed-length messages
 * - every communication failure with a specific client will close that
 * connection and abort the process assigned to it
 */

#include "bel_common.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/*
 * (file descriptor of) the main server socket, which handles new client
 * connections
 */
static int sockfd;

/*
 * (file descriptor of) the socket used to communicate with the client. Each
 * process has its own
 */
static int sockfd_acc;


/*
 * explicitly closes the resources acquired by the current process. Called on
 * process exit
 */
void
cleanup(void)
{
    printf("[DEBUG] resource cleanup\n");
    if (sockfd != 0) bel_close_or_die(sockfd);
    if (sockfd_acc != 0) bel_close_or_die(sockfd_acc);
}

/* server entry point */
int
main(int __unused argc, char __unused **argv)
{
    void bind_to_port(u_short);
    void do_listen_or_die(void);
    void server_loop(void);
    
    printf("[INFO] program started with pid = '%ld'\n", (long) getpid());
    atexit(cleanup);
    bind_to_port(COMM_PORT);
    
    /* still not listening though, but we cannot print this after the fact */
    printf("server listening on port %d\n", COMM_PORT);
    
    do_listen_or_die();
    server_loop();
    return EXIT_SUCCESS;
}


/* 
 * gets all the addresses associated to the given port and binds to the first
 * available one
 */
void
bind_to_port(const u_short port)
{
    int do_bind_res;
    struct addrinfo *servinfo, *currinfo;
    
    int do_bind(struct addrinfo*);

    bel_getaddrinfo_or_die(NULL, port, &servinfo);
    for(currinfo = servinfo; currinfo != NULL; currinfo = currinfo->ai_next) {
        do_bind_res = do_bind(currinfo);
        if (do_bind_res != -1) break;
    }
    if (currinfo == NULL) {
        fprintf(stderr, "[FATAL] failed to bind: exiting\n");
        exit(EXIT_FAILURE);
    }
    freeaddrinfo(servinfo);
}

/*
 * performs the actual binding logic: creates a socket and uses it to bind to
 * the specified address.
 * Saves the file descriptor of the newly-created socket into sockfd.
 * Returns the file descriptor, or -1 on error.
 */
int
do_bind(struct addrinfo *ainfo)
{
	int bind_res;
    const char* const bind_msg  = "[INFO] binding to ";
    void set_reuseaddr_or_die(void);
    
    sockfd = bel_new_sock(*ainfo);
    if (sockfd == -1) return -1;
    
    set_reuseaddr_or_die();
    bel_print_address(bind_msg, ainfo->ai_addr);
    bind_res = bind(sockfd, ainfo->ai_addr, ainfo->ai_addrlen);
    if (bind_res == -1) {
        perror("[WARN] bind()");
        bel_close_or_die(sockfd);
        return -1;
    }
    return sockfd;
}

/*
 * Used before bind to force binding (use "man setsockopt" for details). If this
 * call fails something bad happened, so we exit the program
 */
void
set_reuseaddr_or_die(void)
{
    int yes = 1, setsockopt_res;
    
    setsockopt_res =
            setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
    if (setsockopt_res == -1) {
        perror("[FATAL] setsockopt()");
        exit(EXIT_FAILURE);
    }
}


/* performs the listen() syscall, and exits the program if it fails */
void
do_listen_or_die(void)
{
    int listen_result;
    const int listen_backlog = 10;
    
    listen_result = listen(sockfd, listen_backlog);
	if (listen_result == -1) {
        perror("[FATAL] listen()");
        exit(EXIT_FAILURE);
    }
}


/* the main server loop
 * spawns a child process which handles the client connection on the given
 * socket (file descriptor)

*/
void
server_loop(void)
{
    int accept_incoming(void);
    void handle_client(void);

    for(;;) {
        if (accept_incoming() == -1) continue;
        switch (fork()) {
            case -1:    /* error, did not fork  */
                perror("[FATAL] fork()");
                exit(EXIT_FAILURE);
            case 0:     /* child process  */
                handle_client();
                exit(EXIT_SUCCESS);
            default:    /* parent process  */
                bel_close_or_die(sockfd_acc);
        }
    }
}

/*
 * blocks until an incoming client request arrives, and then creates a socket to
 * serve this specific client.
 * Saves the file descriptor of the newly-created socket into sockfd_acc.
 * Returns the file descriptor, or -1 on error.
 */
int
accept_incoming(void)
{
    int addrlen;
    struct sockaddr_storage client_addr;

    const char* const conn_msg = "[INFO] incoming connection from ";
    const char* const debug_msg =
            "[DEBUG] created socket with fd = '%d' to handle the connection";
    
    addrlen = sizeof(client_addr);
    sockfd_acc = accept(sockfd, (struct sockaddr *) &client_addr, &addrlen);
    if (sockfd_acc == -1) {
        perror("[ERROR] accept()");
        return -1;
    }
    bel_print_address(conn_msg, (struct sockaddr *) &client_addr);
    printf(debug_msg, sockfd_acc);
    return sockfd_acc;
}

void
handle_client(void)
{
    char buf[STD_MSGLEN + 1];  /* +1 for the null terminator */
    
    void authenticate_or_die(void);

    authenticate_or_die();
    for(;;) {
        bel_recvall_or_die(sockfd_acc, buf, STD_MSGLEN + 1);
        
        /* TODO: implement actual commands */
        
        bel_sendall_or_die(sockfd_acc, "OK", ANSWER_MSGLEN);
    }
}

/*
 * asks for username and password and authenticates against the server.
 * Exits the process if an error occurs or wrong credentials are provided
 */
void
authenticate_or_die(void)
{
    /* +2 for \n and \0 */
    char uname[UNAME_MSGLEN + 2];
    char pword[PWORD_MSGLEN + 2];
    
    bel_recvall_or_die(sockfd_acc, uname, UNAME_MSGLEN + 2);
    bel_recvall_or_die(sockfd_acc, pword, PWORD_MSGLEN + 2);

    /* TODO: actual authentication here */
    
    bel_sendall_or_die(sockfd_acc, "OK", ANSWER_MSGLEN);
}
