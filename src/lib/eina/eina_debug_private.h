#ifndef EINA_DEBUG_PRIVATE_H_
# define EINA_DEBUG_PRIVATE_H_

# include "eina_config.h"
# include "eina_lock.h"
# include "eina_thread.h"

#define LOCAL_SERVER_PATH ".edebug"
#define LOCAL_SERVER_NAME "efl_debug"
#define LOCAL_SERVER_PORT 0

#define REMOTE_SERVER_PORT 6666

#ifdef DEBUGON
# define e_debug(fmt, args...) fprintf(stderr, "%d:"__FILE__":%s/%d : " fmt "\n", getpid(), __FUNCTION__, __LINE__, ##args)
# define e_debug_begin(fmt, args...) fprintf(stderr, "%d:"__FILE__":%s/%d : " fmt "", getpid(), __FUNCTION__, __LINE__, ##args)
# define e_debug_continue(fmt, args...) fprintf(stderr, fmt, ##args)
# define e_debug_end() fprintf(stderr, "\n")
#else
# define e_debug(x...) do { } while (0)
# define e_debug_begin(x...) do { } while (0)
# define e_debug_continue(x...) do { } while (0)
# define e_debug_end(x...) do { } while (0)
#endif

/* Max packet size */
#define EINA_DEBUG_MAX_PACKET_SIZE 10000000

typedef struct _Eina_Debug_Session Eina_Debug_Session;

typedef struct _Eina_Debug_Thread Eina_Debug_Thread;

struct _Eina_Debug_Thread
{
   Eina_Thread thread;

   Eina_Debug_Session *cmd_session;
   void *cmd_buffer;
   int cmd_result;

#if defined(__clockid_t_defined)
   struct timespec clok;
#endif
   int thread_id;
   int val;
};

extern Eina_Spinlock                _eina_debug_lock;
extern Eina_Spinlock                _eina_debug_thread_lock;
extern Eina_Thread                    _eina_debug_thread_mainloop;
extern Eina_Debug_Thread           *_eina_debug_thread_active;
extern int                          _eina_debug_thread_active_num;

/* TEMP: should be private to debug thread module */
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

#define EINA_MAX_BT 256
#ifdef HAVE_BACKTRACE
#define EINA_BT(file) \
   do { \
      void *bt[EINA_MAX_BT]; \
      int btlen = backtrace((void **)bt, EINA_MAX_BT); \
      _eina_debug_dump_fhandle_bt(file, bt, btlen); \
   } while (0)
#else
#define EINA_BT(file) do { } while (0)
#endif


Eina_Bool _eina_debug_cpu_init(void);
Eina_Bool _eina_debug_cpu_shutdown(void);

Eina_Bool _eina_debug_bt_init(void);
Eina_Bool _eina_debug_bt_shutdown(void);

Eina_Bool _eina_debug_timer_init(void);
Eina_Bool _eina_debug_timer_shutdown(void);
#endif

