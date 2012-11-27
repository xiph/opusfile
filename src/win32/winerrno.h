#ifndef WINERRNO_H
#define WINERRNO_H

#include <errno.h>

/* XXX: conflicts with MSVC errno definition */
#ifdef ENAMETOOLONG
#undef ENAMETOOLONG
#endif
#ifdef ENOTEMPTY
#undef ENOTEMPTY
#endif

#define EWOULDBLOCK          35
#define EINPROGRESS          36
#define EALREADY             37
#define ENOTSOCK             38
#define EDESTADDRREQ         39
#define EMSGSIZE             40
#define EPROTOTYPE           41
#define ENOPROTOOPT          42
#define EPROTONOSUPPORT      43
#define ESOCKTNOSUPPORT      44
#define EOPNOTSUPP           45
#define EPFNOSUPPORT         46
#define EAFNOSUPPORT         47
#define EADDRINUSE           48
#define EADDRNOTAVAIL        49
#define ENETDOWN             50
#define ENETUNREACH          51
#define ENETRESET            52
#define ECONNABORTED         53
#define ECONNRESET           54
#define ENOBUFS              55
#define EISCONN              56
#define ENOTCONN             57
#define ESHUTDOWN            58
#define ETOOMANYREFS         59
#define ETIMEDOUT            60
#define ECONNREFUSED         61
#define ELOOP                62
#define ENAMETOOLONG         63
#define EHOSTDOWN            64
#define EHOSTUNREACH         65
#define ENOTEMPTY            66
#define EPROCLIM             67
#define EUSERS               68
#define EDQUOT               69
#define ESTALE               70
#define EREMOTE              71

#endif
