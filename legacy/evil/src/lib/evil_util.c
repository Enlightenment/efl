#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <wchar.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef WIN32_LEAN_AND_MEAN

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif /* HAVE_CONFIG_H */

#include "Evil.h"


wchar_t *
evil_char_to_wchar(const char *text)
{
   wchar_t *wtext;
   int      wsize;

   wsize = MultiByteToWideChar(CP_ACP, 0, text, strlen(text) + 1, NULL, 0);
   if ((wsize == 0) ||
       (wsize > (int)(ULONG_MAX / sizeof(wchar_t))))
     return NULL;

   wtext = malloc(wsize * sizeof(wchar_t));
   if (wtext)
     if (!MultiByteToWideChar(CP_ACP, 0, text, strlen(text) + 1, wtext, wsize))
       return NULL;

   return wtext;
}

char *
evil_wchar_to_char(const wchar_t *text)
{
   char * atext;
   int    size;
   int    asize;

   size = wcslen(text) + 1;

   asize = WideCharToMultiByte(CP_ACP, 0, text, size, NULL, 0, NULL, NULL);
   if (asize == 0)
     return NULL;

   atext = (char*)malloc((asize + 1) * sizeof(char));

   if (atext)
     if (!WideCharToMultiByte(CP_ACP, 0, text, size, atext, asize, NULL, NULL))
       return NULL;
   atext[asize] = '\0';

   return atext;
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
                      0, // Default language
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
   str = strdup(msg);
#endif /* UNICODE */

   LocalFree(msg);

   disp = (char *)malloc((strlen(str) + strlen("(00000) ") + 1) * sizeof(char));
   if (!disp)
     return NULL;

   snprintf(disp, strlen(str) + strlen("(00000) ") + 1,
            "(%5ld) %s", err, str);

   free(str);

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
