#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#undef WIN32_LEAN_AND_MEAN

#if defined(_MSC_VER) || \
   (defined(__MINGW32__) && ! defined(__MINGW32CE__))
# include <shlobj.h>
# include <objidl.h>
# include <errno.h>
#endif

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif /* HAVE_CONFIG_H */

#include "Evil.h"

/*
 * Process identifer related functions
 *
 */

pid_t
getpid(void)
{
  return (pid_t)GetCurrentProcessId();
}


/*
 * Symbolic links and directory related functions
 *
 */

#if defined(__CEGCC__) || defined(__MINGW32CE__)

DWORD SHCreateShortcutEx(LPTSTR lpszDir,
                         LPTSTR lpszTarget,
                         LPTSTR szShortcut,
                         LPDWORD lpcbShortcut);

BOOL SHGetShortcutTarget(LPTSTR szShortcut,
                         LPTSTR szTarget,
                         int cbMax );

#endif /* __CEGCC__ || __MINGW32CE__ */


/* REMARK: Windows has no symbolic link. */
/*         Nevertheless, it can create and read .lnk files */
int
symlink(const char *oldpath, const char *newpath)
{
#if defined(__CEGCC__) || defined(__MINGW32CE__)
   wchar_t *w_oldpath;
   wchar_t *w_newpath;
   BOOL     res;

   w_oldpath = evil_char_to_wchar(oldpath);
   if (!w_oldpath)
     return -1;

   w_newpath = evil_char_to_wchar(newpath);
   if (!w_newpath)
     {
        free(w_oldpath);
        return -1;
     }

   res = SHCreateShortcutEx(w_newpath, w_oldpath, NULL, NULL);

   free(w_oldpath);
   free(w_newpath);

   return res ? 0 : -1;
#else
# ifdef UNICODE
   wchar_t        new_path[MB_CUR_MAX];
# endif /* UNICODE */
   IShellLink    *pISL;
   IShellLink   **shell_link;
   IPersistFile  *pIPF;
   IPersistFile **persit_file;
   HRESULT        res;

   res = CoInitialize(NULL);
   if (FAILED(res))
     {
        if (res == E_OUTOFMEMORY)
          errno = ENOMEM;
        return -1;
     }

   /* Hack to cleanly remove a warning */
   shell_link = &pISL;
   if (FAILED(CoCreateInstance(&CLSID_ShellLink,
                               NULL,
                               CLSCTX_INPROC_SERVER,
                               &IID_IShellLink,
                               (void **)shell_link)))
     goto no_instance;

   if (FAILED(pISL->lpVtbl->SetPath(pISL, oldpath)))
     goto no_setpath;

   /* Hack to cleanly remove a warning */
   persit_file = &pIPF;
   if (FAILED(pISL->lpVtbl->QueryInterface(pISL, &IID_IPersistFile, (void **)persit_file)))
     goto no_queryinterface;

# ifdef UNICODE
   mbstowcs(new_path, newpath, MB_CUR_MAX);
   if (FAILED(pIPF->lpVtbl->Save(pIPF, new_path, FALSE)))
     goto no_save;
# else
   if (FAILED(pIPF->lpVtbl->Save(pIPF, newpath, FALSE)))
     goto no_save;
# endif /* ! UNICODE */

   pIPF->lpVtbl->Release(pIPF);
   pISL->lpVtbl->Release(pISL);
   CoUninitialize();

   return 0;

 no_save:
   pIPF->lpVtbl->Release(pIPF);
 no_queryinterface:
 no_setpath:
   pISL->lpVtbl->Release(pISL);
 no_instance:
   CoUninitialize();
   return -1;
#endif /* ! __CEGCC__ && ! __MINGW32CE__ */
}

ssize_t
readlink(const char *path, char *buf, size_t bufsiz)
{
#if defined(__CEGCC__) || defined(__MINGW32CE__)
   wchar_t *w_path;
   wchar_t  w_newpath[MB_CUR_MAX];
   char    *newpath;
   size_t   length;
   BOOL     res;

   w_path = evil_char_to_wchar(path);
   if (!w_path)
     return -1;

   res = SHGetShortcutTarget(w_path, w_newpath, MB_CUR_MAX);

   free(w_path);

   if (!res)
     return -1;

   newpath = evil_wchar_to_char(w_newpath);
   if (!newpath)
     return -1;

   length = strlen(newpath);
   if (length > bufsiz)
     length = bufsiz;

   memcpy(buf, newpath, length);

   free(newpath);

   return length;
#else
# ifdef UNICODE
   wchar_t        old_path[MB_CUR_MAX];
# endif /* UNICODE */
   char           new_path[PATH_MAX];
   IShellLink    *pISL;
   IShellLink   **shell_link;
   IPersistFile  *pIPF;
   IPersistFile **persit_file;
   unsigned int   length;
   HRESULT        res;

   res = CoInitialize(NULL);
   if (FAILED(res))
     {
        if (res == E_OUTOFMEMORY)
          errno = ENOMEM;
        return -1;
     }

   /* Hack to cleanly remove a warning */
   persit_file = &pIPF;
   if (FAILED(CoCreateInstance(&CLSID_ShellLink,
                               NULL,
                               CLSCTX_INPROC_SERVER,
                               &IID_IPersistFile,
                               (void **)persit_file)))
     goto no_instance;

# ifdef UNICODE
   mbstowcs(old_path, path, MB_CUR_MAX);
   if (FAILED(pIPF->lpVtbl->Load(pIPF, old_path, STGM_READWRITE)))
     goto no_load;
# else
   if (FAILED(pIPF->lpVtbl->Load(pIPF, path, STGM_READWRITE)))
     goto no_load;
# endif /* ! UNICODE */

   shell_link = &pISL;
   if (FAILED(pIPF->lpVtbl->QueryInterface(pIPF, &IID_IShellLink, (void **)shell_link)))
     goto no_queryinterface;

   if (FAILED(pISL->lpVtbl->GetPath(pISL, new_path, PATH_MAX, NULL, 0)))
     goto no_getpath;

   length = strlen(new_path);
   if (length > bufsiz)
     length = bufsiz;

   memcpy(buf, new_path, length);

   pISL->lpVtbl->Release(pISL);
   pIPF->lpVtbl->Release(pIPF);
   CoUninitialize();

   return length;

 no_getpath:
   pISL->lpVtbl->Release(pISL);
 no_queryinterface:
 no_load:
   pIPF->lpVtbl->Release(pIPF);
 no_instance:
   CoUninitialize();
   return -1;
#endif /* ! __CEGCC__ && ! __MINGW32CE__ */
}

