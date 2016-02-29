#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include <Eo.h>
#include <Efl.h>
#include <Elementary.h>

#include "elm_view_form.h"
#include "elm_view_list.h"

#include "elm_priv.h"
#include <assert.h>

#define MY_CLASS ELM_VIEW_FORM_CLASS
#define MY_CLASS_NAME "View_Form"

typedef struct _Elm_View_Form_Data Elm_View_Form_Data;
typedef struct _Elm_View_Form_Widget Elm_View_Form_Widget;

/**
 * @brief Local-use callbacks
 */
typedef void (*Elm_View_Form_Event_Cb)(Elm_View_Form_Widget *, Elm_View_Form_Data *, Evas_Object *);
typedef void (*Elm_View_Form_Widget_Object_Set_Cb)(Eo *, Evas_Object *, const Eina_Value *, const char *);

struct _Elm_View_Form_Widget
{
   Eina_Stringshare *widget_propname;
   Evas_Object *widget_obj;
   Elm_View_Form_Event_Cb widget_obj_value_update_cb;
   Elm_View_Form_Widget_Object_Set_Cb widget_obj_set_cb;
};

struct _Elm_View_Form_Data
{
   Eo *model_obj;
   Eina_Value *properties;
   Eina_List *l;
};

static Eina_Bool
_efl_model_properties_change_cb(void *data, const Eo_Event *event)
{
   const Efl_Model_Property_Event *evt = event->event_info;
   const Eina_Value *value;
   const char *prop;
   unsigned int i;
   Elm_View_Form_Data *priv = data;
   Eina_List *l = NULL;
   Elm_View_Form_Widget *w = NULL;
   Eina_Array_Iterator it;

   EINA_SAFETY_ON_NULL_RETURN_VAL(priv, EINA_TRUE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(evt, EINA_TRUE);

   if (!evt->changed_properties)
     return EINA_TRUE;

   //update all widgets with this property
   EINA_LIST_FOREACH(priv->l, l, w)
     {
        EINA_ARRAY_ITER_NEXT(evt->changed_properties, i, prop, it)
          {
            if (!strcmp(w->widget_propname, prop))
              {
                 eo_do(priv->model_obj, efl_model_property_get(prop, &value));
                 w->widget_obj_set_cb(priv->model_obj, w->widget_obj, value, w->widget_propname);
              }
          }
     }

   return EINA_TRUE;
}

static void
_update_model_properties(Elm_View_Form_Data *priv)
{
   const Eina_Value *value;
   Eina_List *l;
   Elm_View_Form_Widget *w;
   //update all widgets property
   EINA_LIST_FOREACH(priv->l, l, w)
     {
        eo_do(priv->model_obj, efl_model_property_get(w->widget_propname, &value));
        w->widget_obj_set_cb(priv->model_obj, w->widget_obj, value, w->widget_propname);
     }
}

/**
 * @brief Set widget.
 * Works, so far, for widget(s): Entry, Label
 */
static void
_elm_evas_object_text_set_cb(Eo *obj EINA_UNUSED, Evas_Object *widget, const Eina_Value *value, const char *propname EINA_UNUSED)
{
   const char *c_text = NULL;
   char *text = NULL;

   EINA_SAFETY_ON_NULL_RETURN(value);
   EINA_SAFETY_ON_NULL_RETURN(widget);

   text = eina_value_to_string(value);
   EINA_SAFETY_ON_NULL_RETURN(text);

   c_text = elm_object_text_get(widget);
   EINA_SAFETY_ON_NULL_RETURN(c_text);

   if (strcmp(text, c_text) != 0)
     {
         elm_object_text_set(widget, text);
     }
   free(text);
}

static void
_elm_evas_object_thumb_set_cb(Eo *obj EINA_UNUSED, Evas_Object *thumb, const Eina_Value *value, const char *propname EINA_UNUSED)
{
   char *filename = NULL;

   EINA_SAFETY_ON_NULL_RETURN(value);
   filename = eina_value_to_string(value);
   EINA_SAFETY_ON_NULL_RETURN(filename);
   if (strlen(filename) < PATH_MAX)
     {
        elm_thumb_file_set(thumb, filename, NULL);
        elm_thumb_reload(thumb);
     }
   free(filename);
}

/**
 * @brief Evas object callback implementation.
 *    Updates Widget's value if not the same object
 *    and the widget itself.
 */
static void
_elm_evas_object_text_changed_cb(void *data, Evas *e EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   Eina_Value value;
   Eina_List *l;
   Elm_View_Form_Data *priv = (Elm_View_Form_Data *)data;
   Elm_View_Form_Widget *w = NULL;

   EINA_LIST_FOREACH(priv->l, l, w)
     {
        if (w->widget_obj == obj)
           break;
     }

   EINA_SAFETY_ON_NULL_RETURN(w);
   eina_value_setup(&value, EINA_VALUE_TYPE_STRING);
   eina_value_set(&value, elm_object_text_get(obj));
   eo_do(priv->model_obj, efl_model_property_set(w->widget_propname, &value));
   eina_value_flush(&value);
}
/**
 * @brief Add new widget object.
 *    Adds new widget object on the list
 *    and perform initial setup.
 */
static Eina_Bool
_elm_view_widget_add(Elm_View_Form_Data *priv, const char *propname, Evas_Object *widget_obj)
{
   const Eina_Value *value = NULL;
   Elm_View_Form_Widget *w = calloc(1, sizeof(Elm_View_Form_Widget));
   EINA_SAFETY_ON_NULL_RETURN_VAL(w, EINA_FALSE);

   w->widget_propname = eina_stringshare_add(propname);
   w->widget_obj = widget_obj;
   priv->l = eina_list_append(priv->l, w);

   if(eo_isa(widget_obj, ELM_ENTRY_CLASS))
     {
        w->widget_obj_set_cb = _elm_evas_object_text_set_cb;
        evas_object_event_callback_add(w->widget_obj, EVAS_CALLBACK_KEY_DOWN, _elm_evas_object_text_changed_cb, priv);
     }
   else if(eo_isa(widget_obj, ELM_LABEL_CLASS))
     {
        w->widget_obj_set_cb = _elm_evas_object_text_set_cb;
     }
   else if(eo_isa(widget_obj, ELM_THUMB_CLASS))
     {
        w->widget_obj_set_cb = _elm_evas_object_thumb_set_cb;
     }
   else
     {
        // Widget yet not supported
        EINA_SAFETY_ON_NULL_RETURN_VAL(NULL, EINA_FALSE);
     }

   eo_do(priv->model_obj, efl_model_property_get(propname, &value));
   if (value)
     {
         w->widget_obj_set_cb(priv->model_obj, w->widget_obj, value, w->widget_propname);
     }

   return EINA_TRUE;
}
/**
 * Helper functions - End
 */


/**
 * @brief constructor
 */
static Eo_Base*
_elm_view_form_eo_base_constructor(Eo *obj EINA_UNUSED, Elm_View_Form_Data *_pd)
{
   Elm_View_Form_Data *priv = (Elm_View_Form_Data *)_pd;
   priv->model_obj = NULL;

   eo_do_super(obj, MY_CLASS, eo_constructor());
   
   return obj;
}

/**
 * @brief destructor
 */
static void
_elm_view_form_eo_base_destructor(Eo *obj, Elm_View_Form_Data *priv)
{
   Elm_View_Form_Widget *w = NULL;
   EINA_LIST_FREE(priv->l, w)
     {
        eina_stringshare_del(w->widget_propname);
        free(w);
     }

   eo_do_super(obj, MY_CLASS, eo_destructor());
}


static void
_elm_view_form_model_set(Eo *obj EINA_UNUSED, Elm_View_Form_Data *priv, Eo *model)
{
   if (priv->model_obj != NULL)
     {
        eo_do(priv->model_obj, eo_event_callback_del(EFL_MODEL_BASE_EVENT_PROPERTIES_CHANGED, _efl_model_properties_change_cb, priv));
        eo_unref(priv->model_obj);
     }

   priv->model_obj = model;

   if (priv->model_obj != NULL)
     {
        eo_ref(priv->model_obj);
        eo_do(priv->model_obj, eo_event_callback_add(EFL_MODEL_BASE_EVENT_PROPERTIES_CHANGED, _efl_model_properties_change_cb, priv));
        _update_model_properties(priv);
     }
}

static void
_elm_view_form_widget_add(Eo *obj EINA_UNUSED, Elm_View_Form_Data *priv, const char *propname, Evas_Object *evas)
{
   Eina_Bool status;

   EINA_SAFETY_ON_NULL_RETURN(evas);
   EINA_SAFETY_ON_NULL_RETURN(propname);

   status = _elm_view_widget_add(priv, propname, evas);
   EINA_SAFETY_ON_FALSE_RETURN(status);
}

#include "elm_view_form.eo.c"
