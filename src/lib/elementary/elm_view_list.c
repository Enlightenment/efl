#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include <Eo.h>
#include <Efl.h>
#include <Elementary.h>

#include "elm_priv.h"
#include "elm_genlist.eo.h"

#include <assert.h>

#define MY_CLASS ELM_VIEW_LIST_CLASS
#define MY_CLASS_NAME "View List"

typedef struct _Elm_View_List_Data Elm_View_List_Data;
typedef struct _View_List_ItemData View_List_ItemData;
typedef struct _View_List_ValueItem View_List_ValueItem;

struct _Elm_View_List_Data
{
   Eo *view;
   Evas_Object *genlist;
   View_List_ItemData *rootdata;
   Elm_Genlist_Item_Class *itc;
   Elm_Genlist_Item_Type itype;

   struct {
      Eina_Hash *properties;
      Eo *model;
   } connect;
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
static void _efl_model_children_added_cb(void *, const Efl_Event *event);
static void _efl_model_children_removed_cb(void *, const Efl_Event *event);
static void _efl_model_properties_change_cb(void *, const Efl_Event *event);

static void _expand_request_cb(void *data EINA_UNUSED, const Efl_Event *event);
static void _contract_request_cb(void *data EINA_UNUSED, const Efl_Event *event);
static void _contracted_cb(void *data EINA_UNUSED, const Efl_Event *event);

/* --- Genlist Callbacks --- */
EFL_CALLBACKS_ARRAY_DEFINE(model_callbacks,
                           { EFL_MODEL_EVENT_CHILD_ADDED, _efl_model_children_added_cb },
                           { EFL_MODEL_EVENT_CHILD_REMOVED, _efl_model_children_removed_cb });
EFL_CALLBACKS_ARRAY_DEFINE(genlist_callbacks,
                          { ELM_GENLIST_EVENT_EXPAND_REQUEST, _expand_request_cb },
                          { ELM_GENLIST_EVENT_CONTRACT_REQUEST, _contract_request_cb },
                          { ELM_GENLIST_EVENT_CONTRACTED, _contracted_cb });

static void
_item_sel_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   View_List_ItemData *idata = data;

   EINA_SAFETY_ON_NULL_RETURN(idata);

   efl_event_callback_legacy_call(idata->priv->view, ELM_VIEW_LIST_EVENT_MODEL_SELECTED, idata->model);
}

static void
_item_del(void *data, Evas_Object *obj EINA_UNUSED)
{
   View_List_ItemData *idata = data;

   if (!idata) return;

   efl_event_callback_array_del(idata->model, model_callbacks(), idata);
   efl_event_callback_del(idata->model, EFL_MODEL_EVENT_PROPERTIES_CHANGED, _efl_model_properties_change_cb, idata);

   efl_unref(idata->model);
   idata->model = NULL;
   idata->item = NULL;
   idata->parent = NULL;
   idata->priv = NULL;

   free(idata);
}

