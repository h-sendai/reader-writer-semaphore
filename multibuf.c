#include "multibuf.h"

char *outfile = NULL;
host_info *host_list = NULL;
int debug = 0;
shared_struct shared;
int so_rcvbuf = 0;

int usage(void)
{
    fprintf(stderr, "Usage: mutibuf ip_address:port\n");
    return 0;
}

int main(int argc, char *argv[])
{
    int i, ch;
    pthread_t tid_reader, tid_writer;
    host_info *p;
    
    while ( (ch = getopt(argc, argv, "do:r:")) != -1) {
        switch (ch) {
            case 'd':
                debug ++;
                break;
            case 'o':
                outfile = optarg;
                break;
            case 'r':
                so_rcvbuf = get_num(optarg);
                break;
            case '?':
                usage();
                exit(1);
            default:
                break;
        }
    }
    argc -= optind;
    argv += optind;
    if (argc == 0) {
        usage();
        exit(1);
    }

    for (i = 0; i < argc; i++) {
        host_list = addend(host_list, new_host(argv[i]));
    }
    if (debug) {
        for (p = host_list; p != NULL; p = p->next) {
            fprintf(stderr,"%s\n", p->ip_address);
        }
        fprintf(stderr, "number of buffers: %d\n", NBUFF);
        fprintf(stderr, "buffer size: %d\n", BUFFSIZE);
    }
    
    if (sem_init(&shared.n_empty,  0, NBUFF) != 0) {
        err(1, "sem_init for shared.n_empty");
    }
    if (sem_init(&shared.n_stored, 0, 0) != 0) {
        err(1, "sem_init for shared.n_stored");
    }
    if (sem_init(&shared.file_preparation, 0, 0) != 0) {
        err(1, "sem_init for file_preparation");
    }

    if (pthread_create(&tid_reader, NULL, reader, NULL) != 0) {
        err(1, "pthread_create on reader");
    }
    if (pthread_create(&tid_writer, NULL, writer, NULL) != 0) {
        err(1, "pthread_create on writer");
    }

    if (pthread_join(tid_reader, NULL) != 0) {
        err(1, "pthread_join on reader");
    }
    if (pthread_join(tid_writer, NULL) != 0) {
        err(1, "pthread_join on reader");
    }

    if (sem_destroy(&shared.n_empty) != 0) {
        err(1, "sem_destroy for shared.n_empty");
    }
    if (sem_destroy(&shared.n_stored) != 0) {
        err(1, "sem_destroy for shared.n_stored");
    }

    return 0;
}
