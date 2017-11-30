#include "efl_ui_item_priv.h"

#define MY_CLASS EFL_UI_ITEM_CLASS

typedef struct {
   Efl_Ui_Item      *self;
   Efl_Ui_Item_List *parent;
   Efl_Model        *model;

   Eina_Stringshare *text, *icon;
   Elm_Image        *icon_obj;

   Eina_List        *pending_futures; // Efl_Future** (wrefs)

   Eina_Bool         model_owned : 1;
   Eina_Bool         selected : 1;
} Efl_Ui_Item_Data;

typedef struct {
   Efl_Ui_Item *object;
   Eina_Array  *properties; // stringshare
} Property_Update_Data;

Eina_Value _eina_value_helper = EINA_VALUE_EMPTY;

static inline Eina_Bool
_selected_set(Eo *obj, Efl_Ui_Item_Data *pd, Eina_Bool sel)
{
   if (pd->selected == sel)
     return EINA_FALSE;

   pd->selected = sel;
   if (efl_finalized_get(obj))
     {
        if (sel) efl_event_callback_call(obj, EFL_UI_EVENT_SELECTED, NULL);
        else efl_event_callback_call(obj, EFL_UI_EVENT_UNSELECTED, NULL);
     }
   return EINA_TRUE;
}

EOLIAN static void
_efl_ui_item_selected_set(Eo *obj, Efl_Ui_Item_Data *pd, Eina_Bool sel)
{
   if (_selected_set(obj, pd, !!sel))
     efl_model_property_set(obj, "selected", _value_bool(sel));
}

EOLIAN static Eina_Bool
_efl_ui_item_selected_get(Eo *obj EINA_UNUSED, Efl_Ui_Item_Data *pd)
{
   return pd->selected;
}

Efl_Ui_Item_List *
_efl_ui_item_item_list_get(Eo *obj EINA_UNUSED, Efl_Ui_Item_Data *pd)
{
   return pd->parent;
}

static Eina_Bool
_text_set(Eo *obj, Efl_Ui_Item_Data *pd, const char *text)
{
   if (!eina_stringshare_replace(&pd->text, text))
     return EINA_FALSE;

   efl_text_set(efl_part(efl_ui_item_view_get(obj), "elm.text"), text);
   return EINA_TRUE;
}

EOLIAN static void
_efl_ui_item_efl_text_text_set(Eo *obj, Efl_Ui_Item_Data *pd, const char *text)
{
   if (!eina_streq(text, pd->text))
     efl_model_property_set(obj, "text", _value_string(text));
}

EOLIAN static const char *
_efl_ui_item_efl_text_text_get(Eo *obj EINA_UNUSED, Efl_Ui_Item_Data *pd)
{
   return pd->text;
}

static Eina_Bool
_icon_set(Efl_Ui_Item_Data *pd, const char *icon)
{
   Eo *obj = pd->self;

   if (!eina_stringshare_replace(&pd->icon, icon))
     return EINA_FALSE;

   if (!icon)
     {
        ELM_SAFE_DEL(pd->icon_obj);
        return EINA_TRUE;
     }

   if (!pd->icon_obj || !eina_streq(efl_ui_image_icon_get(pd->icon_obj), icon))
     {
        if (!pd->icon_obj)
          {
             Eo *view = efl_ui_item_view_get(obj);
             pd->icon_obj = efl_add(EFL_UI_IMAGE_CLASS, view);
             efl_content_set(efl_part(view, "elm.swallow.icon"), pd->icon_obj);
             efl_wref_add(pd->icon_obj, &pd->icon_obj);
          }
        efl_ui_image_icon_set(pd->icon_obj, icon);
     }

   return EINA_TRUE;
}

EOLIAN static void
_efl_ui_item_icon_set(Eo *obj, Efl_Ui_Item_Data *pd, const char *icon)
{
   if (!eina_streq(icon, pd->icon))
     efl_model_property_set(obj, "icon", _value_string(icon));
}

EOLIAN static const char *
_efl_ui_item_icon_get(Eo *obj EINA_UNUSED, Efl_Ui_Item_Data *pd)
{
   return pd->icon;
}

static void
_property_update_free(Property_Update_Data *update)
{
   Eina_Array_Iterator it;
   Eina_Stringshare *prop;
   size_t k;

   if (!update) return;
   EINA_ARRAY_ITER_NEXT(update->properties, k, prop, it)
   {
      eina_stringshare_del(prop);
   }
   eina_array_free(update->properties);
   free(update);
}

#define EINA_VALUE_CHECK(_val, _type) \
   EINA_SAFETY_ON_FALSE_GOTO(eina_value_type_get(_val) == _type, end)

static void
_pending_future_remove(Eo *obj EINA_UNUSED, Efl_Ui_Item_Data *pd, Efl_Future *future)
{
   Efl_Future **fut;
   Eina_List *li;

   EINA_LIST_FOREACH(pd->pending_futures, li, fut)
     if (*fut == future)
       {
          // WTF? where is the unuse function??
          pd->pending_futures = eina_list_remove_list(pd->pending_futures, li);
          efl_wref_del(*fut, fut);
          free(future);
          break;
       }
}

