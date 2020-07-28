#ifndef __EVIL_MAIN_H__
#define __EVIL_MAIN_H__

/**
 * @page evil_main Evil
 * @author Vincent Torri
 * @date 2008 (created)
 *
 * @section evil_toc Table of Contents
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
 * These functions are intended to be used inside the Enlightenment Foundation
 * Libraries as private library and can be compiled only on Windows,
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
 * @li @ref Evil_Locale_Group
 * @li @ref Evil_Stdio_Group
 * @li @ref Evil_Main_Group
 * @li @ref Evil_String_Group
 * @li @ref Evil_Stdlib_Group
 * @li @ref Evil_Time_Group
 */

/**
 * @file evil_main.h
 * @brief The file that provides functions to initialize and shut down Evil.
 * @defgroup Evil_Main_Group Main
 * @ingroup Evil
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
 * using evil_time_get() or pipe(). It returns  0 on failure, otherwise it
 * returns the number of times it has already been called.
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
