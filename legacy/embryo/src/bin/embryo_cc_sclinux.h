#include "config.h"

#if !defined BIG_ENDIAN
# define BIG_ENDIAN    4321
#endif
#if !defined LITTLE_ENDIAN
# define LITTLE_ENDIAN 1234
#endif
#ifdef WORDS_BIGENDIAN
# undef BYTE_ORDER
# define BYTE_ORDER BIG_ENDIAN
#else
# undef BYTE_ORDER
# define BYTE_ORDER LITTLE_ENDIAN
#endif

#define getch           getchar
#define	stricmp(a,b)    strcasecmp(a,b)
#define	strnicmp(a,b,c) strncasecmp(a,b,c)

/* unix sep char- shoudl just assume this. */
#define	DIRECTORY_SEP_CHAR      '/'
#define	DIRECTORY_SEP_STR       "/"