static Evas_Object *
_item_content_get(void *data, Evas_Object *obj EINA_UNUSED, const char *part)
{
   const Eina_Value_Type *vtype;
   const char *prop;
   Eina_Value *value = NULL;
   Evas_Object *content = NULL;
   View_List_ItemData *idata = data;
   EINA_SAFETY_ON_NULL_RETURN_VAL(data, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(part, NULL);

   if (!idata->item) return NULL;

   prop = eina_hash_find(idata->priv->connect.properties, part);
   if (!prop) prop = part;

   value = efl_model_property_get(idata->model, prop);
   if (value == NULL) return NULL;

   vtype = eina_value_type_get(value);
   if (vtype == EINA_VALUE_TYPE_BLOB)
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
   else if (vtype == EINA_VALUE_TYPE_FILE)
     {
        Eina_File *f = NULL;

        eina_value_get(value, &f);

        content = elm_image_add(obj);
        elm_image_mmap_set(content, f, NULL);
     }
   else if (vtype == EINA_VALUE_TYPE_OBJECT)
     {
        eina_value_get(value, &content);
     }
   else
     {
         char *str = NULL;
         str = eina_value_to_string(value);
         content = elm_icon_add(obj);
         if (elm_icon_standard_set(content, str))
           {
               evas_object_size_hint_aspect_set(content, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
           }
         else
           {
               evas_object_del(content);
               content = NULL;
           }
         free(str);
     }
   eina_value_free(value);

   return content;
}

static char *
_item_text_get(void *data, Evas_Object *obj EINA_UNUSED, const char *part)
{
   Eina_Value *value = NULL;
   const char *prop;
   char *text = NULL;
   View_List_ItemData *idata = data;

   EINA_SAFETY_ON_NULL_RETURN_VAL(data, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(part, NULL);
   if (!idata->item) return NULL;

   prop = eina_hash_find(idata->priv->connect.properties, part);
   if (!prop) prop = part;

   value = efl_model_property_get(idata->model, prop);
   if (value == NULL) return NULL;

   text = eina_value_to_string(value);

   eina_value_free(value);

   return text;
}

static void
_expand_request_cb(void *data EINA_UNUSED, const Efl_Event *event)
{
   Elm_Object_Item *item = event->info;
   View_List_ItemData *idata = elm_object_item_data_get(item);

   EINA_SAFETY_ON_NULL_RETURN(idata);

   efl_event_callback_array_add(idata->model, model_callbacks(), idata);

   _efl_model_load_children(idata);
}

static void
_contract_request_cb(void *data EINA_UNUSED, const Efl_Event *event)
{
   Elm_Object_Item *item = event->info;
   View_List_ItemData *idata = elm_object_item_data_get(item);

   efl_event_callback_array_del(idata->model, model_callbacks(), idata);
   elm_genlist_item_expanded_set(item, EINA_FALSE);
}

static void
_contracted_cb(void *data EINA_UNUSED, const Efl_Event *event)
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
        efl_event_callback_array_del(priv->genlist, genlist_callbacks(), priv);
        efl_unref(priv->genlist);
        priv->genlist = NULL;
     }
}

/* --- Efl_Model Callbacks --- */
static void
_efl_model_properties_change_cb(void *data, const Efl_Event *event)
{
   View_List_ItemData *idata = data;
   Efl_Model_Property_Event *evt = event->info;

   EINA_SAFETY_ON_NULL_RETURN(idata);
   EINA_SAFETY_ON_NULL_RETURN(evt);

   if (idata->item)
     elm_genlist_item_update(idata->item);
}

static Eina_Value
_efl_model_load_children_then(void *data, const Eina_Value v,
                              const Eina_Future *ev EINA_UNUSED)
{
   View_List_ItemData *pdata = data;
   Elm_View_List_Data *priv = pdata->priv;
   unsigned int i, len;
   Efl_Model *child = NULL;

   if (eina_value_type_get(&v) == EINA_VALUE_TYPE_ERROR)
     goto end;

   EINA_VALUE_ARRAY_FOREACH(&v, len, i, child)
     {
        View_List_ItemData *idata = calloc(1, sizeof(View_List_ItemData));
        if (!idata) continue ;

        idata->priv = priv;
        idata->parent = pdata;
        idata->model = efl_ref(child);

        efl_event_callback_add(idata->model, EFL_MODEL_EVENT_PROPERTIES_CHANGED,
                               _efl_model_properties_change_cb, idata);

        idata->item = elm_genlist_item_append(priv->genlist, priv->itc, idata, pdata->item,
                                              priv->itype, _item_sel_cb, idata);
     }

   if (i > 0 && pdata->item)
     elm_genlist_item_expanded_set(pdata->item, EINA_TRUE);

 end:
   return v;
}

static void
_efl_model_load_children(View_List_ItemData *pdata)
{
   Eina_Future *f;

   f = efl_model_children_slice_get(pdata->priv->connect.model, 0,
                                    efl_model_children_count_get(pdata->priv->connect.model));
   f = eina_future_then(f, _efl_model_load_children_then, pdata, NULL);
   efl_future_then(pdata->priv->genlist, f);
}

static void
_efl_model_children_removed_cb(void *data, const Efl_Event *event)
{
   Efl_Model_Children_Event* evt = event->info;
   View_List_ItemData *idata = data;
   Elm_Object_Item *item;
   const Eina_List *subitems, *l;
   unsigned int i = 0;

   subitems = elm_genlist_item_subitems_get(idata->item);

   EINA_LIST_FOREACH(subitems, l, item)
     {
        if (i == evt->index) break ;
        i++;
     }

   if (i != evt->index) return ;
   elm_object_item_del(item);
}

static void
_efl_model_children_added_cb(void *data, const Efl_Event *event)
{
   Efl_Model_Children_Event* evt = event->info;
   View_List_ItemData *idata = data;
   Eina_Future *f;

   f = efl_model_children_slice_get(idata->priv->connect.model, evt->index, 1);
   f = eina_future_then(f, _efl_model_load_children_then, idata, NULL);
   efl_future_then(idata->priv->genlist, f);
}

static void
_priv_model_set(Elm_View_List_Data *priv, Eo *model)
{
   if (priv->connect.model)
     {
        efl_event_callback_array_del(priv->connect.model, model_callbacks(), priv->rootdata);
        elm_obj_genlist_clear(priv->genlist);
     }

   efl_replace(&priv->connect.model, model);

   if (model == NULL) return;

   priv->rootdata->model = priv->connect.model;

   efl_event_callback_array_add(priv->connect.model, model_callbacks(), priv->rootdata);
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
   efl_ref(priv->genlist);

   priv->rootdata = calloc(1, sizeof(View_List_ItemData));
   priv->rootdata->priv = priv;

   priv->itc = elm_genlist_item_class_new();
   if (istyle)
     priv->itc->item_style = strdup(istyle);
   priv->itc->func.text_get = _item_text_get;
   priv->itc->func.content_get = _item_content_get;
   priv->itc->func.state_get = NULL;
   priv->itc->func.del = _item_del;
   priv->connect.properties = eina_hash_string_superfast_new(free);

   efl_event_callback_array_add(priv->genlist, genlist_callbacks(), priv);
   evas_object_event_callback_add(priv->genlist, EVAS_CALLBACK_DEL, _genlist_deleted, priv);
}


static void
_elm_view_list_efl_object_destructor(Eo *obj, Elm_View_List_Data *priv)
{
   EINA_SAFETY_ON_NULL_RETURN(priv);
   EINA_SAFETY_ON_NULL_RETURN(obj);

   efl_event_callback_array_del(priv->connect.model, model_callbacks(), priv->rootdata);

   elm_obj_genlist_clear(priv->genlist);
   free((void *)priv->itc->item_style);
   elm_genlist_item_class_free(priv->itc);

   eina_hash_free(priv->connect.properties);
   free(priv->rootdata);
   priv->rootdata = NULL;

   if (priv->genlist)
     {
        evas_object_event_callback_del(priv->genlist, EVAS_CALLBACK_DEL, _genlist_deleted);
        efl_event_callback_array_del(priv->genlist, genlist_callbacks(), priv);
        efl_unref(priv->genlist);
     }

   efl_unref(priv->connect.model);

   efl_destructor(efl_super(obj, MY_CLASS));
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

   EINA_SAFETY_ON_NULL_RETURN(priv->connect.properties);
   EINA_SAFETY_ON_NULL_RETURN(property);
   EINA_SAFETY_ON_NULL_RETURN(part);

   free(eina_hash_set(priv->connect.properties, part, strdup(property)));
}

static void
_elm_view_list_model_set(Eo *obj EINA_UNUSED, Elm_View_List_Data *priv, Efl_Model *model)
{
   _priv_model_set(priv, model);
}

static Efl_Model *
_elm_view_list_model_get(const Eo *obj EINA_UNUSED, Elm_View_List_Data *priv)
{
   return priv->connect.model;
}
#include "elm_view_list.eo.c"
