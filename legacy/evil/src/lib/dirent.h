#ifndef __EVIL_DIRENT_H__
#define __EVIL_DIRENT_H__

#ifdef EAPI
# undef EAPI
#endif /* EAPI */

#ifdef _WIN32
# ifdef EFL_EVIL_BUILD
#  ifdef DLL_EXPORT
#   define EAPI __declspec(dllexport)
#  else
#   define EAPI
#  endif /* ! DLL_EXPORT */
# else
#  define EAPI __declspec(dllimport)
# endif /* ! EFL_EVIL_BUILD */
#endif /* _WIN32 */


#ifdef UNICODE
# include <wchar.h>
#endif


typedef struct DIR DIR;

struct dirent
{
   char    d_name[260 + 1];
   int     d_mode;
};


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


EAPI DIR *opendir(char const *name);

EAPI int closedir(DIR *dir);

EAPI struct dirent *readdir(DIR *dir);


#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* __EVIL_DIRENT_H__ */
