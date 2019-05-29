#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#define EINA_SLSTR_INTERNAL

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Eina.h"
#include "eina_private.h"

// ========================================================================= //

static int _slstr_init = 0;
static Eina_FreeQ *_slstr_main_fq = NULL;
static Eina_TLS _slstr_tls = 0;

// ========================================================================= //

#if 0
// 2 extension ideas here: slices for short-lived raw data buffers
EAPI Eina_Rw_Slice eina_slslice_new(size_t length); // alloc
EAPI Eina_Rw_Slice eina_slslice_copy(Eina_Slice slice); // copies
EAPI Eina_Rw_Slice eina_slslice_free(Eina_Rw_Slice slice); // steals
#endif

static void
_slstr_tls_free_cb(void *ptr)
{
   Eina_FreeQ *fq = ptr;

   eina_freeq_free(fq);
}

Eina_Bool
eina_slstr_init(void)
{
   if (_slstr_init++) return EINA_TRUE;

   _slstr_main_fq = eina_freeq_new(EINA_FREEQ_POSTPONED);
   if (!_slstr_main_fq) goto fail;
   if (!eina_tls_cb_new(&_slstr_tls, _slstr_tls_free_cb)) goto fail_tls;

   return EINA_TRUE;

fail_tls:
   eina_tls_free(_slstr_tls);
   _slstr_tls = 0;
fail:
   eina_freeq_free(_slstr_main_fq);
   _slstr_main_fq = NULL;
   return EINA_FALSE;
}

Eina_Bool
eina_slstr_shutdown(void)
{
   if (_slstr_init == 0) return EINA_FALSE;
   if (--_slstr_init) return EINA_TRUE;

   eina_freeq_free(_slstr_main_fq);
   eina_tls_free(_slstr_tls);
   _slstr_main_fq = NULL;
   _slstr_tls = 0;

   return EINA_TRUE;
}

static inline Eina_FreeQ *
_slstr_freeq_get(Eina_Bool nocreate)
{
   if (eina_main_loop_is())
     return _slstr_main_fq;
   else
     {
        Eina_FreeQ *fq;

        fq = eina_tls_get(_slstr_tls);
        if (!nocreate && EINA_UNLIKELY(!fq))
          {
             fq = eina_freeq_new(EINA_FREEQ_POSTPONED);
             eina_tls_set(_slstr_tls, fq);
          }
        return fq;
     }
}

EAPI Eina_Slstr *
eina_slstr_copy_new(const char *string)
{
   Eina_FreeQ *fq;
   char *copy;
   size_t len = 0;

   if (!string) return NULL;

   fq = _slstr_freeq_get(EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(fq, NULL);

   copy = eina_strdup(string);
   if (!copy) return NULL;
#ifdef DEBUG
   len = strlen(string) + 1;
#endif
   eina_freeq_ptr_add(fq, copy, free, len);
   return copy;
}

EAPI Eina_Slstr *
eina_slstr_copy_new_length(const char *string, size_t len)
{
   Eina_FreeQ *fq;
   char *copy;

   if (!string) return NULL;

   fq = _slstr_freeq_get(EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(fq, NULL);

   copy = eina_strndup(string, len);
   if (!copy) return NULL;
   eina_freeq_ptr_add(fq, copy, free, len);
   return copy;
}

EAPI Eina_Slstr *
eina_slstr_steal_new(char *string)
{
   Eina_FreeQ *fq;
   size_t len = 0;

   if (!string) return NULL;

   fq = _slstr_freeq_get(EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(fq, NULL);

#ifdef DEBUG
   len = strlen(string) + 1;
#endif
   eina_freeq_ptr_add(fq, string, free, len);
   return string;
}

EAPI Eina_Slstr *
eina_slstr_stringshare_new(Eina_Stringshare *string)
{
   Eina_FreeQ *fq;
   size_t len = 0;

   if (!string) return NULL;

   fq = _slstr_freeq_get(EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(fq, NULL);

#ifdef DEBUG
   len = eina_stringshare_strlen(string) + 1;
#endif
   eina_freeq_ptr_add(fq, (void *) string, EINA_FREE_CB(eina_stringshare_del), len);
   return string;
}

EAPI Eina_Slstr *
eina_slstr_tmpstr_new(Eina_Tmpstr *string)
{
   Eina_FreeQ *fq;
   size_t len = 0;

   if (!string) return NULL;

   fq = _slstr_freeq_get(EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(fq, NULL);

#ifdef DEBUG
   len = eina_tmpstr_strlen(string) + 1;
#endif
   eina_freeq_ptr_add(fq, (void *) string, EINA_FREE_CB(eina_tmpstr_del), len);
   return string;
}

EAPI Eina_Slstr *
eina_slstr_strbuf_new(Eina_Strbuf *string)
{
   Eina_FreeQ *fq;
   size_t len = 0;
   char *str;

   if (!string) return NULL;

   fq = _slstr_freeq_get(EINA_FALSE);
   EINA_SAFETY_ON_NULL_GOTO(fq, error);

   str = eina_strbuf_release(string);
#ifdef DEBUG
   len = eina_strbuf_length_get(string) + 1;
#endif
   eina_freeq_ptr_add(fq, str, free, len);
   return str;

error:
   eina_strbuf_free(string);
   return NULL;
}

EAPI Eina_Slstr *
eina_slstr_vasprintf_new(const char *fmt, va_list args)
{
   Eina_FreeQ *fq;
   size_t len = 0;
   char *str;
   int r;

   fq = _slstr_freeq_get(EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(fq, NULL);

   r = vasprintf(&str, fmt, args);
   if (r == -1) return NULL;

#ifdef DEBUG
   len = r + 1;
#endif
   eina_freeq_ptr_add(fq, str, free, len);
   return str;
}

EAPI void
eina_slstr_local_clear(void)
{
   Eina_FreeQ *fq;

   fq = _slstr_freeq_get(EINA_TRUE);
   if (!fq) return;

   eina_freeq_clear(fq);
}
