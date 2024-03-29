/* 
 * bel_server.c - Server part of the OS1 assignment
 *
 * General considerations:
 * - communication protocol is based on fixed-length messages
 * - every communication failure with a specific client will close that
 * connection and abort the process assigned to it
 */

#include "msg_storage.h"
#include "bel_common.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


/* Number of (hardcoded) registered users in the system  */
#define NO_OF_USERS 3

#define NO_OF_COMMANDS 3

#define DB_FILENAME "db.txt"

/* How many messages can be read at once  */
#define MSG_LIST_SIZE 10


typedef struct {
    char uname[UNAME_MSGLEN];
    char pword[PWORD_MSGLEN];
} Credentials;
static const Credentials empty_credentials;

typedef struct {
    char name[CMD_MSGLEN];
    Action action;
} Command;


static void bind_to_port(u_short);
static int do_bind(struct addrinfo*);
static void set_reuseaddr_or_die(void);
static void do_listen_or_die(void);

static void server_loop(void);
static int accept_incoming(void);
static void handle_client(void);
static void authenticate_or_die(void);
static int is_valid_login(const Credentials);

static Action receive_client_command(void);
static void handle_read(void);
static void handle_send(void);
static void handle_delete(void);

static void send_ok(void);
static void send_ko(void);


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


/* Name of the user being served right now  */
static char current_user[UNAME_MSGLEN];


/*
 * Explicitly closes the resources acquired by the current process. Called on
 * process exit
 */
static void
cleanup(void)
{
    printf("[DEBUG] resource cleanup\n");
    if (sockfd != 0) bel_close_or_die(sockfd);
    if (sockfd_acc != 0) bel_close_or_die(sockfd_acc);
}


/* Server entry point  */
int
main(void)
{    
    printf("[DEBUG] program started with pid = '%ld'\n", (long) getpid());
    atexit(cleanup);
    bind_to_port(COMM_PORT);
    
    /* still not listening though, but we cannot print this after the fact */
    printf("server listening on port %d\n", COMM_PORT);
    
    do_listen_or_die();
    msg_init_db_or_die(DB_FILENAME);
    server_loop();
    return EXIT_SUCCESS;
}


/* 
 * Gets all the addresses associated to the given port and binds to the first
 * available one
 */
static void
bind_to_port(const u_short port)
{
    int do_bind_res = 0;
    struct addrinfo *servinfo = NULL, *currinfo = NULL;

    bel_getaddrinfo_or_die(NULL, AF_INET, port, &servinfo);
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
 * Performs the actual binding logic: creates a socket and uses it to bind to
 * the specified address.
 * Saves the file descriptor of the newly-created socket into sockfd.
 * Returns the file descriptor, or -1 on error.
 */
static int
do_bind(struct addrinfo *ainfo)
{
	int bind_res = 0;
    const char* const bind_msg  = "[INFO] binding to ";
    
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
 * Used before bind() to force binding (use "man setsockopt" for details).
 * If this call fails something bad happened, so we exit the program
 */
static void
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


/* Performs the listen() syscall, and exits the program if it fails  */
static void
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


/* The main server loop, spawning child processes to handle clients  */
static void
server_loop(void)
{
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
            break;
        }
    }
}

/*
 * Blocks until an incoming client request arrives, and then creates a socket
 * to serve this specific client.
 * Saves the file descriptor of the newly-created socket into sockfd_acc.
 * Returns the file descriptor, or -1 on error.
 */
static int
accept_incoming(void)
{
    int addrlen;
    struct sockaddr_storage client_addr = {0};

    const char* const conn_msg = "[INFO] incoming connection from ";
    const char* const debug_msg =
            "[DEBUG] created socket with fd = '%d' to handle the connection\n";
    
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

static void
handle_client(void)
{
    Action command = NULL;
    
    authenticate_or_die();
    for(;;) {
        command = receive_client_command();
        if (command == NULL) {
            send_ko();
        } else {
            send_ok();
            command();
        }
    }
}


/*
 * Reads user credentials from the wire and then if they are valid it saves the
 * user name. Otherwise the client-serving process is aborted
 */
static void
authenticate_or_die(void)
{
    Credentials login = empty_credentials;
    
    bel_recvall_or_die(sockfd_acc, login.uname, UNAME_MSGLEN);
    bel_recvall_or_die(sockfd_acc, login.pword, PWORD_MSGLEN);
    if(!is_valid_login(login)) {
        send_ko();
        exit(EXIT_SUCCESS);
    }
    strcpy(current_user, login.uname);
    send_ok();
}

/*
 * Returns 1 if given credentials match one of the registered (hardcoded)
 * users, 0 otherwise
 */
static int
is_valid_login(const Credentials login)
{
    int i, uname_matches, pword_matches;
    const Credentials users[NO_OF_USERS] =
            {{"pippo", "pluto"}, {"admin", "admin"}, {"test", "test1234"}};

    for(i = 0; i < NO_OF_USERS; ++i) {
        uname_matches = strcmp(login.uname, users[i].uname) == 0;
        pword_matches = strcmp(login.pword, users[i].pword) == 0;
        if(uname_matches && pword_matches) return 1;    /* true  */
    }
    return 0;   /* false  */
}


/* Listens for a command from the client and returns the matching Action  */
static Action
receive_client_command(void)
{
    int i;
    char cmd[CMD_MSGLEN] = "";
    const Command commands[NO_OF_COMMANDS] = {
            {CMD_READ,      handle_read},
            {CMD_SEND,      handle_send},
            {CMD_DELETE,    handle_delete}
            };
    
    bel_recvall_or_die(sockfd_acc, cmd, CMD_MSGLEN);
    for(i = 0; i < NO_OF_COMMANDS; ++i) {
        if (strcmp(cmd, commands[i].name) == 0) return commands[i].action;
    }
    fprintf(stderr, "[WARN] unrecognized message '%s'\n", cmd);
    return NULL;
}


static void
handle_read(void)
{
    int msgcount;
    Message messages[MSG_LIST_SIZE];
    char listbuf[MSG_TOSTRING_SIZE * MSG_LIST_SIZE] = "";
    
    msgcount = msg_retrieve_some(messages, MSG_LIST_SIZE);
    msg_arraytostring(messages, msgcount, listbuf);
    
    bel_sendall_or_die(sockfd_acc, listbuf, LIST_MSGLEN);
}


static void
handle_send(void)
{
    Message msg = empty_message;
    
    strcpy(msg.from, current_user);
    bel_recvall_or_die(sockfd_acc, msg.subject, TXT_MSGLEN);
    bel_recvall_or_die(sockfd_acc, msg.body,    TXT_MSGLEN);
    
    msg_trace(msg);
    msg_store(msg);
    send_ok();
}


static void
handle_delete(void)
{
    char id_buf[ID_MSGLEN] = "";
    char *endptr = NULL;
    long id = 0L;
    
    handle_read();
    bel_recvall_or_die(sockfd_acc, id_buf, ID_MSGLEN);
    id = strtol(id_buf, &endptr, 10);   /* 10 is the base   */
    if (*endptr) {  /* could not convert entire string  */
        fprintf(stderr, "[WARN] received non-numeric id '%s'\n", id_buf);
        send_ko();
    } else {
        if(msg_delete(current_user, id)) send_ok(); else send_ko();
    }
}


static void
send_ok(void)
{
    bel_sendall_or_die(sockfd_acc, ANSWER_OK, ANSWER_MSGLEN);
}

static void
send_ko(void)
{
    bel_sendall_or_die(sockfd_acc, ANSWER_KO, ANSWER_MSGLEN);
}
