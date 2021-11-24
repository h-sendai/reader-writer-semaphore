#include "log-et.h"

/*************************************************************************
    System : KEK MLF Module Tester.
    Author : wada@bbtech.co.jp
    Description : BBT MLF Utilites for Linux
    $KMM_TESTER_BEGIN_LICENSE$
    $KMM_TESTER_END_LICENSE$
*************************************************************************/

static struct timeval tv_start;
static struct timeval tv_prev;

int set_start_tv()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    tv_start = tv;
    tv_prev  = tv;
    return 0;
}

int log_et(FILE *pFILE, const char *fmt, ...)
{
    struct timeval now, elapsed, interval;
    int nRET = -1;
    if( pFILE != NULL)
    {
        nRET = -2;
        char buff[2048];
        va_list ap;
        va_start(ap, fmt);
        int n = vsnprintf(buff ,sizeof(buff)-2, fmt, ap);
        va_end(ap);
        if( n > 0 ){
            gettimeofday(&now, NULL);
            timersub(&now, &tv_start, &elapsed);
            timersub(&now, &tv_prev,  &interval);
            nRET = 0;
            nRET += fprintf(pFILE, "%ld.%06ld %ld.%06ld ",
                elapsed.tv_sec,  elapsed.tv_usec,
                interval.tv_sec, interval.tv_usec);
            nRET += fprintf(pFILE,buff);
        }
    }
    tv_prev = now;
    return nRET;
}
