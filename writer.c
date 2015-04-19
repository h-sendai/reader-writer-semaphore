#include "multibuf.h"

unsigned long long n_loop_writer = 0;

void * writer(void *arg)
{
    int i;
    FILE *fp;

    if (outfile == NULL) {
        outfile = "/tmp/data.file";
    }
    if ( (fp = fopen(outfile, "w")) == NULL) {
        err(1, "fopen for %s", outfile);
    }
    if (sem_post(&shared.file_preparation) != 0) {
        err(1, "sem_post on writer for file_preparation");
    }

    for (i = 0; ;) {

        if (debug > 3) {
            fprintf(stderr, "writer: i: %d\n", i);
        }
        if (sem_wait(&shared.n_stored) != 0) {
            err(1, "sem_wait on writer for shared.nstored");
        }
        if (shared.buff[i].n == 0) { /* n == 0 means EOF */
            if (debug > 1) {
                fprintf(stderr, "writer thread: shared.buff[%d].n == 0\n", i);
            }
            return NULL;
        }
        if (fwrite(shared.buff[i].data, 1, shared.buff[i].n, fp) != shared.buff[i].n) {
            ferror(fp);
            exit(1);
        }
        n_loop_writer ++;
        
        if (++i >= NBUFF) {
            i = 0;
        }


        if (sem_post(&shared.n_empty) != 0) {
            err(1, "sem_post on writer for shared.n_empty");
        }
    }
}

