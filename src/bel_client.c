/* 
 * bel_client.c - Client part of the OS1 assignment
 *
 * General considerations:
 * - the communication protocol is based on fixed-length messages
 * - every communication failure or server "KO" answer will shutdown the
 *      program
 */

#include "bel_common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


/* Exact number of the arguments required by the program  */
#define ARGC_OK 2

#define NO_OF_MENUITEMS 4


typedef struct {
    
    /* 
     * beware: lengths include the '\0' terminator, so actual lengths are 1
     * byte shorter
     */
    
        #define MENU_NAME_MAXLEN 8
    char name[MENU_NAME_MAXLEN];
    
        #define MENU_DESCR_MAXLEN 64
    char descr[MENU_DESCR_MAXLEN];
    
    Action action;
} MenuItem;


static void connect_to(const char* const, const u_short);
static int do_connect(struct addrinfo*);

static void authenticate(void);
static void run_client(void);
static void show_menu(void);
static Action read_action_from_user(void);

static void read_all_messages(void);
static void send_new_message(void);
static void delete_message(void);
static void user_quit(void);

static int ok_from_server(void);
static void send_user_input_to_server(const char* const, const int);
static void reset_stdin(void);


const MenuItem menu[NO_OF_MENUITEMS] = {
        {"read",    "read all messages",    read_all_messages},
        {"send",    "send new message",     send_new_message},
        {"delete",  "deletes a message",    delete_message},
        {"quit",    "quits program",        user_quit}
        };


/* (file descriptor of) the socket used to communicate with server  */
static int sockfd;


/*
 * Explicitly closes the resources acquired by the current process. Called on
 * process exit
 */
static void
cleanup(void)
{
    printf("[DEBUG] resource cleanup\n");
    if (sockfd != 0) bel_close_or_die(sockfd);
}


