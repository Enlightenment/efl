#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Elementary.h>
#include <Exactness.h>
#include <exactness_private.h>

typedef struct
{
   Eina_Debug_Session *session;
   int srcid;
   void *buffer;
   unsigned int size;
} _Main_Loop_Info;

#define WRAPPER_TO_XFER_MAIN_LOOP(foo) \
static void \
_intern_main_loop ## foo(void *data) \
{ \
   _Main_Loop_Info *info = data; \
   _main_loop ## foo(info->session, info->srcid, info->buffer, info->size); \
   free(info->buffer); \
   free(info); \
} \
static Eina_Bool \
foo(Eina_Debug_Session *session, int srcid, void *buffer, int size) \
{ \
   _Main_Loop_Info *info = calloc(1, sizeof(*info)); \
   info->session = session; \
   info->srcid = srcid; \
   info->size = size; \
   if (info->size) \
     { \
        info->buffer = malloc(info->size); \
        memcpy(info->buffer, buffer, info->size); \
     } \
   ecore_main_loop_thread_safe_call_async(_intern_main_loop ## foo, info); \
   return EINA_TRUE; \
}

#ifndef WORDS_BIGENDIAN
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

#define EXTRACT_INT(_buf) \
({ \
   int __i; \
   memcpy(&__i, _buf, sizeof(int)); \
   _buf += sizeof(int); \
   SWAP_32(__i); \
})

#define EXTRACT_DOUBLE(_buf) \
({ \
   double __d; \
   memcpy(&__d, _buf, sizeof(double)); \
   _buf += sizeof(double); \
   SWAP_DBL(__d); \
})

#define EXTRACT_STRING(_buf) \
({ \
   char *__s = _buf ? strdup(_buf) : NULL; \
   int __len = (__s ? strlen(__s) : 0) + 1; \
   _buf += __len; \
   __s; \
})

#define STORE_INT(_buf, __i) \
({ \
   int __si = SWAP_32(__i); \
   memcpy(_buf, &__si, sizeof(int)); \
   _buf += sizeof(int); \
})

#define STORE_DOUBLE(_buf, __d) \
{ \
   double __d2 = SWAP_DBL(__d); \
   memcpy(_buf, &__d2, sizeof(double)); \
   _buf += sizeof(double); \
}

#define STORE_STRING(_buf, __s) \
{ \
   int __len = (__s ? strlen(__s) : 0) + 1; \
   if (__s) memcpy(_buf, __s, __len); \
   else *_buf = '\0'; \
   _buf += __len; \
}

Evas *(*_evas_new)(void);

void ex_printf(int verbose, const char *fmt, ...);
int ex_prg_invoke(const char *full_path, int argc, char **argv, Eina_Bool player);
Eina_Stringshare *ex_prg_full_path_guess(const char *prg);
