#include "reader-writer-semaphore.h"

char *outfile = NULL;
host_info *host_list = NULL;
int debug = 0;
shared_struct shared;
int so_rcvbuf = 0;
int n_servers = 0;

int usage(void)
{
    char *message = 
"Usage: reader-writer-semaphore [-d] [-o outfile] [-r so_rcvbuf] ip_address:port\n"
"Options:\n"
"    -d debug (many times)\n"
"    -o outfile\n"
"    -r so_rcvbuf\n";
    fprintf(stderr, "%s\n",message);
    return 0;
}

void sig_int(int signo)
{
    has_interrupt = 1;
    return;
}

int main(int argc, char *argv[])
{
    int ch;
    pthread_t tid_reader, tid_writer;
    
    has_interrupt = 0;
    my_signal(SIGINT, sig_int);
    
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
    if (argc != 1) {
        usage();
        exit(1);
    }
    
    char *remote_host_info = argv[0];

    if (sem_init(&shared.n_empty,  0, NBUFF) != 0) {
        err(1, "sem_init for shared.n_empty");
    }
    if (sem_init(&shared.n_stored, 0, 0) != 0) {
        err(1, "sem_init for shared.n_stored");
    }
    if (sem_init(&shared.file_preparation, 0, 0) != 0) {
        err(1, "sem_init for file_preparation");
    }

    if (pthread_create(&tid_reader, NULL, reader, remote_host_info) != 0) {
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