/* Client entry point  */
int
main(int argc, char **argv)
{    
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
static void
connect_to(const char* const ip, const u_short port)
{
    int do_connect_res = 0;
    struct addrinfo *servinfo = NULL, *currinfo = NULL;
    
    bel_getaddrinfo_or_die(ip, AF_UNSPEC, port, &servinfo);
    for(currinfo = servinfo; currinfo != NULL; currinfo = currinfo->ai_next) {
        do_connect_res = do_connect(currinfo);
        if (do_connect_res != -1) break;
    }
    if (currinfo == NULL) {
        fprintf(stderr, "[FATAL] failed to connect: exiting\n");
        exit(EXIT_FAILURE);
    }
    freeaddrinfo(servinfo);
}

/*
 * Performs the actual connection logic: creates a socket and uses it to
 * connect to the specified address.
 * Saves the file descriptor of the newly-created socket into sockfd.
 * Returns the file descriptor, or -1 on error.
 */
static int
do_connect(struct addrinfo *ainfo)
{
	int connect_res = 0;
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


/*
 * Authenticates against the server with user-submitted credentials.
 * Exits the program on a negative answer
 */
static void
authenticate(void)
{
    send_user_input_to_server("insert your username", UNAME_MSGLEN);
    send_user_input_to_server("insert your password", PWORD_MSGLEN);
    if (!ok_from_server()) {
        printf("wrong username and/or password: exiting\n");
        exit(EXIT_SUCCESS);
    }
}


/* Actual business logic of the client  */
static void
run_client(void)
{
    Action menu_action = NULL;
    
    for (;;) {
        show_menu();
        menu_action = read_action_from_user();
        if (menu_action == NULL) {
            printf("Invalid command entered\n");
            reset_stdin();
        } else {
            menu_action();
        }
    }
}

static void
show_menu(void)
{
    int i;
    char bracketed_name[MENU_NAME_MAXLEN + 2];
    
    for (i = 0; i < NO_OF_MENUITEMS; ++i) {
        memset(bracketed_name, 0, MENU_NAME_MAXLEN + 2);
        sprintf(bracketed_name, "[%s]", menu[i].name);
        printf("\n%*s %s",
                MENU_NAME_MAXLEN + 2, bracketed_name, menu[i].descr);
    }
}

/* Prompts the user and then returns the menu action matching user input  */
static Action
read_action_from_user(void)
{
    int i = 0;
    char input_buf[MENU_NAME_MAXLEN + 1] = "";  /* +1 for \n  */

    printf("\nEnter a command: ");
    bel_chop_newline(fgets(input_buf, sizeof(input_buf), stdin));
    for(i = 0; i < NO_OF_MENUITEMS; ++i) {
        if (strcmp(input_buf, menu[i].name) == 0) return menu[i].action;
    }
    return NULL;
}


static void
read_all_messages(void)
{
    char all_messages[LIST_MSGLEN] = "";
    const char* const no_msgs = "There are no messages to read.\n";
    
    printf("[TRACE] inside read_all_messages\n");
    bel_sendall_or_die(sockfd, CMD_READ, CMD_MSGLEN);
    if(!ok_from_server()) {
        printf("KO answer from server: cannot read");
        return;
    }
    bel_recvall_or_die(sockfd, all_messages, LIST_MSGLEN);
    printf("%s", strcmp("", all_messages) != 0 ? all_messages : no_msgs);
}

static void
send_new_message(void)
{
    printf("[TRACE] inside send_new_message\n");
    bel_sendall_or_die(sockfd, CMD_SEND, CMD_MSGLEN);
    if(!ok_from_server()) {
        printf("KO answer from server: cannot send");
        return;
    }
    send_user_input_to_server("Subject", TXT_MSGLEN);
    send_user_input_to_server("Body", TXT_MSGLEN);
    printf("Server returned %s\n", ok_from_server() ? ANSWER_OK : ANSWER_KO);
}

static void
delete_message(void)
{
    char my_messages[LIST_MSGLEN] = "";
    
    printf("[TRACE] inside delete_message\n");
    bel_sendall_or_die(sockfd, CMD_DELETE, CMD_MSGLEN);
    if(!ok_from_server()) {
        printf("KO answer from server: cannot delete");
        return;
    }
    printf("Here are your messages:\n");
    bel_recvall_or_die(sockfd, my_messages, LIST_MSGLEN);
    printf("%s", my_messages);
    send_user_input_to_server(
            "Enter the ID of the message to delete", ID_MSGLEN);
    printf("Server returned %s\n", ok_from_server() ? ANSWER_OK : ANSWER_KO);
}

static void
user_quit(void)
{
    printf("Goodbye!\n");
    exit(EXIT_SUCCESS);
}


/*
 * Waits for an answer from the server. Returns 1 for a positive answer ("OK")
 * and 0 for a negative one (should be "KO", but does not check for it)
 */
static int
ok_from_server(void)
{
    char answer[ANSWER_MSGLEN] = "";
    
    bel_recvall_or_die(sockfd, answer, ANSWER_MSGLEN);
    return strcmp(answer, ANSWER_OK) == 0;
}


/*
 * Prompts the user with the given message plus a size indication, then reads
 * at most <msglen> bytes from stdin until it encounters a newline character
 * ('\n'), and finally removes it and sends the result to the server
 */
static void
send_user_input_to_server(const char* const prompt_msg, const int buf_len)
{
    char *input_buf = NULL;
    char *full_template;
    const char* const size_template = " (max %d characters): ";
        
    full_template = bel_concat(prompt_msg, size_template);
    printf(full_template, buf_len - 1); /* -1 for '\0'  */
    free(full_template);

    input_buf = calloc(buf_len + 1, 1); /* +1 for '\n'  */
    if (input_buf == NULL) {
        perror("[FATAL] calloc()");
        exit(EXIT_FAILURE);
    }
    bel_chop_newline(fgets(input_buf, buf_len + 1, stdin));
    reset_stdin();
    bel_sendall_or_die(sockfd, input_buf, buf_len);
    free(input_buf);
}


/* Discards all the characters still pending in the stdin buffer  */
static void
reset_stdin(void)
{
    fseek(stdin, 0, SEEK_END);
}
