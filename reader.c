#include <sys/epoll.h>
#include <sys/ioctl.h>
#include <sys/time.h>

#include "reader-writer-semaphore.h"
#include "my_socket.h"
#include "set_timer.h"
#include "log-et.h"

unsigned long long n_loop_reader = 0;

volatile sig_atomic_t has_alarm = 0;

void sig_alarm(int signo)
{
    has_alarm = 1;
    return;
}

void * reader(void *arg)
{
    int i, bytes_in_sock;
    struct timeval tv, start_time, end_time, time_diff;
    char timestamp[1024];

    if (sem_wait(&shared.file_preparation) != 0) {
        err(1, "sem_wait on reader for file_preparation");
    }

    int port = 24;
    char *remote_host_info = (char *)arg;
    char *tmp = strdup(remote_host_info);
    char *remote_host = strsep(&tmp, ":");
    if (tmp != NULL) {
        port = strtol(tmp, NULL, 0);
    }
    
    // socket
    int sockfd = tcp_socket();
    if (sockfd < 0) {
        exit(1);
    }

    if (so_rcvbuf > 0) {
        if (set_so_rcvbuf(sockfd, so_rcvbuf) < 0) {
            errx(1, "set_so_rcvbuf: %d bytes", so_rcvbuf);
        }
    }

    // TCP connect
    if (connect_tcp(sockfd, remote_host, port) < 0) {
        exit(1);
    }

    my_signal(SIGALRM, sig_alarm);
    set_timer(1, 0, 1, 0);

    set_start_tv();
    if (gettimeofday(&start_time, NULL) < 0) {
        err(1, "gettimeofday on reader");
    }

    int interval_read_bytes = 0;
    for (i = 0; ; ) {
        if (has_alarm) {
            has_alarm = 0;
            log_et(stderr, "%d bytes %.3f Mb\n", interval_read_bytes, interval_read_bytes*8/1000000.0);
            interval_read_bytes = 0;
        }
        if (has_interrupt) {
            shared.buff[i].n = 0;
            if (sem_post(&shared.n_stored) != 0) {
                err(1, "sem_post on reader for shared.n_stored (EOF)\n");
            }
            return NULL; /* terminate reader thread */
        }

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
        shared.buff[i].n = readn(sockfd, shared.buff[i].data, BUFFSIZE);
        if (shared.buff[i].n < 0) {
            errx(1, "readn error");
        }
        else if (shared.buff[i].n == 0) {
            if (sem_post(&shared.n_stored) != 0) {
                err(1, "sem_post on reader for shared.n_stored (EOF)\n");
            }
            if (close(sockfd) < 0) {
                err(1, "close on %s", remote_host);
            }
            exit(0);
        }

        interval_read_bytes += shared.buff[i].n;

        if (++i >= NBUFF) {
            i = 0; /* circular buffer */
        }
        if (sem_post(&shared.n_stored) != 0) {
            err(1, "sem_post on reader for shared.n_stored");
        }

        // debug 
        n_loop_reader ++;
        if (debug > 1) {
            long long diff = n_loop_reader - n_loop_writer;
            if (diff > 5) {
                if (gettimeofday(&tv, NULL) < 0) {
                    err(1, "gettimeofday in debug");
                }
                fprintf(stderr, "%ld.%06ld ", tv.tv_sec, tv.tv_usec);
                fprintf(stderr, "reader - writer: %llu (r: %lld w: %lld)\n",
                    diff, n_loop_reader, n_loop_writer);
            }
        }
        if (debug > 2) {
            if (n_loop_reader % 100 == 0) {
                if (ioctl(sockfd, FIONREAD, &bytes_in_sock) < 0) {
                    err(1, "ioctl FIONREAD");
                }
                fprintf(stderr, "reader: %d bytes in socket\n", bytes_in_sock);
            }
        }
    }

    //if (close(sockfd) < 0) {
    //    err(1, "connect to %s fail", p->ip_address);
    //}
    return NULL;
}
