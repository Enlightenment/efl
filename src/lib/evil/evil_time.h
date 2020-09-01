#ifndef __EVIL_TIME_H__
#define __EVIL_TIME_H__


#include <time.h>


/**
 * @file evil_time.h
 * @brief The file that provides functions ported from Unix in time.h.
 * @defgroup Evil_Time_Group Time.h functions
 * @ingroup Evil
 *
 * This header provides functions ported from Unix in time.h.
 *
 * @{
 */

#ifdef _MSC_VER
struct timezone
{
  int tz_minuteswest; /* of Greenwich */
  int tz_dsttime;     /* type of dst correction to apply */
};
#endif

/**
 * @brief Get time and timezone.
 *
 * @param tv A pointer that contains two sockets.
 * @param tz A pointer that contains two sockets.
 * @return 0 on success, -1 otherwise.
 *
 * This function gets the time and timezone. @p tv and @p tz can be
 * @c NULL. It calls GetSystemTimePreciseAsFileTime() on Windows 8or
 * above if _WIN32_WINNT is correctly defined. It returns 0 on
 * success, -1 otherwise.
 *
 * @since 1.25
 */
EAPI int evil_gettimeofday(struct timeval *tv, struct timezone *tz);
#ifndef HAVE_GETTIMEOFDAY
# define HAVE_GETTIMEOFDAY 1
#endif


/**
 * @brief Convert a string representation of time to a time tm structure .
 *
 * @param buf The string to convert.
 * @param fmt The representation of time.
 * @param tm The time tm structure.
 * @return The first character not processed in this function call.
 *
 * This function converts the string @p s to a time tm structure and
 * fill the buffer @p tm. The format of the time is specified by
 * @p format. On success, this function returns the first character
 * not processed in this function call, @c NULL otherwise.
 *
 * Conformity: Non applicable.
 *
 * Supported OS: Windows XP.
 */
EAPI char *strptime(const char *buf, const char *fmt, struct tm *tm);


/**
 * @}
 */


#endif /* __EVIL_TIME_H__ */
