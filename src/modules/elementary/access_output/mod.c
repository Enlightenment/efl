#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include "Elementary.h"

#ifndef EFL_BUILD
# define EFL_BUILD
#endif
#undef ELM_MODULE_HELPER_H
#include "elm_module_helper.h"

/* to enable this module
export ELM_MODULES="access_output>access/api"
export ELM_ACCESS_MODE=1
 */

static void (*cb_func) (void *data);
static void *cb_data;
static Ecore_Exe *espeak = NULL;
static Ecore_Event_Handler *exe_exit_handler = NULL;
static char *tmpf = NULL;
static int tmpfd = -1;

static Eina_Bool
_exe_del(void *data EINA_UNUSED, int type EINA_UNUSED, void *event)
{
   Ecore_Exe_Event_Del *ev = event;

   if ((espeak) && (ev->exe == espeak))
     {
        if (tmpf)
          {
             unlink(tmpf);
             free(tmpf);
             tmpf = NULL;
             close(tmpfd);
             tmpfd = -1;
          }
        espeak = NULL;
        if (cb_func) cb_func(cb_data);
     }
   return ECORE_CALLBACK_RENEW;
}

// module api funcs needed
EAPI int
elm_modapi_init(void *m EINA_UNUSED)
{
   exe_exit_handler =
      ecore_event_handler_add(ECORE_EXE_EVENT_DEL,
                              _exe_del, NULL);
   return 1; // succeed always
}

EAPI int
elm_modapi_shutdown(void *m EINA_UNUSED)
{
   if (exe_exit_handler)
     {
        ecore_event_handler_del(exe_exit_handler);
        exe_exit_handler = NULL;
     }
   return 1; // succeed always
}

// module fucns for the specific module type
EAPI void
out_read(const char *txt)
{
   if (!tmpf)
     {
        char buf[PATH_MAX];
        mode_t cur_umask;

        snprintf(buf, sizeof(buf), "/tmp/.elm-speak-XXXXXX");
        cur_umask = umask(S_IRWXO | S_IRWXG);
        tmpfd = mkstemp(buf);
        umask(cur_umask);
        if (tmpfd >= 0) tmpf = strdup(buf);
        else return;
     }
   if (write(tmpfd, txt, strlen(txt)) < 0) perror("write to tmpfile (espeak)");
}

EAPI void
out_read_done(void)
{
   char buf[PATH_MAX];

   if (espeak)
     {
        ecore_exe_interrupt(espeak);
        espeak = NULL;
     }
   if (tmpf)
     {
        // FIXME: espeak supporets -v XX for voice locale. should provide this
        // based on actual lang/locale
        if (tmpfd >= 0) close(tmpfd);
        tmpfd = -1;
        snprintf(buf, sizeof(buf), "espeak -p 2 -s 120 -k 10 -m -f %s", tmpf);
        espeak = ecore_exe_pipe_run(buf,
                                    ECORE_EXE_NOT_LEADER,
                                    NULL);
     }
}

EAPI void
out_cancel(void)
{
   if (espeak)
     {
        ecore_exe_interrupt(espeak);
        espeak = NULL;
     }
   if (tmpf)
     {
        unlink(tmpf);
        free(tmpf);
        tmpf = NULL;
        close(tmpfd);
        tmpfd = -1;
     }
}

EAPI void
out_done_callback_set(void (*func) (void *data), const void *data)
{
   cb_func = func;
   cb_data = (void *)data;
}

static Eina_Bool
_module_init(void)
{
   return EINA_TRUE;
}

static void
_module_shutdown(void)
{
}

EINA_MODULE_INIT(_module_init);
EINA_MODULE_SHUTDOWN(_module_shutdown);
