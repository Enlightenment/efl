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
typedef struct _Elm_View_Form_Promise Elm_View_Form_Promise;

/**
 * @brief Local-use callbacks
 */
typedef void (*Elm_View_Form_Event_Cb)(Elm_View_Form_Widget *, Elm_View_Form_Data *, Evas_Object *);
typedef void (*Elm_View_Form_Widget_Object_Set_Cb)(Evas_Object *, const Eina_Value *, const char *);

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
   Eina_List *widgets;
};

struct _Elm_View_Form_Promise
{
   Elm_View_Form_Data *priv;
   Eina_Stringshare *property_name;
};


static void
_efl_promise_then_widget(void* data, void* v)
{
    Elm_View_Form_Widget *w = data;
    Eina_Value *value = v;
    w->widget_obj_set_cb(w->widget_obj, value, w->widget_propname);
}

static void
_efl_promise_error_widget(void *data EINA_UNUSED, Eina_Error err EINA_UNUSED)
{
}

static void
_efl_model_promise_then_cb(void* data, void* v)
{
   Elm_View_Form_Promise *p = data;
   Eina_Value *value = v;
   Elm_View_Form_Data *priv = p->priv;
   Elm_View_Form_Widget *w = NULL;
   Eina_List *l = NULL;

   EINA_SAFETY_ON_NULL_RETURN(p);

   EINA_LIST_FOREACH(priv->widgets, l, w)
     {
        if (!strcmp(w->widget_propname, p->property_name))
          {
             w->widget_obj_set_cb(w->widget_obj, value, w->widget_propname);
          }
     }

   eina_stringshare_del(p->property_name);
   free(p);
}

static void
_efl_model_promise_error_cb(void* data, Eina_Error error EINA_UNUSED)
{
   Elm_View_Form_Promise *p = data;
   EINA_SAFETY_ON_NULL_RETURN(p);

   eina_stringshare_del(p->property_name);
   free(p);
}

static void
_efl_model_properties_change_cb(void *data, const Eo_Event *event)
{
   const Efl_Model_Property_Event *evt = event->info;
   Eina_Promise *promise;
   const char *prop;
   unsigned int i;
   Elm_View_Form_Data *priv = data;
   Elm_View_Form_Promise *p = NULL;
   Eina_Array_Iterator it;

   EINA_SAFETY_ON_NULL_RETURN(priv);
   EINA_SAFETY_ON_NULL_RETURN(evt);

   if (!evt->changed_properties)
     return;

   //update all widgets with this property
   EINA_ARRAY_ITER_NEXT(evt->changed_properties, i, prop, it)
     {
        p = calloc(1, sizeof(Elm_View_Form_Promise));
        p->property_name = eina_stringshare_add(prop);
        p->priv = priv;
        promise = efl_model_property_get(priv->model_obj, prop);
        eina_promise_then(promise, &_efl_model_promise_then_cb,
                          &_efl_model_promise_error_cb, p);
     }
}

static void
_update_model_properties(Elm_View_Form_Data *priv)
{
   Eina_List *l;
   Elm_View_Form_Widget *w;
   Eina_Promise *promise;
   //update all widgets property
   if (priv->model_obj == NULL)
     return;

   EINA_LIST_FOREACH(priv->widgets, l, w)
     {
        promise = efl_model_property_get(priv->model_obj, w->widget_propname);
        eina_promise_then(promise, &_efl_promise_then_widget, &_efl_promise_error_widget, w);
     }
}

/**
 * @brief Set widget.
 * Works, so far, for widget(s): Entry, Label
 */
static void
_elm_evas_object_text_set_cb(Evas_Object *widget, const Eina_Value *value, const char *propname EINA_UNUSED)
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
_elm_evas_object_thumb_set_cb(Evas_Object *thumb, const Eina_Value *value, const char *propname EINA_UNUSED)
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

   EINA_LIST_FOREACH(priv->widgets, l, w)
     {
        if (w->widget_obj == obj)
           break;
     }

   EINA_SAFETY_ON_NULL_RETURN(w);
   eina_value_setup(&value, EINA_VALUE_TYPE_STRING);
   eina_value_set(&value, elm_object_text_get(obj));
   efl_model_property_set(priv->model_obj, w->widget_propname, &value, NULL);
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
   Eina_Promise *promise = NULL;
   Elm_View_Form_Widget *w = calloc(1, sizeof(Elm_View_Form_Widget));
   EINA_SAFETY_ON_NULL_RETURN_VAL(w, EINA_FALSE);

   w->widget_propname = eina_stringshare_add(propname);
   w->widget_obj = widget_obj;
   priv->widgets = eina_list_append(priv->widgets, w);

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

   if (priv->model_obj != NULL)
     {
         promise = efl_model_property_get(priv->model_obj, w->widget_propname);
         eina_promise_then(promise, &_efl_promise_then_widget,
                           &_efl_promise_error_widget, priv);
     }
   return EINA_TRUE;
}
/**
 * Helper functions - End
 */


/**
 * @brief constructor
 */
static Efl_Object*
_elm_view_form_efl_object_constructor(Eo *obj EINA_UNUSED, Elm_View_Form_Data *_pd)
{
   Elm_View_Form_Data *priv = (Elm_View_Form_Data *)_pd;
   priv->model_obj = NULL;

   efl_constructor(eo_super(obj, MY_CLASS));

   return obj;
}

/**
 * @brief destructor
 */
static void
_elm_view_form_efl_object_destructor(Eo *obj, Elm_View_Form_Data *priv)
{
   Elm_View_Form_Widget *w = NULL;
   EINA_LIST_FREE(priv->widgets, w)
     {
        eina_stringshare_del(w->widget_propname);
        free(w);
        w = NULL;
     }
   priv->widgets = NULL;

   efl_destructor(eo_super(obj, MY_CLASS));
}


static void
_elm_view_form_model_set(Eo *obj EINA_UNUSED, Elm_View_Form_Data *priv, Eo *model)
{
   if (priv->model_obj != NULL)
     {
        efl_event_callback_del(priv->model_obj, EFL_MODEL_EVENT_PROPERTIES_CHANGED, _efl_model_properties_change_cb, priv);
        eo_unref(priv->model_obj);
     }

   priv->model_obj = model;

   if (priv->model_obj != NULL)
     {
        eo_ref(priv->model_obj);
        efl_event_callback_add(priv->model_obj, EFL_MODEL_EVENT_PROPERTIES_CHANGED, _efl_model_properties_change_cb, priv);
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
