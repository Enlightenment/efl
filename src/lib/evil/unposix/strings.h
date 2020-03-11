#ifndef UNPOSIX_STRINGS_H
#define UNPOSIX_STRINGS_H

#include <sys/types.h>
#include <locale.h>

inline int ffs(int i)
{
    #warning ffs is not implemented.
    return 0;
}

inline int strcasecmp(const char* s1, const char* s2)
{
    #warning strcasecmp is not implemented.
    return 0;
}

inline int strcasecmp_l(const char* s1, const char* s2, locale_t locale)
{
    #warning strcasecmp_l is not implemented.
    return 0;
}

inline int strncasecmp(const char* s1, const char* s2, size_t size)
{
    #warning strncasecmp is not implemented.
    return 0;
}

inline int strncasecmp_l(const char* s1, const char* s2, size_t size, locale_t locale)
{
    #warning strncasecmp_l is not implemented.
    return 0;
}

#endif
