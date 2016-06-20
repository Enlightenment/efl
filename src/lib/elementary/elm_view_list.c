#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include <Eo.h>
#include <Efl.h>
#include <Elementary.h>

#include "elm_priv.h"
#include <assert.h>

#define MY_CLASS ELM_VIEW_LIST_CLASS
#define MY_CLASS_NAME "View List"

struct _Elm_View_List_Data;
typedef struct _Elm_View_List_Data Elm_View_List_Data;

struct _View_List_ItemData;
typedef struct _View_List_ItemData View_List_ItemData;

struct _View_List_ValueItem;
typedef struct _View_List_ValueItem View_List_ValueItem;

struct _Elm_View_List_Data
{
   Eo *view;
   Evas_Object *genlist;
   View_List_ItemData *rootdata;
   Elm_Genlist_Item_Class *itc;
   Elm_Genlist_Item_Type itype;
   Eina_Hash *prop_con;
   Eo *model;
};

struct _View_List_ItemData
{
   Elm_View_List_Data *priv;
   Elm_Object_Item *item;
   Eo *model;
   View_List_ItemData *parent;
   Eina_List *values;
};

struct _View_List_ValueItem
{
   Eina_Stringshare *part;
   Eina_Value *value;
   Elm_Object_Item *item;
};

static void _efl_model_load_children(View_List_ItemData *);
static void _efl_model_children_count_change_cb(void *, const Eo_Event *event);
static void _efl_model_properties_change_cb(void *, const Eo_Event *event);

static void _expand_request_cb(void *data EINA_UNUSED, const Eo_Event *event);
static void _contract_request_cb(void *data EINA_UNUSED, const Eo_Event *event);
static void _contracted_cb(void *data EINA_UNUSED, const Eo_Event *event);

/* --- Genlist Callbacks --- */
EO_CALLBACKS_ARRAY_DEFINE(model_callbacks,
                          { EFL_MODEL_EVENT_CHILDREN_COUNT_CHANGED, _efl_model_children_count_change_cb });
EO_CALLBACKS_ARRAY_DEFINE(genlist_callbacks,
                          { ELM_GENLIST_EVENT_EXPAND_REQUEST, _expand_request_cb },
                          { ELM_GENLIST_EVENT_CONTRACT_REQUEST, _contract_request_cb },
                          { ELM_GENLIST_EVENT_CONTRACTED, _contracted_cb });

static void
_item_sel_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   View_List_ItemData *idata = data;

   EINA_SAFETY_ON_NULL_RETURN(idata);

   eo_event_callback_call(idata->priv->view, ELM_VIEW_LIST_EVENT_MODEL_SELECTED, idata->model);
}

static void
_item_del(void *data, Evas_Object *obj EINA_UNUSED)
{
   View_List_ItemData *idata = data;
   if (!idata)
      return;

   eo_event_callback_array_del(idata->model, model_callbacks(), idata);
   eo_event_callback_del(idata->model, EFL_MODEL_EVENT_PROPERTIES_CHANGED, _efl_model_properties_change_cb, idata);

   eo_unref(idata->model);
   idata->model = NULL;
   idata->item = NULL;
   idata->parent = NULL;
   idata->priv = NULL;

   free(idata);
}

static void
_property_get_cb(void* data, void* v)
{
    View_List_ValueItem *vitem = data;
    Eina_Value *value = v;
    vitem->value = eina_value_new(eina_value_type_get(value));
    eina_value_copy(value, vitem->value);

    if (vitem->item)
      elm_genlist_item_update(vitem->item);
}

static void
_property_get_error_cb(void* data, Eina_Error err EINA_UNUSED)
{
    View_List_ValueItem *vitem = data;
    eina_stringshare_del(vitem->part);
}

static Eina_Value *
_item_get_value(View_List_ItemData *idata, const char *part)
{
   View_List_ValueItem *vitem = NULL;
   Eina_Value *value = NULL;
   Eina_List *l, *ln;

   EINA_LIST_FOREACH_SAFE(idata->values, l, ln, vitem)
     {
         if (vitem->part == NULL)
           {
              free(vitem);
              unsigned i = eina_list_count(idata->values);
              if (i == 1)
                {
                   idata->values = eina_list_remove(idata->values, vitem);
                   idata->values = NULL;
                   break;
                }
              idata->values = eina_list_remove_list(idata->values, l);
              continue;
           }

         if (strcmp(vitem->part, part) == 0)
           {
               value = vitem->value;
               break;
           }
     }

   if (value == NULL)
     {
         Eina_Promise *promise;
         vitem = calloc(1, sizeof(View_List_ItemData));
         const char *prop = eina_hash_find(idata->priv->prop_con, part);

         if (prop == NULL) prop = part;

         vitem->part = eina_stringshare_add(part);
         vitem->item = idata->item;
         idata->values = eina_list_append(idata->values, vitem);
         promise = efl_model_property_get(idata->model, prop);
         eina_promise_then(promise, &_property_get_cb,
                           &_property_get_error_cb, vitem);
     }
   else
     {
         unsigned i = eina_list_count(idata->values);
         idata->values = eina_list_remove(idata->values, vitem);
         if (i == 1) idata->values = NULL;

         eina_stringshare_del(vitem->part);
         free(vitem);
     }

   return value;
}

