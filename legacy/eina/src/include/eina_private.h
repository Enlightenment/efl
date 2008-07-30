#ifndef EINA_PRIVATE_H_
#define EINA_PRIVATE_H_

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sys/time.h>
#include <sys/types.h>
#include <stddef.h>
#include <unistd.h>
#include <signal.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <limits.h>
#include <dirent.h>
#include <string.h>

#define DEBUG
#include <assert.h>

#ifndef PATH_MAX
# define PATH_MAX 4096
#endif

#ifndef MIN
# define MIN(x, y) (((x) > (y)) ? (y) : (x))
#endif

#ifndef MAX
# define MAX(x, y) (((x) > (y)) ? (x) : (y))
#endif

#ifndef ABS
# define ABS(x) ((x) < 0 ? -(x) : (x))
#endif

#ifndef CLAMP
# define CLAMP(x, min, max) (((x) > (max)) ? (max) : (((x) < (min)) ? (min) : (x)))
#endif

#define READBUFSIZ 65536

#define EINA_MAGIC_NONE            0x1234fedc
#define EINA_MAGIC_EXE             0xf7e812f5
#define EINA_MAGIC_TIMER           0xf7d713f4
#define EINA_MAGIC_IDLER           0xf7c614f3
#define EINA_MAGIC_IDLE_ENTERER    0xf7b515f2
#define EINA_MAGIC_IDLE_EXITER     0xf7601afd
#define EINA_MAGIC_FD_HANDLER      0xf7a416f1
#define EINA_MAGIC_EVENT_HANDLER   0xf79317f0
#define EINA_MAGIC_EVENT_FILTER    0xf78218ff
#define EINA_MAGIC_EVENT           0xf77119fe
#define EINA_MAGIC_ANIMATOR        0xf7643ea5

#define EINA_MAGIC                 Eina_Magic  __magic

#define EINA_MAGIC_SET(d, m)       (d)->__magic = (m)
#define EINA_MAGIC_CHECK(d, m)     ((d) && ((d)->__magic == (m)))
#define EINA_MAGIC_FAIL(d, m, fn)  _eina_magic_fail((d), (d) ? (d)->__magic : 0, (m), (fn));

/* undef the following, we want out version */
#undef FREE
#define FREE(ptr) free(ptr); ptr = NULL;

#undef IF_FREE
#define IF_FREE(ptr) if (ptr) free(ptr); ptr = NULL;

#undef IF_FN_DEL
#define IF_FN_DEL(_fn, ptr) if (ptr) { _fn(ptr); ptr = NULL; }

inline void eina_print_warning(const char *function, const char *sparam);

/* convenience macros for checking pointer parameters for non-NULL */
#undef CHECK_PARAM_POINTER_RETURN
#define CHECK_PARAM_POINTER_RETURN(sparam, param, ret) \
     if (!(param)) \
	 { \
	    eina_print_warning(__FUNCTION__, sparam); \
	    return ret; \
	 }

#undef CHECK_PARAM_POINTER
#define CHECK_PARAM_POINTER(sparam, param) \
     if (!(param)) \
	 { \
	    eina_print_warning(__FUNCTION__, sparam); \
	    return; \
	 }

typedef unsigned int              Eina_Magic;

typedef struct _Eina_List2       Eina_List2;
typedef struct _Eina_List2_Data  Eina_List2_Data;

struct _Eina_List2
{
   Eina_List2  *next, *prev;
   Eina_List2  *last;
};

struct _Eina_List2_Data
{
   Eina_List2   __list_data;
   void *data;
};

#ifndef EINA_H

#endif

EAPI void          _eina_magic_fail(void *d, Eina_Magic m, Eina_Magic req_m, const char *fname);

EAPI void         *_eina_list2_append           (void *in_list, void *in_item);
EAPI void         *_eina_list2_prepend          (void *in_list, void *in_item);
EAPI void         *_eina_list2_append_relative  (void *in_list, void *in_item, void *in_relative);
EAPI void         *_eina_list2_prepend_relative (void *in_list, void *in_item, void *in_relative);
EAPI void         *_eina_list2_remove           (void *in_list, void *in_item);
EAPI void         *_eina_list2_find             (void *in_list, void *in_item);

void          _eina_fps_debug_init(void);
void          _eina_fps_debug_shutdown(void);
void          _eina_fps_debug_runtime_add(double t);



extern int    _eina_fps_debug;


/* old code finish */
/* mp */
typedef struct _Eina_Mempool_Backend
{
	void *(*init)(void *buffer, unsigned int size, const char *options, va_list args);
	void (*free)(void *data, void *element);
	void *(*alloc)(void *data, unsigned int size);
	void *(*realloc)(void *data, void *element, unsigned int size);
	void (*garbage_collect)(void);
	void (*statistics)(void);
	void (*shutdown)(void *data);
} Eina_Mempool_Backend;


#endif /* EINA_PRIVATE_H_ */
