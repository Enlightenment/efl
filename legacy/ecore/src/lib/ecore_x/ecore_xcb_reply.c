/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#include <stdlib.h>

#include <Ecore_Data.h>


/* 
 * FIXME:
 * - in ecore_xcb_cookie_cache, should provide better error management
 *   when memory allocation fails
 * - Use an array instead of a list
 * - Is ecore_xcb_reply_free really needed ?
 *   _ecore_xcb_reply_cache frees the current reply and
 *   _ecore_x_reply_shutdown frees the last reply to free.
 *   I keep it in case it is need for memory improvements,
 *   but its code is commented.
 */

static Ecore_List *_ecore_xcb_cookies = NULL;
static void       *_ecore_xcb_reply = NULL;

typedef struct _Ecore_Xcb_Data Ecore_Xcb_Data;

struct _Ecore_Xcb_Data
{
  unsigned int         cookie;
};


int
_ecore_x_reply_init ()
{
   _ecore_xcb_cookies = ecore_list_new();
   if (!_ecore_xcb_cookies)
     return 0;

   if (!ecore_list_init(_ecore_xcb_cookies))
     {
        ecore_list_destroy(_ecore_xcb_cookies);
        return 0;
     }

   if (!ecore_list_free_cb_set(_ecore_xcb_cookies, ECORE_FREE_CB(free)))
     {
        ecore_list_destroy(_ecore_xcb_cookies);
        return 0;
     }

   return 1;
}

void
_ecore_x_reply_shutdown ()
{
  if (_ecore_xcb_reply)
    free(_ecore_xcb_reply);

   if (!_ecore_xcb_cookies)
     return;

   ecore_list_destroy(_ecore_xcb_cookies);
}

void
_ecore_xcb_cookie_cache (unsigned int cookie)
{
   Ecore_Xcb_Data *data;

   if (!_ecore_xcb_cookies)
     return;

   data = (Ecore_Xcb_Data *)malloc(sizeof(Ecore_Xcb_Data));
   if (!data)
     return;

   data->cookie = cookie;

   if (!ecore_list_append(_ecore_xcb_cookies, data))
     {
        free(data);
        return;
     }
}

unsigned int
_ecore_xcb_cookie_get (void)
{
   Ecore_Xcb_Data *data;
   unsigned int    cookie;

   if (!_ecore_xcb_cookies)
     return 0;

   data = ecore_list_first_remove(_ecore_xcb_cookies);
   if (data)
     {
        cookie = data->cookie;
        free(data);

        return cookie;
     }

   return 0;
}

void
_ecore_xcb_reply_cache (void *reply)
{
   if (_ecore_xcb_reply)
     free(_ecore_xcb_reply);
   _ecore_xcb_reply = reply;
}

void *
_ecore_xcb_reply_get (void)
{
   return _ecore_xcb_reply;
}

EAPI void
ecore_xcb_reply_free()
{
/*   if (_ecore_xcb_reply) */
/*     { */
/*        free(_ecore_xcb_reply); */
/*        _ecore_xcb_reply = NULL; */
/*     } */
}
