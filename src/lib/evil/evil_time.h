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


/**
 * @brief Convert a string representation of time to a time tm structure .
 *
 * @param buf The string to convert.
 * @param fmt The representation of time.
 * @aram tm The time tm structure.
 * @return The first character not processed in this function call.
 *
 * This function converts the string @p s to a time tm structure and
 * fill the buffer @p tm. The format of the time is specified by
 * @p format. on success, this function returns the first character
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
