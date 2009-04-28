/* $Id: utimer.c,v 1.2 2008/10/31 19:09:22 rtrummer Exp $ */

/*
 * Copyright (c) Harald Roeck hroeck@cs.uni-salzburg.at
 *
 * University Salzburg, www.uni-salzburg.at
 * Department of Computer Science, cs.uni-salzburg.at
 */

/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include <sys/time.h>
#include <time.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include "utimer.h"
#include "controller-config.h"

#if 0

#include <syscall.h>
static inline int clock_nanosleep(clockid_t clock, int flags, const struct timespec* tsave, struct timespec* rmtp)
{
        return syscall(__NR_clock_nanosleep, clock, flags, tsave, rmtp);
}

#endif

unsigned long long getutime()
{
    struct timeval tv;
    int err;
    long long retval;

    err = gettimeofday(&tv, NULL);
    if (err) {
        fprintf(stderr, "gettimeofday error: %d %s\n", errno, strerror(errno));
        retval = -1;
    } else {
        retval = tv.tv_sec;
        retval *= USECS_PER_SEC;
        retval += tv.tv_usec;
    }

    return retval;
}

int sleep_until(unsigned long long time)
{
    struct timespec rqtp;
    rqtp.tv_sec  = time/USECS_PER_SEC;
    rqtp.tv_nsec = (time - rqtp.tv_sec * USECS_PER_SEC) * NSECS_PER_USEC;
    int err;

    while ((err = clock_nanosleep(CLOCK_REALTIME, TIMER_ABSTIME, &rqtp, NULL))) {
        if (err == EINTR) continue;

        fprintf(stderr, "clock_nanosleep error: %d %s\n", err, strerror(err));
        return -1;
    }
    return 0;
}

int sleep_for(long long usecs)
{
    struct timespec rqtp, rmtp, *prqtp, *prmtp;
    int err;

    rqtp.tv_sec  = usecs/USECS_PER_SEC;
    rqtp.tv_nsec = (usecs - rqtp.tv_sec * USECS_PER_SEC) * NSECS_PER_USEC;
    prqtp = &rqtp;
    prmtp = &rmtp;

    while ((err = clock_nanosleep(CLOCK_REALTIME, 0, prqtp, prmtp))) {
        if (err == EINTR) {
            struct timespec *t = prqtp;
            prqtp = prmtp;
            prmtp = t;
            continue;
        }
        fprintf(stderr, "clock_nanosleep error: %d %s\n", err, strerror(err));
        return -1;
    }

    return 0;
}
