#ifndef _SYS_TIME_H_
#define _SYS_TIME_H_

#ifdef EVIL_COMMON_API
# undef EVIL_COMMON_API
#endif /* EVIL_COMMON_API */

# ifdef EFL_EVIL_BUILD
#  ifdef DLL_EXPORT
#   define EVIL_COMMON_API __declspec(dllexport)
#  else
#   define EVIL_COMMON_API
#  endif /* ! DLL_EXPORT */
# else
#  define EVIL_COMMON_API __declspec(dllimport)
# endif /* ! EFL_EVIL_BUILD */

#ifdef	__cplusplus
extern "C" {
#endif

#ifndef _WINSOCK2API_
#ifndef timercmp  //_TIMEVAL_DEFINED 
#define _TIMEVAL_DEFINED
struct timeval {
  long tv_sec;
  long tv_usec;
};
#define timerisset(tvp)	 ((tvp)->tv_sec || (tvp)->tv_usec)
#define timercmp(tvp, uvp, cmp) \
	(((tvp)->tv_sec != (uvp)->tv_sec) ? \
	((tvp)->tv_sec cmp (uvp)->tv_sec) : \
	((tvp)->tv_usec cmp (uvp)->tv_usec))
#define timerclear(tvp)	 (tvp)->tv_sec = (tvp)->tv_usec = 0
#endif /* _TIMEVAL_DEFINED */
#endif

/* Provided for compatibility with code that assumes that
   the presence of gettimeofday function implies a definition
   of struct timezone. */
struct timezone
{
  int tz_minuteswest; /* of Greenwich */
  int tz_dsttime;     /* type of dst correction to apply */
};

/*
   Implementation as per:
   The Open Group Base Specifications, Issue 6
   IEEE Std 1003.1, 2004 Edition

   The timezone pointer arg is ignored.  Errors are ignored.
*/ 
EVIL_COMMON_API int __cdecl gettimeofday(struct timeval *tv, void *tzp);

#define HAVE_GETTIMEOFDAY

#ifdef	__cplusplus
}
#endif


#endif /* _SYS_TIME_H_ */
