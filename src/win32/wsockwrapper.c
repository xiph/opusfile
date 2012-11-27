#include <stdio.h>
#include "wsockwrapper.h"

int win32_poll(struct pollfd *fds, unsigned nfds, int timeout)
{
    fd_set ifds, ofds, efds;
    struct timeval tv;
    unsigned i;
    int rc;

    FD_ZERO(&ifds);
    FD_ZERO(&ofds);
    FD_ZERO(&efds);
    for (i = 0; i < nfds; ++i) {
	fds[i].revents = 0;
	if (fds[i].events & POLLIN)
	    FD_SET(fds[i].fd, &ifds);
	if (fds[i].events & POLLOUT)
	    FD_SET(fds[i].fd, &ofds);
	FD_SET(fds[i].fd, &efds);
    }
    if (timeout >= 0) {
	tv.tv_sec = timeout / 1000;
	tv.tv_usec = (timeout - tv.tv_sec * 1000) * 1000;
    }
    rc = select(255, &ifds, &ofds, &efds, timeout < 0 ? 0 : &tv);
    if (rc > 0) {
	for (i = 0; i < nfds; ++i) {
	    if (FD_ISSET(fds[i].fd, &ifds))
		fds[i].revents |= POLLIN;
	    if (FD_ISSET(fds[i].fd, &ofds))
		fds[i].revents |= POLLOUT;
	    if (FD_ISSET(fds[i].fd, &efds))
		fds[i].revents |= POLLHUP;
	}
    }
    return rc;
}

int win32_ftime(struct timeb *timer)
{
    ftime(timer);
    return 0;
}
