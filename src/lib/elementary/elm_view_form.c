#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include <Eo.h>
#include <Efl.h>
#include <Elementary.h>

#include "elm_view_form.h"
#include "elm_view_list.h"

#include "elm_priv.h"
#include "elm_entry_eo.h"
#include "elm_thumb.eo.h"
#include "elm_label.eo.h"

#include <assert.h>

#define MY_CLASS ELM_VIEW_FORM_CLASS
#define MY_CLASS_NAME "View_Form"

typedef struct _Elm_View_Form_Data Elm_View_Form_Data;
typedef struct _Elm_View_Form_Widget Elm_View_Form_Widget;
typedef struct _Elm_View_Form_Promise Elm_View_Form_Promise;

/**
 * @brief Local-use callbacks
 */
struct _Elm_View_Form_Data
{
   Eo *model;
   Eina_List *links; // List of linked object
};

static void
_link_dying(void* data, Efl_Event const* event)
{
   Elm_View_Form_Data *priv = data;

   priv->links = eina_list_remove(priv->links, event->object);
}

/**
 * @brief Add new widget object.
 *    Adds new widget object on the list
 *    and perform initial setup.
 */
static Eina_Bool
_elm_view_widget_add(Elm_View_Form_Data *priv, const char *property, Evas_Object *link)
{
   if (!efl_isa(link, EFL_UI_PROPERTY_BIND_INTERFACE)) return EINA_FALSE;
   if (!property) property = "default";

   efl_ui_view_model_set(link, priv->model);
   efl_ui_property_bind(link, "default", property);
   efl_event_callback_add(link, EFL_EVENT_DEL, _link_dying, priv);

   priv->links = eina_list_append(priv->links, link);

   return EINA_TRUE;
}
/**
 * Helper functions - End
 */

/**
 * @brief destructor
 */
static void
_elm_view_form_efl_object_destructor(Eo *obj, Elm_View_Form_Data *priv)
{
   priv->links = eina_list_free(priv->links);
   efl_unref(priv->model);
   priv->model = NULL;

   efl_destructor(efl_super(obj, MY_CLASS));
}


static void
_elm_view_form_model_set(Eo *obj EINA_UNUSED, Elm_View_Form_Data *priv, Eo *model)
{
   Efl_Object *link;
   Eina_List *l;

   efl_replace(&priv->model, model);

   EINA_LIST_FOREACH(priv->links, l, link)
     efl_ui_view_model_set(link, priv->model);
}

static void
_elm_view_form_widget_add(Eo *obj EINA_UNUSED, Elm_View_Form_Data *priv, const char *propname, Evas_Object *evas)
{
   EINA_SAFETY_ON_NULL_RETURN(evas);

   _elm_view_widget_add(priv, propname, evas);
}

#include "elm_view_form.eo.c"
