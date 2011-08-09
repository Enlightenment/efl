/* EINA - EFL data type library
 * Copyright (C) 2011 Carsten Haitzler
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library;
 * if not, see <http://www.gnu.org/licenses/>.
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#ifndef _WIN32

#ifdef STDC_HEADERS
# include <stdlib.h>
# include <stddef.h>
#else
# ifdef HAVE_STDLIB_H
#  include <stdlib.h>
# endif
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <limits.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>
# include <unistd.h>

#endif

#include "eina_config.h"
#include "eina_private.h"
#include "eina_log.h"
#include "eina_mmap.h"

/*============================================================================*
 *                                 Local                                      *
 *============================================================================*/

static Eina_Bool mmap_safe = EINA_FALSE;
#ifndef _WIN32
static int _eina_mmap_log_dom = -1;
static int _eina_mmap_zero_fd = -1;
static long _eina_mmap_pagesize = -1;

#ifdef ERR
#undef ERR
#endif
#define ERR(...) EINA_LOG_DOM_ERR(_eina_mmap_log_dom, __VA_ARGS__)

#ifdef DBG
#undef DBG
#endif
#define DBG(...) EINA_LOG_DOM_DBG(_eina_mmap_log_dom, __VA_ARGS__)

static void
_eina_mmap_safe_sigbus(int sig __UNUSED__,
                       siginfo_t *siginfo,
                       void *ptr __UNUSED__)
{
   unsigned char *addr = (unsigned char *)(siginfo->si_addr);
   int perrno;

   /* save previous errno */
   perrno = errno;
   /* if problems was an unaligned access - complain accordingly and abort */
   if (siginfo->si_code == BUS_ADRALN)
     {
        ERR("Unaligned memory access. SIGBUS!!!");
        errno = perrno; 
        abort();
     }
   /* send this to stderr - not eina_log. Specifically want this on stderr */
   fprintf(stderr,
           "EINA: Data at address 0x%lx is invalid. Replacing with zero page.\n",
           (unsigned long)addr);
   /* align address to the lower page boundary */
   addr = (unsigned char *)((long)addr & (~(_eina_mmap_pagesize - 1)));
   /* mmap a pzge of zero's from /dev/zero in there */
   if (mmap(addr, _eina_mmap_pagesize,
            PROT_READ | PROT_WRITE | PROT_EXEC,
            MAP_PRIVATE | MAP_FIXED, 
            _eina_mmap_zero_fd, 0) == MAP_FAILED)
     {
        /* mmap of /dev/zero failed :( */
        perror("mmap");
        ERR("Failed to mmap() /dev/zero in place of page. SIGBUS!!!");
        errno = perrno; 
        abort();
     }
   /* restore previous errno */
   errno = perrno; 
}
#endif

/*============================================================================*
 *                                   API                                      *
 *============================================================================*/

EAPI Eina_Bool
eina_mmap_safety_enabled_set(Eina_Bool enabled)
{
#ifdef _WIN32
   return EINA_FALSE;
#else
   if (_eina_mmap_log_dom < 0)
     {
        _eina_mmap_log_dom = eina_log_domain_register("eina_mmap",
                                                         EINA_LOG_COLOR_DEFAULT);
        if (_eina_mmap_log_dom < 0)
          {
             EINA_LOG_ERR("Could not register log domain: eina_mmap");
             return EINA_FALSE;
          }
     }
   
   enabled = !!enabled;
   
   if (mmap_safe == enabled) return mmap_safe;
   if (enabled)
     {
        struct sigaction  sa;

        /* find out system page size the cleanest way we can */
#ifdef _SC_PAGESIZE   
        _eina_mmap_pagesize = sysconf(_SC_PAGESIZE);
        if (_eina_mmap_pagesize <= 0) return EINA_FALSE;
#else
        _eina_mmap_pagesize = 4096;
#endif
        /* no zero page device - open it */
        if (_eina_mmap_zero_fd < 0)
          {
             _eina_mmap_zero_fd = open("/dev/zero", O_RDWR);
             /* if we don;'t have one - fail to set up mmap safety */
             if (_eina_mmap_zero_fd < 0) return EINA_FALSE;
          }
        /* set up signal handler for SIGBUS */
        sa.sa_sigaction = _eina_mmap_safe_sigbus;
        sa.sa_flags = SA_RESTART | SA_SIGINFO;
        sigemptyset(&sa.sa_mask);
        if (sigaction(SIGBUS, &sa, NULL) == 0) return EINA_FALSE;
        /* setup of SIGBUS handler failed, lets close zero page dev and fail */
        close(_eina_mmap_zero_fd);
        _eina_mmap_zero_fd = -1;
        return EINA_FALSE;
     }
   else
     {
        /* reset signal handler to default for SIGBUS */
        signal(SIGBUS, SIG_DFL);
     }
   mmap_safe = enabled;
   return mmap_safe;
#endif   
}

EAPI Eina_Bool
eina_mmap_safety_enabled_get(void)
{
   return mmap_safe;
}
