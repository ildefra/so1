/*
 * msg_storage - operations on messages, mainly storing and retrieving them
 */

#include "msg_storage.h"
#include <stdlib.h>

#define DB_FILENAME "db.txt"
#define NO_OF_MSG_FIELDS 3

static const Message empty_msg;


static void retrieve_one_or_die(FILE*, Message*);

static FILE* fopen_or_die(const char* const, const char* const);
static void fclose_or_die(FILE*);
static long filesize_or_die(FILE*);


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
msg_store(const Message msg)
{
    FILE* db;
    
    db = fopen_or_die(DB_FILENAME, "ab");
    fprintf(db, "%s\n%s\n%s\n\n", msg.from, msg.subject, msg.body);
    fclose_or_die(db);
}


int
msg_retrieve_some(Message* ret, const int count)
{
    int i;
    FILE *db;
    
    db = fopen_or_die(DB_FILENAME, "rb");
    for (i = 0; i < count; ++i) {
        retrieve_one_or_die(db, &ret[i]);
        if (&ret[i] == NULL) break;
    }
    fclose_or_die(db);
    return i;
}

static void
retrieve_one_or_die(FILE *fp, Message *msgptr)
{
    bel_chop_newline(fgets(msgptr->from,    FROM_MAXLEN, fp));
    bel_chop_newline(fgets(msgptr->subject, TXT_MAXLEN, fp));
    bel_chop_newline(fgets(msgptr->body,    TXT_MAXLEN, fp));
    msg_trace(*msgptr);
}


static FILE*
fopen_or_die(const char* const filename, const char* const mode)
{
    FILE *fp;
    
    fp = fopen(filename, mode);
    if (fp == NULL) {
        perror("[ERROR] fopen()");
        exit(EXIT_FAILURE);
    }
    return fp;
}

static void
fclose_or_die(FILE* fp)
{
    int fclose_res;
    
    fclose_res = fclose(fp);
    if (fclose_res == EOF) {
        perror("[ERROR] fclose()");
        exit(EXIT_FAILURE);
    }
}

static long
filesize_or_die(FILE* fp)
{
    int fseek_res;
    long filesize;
    
    fseek_res = fseek(fp, 0L, SEEK_END);
    if (fseek_res == -1) {
        perror("[ERROR] fseek()");
        exit(EXIT_FAILURE);
    }
    filesize = ftell(fp);
    rewind(fp);
    return filesize;
}
