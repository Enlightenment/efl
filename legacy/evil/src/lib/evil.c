#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#undef WIN32_LEAN_AND_MEAN

#include <stdio.h>

#ifndef __CEGCC__
# include <errno.h>
# include <sys/locking.h>
# include <io.h>
# include <share.h>
# include <shlobj.h>
# include <objidl.h>
#else
# include <sys/syslimits.h>
#endif /* __CEGCC__ */

#if defined(__CEGCC__) || defined(__MINGW32CE__)
# include <wchar.h>
#endif /* __CEGCC__ || __MINGW32CE__ */

#include <sys/types.h>
#include <sys/timeb.h>
#include <fcntl.h>
#include <sys/stat.h>

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif /* HAVE_CONFIG_H */

#if HAVE___ATTRIBUTE__
# define __UNUSED__ __attribute__((unused))
#else
# define __UNUSED__
#endif /* HAVE___ATTRIBUTE__ */

#include "Evil.h"

#ifndef __CEGCC__

int fcntl(int fd, int cmd, ...)
{
   va_list va;
   HANDLE  h;
   int     res = -1;

   va_start (va, cmd);

   h = (HANDLE)_get_osfhandle(fd);
   if (h == INVALID_HANDLE_VALUE)
     return -1;

   if (cmd == F_SETFD)
     {
        long flag;

        flag = va_arg(va, long);
        if (flag == FD_CLOEXEC)
          {
             if (SetHandleInformation(h, HANDLE_FLAG_INHERIT, 0))
               res = 0;
          }
     }
   else if ((cmd == F_SETLK) || (cmd == F_SETLKW))
     {
        struct flock fl;
        off_t        length = 0;
        long         pos;

        fl = va_arg(va, struct flock);

        if (fl.l_len == 0)
          {
             length = _lseek(fd, 0L, SEEK_END);
             if (length != -1L)
               res = 0;
          }
        fl.l_len = length - fl.l_start - 1;

        pos = _lseek(fd, fl.l_start, fl.l_whence);
        if (pos != -1L)
          res = 0;

        if ((fl.l_type == F_RDLCK) || (fl.l_type == F_WRLCK))
          {
             if (cmd == F_SETLK)
               res = _locking(fd, _LK_NBLCK, fl.l_len); /* if cannot be locked, we return immediatly */
             else /* F_SETLKW */
               res = _locking(fd, _LK_LOCK, fl.l_len); /* otherwise, we try several times */
          }

        if (fl.l_type == F_UNLCK)
          res = _locking(fd, _LK_UNLCK, fl.l_len);
     }

   va_end(va);

   return res;
}

int
mkstemp(char *template)
{
   int fd;

#ifdef __MINGW32__
   if (!_mktemp(template))
     return -1;

   fd = _sopen(template, _O_RDWR | _O_BINARY | _O_CREAT | _O_EXCL, _SH_DENYNO, _S_IREAD | _S_IWRITE);
#else
   if (_mktemp_s(template, _MAX_PATH) != 0)
     return -1;

   _sopen_s(&fd, template, _O_RDWR | _O_BINARY | _O_CREAT, _SH_DENYNO, _S_IREAD | _S_IWRITE);
#endif /* ! __MINGW32__ */

   return fd;
}

/* REMARK: Windows has no symbolic link. */
/*         Nevertheless, it can create and read .lnk files */
int
symlink(const char *oldpath, const char *newpath)
{
   wchar_t        new_path[MB_CUR_MAX];
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

   /* FIXME: is it for cegcc ??? */
   mbstowcs(new_path, newpath, MB_CUR_MAX);
   if (FAILED(pIPF->lpVtbl->Save(pIPF, new_path, FALSE)))
     goto no_save;

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
}

ssize_t
readlink(const char *path, char *buf, size_t bufsiz)
{
   wchar_t        old_path[MB_CUR_MAX];
   char           new_path[MB_CUR_MAX];
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

   mbstowcs(old_path, path, MB_CUR_MAX);
   if (FAILED(pIPF->lpVtbl->Load(pIPF, old_path, STGM_READWRITE)))
     goto no_load;

   shell_link = &pISL;
   if (FAILED(pIPF->lpVtbl->QueryInterface(pIPF, &IID_IShellLink, (void **)shell_link)))
     goto no_queryinterface;

   if (FAILED(pISL->lpVtbl->GetPath(pISL, new_path, MB_CUR_MAX, NULL, 0)))
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
}

/*
 * The code of the following functions has been kindly offered
 * by Tor Lillqvist.
 */
int
pipe(int *fds)
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

#endif /* ! __CEGCC__ */

char *
realpath(const char *file_name, char *resolved_name)
{
#if ! ( defined(__CEGCC__) || defined(__MINGW32CE__) )
   return _fullpath(resolved_name, file_name, PATH_MAX);
#else
   int length;

   length = strlen(file_name);
   if ((length + 1) > PATH_MAX)
     length = PATH_MAX - 1;
   memcpy(resolved_name, file_name, length);
   resolved_name[length] = '\0';

   return resolved_name;
#endif /* __CEGCC__ || __MINGW32CE__ */
}

int
evil_sockets_init(void)
{
   WSADATA wsa_data;

   return (WSAStartup(MAKEWORD( 2, 2 ), &wsa_data) == 0) ? 1 : 0;
}

void
evil_sockets_shutdown(void)
{
   WSACleanup();
}

const char *
evil_tmpdir_get(void)
{
   char *tmpdir;

   tmpdir = getenv("TMP");
   if (!tmpdir) tmpdir = getenv("TEMP");
   if (!tmpdir) tmpdir = getenv("USERPROFILE");
   if (!tmpdir) tmpdir = getenv("WINDIR");
   if (!tmpdir) tmpdir="C:\\";

   return tmpdir;
}


#if defined(__CEGCC__) || defined(__MINGW32CE__)

wchar_t *
evil_char_to_wchar(const char *text)
{
   wchar_t *wtext;
   int      wsize;

   wsize = MultiByteToWideChar(CP_ACP, 0, text, strlen(text) + 1, NULL, 0);
   if ((wsize == 0) ||
       (wsize > (int)(ULONG_MAX/sizeof(wchar_t))))
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

#endif /* __CEGCC__ || __MINGW32CE__ */
