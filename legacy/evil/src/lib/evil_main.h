#ifndef __EVIL_MAIN_H__
#define __EVIL_MAIN_H__


/**
 * @file evil_main.h
 * @brief The file that provides functions to initialize and shut down Evil.
 * @defgroup Evil_Main_Group Main
 *
 * This header provides functions to initialize and shut down the Evil
 * library.
 *
 * @{
 */


/**
 * @brief Initialize the Evil library.
 *
 * This function initializes the Evil library. It must be called before
 * using evil_time_get(), gettimeofday() or pipe(). It returns  0 on
 * failure, otherwise it returns the number of times it has already been
 * called.
 *
 * When Evil is not used anymore, call evil_shutdown() to shut down
 * the Evil library.
 */
EAPI int evil_init(void);

/**
 * @brief Shut down the Evil library.
 *
 * @return 0 when the Evil library is completely shut down, 1 or
 * greater otherwise.
 *
 * This function shuts down the Evil library. It returns 0 when it has
 * been called the same number of times than evil_init().
 *
 * Once this function succeeds (that is, @c 0 is returned), you must
 * not call any of the Evil function listed in evil_init()
 * documentation anymore . You must call evil_init() again to use these
 * functions again.
 */
EAPI int evil_shutdown(void);


/**
 * @}
 */


#endif /* __EVIL_MAIN_H__ */
