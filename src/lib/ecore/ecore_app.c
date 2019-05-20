#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdlib.h>
#include <unistd.h>

#include "Ecore.h"
#include "ecore_private.h"

static int app_argc = 0;
static char **app_argv = NULL;

EAPI void
ecore_app_args_set(int          argc,
                   const char **argv)
{
   EINA_MAIN_LOOP_CHECK_RETURN;

   if ((argc < 1) ||
       (!argv)) return;
   app_argc = argc;
   app_argv = (char **)argv;
}

EAPI void
ecore_app_args_get(int    *argc,
                   char ***argv)
{
   EINA_MAIN_LOOP_CHECK_RETURN;

   if (argc) *argc = app_argc;
   if (argv) *argv = app_argv;
}

EAPI void
ecore_app_restart(void)
{
   EINA_MAIN_LOOP_CHECK_RETURN;
#ifdef HAVE_EXECVP
   char *args[4096];
   int i;

   if ((app_argc < 1) || (!app_argv)) return;
   if (app_argc >= 4096) return;
   for (i = 0; i < app_argc; i++) args[i] = app_argv[i];
   args[i] = NULL;
   execvp(app_argv[0], args);
#endif
}
