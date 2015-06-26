/*
 * msg_storage - operations on messages, mainly storing and retrieving them
 */

#include "msg_storage.h"
#include <stdlib.h>

#define NO_OF_MSG_FIELDS 3

static const Message empty_msg;

static FILE* db;


static void close_db(void);
static Message* retrieve_one_or_die(void);


void
msg_trace(const Message msg)
{
    printf("[TRACE] from = '%s', subject = '%s', body='%s'\n",
            msg.from, msg.subject, msg.body);    
}

void
msg_tostring(const Message msg, char buf[MSG_TOSTRING_SIZE])
{
    sprintf(buf, "%s\n%s\n%s\n\n", msg.from, msg.subject, msg.body);
}

void
msg_arraytostring(const Message* msg, const int array_size, char *buf)
{
    int i, buf_idx;
    char msgbuf[MSG_TOSTRING_SIZE];
    
    printf("[TRACE] msg_arraytostring - array_size = '%d', buf='%s'\n",
            array_size, buf);
    for (i = 0; i < array_size; ++msg, ++i) {
        memset(msgbuf, 0, MSG_TOSTRING_SIZE);
        msg_tostring(*msg, msgbuf);
        buf_idx += sprintf(buf + buf_idx, "%s", msgbuf);
    }
}


void
msg_init_db_or_die(const char* const file_path)
{
    db = fopen(file_path, "ab+");
    if (db == NULL) {
        perror("[ERROR] fopen()");
        exit(EXIT_FAILURE);
    }
    atexit(close_db);
}

static void
close_db(void)
{
    int fclose_res;
    
    fclose_res = fclose(db);
    if (fclose_res == EOF) {
        perror("[ERROR] fclose()");
        exit(EXIT_FAILURE);
    }    
}


void
msg_store(const Message msg)
{
    fseek(db, 0L, SEEK_END);
    fprintf(db, "%s\n%s\n%s\n\n", msg.from, msg.subject, msg.body);
}


int
msg_retrieve_some(Message* ret, const int count)
{
    int i;
    Message *msg;
    
    fseek(db, 0L, SEEK_SET);
    for (i = 0; i < count; ++i) {
        msg = retrieve_one_or_die();
        if (msg == NULL) break;
        ret[i] = *msg;
    }
    return i;
}

static Message*
retrieve_one_or_die(void)
{
    int fscanf_res;
    Message *msgptr;
    
    msgptr = calloc(1, sizeof(Message));
    if (msgptr == NULL) return NULL;
    fscanf_res = fscanf(db, "%[^\n]\n%[^\n]\n%[^\n]\n\n",
            msgptr->from, msgptr->subject, msgptr->body);
    printf("[TRACE] fscanf_res = '%d'\n", fscanf_res);
    msg_trace(*msgptr);
    if (fscanf_res == EOF) {
        printf("[TRACE] reached End Of File\n");
        return NULL;
    } else if (fscanf_res != NO_OF_MSG_FIELDS) {
        fprintf(stderr, "[ERROR] database is corrupted: exiting\n");
        exit(EXIT_FAILURE);
    }
    return msgptr;
}


void
msg_delete(const int msgid)
{
    /* TODO: implement  */
}
