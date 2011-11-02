#ifdef HAVE_CONFIG_H
# include "config.h"
#endif /* HAVE_CONFIG_H */

#include "Evil.h"
#include "evil_private.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <wchar.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef WIN32_LEAN_AND_MEAN


wchar_t *
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
          _evil_last_error_display(__FUNCTION__);
        return NULL;
     }

   wtext = malloc(wsize * sizeof(wchar_t));
   if (wtext)
     if (!MultiByteToWideChar(CP_ACP, 0, text, (int)strlen(text) + 1, wtext, wsize))
     {
        _evil_last_error_display(__FUNCTION__);
        return NULL;
     }

   return wtext;
}

char *
evil_wchar_to_char(const wchar_t *text)
{
   char  *atext;
   int    asize;

   if (!text)
     return NULL;

   asize = WideCharToMultiByte(CP_ACP, 0, text, -1, NULL, 0, NULL, NULL);
   if (asize == 0)
     {
        _evil_last_error_display(__FUNCTION__);
        return NULL;
     }

   atext = (char*)malloc(asize * sizeof(char));
   if (!atext)
     return NULL;

   asize = WideCharToMultiByte(CP_ACP, 0, text, -1, atext, asize, NULL, NULL);
   if (asize == 0)
     {
        _evil_last_error_display(__FUNCTION__);
        return NULL;
     }

   return atext;
}

char *
evil_utf16_to_utf8(const wchar_t *text16)
{
   char  *text8;
   DWORD  flag = 0;
   int    size8;

   if (!text16)
     return NULL;

#if _WIN32_WINNT >= 0x0600
   flag = WC_ERR_INVALID_CHARS;;
#endif

   size8 = WideCharToMultiByte(CP_UTF8, flag, text16, -1, NULL, 0, NULL, NULL);
   if (size8 == 0)
     {
        _evil_last_error_display(__FUNCTION__);
        return NULL;
     }

   text8 = (char*)malloc(size8 * sizeof(char));
   if (!text8)
     return NULL;

   size8 = WideCharToMultiByte(CP_UTF8, flag, text16, -1, text8, size8, NULL, NULL);
   if (size8 == 0)
     {
        _evil_last_error_display(__FUNCTION__);
        return NULL;
     }

   return text8;
}

char *
evil_format_message(long err)
{
   LPTSTR msg;
   char  *str;
   char  *disp;

   if (!FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                      NULL,
                      err,
                      0, /* Default language */
                      (LPTSTR)&msg,
                      0,
                      NULL))
     {
        char buf[4096];

        snprintf(buf, 4096, "FormatMessage failed with error %ld\n", GetLastError());
        return strdup(buf);
     }

#ifdef UNICODE
   str = evil_wchar_to_char(msg);
#else
   str = msg;
#endif /* UNICODE */

   LocalFree(msg);

   disp = (char *)malloc((strlen(str) + strlen("(00000) ") + 1) * sizeof(char));
   if (!disp)
     return NULL;

   snprintf(disp, strlen(str) + strlen("(00000) ") + 1,
            "(%5ld) %s", err, str);

#ifdef UNICODE
   free(str);
#endif /* UNICODE */

   return disp;
}

void
_evil_error_display(const char *fct, LONG res)
{
   char *error;

   error = evil_format_message(res);
   fprintf(stderr, "[Evil] [%s] ERROR (%ld): %s\n", fct, res, error);
   free(error);
}

char *
evil_last_error_get(void)
{
   DWORD  err;

   err = GetLastError();
   return evil_format_message(err);
}

void
_evil_last_error_display(const char *fct)
{
   char *error;

   error = evil_last_error_get();
   fprintf(stderr, "[Evil] [%s] ERROR: %s\n", fct, error);
   free(error);
}


const char *
evil_tmpdir_get(void)
{
#ifdef _WIN32_WCE
   return "\\temp";
#else
   char *tmpdir;

   tmpdir = getenv("TMP");
   if (!tmpdir) tmpdir = getenv("TEMP");
   if (!tmpdir) tmpdir = getenv("USERPROFILE");
   if (!tmpdir) tmpdir = getenv("WINDIR");
   if (!tmpdir) tmpdir="C:\\";

   return tmpdir;
#endif /* ! _WIN32_WCE */
}

const char *
evil_homedir_get(void)
{
#ifdef _WIN32_WCE
   return "\\my documents";
#else
   char *homedir;

   homedir = getenv("HOME");
   if (!homedir) homedir = getenv("USERPROFILE");
   if (!homedir) homedir = getenv("WINDIR");
   if (!homedir) homedir="C:\\";

   return homedir;
#endif /* ! _WIN32_WCE */
}
