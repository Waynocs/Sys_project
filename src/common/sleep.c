/**
 * @ Author: SUBLET Tom & SERANO Waïan
 * @ Create Time: 2021-01-02 00:30:36
 * @ Description: A sleep function
 */

#include <time.h>
#include <errno.h>

#include "sleep.h"

/** Sleep for the requested number of milliseconds.
 * Source: https://stackoverflow.com/a/1157217
 */
int msleep(long msec)
{
    struct timespec ts;
    int res;

    if (msec < 0)
    {
        errno = EINVAL;
        return -1;
    }

    ts.tv_sec = msec / 1000;
    ts.tv_nsec = (msec % 1000) * 1000000;

    do
    {
        res = nanosleep(&ts, &ts);
    } while (res && errno == EINTR);

    return res;
}