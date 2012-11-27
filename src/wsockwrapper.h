#ifndef WSOCKWRAPPER_H
#define WSOCKWRAPPER_H

#include <winsock2.h>
#include <sys/timeb.h>

#define POLLIN      0x0001    /* There is data to read */
#define POLLPRI     0x0002    /* There is urgent data to read */
#define POLLOUT     0x0004    /* Writing now will not block */
#define POLLERR     0x0008    /* Error condition */
#define POLLHUP     0x0010    /* Hung up */
#define POLLNVAL    0x0020    /* Invalid request: fd not open */

struct pollfd {
    SOCKET fd;        /* file descriptor */
    short events;     /* requested events */
    short revents;    /* returned events */
};

#define poll(x, y, z) win32_poll(x, y, z)
int win32_poll(struct pollfd *, unsigned int, int);

int win32_ftime(struct timeb *timer);

#endif
