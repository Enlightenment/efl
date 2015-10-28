#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Eina.h>
#include <Eo.h>

#include "ecordova_console_private.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#define MY_CLASS ECORDOVA_CONSOLE_CLASS
#define MY_CLASS_NAME "Ecordova_Console"

#ifdef EAPI
# undef EAPI
#endif

#ifdef _WIN32
# ifdef DLL_EXPORT
#  define EAPI __declspec(dllexport)
# else
#  define EAPI
# endif /* ! DLL_EXPORT */
#else
# ifdef __GNUC__
#  if __GNUC__ >= 4
#   define EAPI __attribute__ ((visibility("default")))
#  else
#   define EAPI
#  endif
# else
#  define EAPI
# endif
#endif /* ! _WIN32 */

/* logging support */
extern int _ecordova_log_dom;

#define CRI(...) EINA_LOG_DOM_CRIT(_ecordova_log_dom, __VA_ARGS__)
#define ERR(...) EINA_LOG_DOM_ERR(_ecordova_log_dom, __VA_ARGS__)
#define WRN(...) EINA_LOG_DOM_WARN(_ecordova_log_dom, __VA_ARGS__)
#define INF(...) EINA_LOG_DOM_INFO(_ecordova_log_dom, __VA_ARGS__)
#define DBG(...) EINA_LOG_DOM_DBG(_ecordova_log_dom, __VA_ARGS__)

static void _ecordova_console_level_log(Ecordova_Console_Data *, Ecordova_Console_LoggerLevel, const char *);

static Eo_Base *
_ecordova_console_eo_base_constructor(Eo *obj, Ecordova_Console_Data *pd)
{
   DBG("(%p)", obj);

   pd->obj = obj;
   pd->logger_use = EINA_TRUE;

   return eo_do_super_ret(obj, MY_CLASS, obj, eo_constructor());
}

static void
_ecordova_console_constructor(Eo *obj EINA_UNUSED,
                              Ecordova_Console_Data *pd EINA_UNUSED)
{
   DBG("(%p)", obj);
}

static void
_ecordova_console_eo_base_destructor(Eo *obj,
                                     Ecordova_Console_Data *pd EINA_UNUSED)
{
   DBG("(%p)", obj);

   eo_do_super(obj, MY_CLASS, eo_destructor());
}

static Ecordova_Console_LoggerLevel
_ecordova_console_level_get(Eo *obj EINA_UNUSED, Ecordova_Console_Data *pd)
{
   return pd->level;
}

static void
_ecordova_console_level_set(Eo *obj EINA_UNUSED,
                            Ecordova_Console_Data *pd,
                            Ecordova_Console_LoggerLevel value)
{
   pd->level = value;
}

static Eina_Bool
_ecordova_console_console_use_get(Eo *obj EINA_UNUSED, Ecordova_Console_Data *pd)
{
   return pd->console_use;
}

static void
_ecordova_console_console_use_set(Eo *obj EINA_UNUSED,
                                  Ecordova_Console_Data *pd,
                                  Eina_Bool value)
{
   pd->console_use = value;
}

static Eina_Bool
_ecordova_console_logger_use_get(Eo *obj EINA_UNUSED, Ecordova_Console_Data *pd)
{
   return pd->logger_use;
}

static void
_ecordova_console_logger_use_set(Eo *obj EINA_UNUSED,
                                 Ecordova_Console_Data *pd,
                                 Eina_Bool value)
{
   pd->logger_use = value;
}

static void
_ecordova_console_log(Eo *obj EINA_UNUSED,
                      Ecordova_Console_Data *pd,
                      const char *message)
{
   _ecordova_console_level_log(pd, ECORDOVA_CONSOLE_LOGGERLEVEL_LOG, message);
}

static void
_ecordova_console_error(Eo *obj EINA_UNUSED,
                        Ecordova_Console_Data *pd,
                        const char *message)
{
   _ecordova_console_level_log(pd, ECORDOVA_CONSOLE_LOGGERLEVEL_ERROR, message);
}

static void
_ecordova_console_warn(Eo *obj EINA_UNUSED,
                       Ecordova_Console_Data *pd,
                       const char *message)
{
   _ecordova_console_level_log(pd, ECORDOVA_CONSOLE_LOGGERLEVEL_WARN, message);
}

static void
_ecordova_console_info(Eo *obj EINA_UNUSED,
                       Ecordova_Console_Data *pd,
                       const char *message)
{
   _ecordova_console_level_log(pd, ECORDOVA_CONSOLE_LOGGERLEVEL_INFO, message);
}

static void
_ecordova_console_debug(Eo *obj EINA_UNUSED,
                        Ecordova_Console_Data *pd,
                        const char *message)
{
   _ecordova_console_level_log(pd, ECORDOVA_CONSOLE_LOGGERLEVEL_DEBUG, message);
}

static void
_ecordova_console_level_log(Ecordova_Console_Data *pd,
                            Ecordova_Console_LoggerLevel level,
                            const char *message)
{
   EINA_SAFETY_ON_NULL_RETURN(message);

   if (level < 0 || pd->level >= ECORDOVA_CONSOLE_LOGGERLEVEL_LAST)
     {
        ERR("Invalid logging level: %d", level);
        return;
     }

   if (level > pd->level) return;

   if (pd->logger_use)
     {
        switch (level)
          {
           case ECORDOVA_CONSOLE_LOGGERLEVEL_LOG:
             CRI("%s", message);
             break;
           case ECORDOVA_CONSOLE_LOGGERLEVEL_ERROR:
             ERR("%s", message);
             break;
           case ECORDOVA_CONSOLE_LOGGERLEVEL_WARN:
             WRN("%s", message);
             break;
           case ECORDOVA_CONSOLE_LOGGERLEVEL_INFO:
             INF("%s", message);
             break;
           case ECORDOVA_CONSOLE_LOGGERLEVEL_DEBUG:
             DBG("%s", message);
             break;
           default: break; // removes warning
          }
     }

   const char *level_str[ECORDOVA_CONSOLE_LOGGERLEVEL_LAST] = {
     [ECORDOVA_CONSOLE_LOGGERLEVEL_LOG]   = "",
     [ECORDOVA_CONSOLE_LOGGERLEVEL_ERROR] = "ERROR: ",
     [ECORDOVA_CONSOLE_LOGGERLEVEL_WARN]  = "WARN: ",
     [ECORDOVA_CONSOLE_LOGGERLEVEL_INFO]  = "INFO: ",
     [ECORDOVA_CONSOLE_LOGGERLEVEL_DEBUG] = "DEBUG: ",
   };

   if (pd->console_use)
     printf("%s%s\n", level_str[level], message);
}

#undef EOAPI
#define EOAPI EAPI

#include "undefs.h"

#define ecordova_console_class_get ecordova_console_impl_class_get

#include "ecordova_console.eo.c"
