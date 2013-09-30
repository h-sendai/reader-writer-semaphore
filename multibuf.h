#ifndef _MULTIBUF_H
#define _MULTIBUF_H

#include <sys/time.h>

#include <err.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>

#include "get_num.h"
#include "host_info.h"
#include "readn.h"

extern host_info *host_list;
extern char *outfile;
extern int debug;
extern int so_rcvbuf;
extern unsigned long long n_loop_reader;
extern unsigned long long n_loop_writer;

extern void *reader(void *);
extern void *writer(void *);

#define NBUFF 4096
/* #define NBUFF 256 */
#define BUFFSIZE 32*1024

typedef struct {
    struct {
        char data[BUFFSIZE];
        ssize_t n;
    } buff[NBUFF];
    sem_t n_empty, n_stored, file_preparation;
} shared_struct;

extern shared_struct shared;
#endif
