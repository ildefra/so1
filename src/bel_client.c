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
#define NO_OF_MENUITEMS 4


typedef struct {
    
    /* 
     * beware: lengths include the '\0' terminator, so actual lengths are 1 byte
     * shorter
     */
    
        #define MENU_NAME_MAXLEN 8
    char name[MENU_NAME_MAXLEN];
    
        #define MENU_DESCR_MAXLEN 64
    char descr[MENU_DESCR_MAXLEN];
    
    void (*action)();
} MenuItem, *Menu;


void chop_newline(char*);
int ok_from_server(void);

void read_all_messages(void);
void send_new_message(void);
void delete_message(void);
void user_quit(void);


const MenuItem menu[NO_OF_MENUITEMS] = {
        {"read", "read all messages", read_all_messages},
        {"send", "send new message", send_new_message},
        {"delete", "deletes a message", delete_message},
        {"quit", "quits program", user_quit}
        };


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


/* Authenticates against the server and exits the program on bad credentials  */
void
authenticate(void) {
    void send_credentials(void);
    
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
    char uname[UNAME_MSGLEN + 1] = "";  /* +1 for \n  */
    char pword[PWORD_MSGLEN + 1] = "";  /* +1 for \n  */
        
    printf("insert your username (max %d characters): ", UNAME_MSGLEN - 1);
    chop_newline(fgets(uname, sizeof(uname), stdin));
    bel_sendall_or_die(sockfd, uname, UNAME_MSGLEN);
    
    printf("insert your password (max %d characters): ", PWORD_MSGLEN - 1);
    chop_newline(fgets(pword, sizeof(pword), stdin));
    bel_sendall_or_die(sockfd, pword, PWORD_MSGLEN);
}


/* Actual business logic of the client  */
void
run_client(void)
{
    char input_buf[MENU_NAME_MAXLEN + 1] = "";  /* +1 for \n  */
    
    void show_menu();
    void read_user_choice(char*);
    void (*retrieve_menu_action(char*))(void);
    
    for (;;) {
        show_menu();
        read_user_choice(input_buf);
        retrieve_menu_action(input_buf)();
    }
}

void
show_menu()
{
    int i;
    char bracketed_name[MENU_NAME_MAXLEN + 2] = "";
    
    for (i = 0; i < NO_OF_MENUITEMS; ++i) {
        sprintf(bracketed_name, "[%s]", menu[i].name);
        printf("\n%*s %s", MENU_NAME_MAXLEN + 2, bracketed_name, menu[i].descr);
    }
}

/* Prompts the user and then fills the given buffer with user input  */
void
read_user_choice(char* input_buf)
{
    fflush(stdin);  /* clearing the keyboard buffer to avoid surprises  */

    printf("\nEnter a command: ");
    chop_newline(fgets(input_buf, sizeof(input_buf), stdin));
}

/*
 * Returns the menu action that matches the given name. If no action matches,
 * the "null-action" invalid_command is returned instead
 */
void
(*retrieve_menu_action(char* menu_item_name))(void)
{
    int i;
    
    void invalid_command(void);
    
    for(i = 0; i < NO_OF_MENUITEMS; ++i) {
        if (strcmp(menu_item_name, menu[i].name) == 0) return menu[i].action;
    }
    return invalid_command;
}

/*
 * Just informs the user that the command entered did not match any action in
 * the menu. This behaviour is defined as a function to avoid NULL-checking.
 */
void
invalid_command(void)
{
    printf("Invalid command entered\n");
}


void
read_all_messages(void) {
    printf("[TRACE] inside read_all_messages\n");
    
    /* TODO: implement  */
    
}

void
send_new_message(void) {
    printf("[TRACE] inside send_new_message\n");
    
    /* TODO: implement  */
    
}

void
delete_message(void) {
    printf("[TRACE] inside delete_message\n");
    
    /* TODO: implement  */
    
}

void
user_quit(void) {
    printf("Goodbye!\n");
    exit(EXIT_SUCCESS);
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
    return strcmp(answer, ANSWER_OK) == 0;
}


/* Removes the last character of the given string if it is a newline  */
void
chop_newline(char *str)
{
    size_t len;
    
    len = strlen(str);
    if (len > 0 && str[len-1] == '\n') str[len-1] = '\0';
}
