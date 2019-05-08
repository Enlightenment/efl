#ifndef __EVIL_MACRO_WRAPPER_H__
#define __EVIL_MACRO_WRAPPER_H__


/*
 * evil_locale.h
 */

/**
 * @def setlocale(cat, loc)
 *
 * Wrapper around evil_setlocale().
 * @since 1.16
 */
#ifdef setlocale /* libintl.h defines setlocale() but always returns "C" */
# undef setlocale
#endif
#define setlocale(cat, loc) evil_setlocale(cat, loc)

/*
 * evil_stdio.h
 */

/**
 * @def rename(src, dest)
 *
 * Wrapper around evil_rename().
 *
 * @since 1.8
 */
#ifdef rename
# undef rename
#endif
#define rename(src, dst) evil_rename(src, dst)

/**
 * @def mkdir(dirname, mode)
 *
 * Wrapper around evil_mkdir().
 *
 * @since 1.15
 */
#ifdef mkdir
# undef mkdir
#endif
#define mkdir(dirname, mode) evil_mkdir(dirname, mode)

/*
 * evil_time.h
 */

/**
 * @def localtime_r(t, r)
 *
 * Wrapper around evil_localtime_r().
 */
#ifdef localtime_r
# undef localtime_r
#endif
#define localtime_r(t, r) evil_localtime_r(t, r)

/*
 * evil_unistd.h
 */

/**
 * @def pipe(fds)
 *
 * Wrapper around evil_pipe().
 */
#ifdef pipe
# undef pipe
#endif
#define pipe(fds) evil_pipe(fds)


#endif
