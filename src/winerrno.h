/********************************************************************
 *                                                                  *
 * THIS FILE IS PART OF THE libopusfile SOFTWARE CODEC SOURCE CODE. *
 * USE, DISTRIBUTION AND REPRODUCTION OF THIS LIBRARY SOURCE IS     *
 * GOVERNED BY A BSD-STYLE SOURCE LICENSE INCLUDED WITH THIS SOURCE *
 * IN 'COPYING'. PLEASE READ THESE TERMS BEFORE DISTRIBUTING.       *
 *                                                                  *
 * THE libopusfile SOURCE CODE IS (C) COPYRIGHT 2012                *
 * by the Xiph.Org Foundation and contributors http://www.xiph.org/ *
 *                                                                  *
 ********************************************************************/
#if !defined(_opusfile_winerrno_h)
# define _opusfile_winerrno_h (1)

# include <errno.h>
# include <winerror.h>

/*These conflict with the MSVC errno definitions, but we don't need to use the
   original ones in any file that deals with sockets.*/
# undef ENAMETOOLONG
# undef ENOTEMPTY

# define EWOULDBLOCK     (WSAEWOULDBLOCK-WSABASEERR)
# define EINPROGRESS     (WSAEINPROGRESS-WSABASEERR)
# define EALREADY        (WSAEALREADY-WSABASEERR)
# define ENOTSOCK        (WSAENOTSOCK-WSABASEERR)
# define EDESTADDRREQ    (WSAEDESTADDRREQ-WSABASEERR)
# define EMSGSIZE        (WSAEMSGSIZE-WSABASEERR)
# define EPROTOTYPE      (WSAEPROTOTYPE-WSABASEERR)
# define ENOPROTOOPT     (WSAENOPROTOOPT-WSABASEERR)
# define EPROTONOSUPPORT (WSAEPROTONOSUPPORT-WSABASEERR)
# define ESOCKTNOSUPPORT (WSAESOCKTNOSUPPORT-WSABASEERR)
# define EOPNOTSUPP      (WSAEOPNOTSUPP-WSABASEERR)
# define EPFNOSUPPORT    (WSAEPFNOSUPPORT-WSABASEERR)
# define EAFNOSUPPORT    (WSAEAFNOSUPPORT-WSABASEERR)
# define EADDRINUSE      (WSAEADDRINUSE-WSABASEERR)
# define EADDRNOTAVAIL   (WSAEADDRNOTAVAIL-WSABASEERR)
# define ENETDOWN        (WSAENETDOWN-WSABASEERR)
# define ENETUNREACH     (WSAENETUNREACH-WSABASEERR)
# define ENETRESET       (WSAENETRESET-WSABASEERR)
# define ECONNABORTED    (WSAECONNABORTED-WSABASEERR)
# define ECONNRESET      (WSAECONNRESET-WSABASEERR)
# define ENOBUFS         (WSAENOBUFS-WSABASEERR)
# define EISCONN         (WSAEISCONN-WSABASEERR)
# define ENOTCONN        (WSAENOTCONN-WSABASEERR)
# define ESHUTDOWN       (WSAESHUTDOWN-WSABASEERR)
# define ETOOMANYREFS    (WSAETOOMANYREFS-WSABASEERR)
# define ETIMEDOUT       (WSAETIMEDOUT-WSABASEERR)
# define ECONNREFUSED    (WSAECONNREFUSED-WSABASEERR)
# define ELOOP           (WSAELOOP-WSABASEERR)
# define ENAMETOOLONG    (WSAENAMETOOLONG-WSABASEERR)
# define EHOSTDOWN       (WSAEHOSTDOWN-WSABASEERR)
# define EHOSTUNREACH    (WSAEHOSTUNREACH-WSABASEERR)
# define ENOTEMPTY       (WSAENOTEMPTY-WSABASEERR)
# define EPROCLIM        (WSAEPROCLIM-WSABASEERR)
# define EUSERS          (WSAEUSERS-WSABASEERR)
# define EDQUOT          (WSAEDQUOT-WSABASEERR)
# define ESTALE          (WSAESTALE-WSABASEERR)
# define EREMOTE         (WSAEREMOTE-WSABASEERR)

#endif