static void
_properties_changed_success(void *data, const Efl_Event *event)
{
   Efl_Future_Event_Success *ev = event->info;
   Property_Update_Data *update = data;
   Eina_Accessor *prop_accessor = ev->value;
   Efl_Ui_Item_Data *pd;
   Efl_Ui_Item *obj;
   Eina_Array_Iterator array_itr;
   Eina_Stringshare *prop;
   Eina_Value *val;
   unsigned k;

   obj = update->object;
   pd = efl_data_scope_safe_get(obj, MY_CLASS);
   EINA_SAFETY_ON_NULL_GOTO(pd, end);

   _pending_future_remove(obj, pd, event->object);

   EINA_ARRAY_ITER_NEXT(update->properties, k, prop, array_itr)
   {
      if (!eina_accessor_data_get(prop_accessor, k, (void **) &val) || !val)
        {
           ERR("Invalid response from model");
           break;
        }

      if (eina_streq(prop, "selected"))
        {
           EINA_VALUE_CHECK(val, EINA_VALUE_TYPE_UCHAR);
           _selected_set(obj, pd, _value_as_bool(val));
        }
      else if (eina_streq(prop, "text"))
        {
           EINA_VALUE_CHECK(val, EINA_VALUE_TYPE_STRING);
           _text_set(obj, pd, _value_as_string(val));
        }
      else if (eina_streq(prop, "icon"))
        {
           EINA_VALUE_CHECK(val, EINA_VALUE_TYPE_STRING);
           _icon_set(pd, _value_as_string(val));
        }
   }

end:
   _property_update_free(update);
}

static void
_properties_changed_error(void *data EINA_UNUSED, const Efl_Event *event EINA_UNUSED)
{
   //Efl_Future_Event_Failure *ev = event->info;
   Property_Update_Data *update = data;
   Efl_Ui_Item_Data *pd;
   Efl_Ui_Item *obj;

   obj = update->object;
   pd = efl_data_scope_safe_get(obj, MY_CLASS);
   EINA_SAFETY_ON_NULL_GOTO(pd, end);

   _pending_future_remove(obj, pd, event->object);

end:
   ERR("Model error!");
   _property_update_free(update);

}

static void
_properties_update(Eo *obj, Efl_Ui_Item_Data *pd, Eina_Array *properties)
{
   Property_Update_Data *update;
   Efl_Future **futures, *all, **wfut;
   Eina_Array_Iterator it;
   const char *prop;
   unsigned k, len;

   if (!properties) return;
   len = eina_array_count(properties);
   if (!len) return;

   futures = calloc(len + 1, sizeof(futures[0]));
   update = calloc(1, sizeof(*update));
   update->object = obj;
   update->properties = eina_array_new(len);

   EINA_ARRAY_ITER_NEXT(properties, k, prop, it)
   {
      Efl_Future *future = efl_model_property_get(obj, prop);
      eina_array_push(update->properties, eina_stringshare_add(prop));
      futures[k] = future;
   }

   all = efl_future_iterator_all(eina_carray_iterator_new((void **) futures));
   efl_future_then(all, _properties_changed_success,
                   _properties_changed_error, NULL, update);
   free(futures);

   wfut = calloc(1, sizeof(*wfut));
   efl_future_use(wfut, all);
   pd->pending_futures = eina_list_append(pd->pending_futures, wfut);
}

static void
_properties_changed_cb(void *data, const Efl_Event *event)
{
   Efl_Model_Property_Event *ev = event->info;
   //Efl_Model *model = event->object;
   Efl_Ui_Item_Data *pd;
   Efl_Ui_Item *obj = data;
   Eina_Array_Iterator it;
   const char *prop;
   unsigned k;

   pd = efl_data_scope_safe_get(obj, MY_CLASS);
   EINA_SAFETY_ON_NULL_RETURN(pd);

   if (ev->invalidated_properties)
     {
        EINA_ARRAY_ITER_NEXT(ev->invalidated_properties, k, prop, it)
        {
           if (eina_streq(prop, "selected"))
             _selected_set(obj, pd, EINA_FALSE);
           else if (eina_streq(prop, "text"))
             eina_stringshare_replace(&pd->text, NULL);
           else if (eina_streq(prop, "icon"))
             eina_stringshare_replace(&pd->icon, NULL);
        }
     }

   _properties_update(obj, pd, ev->changed_properties);
}

EOLIAN static void
_efl_ui_item_model_set(Eo *obj, Efl_Ui_Item_Data *pd, Efl_Model *model)
{
   Eina_Array *properties;
   Efl_Future **wfut;

   EINA_SAFETY_ON_NULL_RETURN(model);
   if (pd->model == model) return;

   if (pd->model)
     {
        efl_event_callback_del(pd->model, EFL_MODEL_EVENT_PROPERTIES_CHANGED,
                               _properties_changed_cb, obj);
        efl_composite_detach(obj, pd->model);
        if (pd->model_owned)
          efl_del(pd->model);
     }

   EINA_LIST_FREE(pd->pending_futures, wfut)
     {
        efl_future_cancel(*wfut);
        free(wfut);
     }

   pd->model = model;
   pd->model_owned = EINA_FALSE;
   if (!model) return;

   efl_composite_attach(obj, pd->model);
   efl_event_callback_add(pd->model, EFL_MODEL_EVENT_PROPERTIES_CHANGED,
                          _properties_changed_cb, obj);

   properties = eina_array_new(8);
   eina_array_push(properties, "text");
   eina_array_push(properties, "icon");
   eina_array_push(properties, "selected");
   _properties_update(obj, pd, properties);
   eina_array_free(properties);
}

EOLIAN static Efl_Model *
_efl_ui_item_model_get(Eo *obj EINA_UNUSED, Efl_Ui_Item_Data *pd)
{
   return pd->model;
}

EOLIAN static Efl_Object *
_efl_ui_item_efl_object_constructor(Eo *obj, Efl_Ui_Item_Data *pd)
{
   pd->self = obj;
   obj = efl_constructor(efl_super(obj, MY_CLASS));

   pd->model_owned = EINA_TRUE;
   efl_ui_item_model_set(obj, efl_add(EFL_MODEL_ITEM_CLASS, obj));

   return obj;
}

#include "efl_ui_item.eo.c"
