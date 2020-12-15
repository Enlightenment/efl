#ifndef __EVIL_STAT_H__
#define __EVIL_STAT_H__

#ifdef _MSC_VER
# include <evil_private.h>
# include <sys/types.h>
# include <sys/stat.h>
# include <errno.h>
# include <corecrt_io.h>
# include <corecrt.h>

// Missing definitions:
# define S_IRWXU 0        /* RWX user */
# define S_IRUSR S_IREAD  /* Read user */
# define S_IWUSR S_IWRITE /* Write user */
# define S_IXUSR 0        /* Execute user */
# define S_IRWXG 0        /* RWX group */
# define S_IRGRP 0        /* Read group */
# define S_IWGRP 0        /* Write group */
# define S_IXGRP 0        /* Execute group */
# define S_IRWXO 0        /* RWX others */
# define S_IROTH 0        /* Read others */
# define S_IWOTH 0        /* Write others */
# define S_IXOTH 0        /* Execute others */

// Missing definitions from ucrt
# define S_IFIFO  _S_IFIFO

# define S_ISDIR(mode)  (((mode) & S_IFMT) == S_IFDIR)
# define S_ISREG(mode)  (((mode) & S_IFMT) == S_IFREG)
# define S_ISFIFO(mode) (((mode) & S_IFMT) == S_IFIFO)
# define S_ISDIR(mode)  (((mode) & S_IFMT) == S_IFDIR)
# define S_ISREG(mode)  (((mode) & S_IFMT) == S_IFREG)
# define S_ISCHR(mode)  (((mode) & S_IFMT) == S_IFCHR)

#endif /* _MSV_VER */

#endif /* __EVIL_STAT_H__ */
