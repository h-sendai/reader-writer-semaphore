#ifndef _MULTIBUF_H
#define _MULTIBUF_H

#include <sys/time.h>

#include <err.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>

#include "get_num.h"
#include "readn.h"
#include "my_signal.h"

extern char *outfile;
extern int debug;
extern int so_rcvbuf;
extern unsigned long long n_loop_reader;
extern unsigned long long n_loop_writer;
extern int n_servers;

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
volatile sig_atomic_t has_interrupt;

#endif
