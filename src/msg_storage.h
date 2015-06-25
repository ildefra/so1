#ifndef IOFILEUTIL_H_INCLUDED
#define IOFILEUTIL_H_INCLUDED

#include <stdio.h>

FILE* io_fopen_or_die(const char* const, const char* const);
void io_fclose_or_die(FILE*);
long io_filesize_or_die(FILE*);

#endif	/* IOFILEUTIL_H_INCLUDED */