char *
evil_getcwd(char *buffer, size_t size)
{
#if defined(__CEGCC__) || defined(__MINGW32CE__)
   wchar_t wpath[PATH_MAX];
   char   *cpath;
   char   *delim;
   int     ret = 0;

   if (size <= 0)
     return NULL;

   ret = GetModuleFileName(GetModuleHandle(NULL), (LPWSTR)&wpath, PATH_MAX);

   if (!ret)
     return NULL;

   cpath = evil_wchar_to_char(wpath);
   if (!cpath)
     return NULL;

   if (strlen(cpath) >= (size - 1))
     {
        free(cpath);
        return NULL;
     }

   delim = strrchr(cpath, '\\');
   if (delim)
     *delim = '\0';

   if (!buffer)
     {
        buffer = (char *)malloc(sizeof(char) * size);
        if (!buffer)
          {
             free(cpath);
             return NULL;
          }
     }

   strcpy(buffer, cpath);
   free(cpath);

   return buffer;
#else
   return _getcwd(buffer, size);
#endif /* ! __CEGCC__ && ! __MINGW32CE__ */
}


/*
 * Sockets and pipe related functions
 *
 */

int
evil_sockets_init(void)
{
   WSADATA wsa_data;

   return (WSAStartup(MAKEWORD(2, 2), &wsa_data) == 0) ? 1 : 0;
}

void
evil_sockets_shutdown(void)
{
   WSACleanup();
}

/*
 * The code of the following functions has been kindly offered
 * by Tor Lillqvist.
 */
int
evil_pipe(int *fds)
{
   struct sockaddr_in saddr;
   struct timeval     tv;
   SOCKET             temp;
   SOCKET             socket1 = INVALID_SOCKET;
   SOCKET             socket2 = INVALID_SOCKET;
   u_long             arg;
   fd_set             read_set;
   fd_set             write_set;
   int                len;

   temp = socket (AF_INET, SOCK_STREAM, 0);

   if (temp == INVALID_SOCKET)
     goto out0;

   arg = 1;
   if (ioctlsocket (temp, FIONBIO, &arg) == SOCKET_ERROR)
     goto out0;

   memset (&saddr, 0, sizeof (saddr));
   saddr.sin_family = AF_INET;
   saddr.sin_port = 0;
   saddr.sin_addr.s_addr = htonl (INADDR_LOOPBACK);

   if (bind (temp, (struct sockaddr *)&saddr, sizeof (saddr)))
     goto out0;

   if (listen (temp, 1) == SOCKET_ERROR)
     goto out0;

   len = sizeof (saddr);
   if (getsockname (temp, (struct sockaddr *)&saddr, &len))
     goto out0;

   socket1 = socket (AF_INET, SOCK_STREAM, 0);

   if (socket1 == INVALID_SOCKET)
     goto out0;

   arg = 1;
   if (ioctlsocket (socket1, FIONBIO, &arg) == SOCKET_ERROR)
      goto out1;

   if ((connect (socket1, (struct sockaddr  *)&saddr, len) == SOCKET_ERROR) &&
       (WSAGetLastError () != WSAEWOULDBLOCK))
     goto out1;

   FD_ZERO (&read_set);
   FD_SET (temp, &read_set);

   tv.tv_sec = 0;
   tv.tv_usec = 0;

   if (select (0, &read_set, NULL, NULL, NULL) == SOCKET_ERROR)
     goto out1;

   if (!FD_ISSET (temp, &read_set))
     goto out1;

   socket2 = accept (temp, (struct sockaddr *) &saddr, &len);
   if (socket2 == INVALID_SOCKET)
     goto out1;

   FD_ZERO (&write_set);
   FD_SET (socket1, &write_set);

   tv.tv_sec = 0;
   tv.tv_usec = 0;

   if (select (0, NULL, &write_set, NULL, NULL) == SOCKET_ERROR)
     goto out2;

   if (!FD_ISSET (socket1, &write_set))
     goto out2;

   arg = 0;
   if (ioctlsocket (socket1, FIONBIO, &arg) == SOCKET_ERROR)
     goto out2;

   arg = 0;
   if (ioctlsocket (socket2, FIONBIO, &arg) == SOCKET_ERROR)
     goto out2;

   fds[0] = socket1;
   fds[1] = socket2;

   closesocket (temp);

   return 0;

 out2:
   closesocket (socket2);
 out1:
   closesocket (socket1);
 out0:
   closesocket (temp);

   fds[0] = INVALID_SOCKET;
   fds[1] = INVALID_SOCKET;

   return -1;
}
