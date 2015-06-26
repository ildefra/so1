#ifndef MSGSTORAGE_H_INCLUDED
#define MSGSTORAGE_H_INCLUDED

#include <stdio.h>

#define FROM_MAXLEN 32
#define TXT_MAXLEN 256
#define MSG_TOSTRING_SIZE FROM_MAXLEN + TXT_MAXLEN * 2

typedef struct {
    char from[FROM_MAXLEN];
    char subject[TXT_MAXLEN];
    char body[TXT_MAXLEN];
} Message;


/* Prints the given message (for debugging purposes)  */
extern void msg_trace(const Message msg);

extern void msg_tostring(const Message, char[MSG_TOSTRING_SIZE]);

/* Writes a Message array <msg> of <array_size> elements into <buf>  */
extern void
msg_arraytostring(const Message* msg, const int array_size, char *buf);


/*
 * Creates the database file at the specified location if it does not exist
 * yet. To be called before any store or retrieve operation.
 * Exits on failure
 */
extern void msg_init_db_or_die(const char* const);

/* Stores <msg> in the last position of the database  */
extern void msg_store(const Message msg);

/*
 * Fills <buf> with the first Messages from the database, filling in at most
 * <count> items.
 * Returns the number of filled items
 */
extern int msg_retrieve_some(Message* buf, const int count);

extern void msg_delete(const int);

#endif	/* MSGSTORAGE_H_INCLUDED */
