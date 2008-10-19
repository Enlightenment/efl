#ifndef	__EVIL_ERRNO_H__
#define	__EVIL_ERRNO_H__

#ifdef EAPI
# undef EAPI
#endif /* EAPI */

#ifdef _WIN32
# ifdef EFL_EVIL_BUILD
#  ifdef DLL_EXPORT
#   define EAPI __declspec(dllexport)
#  else
#   define EAPI
#  endif /* ! DLL_EXPORT */
# else
#  define EAPI __declspec(dllimport)
# endif /* ! EFL_EVIL_BUILD */
#endif /* _WIN32 */

#ifdef	__cplusplus
extern "C" {
#endif

extern EAPI int errno;

/* Fake values */
#define E2BIG           1
#define EACCES          2
#define EAGAIN          3
#define EBADF           4
#define EBADMSG         5
#define EBUSY           6
#define ECANCELED       7
#define ECHILD          9
#define EDEADLK         10
#define EDOM            11
#define EEXIST          12
#define EFAULT          13
#define EFBIG           14
#define EINPROGRESS     15
#define EINTR           16
#define EINVAL          17
#define EIO             18
#define EISDIR          19
#define EMFILE          20
#define EMLINK          21
#define EMSGSIZE        22
#define ENAMETOOLONG    23
#define ENFILE          24
#define ENODEV          25
#define ENOENT          26
#define ENOEXEC         27
#define ENOLCK          28
#define ENOMEM          29
#define ENOSPC          30
#define ENOSYS          31
#define ENOTDIR         32
#define ENOTEMPTY       33
#define ENOTSUP         34
#define ENOTTY          35
#define ENXIO           36
#define EPERM           37
#define EPIPE           38
#define ERANGE          39
#define EROFS           40
#define ESPIPE          41
#define ESRCH           42
#define ETIMEDOUT       43
#define EXDEV           44
#define EADDRINUSE      45
#define EADDRNOTAVAIL   46
#define EAFNOSUPPORT    47
#define EALREADY        48
#define ECONNABORTED    49
#define ECONNREFUSED    50
#define ECONNRESET      51
#define EDESTADDRREQ    52
#define EDQUOT          53
#define EHOSTUNREACH    54
#define EIDRM           55
#define EILSEQ          56
#define EISCONN         57
#define ELOOP           58
#define EMULTIHOP       59
#define ENETDOWN        60
#define ENETRESET       61
#define ENETUNREACH     62
#define ENOBUFS         63
#define ENODATA         64
#define ENOLINK         65
#define ENOMSG          66
#define ENOPROTOOPT     67
#define ENOSR           68
#define ENOSTR          69
#define ENOTCONN        70
#define ENOTSOCK        71
#define EOPNOTSUPP      72
#define EOVERFLOW       73
#define EPROTO          74
#define EPROTONOSUPPORT 75
#define EPROTOTYPE      76
#define ESTALE          77
#define ETIME           78
#define ETXTBSY         79
#define EWOULDBLOCK     80

#ifdef	__cplusplus
}
#endif

#endif /* __EVIL_ERRNO_H__ */
