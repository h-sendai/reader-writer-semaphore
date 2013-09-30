#include <sys/ioctl.h>
#include <sys/time.h>

#include "multibuf.h"
#include "my_socket.h"

unsigned long long n_loop_reader = 0;

void * reader(void *arg)
{
    int i, bytes_in_sock;
    host_info *p;
    struct timeval tv, start_time, end_time, time_diff;
    char timestamp[1024];
    socklen_t len = sizeof(so_rcvbuf);

    if (sem_wait(&shared.file_preparation) != 0) {
        err(1, "sem_wait on reader for file_preparation");
    }

    for (p = host_list; p != NULL; p = p->next) {
        if ( (p->sockfd = tcp_socket()) < 0) {
            errx(1, "socket create fail");
        }
        if (so_rcvbuf > 0) {
            if (setsockopt(p->sockfd, SOL_SOCKET, SO_RCVBUF, &so_rcvbuf, len) < 0) {
                err(1, "setsockopt SO_RCVBUF %d", so_rcvbuf);
            }
        }
        if (debug) {
            if (getsockopt(p->sockfd, SOL_SOCKET, SO_RCVBUF, &so_rcvbuf, &len) < 0) {
                err(1, "getsockopt SO_RCVBUF");
            }
            fprintf(stderr, "# SO_RCVBUF: %d\n", so_rcvbuf);
        }
    }
    for (p = host_list; p != NULL; p = p->next) {
        if (connect_tcp(p->sockfd, p->ip_address, p->port) < 0) {
            errx(1, "connect to %s fail", p->ip_address);
        }
    }
    if (gettimeofday(&start_time, NULL) < 0) {
        err(1, "gettimeofday on reader");
    }

    /* XXX: one host for now */
    p = host_list;
    for (i = 0; ; ) {
        n_loop_reader ++;

        if (debug > 1) {
            if ((n_loop_reader - n_loop_writer) > 1) {
                if (gettimeofday(&tv, NULL) < 0) {
                    err(1, "gettimeofday in debug");
                }
                fprintf(stderr, "%ld.%06ld ", tv.tv_sec, tv.tv_usec);
                fprintf(stderr, "reader - writer: %llu\n", n_loop_reader - n_loop_writer);
            }
        }
        if (debug > 2) {
            if (n_loop_reader % 100 == 0) {
                if (ioctl(p->sockfd, FIONREAD, &bytes_in_sock) < 0) {
                    err(1, "ioctl FIONREAD");
                }
                fprintf(stderr, "reader: %d bytes in socket\n", bytes_in_sock);
            }
        }

        //if (sem_wait(&shared.n_empty) != 0) {
        //    err(1, "sem_wait on reader for shared.n_empty");
        //}
        if (sem_trywait(&shared.n_empty) != 0) {
            if (errno == EAGAIN) {
                fprintf(stderr, "cannot get empty buffer. exit.\n");
                fprintf(stderr, "read %d bytes %llu counts\n", BUFFSIZE, n_loop_reader);
                if (gettimeofday(&end_time, NULL) < 0) {
                    err(1, "gettimeofday on reader");
                }
                timersub(&end_time, &start_time, &time_diff);
                strftime(timestamp, sizeof(timestamp), "%F %T", localtime(&end_time.tv_sec));
                fprintf(stderr, "%s ( running time %ld.%06ld )\n",
                    timestamp, time_diff.tv_sec, (long) time_diff.tv_usec);
                exit(1);
            }
        }

        shared.buff[i].n = readn(p->sockfd, shared.buff[i].data, BUFFSIZE);
        if (shared.buff[i].n < 0) {
            errx(1, "readn error");
        }
        else if (shared.buff[i].n == 0) {
            if (sem_post(&shared.n_stored) != 0) { err(1, "sem_post on reader for shared.n_stored (EOF)\n");
            }
        }

        if (++i >= NBUFF) {
            i = 0; /* circular buffer */
        }
        if (sem_post(&shared.n_stored) != 0) {
            err(1, "sem_post on reader for shared.n_stored");
        }
    }

    for (p = host_list; p != NULL; p = p->next) {
        if (close(p->sockfd) < 0) {
            err(1, "connect to %s fail", p->ip_address);
        }
    }
    return NULL;
}
