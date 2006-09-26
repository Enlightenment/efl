/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#ifndef _ECORE_DESKTOP_PRIVATE_H
# define _ECORE_DESKTOP_PRIVATE_H

#include <string.h>
#include <strings.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <Ecore.h>
#include <Ecore_File.h>

#define E_FN_DEL(_fn, _h) if (_h) { _fn(_h); _h = NULL; }
#define E_REALLOC(p, s, n) p = (s *)realloc(p, sizeof(s) * n)
#define E_NEW(s, n) (s *)calloc(n, sizeof(s))
#define E_NEW_BIG(s, n) (s *)malloc(n * sizeof(s))
#define E_FREE(p) { if (p) {free(p); p = NULL;} }

extern Ecore_List  *ecore_desktop_paths_config;
extern Ecore_List  *ecore_desktop_paths_menus;
extern Ecore_List  *ecore_desktop_paths_directories;
extern Ecore_List  *ecore_desktop_paths_desktops;
extern Ecore_List  *ecore_desktop_paths_icons;
extern Ecore_List  *ecore_desktop_paths_kde_legacy;
extern Ecore_List  *ecore_desktop_paths_xsessions;
extern struct _Ecore_Desktop_Instrumentation instrumentation;


# ifdef __cplusplus
extern              "C"
{
# endif
# ifdef __cplusplus
}
# endif

#endif
