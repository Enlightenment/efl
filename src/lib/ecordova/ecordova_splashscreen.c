#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "ecordova_splashscreen_private.h"

#define MY_CLASS ECORDOVA_SPLASHSCREEN_CLASS
#define MY_CLASS_NAME "Ecordova_Splashscreen"

static Eo_Base *
_ecordova_splashscreen_eo_base_constructor(Eo *obj,
                                           Ecordova_Splashscreen_Data *pd)
{
   DBG("(%p)", obj);

   pd->obj = obj;

   return eo_do_super_ret(obj, MY_CLASS, obj, eo_constructor());
}

static void
_ecordova_splashscreen_constructor(Eo *obj EINA_UNUSED,
                                   Ecordova_Splashscreen_Data *pd EINA_UNUSED)
{
   DBG("(%p)", obj);
}

static void
_ecordova_splashscreen_eo_base_destructor(Eo *obj,
                                          Ecordova_Splashscreen_Data *pd EINA_UNUSED)
{
   DBG("(%p)", obj);

   eo_do_super(obj, MY_CLASS, eo_destructor());
}

static void
_ecordova_splashscreen_show(Eo *obj EINA_UNUSED,
                            Ecordova_Splashscreen_Data *pd EINA_UNUSED)
{
   ERR("Not implemented.");
}

static void
_ecordova_splashscreen_hide(Eo *obj EINA_UNUSED,
                            Ecordova_Splashscreen_Data *pd EINA_UNUSED)
{
   ERR("Not implemented.");
}

#include "ecordova_splashscreen.eo.c"
