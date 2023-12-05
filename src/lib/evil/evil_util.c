#ifdef HAVE_CONFIG_H
# include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <wchar.h>

#include "evil_private.h"

DWORD _evil_tls_index;

/* static void _evil_error_display(const char *fct, LONG res); */
static void _evil_last_error_display(const char *fct);

EVIL_API wchar_t *
evil_char_to_wchar(const char *text)
{
   wchar_t *wtext;
   int      wsize;

   if (!text)
     return NULL;

   wsize = MultiByteToWideChar(CP_ACP, 0, text, (int)strlen(text) + 1, NULL, 0);
   if ((wsize == 0) ||
       (wsize > (int)(ULONG_MAX / sizeof(wchar_t))))
     {
        if (wsize == 0)
          _evil_last_error_display(__func__);
        return NULL;
     }

   wtext = malloc(wsize * sizeof(wchar_t));
   if (wtext)
     if (!MultiByteToWideChar(CP_ACP, 0, text, (int)strlen(text) + 1, wtext, wsize))
     {
        _evil_last_error_display(__func__);
        return NULL;
     }

   return wtext;
}

EVIL_API char *
evil_wchar_to_char(const wchar_t *text)
{
   char  *atext;
   int    asize;

   if (!text)
     return NULL;

   asize = WideCharToMultiByte(CP_ACP, 0, text, -1, NULL, 0, NULL, NULL);
   if (asize == 0)
     {
        _evil_last_error_display(__func__);
        return NULL;
     }

   atext = (char*)malloc(asize * sizeof(char));
   if (!atext)
     return NULL;

   asize = WideCharToMultiByte(CP_ACP, 0, text, -1, atext, asize, NULL, NULL);
   if (asize == 0)
     {
        _evil_last_error_display(__func__);
        return NULL;
     }

   return atext;
}

EVIL_API char *
evil_utf16_to_utf8(const wchar_t *text16)
{
   char  *text8;
   DWORD  flag = 0;
   int    size8;

   if (!text16)
     return NULL;

   flag = WC_ERR_INVALID_CHARS;

   size8 = WideCharToMultiByte(CP_UTF8, flag, text16, -1, NULL, 0, NULL, NULL);
   if (size8 == 0)
     {
        _evil_last_error_display(__func__);
        return NULL;
     }

   text8 = (char*)malloc(size8 * sizeof(char));
   if (!text8)
     return NULL;

   size8 = WideCharToMultiByte(CP_UTF8, flag, text16, -1, text8, size8, NULL, NULL);
   if (size8 == 0)
     {
        _evil_last_error_display(__func__);
        return NULL;
     }

   return text8;
}

EVIL_API wchar_t *
evil_utf8_to_utf16(const char *text)
{
   wchar_t *text16;
   DWORD flag = MB_ERR_INVALID_CHARS;
   int size16;

   if (!text)
     return NULL;

   size16 = MultiByteToWideChar(CP_UTF8, flag, text, -1, NULL, 0);
   if (size16 == 0)
     {
        _evil_last_error_display(__func__);
        return NULL;
     }

   text16 = malloc(size16 * sizeof(wchar_t));
   if (text16)
     if (!MultiByteToWideChar(CP_UTF8, flag, text, -1, text16, size16))
     {
        _evil_last_error_display(__func__);
        return NULL;
     }

   return text16;
}

EVIL_API const char *
evil_format_message(long err)
{
   char *buf;
   LPTSTR msg;
   char  *str;

   if (!FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                      NULL,
                      err,
                      0, /* Default language */
                      (LPTSTR)&msg,
                      0,
                      NULL))
     {
        buf = (char *)TlsGetValue(_evil_tls_index);
        snprintf(buf, 4096,
                 "FormatMessage failed with error %ld\n", GetLastError());
        return (const char *)buf;
     }

#ifdef UNICODE
   str = evil_wchar_to_char(msg);
#else
   str = msg;
#endif /* UNICODE */

   buf = (char *)TlsGetValue(_evil_tls_index);
   snprintf(buf, 4096, "(%5ld) %s", err, str);

#ifdef UNICODE
   free(str);
#endif /* UNICODE */

   LocalFree(msg);

   return (const char *)buf;
}

EVIL_API const char *
evil_last_error_get(void)
{
   DWORD  err;

   err = GetLastError();
   return evil_format_message(err);
}

static void
_evil_last_error_display(const char *fct)
{
   fprintf(stderr, "[Evil] [%s] ERROR: %s\n", fct, evil_last_error_get());
}
