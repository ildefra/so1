/* io_fileutil - general I/O utility functions  */

#include "io_fileutil.h"
#include <stdlib.h>

FILE*
io_fopen_or_die(const char* const filename, const char* const mode)
{
    FILE *fp;
    
    fp = fopen(filename, mode);
    if (fp == NULL) {
        perror("[ERROR] fopen()");
        exit(EXIT_FAILURE);
    }
    return fp;
}

void
io_fclose_or_die(FILE* fp)
{
    int fclose_res;
    
    fclose_res = fclose(fp);
    if (fclose_res == EOF) {
        perror("[ERROR] fclose()");
        exit(EXIT_FAILURE);
    }
}

long
io_filesize_or_die(FILE* fp)
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
