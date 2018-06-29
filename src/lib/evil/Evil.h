#ifndef __EVIL_H__
#define __EVIL_H__

/**
 * @file Evil.h
 * @brief The file that provides miscellaneous functions ported from Unix.
 */

/**
 * @page evil_main Evil
 * @author Vincent Torri
 * @date 2008 (created)
 *
 * @section toc Table of Contents
 *
 * @li @ref evil_main_intro
 * @li @ref evil_main_ack
 * @li @ref evil_main_compiling
 * @li @ref evil_main_next_steps
 *
 * @section evil_main_intro Introduction
 *
 * The Evil library is an evil library that ports some evil Unix
 * functions to the Windows (XP or above, or Mobile) platform. The
 * evilness is so huge that the most of the functions are not POSIX or
 * BSD compliant.
 *
 * These functions are intended to be used in the Enlightenment
 * Foundation Libraries only and can be compiled only on Windows,
 * using MSYS/MinGW on Windows, and cross-compilation on Unix. This
 * library is minimal in the sense that only the functions needed to
 * compile the EFL are available. The purpose of this library is NOT
 * to have a full POSIX emulation et it is NOT a replacement of
 * cygwin. To compare the size of the DLL themselves, Evil is around
 * 33 KB and cygwin DLL is around 800 KB.
 *
 * @section evil_main_ack Acknowledgments
 *
 * This library has receive some from people interested in the EFL or
 * not. Among them, evil thanks to Lars Munch, Raoul Hecky, Nicolas
 * Aguirre, Tor Lillqvist, Lance Fetters, Vincent Richomme, Paul
 * Vixie, Daniel Stenberg, who helped the author of the library in
 * different fields (code and tests).
 *
 * @section evil_main_compiling How to compile
 *
 * Evil is a library your application links to. The procedure for
 * this is very simple. You simply have to compile your application
 * with the appropriate compiler flags that the @p pkg-config script
 * outputs. For example:
 *
 * Compiling C or C++ files into object files:
 *
 * @verbatim
   gcc -c -o main.o main.c `pkg-config --cflags evil`
   @endverbatim
 *
 * Linking object files into a binary executable:
 *
 * @verbatim
   gcc -o my_application main.o `pkg-config --libs evil`
   @endverbatim
 *
 * See @ref pkgconfig
 *
 * @section evil_main_next_steps Next Steps
 *
 * After you understood what Evil is and installed it in your system
 * you should proceed understanding the programming interface.
 *
 * Recommended reading:
 *
 * @li @ref Evil_Mman
 * @li @ref Evil_Unistd_Group
 * @li @ref Evil_Dlfcn
 * @li @ref Evil_Langinfo_Group
 * @li @ref Evil_Locale_Group
 * @li @ref Evil_Pwd_Group
 * @li @ref Evil_Stdio_Group
 * @li @ref Evil_Main_Group
 * @li @ref Evil_Inet_Group
 * @li @ref Evil_Dirent_Group
 * @li @ref Evil_String_Group
 * @li @ref Evil_Stdlib_Group
 * @li @ref Evil_Time_Group
 */

/**
 * @cond LOCAL
 */

#ifdef __cplusplus
extern "C" {
#endif


#ifndef WIN32_LEAN_AND_MEAN
# define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#undef WIN32_LEAN_AND_MEAN

#include <sys/stat.h> /* for mkdir in evil_macro_wrapper */


typedef unsigned long  uid_t;
typedef unsigned long  gid_t;

#ifdef _MSC_VER
# ifdef _WIN64
typedef __int64 pid_t;
# else
typedef int pid_t;
# endif
typedef SSIZE_T ssize_t;
typedef unsigned short mode_t;
# define strdup(str) _strdup(str)
#endif


#include "evil_macro.h"
#include "evil_dlfcn.h"
#include "evil_fcntl.h"
#include "evil_langinfo.h"
#include "evil_locale.h"
#include "evil_main.h"
#include "evil_stdlib.h"
#include "evil_stdio.h"
#include "evil_string.h"
#include "evil_time.h"
#include "evil_unistd.h"
#include "evil_util.h"

#ifndef S_ISDIR
# define S_ISDIR(m) (((m) & _S_IFMT) == _S_IFDIR)
#endif

#ifndef S_ISREG
# define S_ISREG(m) (((m) & _S_IFMT) == _S_IFREG)
#endif

#define S_ISLNK(m) 0

#define S_IRUSR _S_IRUSR
#ifndef S_IRGRP
# define S_IRGRP S_IRUSR
#endif
#ifndef S_IROTH
# define S_IROTH S_IRUSR
#endif

#define S_IWUSR _S_IWUSR
#ifndef S_IWGRP
# define S_IWGRP S_IWUSR
#endif
#ifndef S_IWOTH
# define S_IWOTH S_IWUSR
#endif

#define S_IXUSR _S_IXUSR
#ifndef S_IXGRP
# define S_IXGRP S_IXUSR
#endif
#ifndef S_IXOTH
# define S_IXOTH S_IXUSR
#endif

#define _S_IRWXU (_S_IREAD | _S_IWRITE | _S_IEXEC)
#ifndef S_IRWXG
# define S_IRWXG _S_IRWXU
#endif
#ifndef S_IRWXO
# define S_IRWXO _S_IRWXU
#endif

#define _S_IXUSR _S_IEXEC
#define _S_IWUSR _S_IWRITE
#define _S_IRUSR _S_IREAD

#define sigsetjmp(Env, Save) setjmp(Env)

#include "evil_macro_wrapper.h"
#include "evil_macro_pop.h"

#ifdef __cplusplus
}
#endif

/**
 * @endcond
 */

#endif /* __EVIL_H__ */