static Evas_Object *
_item_content_get(void *data, Evas_Object *obj EINA_UNUSED, const char *part)
{
   const Eina_Value_Type *vtype;
   Evas_Object *content = NULL;
   View_List_ItemData *idata = data;
   EINA_SAFETY_ON_NULL_RETURN_VAL(data, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(part, NULL);

   if (!idata->item)
     return NULL;

   Eina_Value *value = _item_get_value(idata, part);

   if (value == NULL)
     return NULL;

   vtype = eina_value_type_get(value);
   if (vtype == EINA_VALUE_TYPE_STRING || vtype == EINA_VALUE_TYPE_STRINGSHARE)
     {
         char *content_s = NULL;
         content_s = eina_value_to_string(value);
         content = elm_icon_add(obj);
         if (elm_icon_standard_set(content, content_s))
           {
               evas_object_size_hint_aspect_set(content, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
           }
         else
           {
               evas_object_del(content);
               content = NULL;
           }
         free(content_s);
     }
   else if (vtype == EINA_VALUE_TYPE_BLOB)
     {
         Eina_Value_Blob out;
         eina_value_get(value, &out);
         if (out.memory != NULL)
           {
               content = elm_image_add(obj);
               //XXX: need copy memory??
               elm_image_memfile_set(content, out.memory, out.size, NULL, NULL);
           }
     }
   eina_value_flush(value);

   return content;
}

static char *
_item_text_get(void *data, Evas_Object *obj EINA_UNUSED, const char *part)
{
   char *text = NULL;
   View_List_ItemData *idata = data;

   EINA_SAFETY_ON_NULL_RETURN_VAL(data, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(part, NULL);
   if (!idata->item)
     return text;

   Eina_Value *value = _item_get_value(idata, part);

   if (value)
     {
        text = eina_value_to_string(value);
        eina_value_flush(value);
     }

   return text;
}

static void
_expand_request_cb(void *data EINA_UNUSED, const Eo_Event *event)
{
   Elm_Object_Item *item = event->info;
   View_List_ItemData *idata = elm_object_item_data_get(item);

   EINA_SAFETY_ON_NULL_RETURN(idata);

   eo_event_callback_array_add(idata->model, model_callbacks(), idata);

   _efl_model_load_children(idata);
}

static void
_contract_request_cb(void *data EINA_UNUSED, const Eo_Event *event)
{
   Elm_Object_Item *item = event->info;
   View_List_ItemData *idata = elm_object_item_data_get(item);

   eo_event_callback_array_del(idata->model, model_callbacks(), idata);
   elm_genlist_item_expanded_set(item, EINA_FALSE);
}

static void
_contracted_cb(void *data EINA_UNUSED, const Eo_Event *event)
{
   Elm_Object_Item *glit = event->info;
   elm_genlist_item_subitems_clear(glit);
}

static void
_genlist_deleted(void *data, Evas *e EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   Elm_View_List_Data *priv = data;

   if (priv && priv->genlist && priv->genlist == obj)
     {
        eo_event_callback_array_del(priv->genlist, genlist_callbacks(), priv);
        eo_unref(priv->genlist);
        priv->genlist = NULL;
     }
}

/* --- Efl_Model Callbacks --- */
static void
_efl_model_properties_change_cb(void *data, const Eo_Event *event)
{
   View_List_ItemData *idata = data;
   Efl_Model_Property_Event *evt = event->info;

   EINA_SAFETY_ON_NULL_RETURN(idata);
   EINA_SAFETY_ON_NULL_RETURN(evt);

   if (idata->item)
     elm_genlist_item_update(idata->item);
}

static void
_efl_model_load_children_then(void * data, void* value)
{
   View_List_ItemData *pdata = data;
   Eina_Accessor *accessor = value;
   Eo *child;
   unsigned i = 0;
   EINA_SAFETY_ON_NULL_RETURN(pdata);
   EINA_SAFETY_ON_NULL_RETURN(pdata->priv);
   EINA_SAFETY_ON_NULL_RETURN(accessor);

   Elm_View_List_Data *priv = pdata->priv;

   EINA_ACCESSOR_FOREACH(accessor, i, child)
     {
        View_List_ItemData *idata = calloc(1, sizeof(View_List_ItemData));
        EINA_SAFETY_ON_NULL_RETURN(idata);
        idata->priv = priv;
        idata->parent = pdata;
        idata->model = child;
        eo_ref(child);
        eo_event_callback_add(child, EFL_MODEL_EVENT_PROPERTIES_CHANGED, _efl_model_properties_change_cb, idata);
        idata->item = elm_genlist_item_append(priv->genlist, priv->itc, idata, pdata->item,
                                                       priv->itype, _item_sel_cb, idata);
     }

   if (i > 0 && pdata->item)
     elm_genlist_item_expanded_set(pdata->item, EINA_TRUE);
}

static void
_efl_model_load_children(View_List_ItemData *pdata)
{
   Eina_Promise *promise;

   promise = efl_model_children_slice_get(pdata->model, 0, 0);
   eina_promise_then(promise, &_efl_model_load_children_then, NULL, pdata);
}

static void
_efl_model_children_count_change_cb(void *data, const Eo_Event *event EINA_UNUSED)
{
   View_List_ItemData *idata = data;
   EINA_SAFETY_ON_NULL_RETURN(idata);
   EINA_SAFETY_ON_NULL_RETURN(idata->priv);
   EINA_SAFETY_ON_NULL_RETURN(idata->priv->genlist);

   elm_genlist_item_subitems_clear(idata->item);

   _efl_model_load_children(idata);
}

static void
_priv_model_set(Elm_View_List_Data *priv, Eo *model)
{
   if (priv->model != NULL)
     {
         eo_event_callback_array_del(priv->model, model_callbacks(), priv->rootdata);
         elm_obj_genlist_clear(priv->genlist);
         eo_unref(priv->model);
         priv->model = NULL;
     }

   if (model == NULL)
     return;

   priv->model = model;
   priv->rootdata->model = priv->model;
   eo_ref(priv->model);

   eo_event_callback_array_add(priv->model, model_callbacks(), priv->rootdata);
   _efl_model_load_children(priv->rootdata);
}

/**
 * @brief Elm View List Class impl.
 */
static void
_elm_view_list_genlist_set(Eo *obj, Elm_View_List_Data *priv, Evas_Object *genlist,
                Elm_Genlist_Item_Type itype, const char *istyle)
{
   priv->view = obj;
   priv->genlist = genlist;
   priv->itype = itype;
   EINA_SAFETY_ON_NULL_RETURN(priv->genlist);
   eo_ref(priv->genlist);

   priv->rootdata = calloc(1, sizeof(View_List_ItemData));
   priv->rootdata->priv = priv;

   priv->itc = elm_genlist_item_class_new();
   if (istyle)
     priv->itc->item_style = strdup(istyle);
   priv->itc->func.text_get = _item_text_get;
   priv->itc->func.content_get = _item_content_get;
   priv->itc->func.state_get = NULL;
   priv->itc->func.del = _item_del;
   priv->prop_con = eina_hash_string_superfast_new(free);

   eo_event_callback_array_add(priv->genlist, genlist_callbacks(), priv);
   evas_object_event_callback_add(priv->genlist, EVAS_CALLBACK_DEL, _genlist_deleted, priv);
}


static void
_elm_view_list_eo_base_destructor(Eo *obj, Elm_View_List_Data *priv)
{
   EINA_SAFETY_ON_NULL_RETURN(priv);
   EINA_SAFETY_ON_NULL_RETURN(obj);

   eo_event_callback_array_del(priv->model, model_callbacks(), priv->rootdata);

   elm_obj_genlist_clear(priv->genlist);
   free((void *)priv->itc->item_style);
   elm_genlist_item_class_free(priv->itc);

   eina_hash_free(priv->prop_con);
   free(priv->rootdata);
   priv->rootdata = NULL;
   if (priv->genlist)
     {
         evas_object_event_callback_del(priv->genlist, EVAS_CALLBACK_DEL, _genlist_deleted);
         eo_event_callback_array_del(priv->genlist, genlist_callbacks(), priv);
         eo_unref(priv->genlist);
     }

   eo_unref(priv->model);

   eo_destructor(eo_super(obj, MY_CLASS));
}

static void
_elm_view_list_evas_object_get(Eo *obj, Elm_View_List_Data *priv, Evas_Object **widget)
{
   EINA_SAFETY_ON_NULL_RETURN(priv);
   EINA_SAFETY_ON_NULL_RETURN(obj);
   EINA_SAFETY_ON_NULL_RETURN(widget);

   *widget = priv->genlist;
}

static void
_elm_view_list_property_connect(Eo *obj EINA_UNUSED, Elm_View_List_Data *priv,
                const char *property, const char *part)
{
   EINA_SAFETY_ON_NULL_RETURN(priv);

   EINA_SAFETY_ON_NULL_RETURN(priv->prop_con);
   EINA_SAFETY_ON_NULL_RETURN(property);
   EINA_SAFETY_ON_NULL_RETURN(part);

   free(eina_hash_set(priv->prop_con, part, strdup(property)));
}

static void
_elm_view_list_model_set(Eo *obj EINA_UNUSED, Elm_View_List_Data *priv, Efl_Model *model)
{
   EINA_SAFETY_ON_NULL_RETURN(priv);
   EINA_SAFETY_ON_NULL_RETURN(model);
   _priv_model_set(priv, model);
}

static void
_elm_view_list_model_unset(Eo *obj EINA_UNUSED, Elm_View_List_Data *priv)
{
   EINA_SAFETY_ON_NULL_RETURN(priv);
   _priv_model_set(priv, NULL);
}

static void
_elm_view_list_model_get(Eo *obj EINA_UNUSED, Elm_View_List_Data *priv, Eo **model)
{
   EINA_SAFETY_ON_NULL_RETURN(priv);
   EINA_SAFETY_ON_NULL_RETURN(model);
   *model = priv->model;
}
#include "elm_view_list.eo.c"
