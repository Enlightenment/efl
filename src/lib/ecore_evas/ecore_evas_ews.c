#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include <Evas.h>
#include <Evas_Engine_Buffer.h>
#include <Ecore.h>
#include "ecore_private.h"
#include <Ecore_Input.h>

#include "Ecore_Evas.h"
#include "ecore_evas_private.h"

EAPI int ECORE_EVAS_EWS_EVENT_MANAGER_CHANGE = 0;
EAPI int ECORE_EVAS_EWS_EVENT_ADD = 0;
EAPI int ECORE_EVAS_EWS_EVENT_DEL = 0;
EAPI int ECORE_EVAS_EWS_EVENT_RESIZE = 0;
EAPI int ECORE_EVAS_EWS_EVENT_MOVE = 0;
EAPI int ECORE_EVAS_EWS_EVENT_SHOW = 0;
EAPI int ECORE_EVAS_EWS_EVENT_HIDE = 0;
EAPI int ECORE_EVAS_EWS_EVENT_FOCUS = 0;
EAPI int ECORE_EVAS_EWS_EVENT_UNFOCUS = 0;
EAPI int ECORE_EVAS_EWS_EVENT_RAISE = 0;
EAPI int ECORE_EVAS_EWS_EVENT_LOWER = 0;
EAPI int ECORE_EVAS_EWS_EVENT_ACTIVATE = 0;

EAPI int ECORE_EVAS_EWS_EVENT_ICONIFIED_CHANGE = 0;
EAPI int ECORE_EVAS_EWS_EVENT_MAXIMIZED_CHANGE = 0;
EAPI int ECORE_EVAS_EWS_EVENT_LAYER_CHANGE = 0;
EAPI int ECORE_EVAS_EWS_EVENT_FULLSCREEN_CHANGE = 0;
EAPI int ECORE_EVAS_EWS_EVENT_CONFIG_CHANGE = 0;

EAPI Ecore_Evas *
ecore_evas_ews_new(int x EINA_UNUSED, int y EINA_UNUSED, int w EINA_UNUSED, int h EINA_UNUSED)
{
   return NULL;
}

EAPI Evas_Object *
ecore_evas_ews_backing_store_get(const Ecore_Evas *ee EINA_UNUSED)
{
   return NULL;
}

EAPI void
ecore_evas_ews_delete_request(Ecore_Evas *ee EINA_UNUSED)
{
}


EAPI Eina_Bool
ecore_evas_ews_engine_set(const char *engine EINA_UNUSED, const char *options EINA_UNUSED)
{
   return EINA_FALSE;
}

EAPI Eina_Bool
ecore_evas_ews_setup(int x EINA_UNUSED, int y EINA_UNUSED, int w EINA_UNUSED, int h EINA_UNUSED)
{
   return EINA_FALSE;
}

EAPI Ecore_Evas *
ecore_evas_ews_ecore_evas_get(void)
{
   return NULL;
}

EAPI Evas *
ecore_evas_ews_evas_get(void)
{
   return NULL;
}

EAPI Evas_Object *
ecore_evas_ews_background_get(void)
{
   return NULL;
}

EAPI void
ecore_evas_ews_background_set(Evas_Object *o EINA_UNUSED)
{
}


EAPI const Eina_List *
ecore_evas_ews_children_get(void)
{
   return NULL;
}

EAPI void
ecore_evas_ews_manager_set(const void *manager EINA_UNUSED)
{
}

EAPI const void *
ecore_evas_ews_manager_get(void)
{
   return NULL;
}
