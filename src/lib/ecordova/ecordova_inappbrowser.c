#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "ecordova_inappbrowser_private.h"

#define MY_CLASS ECORDOVA_INAPPBROWSER_CLASS
#define MY_CLASS_NAME "Ecordova_InAppBrowser"

static Eo_Base *
_ecordova_inappbrowser_eo_base_constructor(Eo *obj,
                                           Ecordova_InAppBrowser_Data *pd)
{
   DBG("(%p)", obj);

   pd->obj = obj;

   return eo_do_super_ret(obj, MY_CLASS, obj, eo_constructor());
}

static void
_ecordova_inappbrowser_constructor(Eo *obj EINA_UNUSED,
                                   Ecordova_InAppBrowser_Data *pd EINA_UNUSED)
{
   DBG("(%p)", obj);
}

static void
_ecordova_inappbrowser_eo_base_destructor(Eo *obj,
                                          Ecordova_InAppBrowser_Data *pd EINA_UNUSED)
{
   DBG("(%p)", obj);

   eo_do_super(obj, MY_CLASS, eo_destructor());
}

static Ecordova_InAppBrowser*
_ecordova_inappbrowser_open(Eo *obj EINA_UNUSED,
                            void *pd EINA_UNUSED,
                            const char *url EINA_UNUSED,
                            const char *target EINA_UNUSED,
                            const Eina_Hash *options EINA_UNUSED)
{
   ERR("Not implemented.");
   return NULL;
}

static void
_ecordova_inappbrowser_close(Eo *obj EINA_UNUSED,
                             Ecordova_InAppBrowser_Data *pd EINA_UNUSED)
{
   ERR("Not implemented.");
}

static void
_ecordova_inappbrowser_show(Eo *obj EINA_UNUSED,
                            Ecordova_InAppBrowser_Data *pd EINA_UNUSED)
{
   ERR("Not implemented.");
}

static void
_ecordova_inappbrowser_script_execute(Eo *obj EINA_UNUSED,
                                      Ecordova_InAppBrowser_Data *pd EINA_UNUSED,
                                      const Ecordova_InAppBrowser_InjectDetails *injectDetails EINA_UNUSED)
{
   ERR("Not implemented.");
}

static void
_ecordova_inappbrowser_css_insert(Eo *obj EINA_UNUSED,
                                  Ecordova_InAppBrowser_Data *pd EINA_UNUSED,
                                  const Ecordova_InAppBrowser_InjectDetails *injectDetails EINA_UNUSED)
{
   ERR("Not implemented.");
}

#include "ecordova_inappbrowser.eo.c"
