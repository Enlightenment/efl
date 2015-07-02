#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "ecordova_dialogs_private.h"

#define MY_CLASS ECORDOVA_DIALOGS_CLASS
#define MY_CLASS_NAME "Ecordova_Dialogs"

static Eo_Base *
_ecordova_dialogs_eo_base_constructor(Eo *obj, Ecordova_Dialogs_Data *pd)
{
   DBG("(%p)", obj);

   pd->obj = obj;

   return eo_do_super_ret(obj, MY_CLASS, obj, eo_constructor());
}

static void
_ecordova_dialogs_constructor(Eo *obj EINA_UNUSED,
                              Ecordova_Dialogs_Data *pd EINA_UNUSED)
{
   DBG("(%p)", obj);
}

static void
_ecordova_dialogs_eo_base_destructor(Eo *obj,
                                     Ecordova_Dialogs_Data *pd EINA_UNUSED)
{
   DBG("(%p)", obj);

   eo_do_super(obj, MY_CLASS, eo_destructor());
}

static void
_ecordova_dialogs_alert(Eo *obj EINA_UNUSED,
                        Ecordova_Dialogs_Data *pd EINA_UNUSED,
                        const char *message EINA_UNUSED,
                        const char *title EINA_UNUSED,
                        Eina_List *buttons EINA_UNUSED)
{
   ERR("Not implemented.");
}

static void
_ecordova_dialogs_confirm(Eo *obj EINA_UNUSED,
                          Ecordova_Dialogs_Data *pd EINA_UNUSED,
                          const char *message EINA_UNUSED,
                          const char *title EINA_UNUSED,
                          Eina_List *buttons EINA_UNUSED)
{
   ERR("Not implemented.");
}

static void
_ecordova_dialogs_prompt(Eo *obj EINA_UNUSED,
                         Ecordova_Dialogs_Data *pd EINA_UNUSED,
                         const char *message EINA_UNUSED,
                         const char *title EINA_UNUSED,
                         Eina_List *buttons EINA_UNUSED,
                         const char *default_text EINA_UNUSED)
{
   ERR("Not implemented.");
}

static void
_ecordova_dialogs_beep(Eo *obj EINA_UNUSED,
                       Ecordova_Dialogs_Data *pd EINA_UNUSED,
                       int times EINA_UNUSED)
{
   ERR("Not implemented.");
}

#include "ecordova_dialogs.eo.c"
