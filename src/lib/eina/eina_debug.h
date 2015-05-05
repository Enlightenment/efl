#ifndef EINA_DEBUG_H_
# define EINA_DEBUG_H_

# ifdef HAVE_CONFIG_H
#  include "config.h"
# endif

# include <stdio.h>
# include <string.h>
# include <stdlib.h>
# include <unistd.h>
# if defined(HAVE_EXECINFO_H) && defined(HAVE_BACKTRACE) && defined(HAVE_DLADDR) && defined(HAVE_UNWIND)
#  include <execinfo.h>
#  ifndef _GNU_SOURCE
#   define _GNU_SOURCE 1
#  endif
#  include <errno.h>
#  include <stdio.h>
#  include <string.h>
#  include <unistd.h>
#  include <dlfcn.h>
#  include <sys/select.h>
#  include <sys/time.h>
#  include <sys/types.h>
#  include <sys/stat.h>
#  include <pthread.h>
#  include <signal.h>
#  include <time.h>
#  include <sys/types.h>
#  include <sys/stat.h>
#  include <sys/socket.h>
#  include <sys/un.h>
#  include <fcntl.h>
#  include <libunwind.h>

#  include "eina_config.h"
#  include "eina_private.h"
#  include "eina_inlist.h"
#  include "eina_lock.h"
#  include "eina_thread.h"
#  include "eina_convert.h"
#  include "eina_strbuf.h"
#  include "eina_safety_checks.h"
#  include "eina_log.h"
#  include "eina_inline_private.h"

#  define EINA_HAVE_DEBUG 1

#  define EINA_MAX_BT 256

extern Eina_Spinlock  _eina_debug_lock;
extern Eina_Spinlock  _eina_debug_thread_lock;
extern Eina_Semaphore _eina_debug_monitor_return_sem;
extern int            _eina_debug_monitor_service_fd;

void _eina_debug_thread_add(void *th);
void _eina_debug_thread_del(void *th);
void _eina_debug_thread_mainloop_set(void *th);

void *_eina_debug_chunk_push(int size);
void *_eina_debug_chunk_realloc(int size);
char *_eina_debug_chunk_strdup(const char *str);
void *_eina_debug_chunk_tmp_push(int size);
void  _eina_debug_chunk_tmp_reset(void);

const char *_eina_debug_file_get(const char *fname);

void _eina_debug_dump_fhandle_bt(FILE *f, void **bt, int btlen);

void _eina_debug_monitor_thread_start(void);
void _eina_debug_monitor_signal_init(void);
void _eina_debug_monitor_service_connect(void);

void _eina_debug_monitor_service_greet(void);

#  define EINA_BT(file) \
   do { \
      void *bt[EINA_MAX_BT]; \
      int btlen = backtrace((void **)bt, EINA_MAX_BT); \
      _eina_debug_dump_fhandle_bt(file, bt, btlen); \
   } while (0)
# else
#  define EINA_BT(file) do { } while (0)
# endif

#endif
