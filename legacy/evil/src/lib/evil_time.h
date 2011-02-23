#ifndef __EVIL_TIME_H__
#define __EVIL_TIME_H__


/**
 * @file evil_time.h
 * @brief The file that provides functions ported from Unix in time.h.
 * @defgroup Evil_Time_Group Time.h functions
 *
 * This header provides functions ported from Unix in time.h.
 *
 * @{
 */


/**
 * @brief Convert the calendar time to broken-time representation in a
 * user supplied data.
 *
 * @param timep The calender time.
 * @param result The broken-down time representation.
 * @return The broken-down time representation.
 *
 * This function converts the calendar time @p timep to a broken-time
 * representation. The result is stored in the buffer  @p result
 * supplied by the user. If @p timep or @p result are @c NULL, or if
 * an error occurred, this function returns @c NULL and the values in
 * @p result might be undefined. Otherwise it returns @p result.
 *
 * Conformity: Non applicable.
 *
 * Supported OS: Windows XP.
 */
EAPI struct tm *localtime_r(const time_t *timep, struct tm *result);

#ifdef UNDER_CE

/**
 * @brief Stub implementation of tzset().
 *
 * This function does nothing.
 *
 * Conformity: Non applicable.
 *
 * Supported OS: Windows CE.
 */
EAPI void tzset(void);

#endif


/**
 * @}
 */


#endif /* __EVIL_TIME_H__ */
