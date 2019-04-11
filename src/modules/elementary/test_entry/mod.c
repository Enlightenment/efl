#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#include "Elementary.h"

#ifndef EFL_BUILD
# define EFL_BUILD
#endif
#undef ELM_MODULE_HELPER_H
#include "elm_module_helper.h"

// module api funcs needed
EAPI int
elm_modapi_init(void *m EINA_UNUSED)
{
   return 1; // succeed always
}

EAPI int
elm_modapi_shutdown(void *m EINA_UNUSED)
{
   return 1; // succeed always
}

// module fucns for the specific module type
EAPI void
obj_hook(Evas_Object *obj)
{
   printf("hook: %p\n", obj);
}

EAPI void
obj_unhook(Evas_Object *obj)
{
   printf("unhook: %p\n", obj);
}

EAPI void
obj_longpress(Evas_Object *obj)
{
   printf("longpress: %p\n", obj);
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

EINA_MODULE_VERSION("0.1");
EINA_MODULE_AUTHOR("Enlightenment Community");
EINA_MODULE_DESCRIPTION("Entry test");
EINA_MODULE_LICENSE("GPLv2");

EINA_MODULE_INIT(_module_init);
EINA_MODULE_SHUTDOWN(_module_shutdown);
