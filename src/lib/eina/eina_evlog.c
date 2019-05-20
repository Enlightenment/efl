/* EINA - EFL data type library
 * Copyright (C) 2015 Carsten Haitzler
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

#ifdef HAVE_VALGRIND
# include <valgrind.h>
# include <memcheck.h>
#endif

#include "Eina.h"
#include "eina_evlog.h"
#include "eina_debug.h"

#if defined(__APPLE__) && defined(__MACH__)
# include <mach/mach_time.h>
#endif

#include <time.h>
#include <unistd.h>

# ifdef HAVE_MMAP
#  include <sys/mman.h>
# endif

#if __BYTE_ORDER == __LITTLE_ENDIAN
#define SWAP_64(x) x
#define SWAP_32(x) x
#define SWAP_16(x) x
#define SWAP_DBL(x) x
#else
#define SWAP_64(x) eina_swap64(x)
#define SWAP_32(x) eina_swap32(x)
#define SWAP_16(x) eina_swap16(x)
#define SWAP_DBL(x) SWAP_64(x)
#endif

# define EVLOG_BUF_SIZE (4 * (1024 * 1024))

static Eina_Spinlock   _evlog_lock;
static int             _evlog_go = 0;

static Eina_Evlog_Buf *buf; // current event log we are writing events to
static Eina_Evlog_Buf  buffers[2]; // double-buffer our event log buffers

#if defined (HAVE_CLOCK_GETTIME) || defined (EXOTIC_PROVIDE_CLOCK_GETTIME)
static clockid_t _eina_evlog_time_clock_id = -1;
#elif defined(__APPLE__) && defined(__MACH__)
static double _eina_evlog_time_clock_conversion = 1e-9;
#endif

static int _evlog_get_opcode = EINA_DEBUG_OPCODE_INVALID;

static inline double
get_time(void)
{
#if defined (HAVE_CLOCK_GETTIME) || defined (EXOTIC_PROVIDE_CLOCK_GETTIME)
   struct timespec t;

   if (EINA_UNLIKELY(clock_gettime(_eina_evlog_time_clock_id, &t)))
     {
        struct timeval timev;
        gettimeofday(&timev, NULL);
        return (double)timev.tv_sec + (((double)timev.tv_usec) / 1000000.0);
     }
   return (double)t.tv_sec + (((double)t.tv_nsec) / 1000000000.0);
#elif defined(_WIN32)
   return evil_time_get();
#elif defined(__APPLE__) && defined(__MACH__)
   return _eina_evlog_time_clock_conversion * (double)mach_absolute_time();
#else
   struct timeval timev;
   gettimeofday(&timev, NULL);
   return (double)timev.tv_sec + (((double)timev.tv_usec) / 1000000.0);
#endif
}

static void
alloc_buf(Eina_Evlog_Buf *b, unsigned int size)
{
   if (b->buf) return;
   b->size = size;
   b->top = 0;
#ifdef HAVE_MMAP
# ifdef HAVE_VALGRIND
   if (RUNNING_ON_VALGRIND) b->buf = malloc(size);
   else
# endif
     {
        b->buf = mmap(NULL, size, PROT_READ | PROT_WRITE,
                      MAP_PRIVATE | MAP_ANON, -1, 0);
        if (b->buf == MAP_FAILED) b->buf = NULL;
     }
#else
   b->buf = malloc(size);
#endif
   b->overflow = 0;
}

static void
free_buf(Eina_Evlog_Buf *b)
{
   if (!b->buf) return;
#ifdef HAVE_MMAP
# ifdef HAVE_VALGRIND
   if (RUNNING_ON_VALGRIND) free(b->buf);
   else
# endif
   munmap(b->buf, b->size);
#else
   free(b->buf);
#endif
   b->buf = NULL;
   b->size = 0;
   b->top = 0;
}

static inline void *
push_buf(Eina_Evlog_Buf *b, unsigned int size)
{
   void *ptr;

   if (b->size < size) abort();
   if ((b->top + size) > (b->size))
     {
        b->overflow++;
        b->top = 0;
     }
   ptr = (b->buf + b->top);
   b->top += size;
   return ptr;
}

EAPI void
eina_evlog(const char *event, void *obj, double srctime, const char *detail)
{
   Eina_Evlog_Item *item;
   char *strings;
   double now;
   int size;
   unsigned short detail_offset, event_size;

   if (!_evlog_go) return;
   now                 = get_time();
   event_size          = strlen(event) + 1;
   size                = sizeof(Eina_Evlog_Item) + event_size;
   detail_offset       = 0;
   if (detail)
     {
        detail_offset  = size;
        size          += strlen(detail) + 1;
     }
   size                = sizeof(double) * ((size + sizeof(double) - 1)
                                           / sizeof(double));
   eina_spinlock_take(&_evlog_lock);
   strings             = push_buf(buf, size);
   item                = (Eina_Evlog_Item *)strings;
   item->tim           = SWAP_DBL(now);
   item->srctim        = SWAP_DBL(srctime);
   item->thread        = SWAP_64((unsigned long long)(uintptr_t)pthread_self());
   item->obj           = SWAP_64((unsigned long long)(uintptr_t)obj);
   item->event_offset  = SWAP_16(sizeof(Eina_Evlog_Item));
   item->detail_offset = SWAP_16(detail_offset);
   item->event_next    = SWAP_16(size);
   strcpy(strings + sizeof(Eina_Evlog_Item), event);
   if (detail_offset > 0) strcpy(strings + detail_offset, detail);
   eina_spinlock_release(&_evlog_lock);
}

EAPI Eina_Evlog_Buf *
eina_evlog_steal(void)
{
   Eina_Evlog_Buf *stolen = NULL;

   eina_spinlock_take(&_evlog_lock);
   if (buf == &(buffers[0]))
     {
        buf = &(buffers[1]);
        buf->top = 0;
        buf->overflow = 0;
        stolen = &(buffers[0]);
     }
   else
     {
        buf = &(buffers[0]);
        buf->top = 0;
        buf->overflow = 0;
        stolen = &(buffers[1]);
     }
   eina_spinlock_release(&_evlog_lock);
   return stolen;
}

EAPI void
eina_evlog_start(void)
{
   eina_spinlock_take(&_evlog_lock);
   _evlog_go++;
   if (_evlog_go == 1)
     {
        // alloc 2 buffers for spinning around in
        alloc_buf(&(buffers[0]), EVLOG_BUF_SIZE);
        alloc_buf(&(buffers[1]), EVLOG_BUF_SIZE);
     }
   eina_spinlock_release(&_evlog_lock);
}

EAPI void
eina_evlog_stop(void)
{
   eina_spinlock_take(&_evlog_lock);
   _evlog_go--;
   if (_evlog_go == 0)
     {
        free_buf(&(buffers[0]));
        free_buf(&(buffers[1]));
     }
   eina_spinlock_release(&_evlog_lock);
}

// get evlog
static Eina_Bool
_get_cb(Eina_Debug_Session *session EINA_UNUSED, int cid EINA_UNUSED, void *buffer EINA_UNUSED, int size EINA_UNUSED)
{
   Eina_Evlog_Buf *evlog = eina_evlog_steal();
   int resp_size = 0;
   unsigned char *resp_buf = NULL;

   if ((evlog) && (evlog->buf))
     {
        int ovf = SWAP_32(evlog->overflow);
        resp_size = evlog->top + sizeof(evlog->overflow);
        resp_buf = malloc(resp_size);
        memcpy(resp_buf, &ovf, sizeof(ovf));
        memcpy(resp_buf + sizeof(evlog->overflow), evlog->buf, evlog->top);
     }
   printf("send evlog size %d\n", resp_size);
   eina_debug_session_send(session, cid, _evlog_get_opcode, resp_buf, resp_size);
   free(resp_buf);

   return EINA_TRUE;
}

// enable evlog
static Eina_Bool
_start_cb(Eina_Debug_Session *session EINA_UNUSED, int cid EINA_UNUSED, void *buffer EINA_UNUSED, int size EINA_UNUSED)
{
   eina_evlog_start();
   return EINA_TRUE;
}

// stop evlog
static Eina_Bool
_stop_cb(Eina_Debug_Session *session EINA_UNUSED, int cid EINA_UNUSED, void *buffer EINA_UNUSED, int size EINA_UNUSED)
{
   eina_evlog_stop();
   return EINA_TRUE;
}

EINA_DEBUG_OPCODES_ARRAY_DEFINE(_EINA_DEBUG_EVLOG_OPS,
      {"EvLog/on", NULL, &_start_cb},
      {"EvLog/off", NULL, &_stop_cb},
      {"EvLog/get", &_evlog_get_opcode, &_get_cb},
      {NULL, NULL, NULL}
);

Eina_Bool
eina_evlog_init(void)
{
   eina_spinlock_new(&_evlog_lock);
   buf = &(buffers[0]);
#if defined (HAVE_CLOCK_GETTIME) || defined (EXOTIC_PROVIDE_CLOCK_GETTIME)
     {
        struct timespec t;

        if (!clock_gettime(CLOCK_MONOTONIC, &t))
          _eina_evlog_time_clock_id = CLOCK_MONOTONIC;
        else
          _eina_evlog_time_clock_id = CLOCK_REALTIME;
     }
#endif
   eina_evlog("+eina_init", NULL, 0.0, NULL);
   eina_debug_opcodes_register(NULL, _EINA_DEBUG_EVLOG_OPS(), NULL, NULL);
   return EINA_TRUE;
}

Eina_Bool
eina_evlog_shutdown(void)
{
   // yes - we don't free tyhe evlog buffers. they may be in used by debug th
   eina_spinlock_free(&_evlog_lock);
   return EINA_TRUE;
}
