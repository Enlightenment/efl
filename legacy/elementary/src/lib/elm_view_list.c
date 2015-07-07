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
  Efl_Model_Load_Status status;
};

static void _efl_model_load_children(View_List_ItemData *);
static Eina_Bool _efl_model_load_status_change_cb(void *, Eo *, const Eo_Event_Description *, void *);
static Eina_Bool _efl_model_children_count_change_cb(void *, Eo *, const Eo_Event_Description *, void *);
static Eina_Bool _efl_model_properties_change_cb(void *, Eo *, const Eo_Event_Description *, void *);

/* --- Genlist Callbacks --- */
static void
_item_sel_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   View_List_ItemData *idata = data;

   EINA_SAFETY_ON_NULL_RETURN(data);

   eo_do(idata->priv->view, eo_event_callback_call(ELM_VIEW_LIST_EVENT_MODEL_SELECTED, idata->model));
}

static void
_item_del(void *data, Evas_Object *obj EINA_UNUSED)
{
   View_List_ItemData *idata = data;
   if (!idata)
      return;

   eo_do(idata->model, eo_event_callback_del(EFL_MODEL_BASE_EVENT_PROPERTIES_CHANGED,
                           _efl_model_properties_change_cb, idata));
   eo_do(idata->model, eo_event_callback_del(EFL_MODEL_BASE_EVENT_LOAD_STATUS,
                           _efl_model_load_status_change_cb, idata));
   eo_do(idata->model, eo_event_callback_del(EFL_MODEL_BASE_EVENT_CHILDREN_COUNT_CHANGED,
                           _efl_model_children_count_change_cb, idata));

   eo_unref(idata->model);
   idata->model = NULL;
   idata->item = NULL;
   idata->parent = NULL;
   idata->priv = NULL;

   free(idata);
}

static Evas_Object *
_item_content_get(void *data, Evas_Object *obj EINA_UNUSED, const char *part)
{
   const Eina_Value *value = NULL;
   const Eina_Value_Type *vtype;
   Evas_Object *content = NULL;
   View_List_ItemData *idata = data;
   EINA_SAFETY_ON_NULL_RETURN_VAL(data, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(part, NULL);
   Elm_View_List_Data *priv = idata->priv;

   const char *prop = eina_hash_find(priv->prop_con, part);
   if (prop == NULL) prop = part;

   eo_do(idata->model, efl_model_property_get(prop, &value));
   if (value == NULL)
     return content;

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

   return content;
}

static char *
_item_text_get(void *data, Evas_Object *obj EINA_UNUSED, const char *part)
{
   const Eina_Value *value = NULL;
   char *text = NULL;
   View_List_ItemData *idata = data;

   EINA_SAFETY_ON_NULL_RETURN_VAL(data, NULL);
   EINA_SAFETY_ON_NULL_RETURN_VAL(part, NULL);
   Elm_View_List_Data *priv = idata->priv;
   EINA_SAFETY_ON_NULL_RETURN_VAL(idata->model, NULL);

   const char *prop = eina_hash_find(priv->prop_con, part);
   if (prop == NULL) prop = part;

   eo_do(idata->model, efl_model_property_get(prop, &value));
   if (value)
     text = eina_value_to_string(value);

   return text;
}

static Eina_Bool
_expand_request_cb(void *data EINA_UNUSED, Eo *obj EINA_UNUSED, const Eo_Event_Description *desc EINA_UNUSED, void *event_info)
{
   Elm_Object_Item *item = event_info;
   View_List_ItemData *idata = elm_object_item_data_get(item);
   Efl_Model_Load_Status st = EFL_MODEL_LOAD_STATUS_ERROR;

   EINA_SAFETY_ON_NULL_RETURN_VAL(idata, EINA_TRUE);

   eo_do(idata->model, st = efl_model_load_status_get());
   eo_do(idata->model, eo_event_callback_add(EFL_MODEL_BASE_EVENT_LOAD_STATUS,
                           _efl_model_load_status_change_cb, idata));
   eo_do(idata->model, eo_event_callback_add(EFL_MODEL_BASE_EVENT_CHILDREN_COUNT_CHANGED,
                           _efl_model_children_count_change_cb, idata));

   if (st & EFL_MODEL_LOAD_STATUS_LOADED_CHILDREN)
     {
        _efl_model_load_children(idata);
     }
   else
     {
        eo_do(idata->model, efl_model_children_load());
     }
   return EINA_TRUE;
}

static Eina_Bool
_contract_request_cb(void *data EINA_UNUSED, Eo *obj EINA_UNUSED, const Eo_Event_Description *desc EINA_UNUSED, void *event_info)
{
   Elm_Object_Item *item = event_info;
   View_List_ItemData *idata = elm_object_item_data_get(item);

   eo_do(idata->model, eo_event_callback_del(EFL_MODEL_BASE_EVENT_LOAD_STATUS,
                           _efl_model_load_status_change_cb, idata));
   eo_do(idata->model, eo_event_callback_del(EFL_MODEL_BASE_EVENT_CHILDREN_COUNT_CHANGED,
                           _efl_model_children_count_change_cb, idata));
   elm_genlist_item_expanded_set(item, EINA_FALSE);
   return EINA_TRUE;
}

static Eina_Bool
_contracted_cb(void *data EINA_UNUSED, Eo *obj EINA_UNUSED, const Eo_Event_Description *desc EINA_UNUSED, void *event_info)
{
   Elm_Object_Item *glit = event_info;
   elm_genlist_item_subitems_clear(glit);
   return EINA_TRUE;
}

static void
_genlist_deleted(void *data, Evas *e EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   Elm_View_List_Data *priv = data;

   if (priv && priv->genlist && priv->genlist == obj)
     {
        eo_do(priv->genlist,
              eo_event_callback_del(ELM_GENLIST_EVENT_EXPAND_REQUEST, _expand_request_cb, priv),
              eo_event_callback_del(ELM_GENLIST_EVENT_CONTRACT_REQUEST, _contract_request_cb, priv),
              eo_event_callback_del(ELM_GENLIST_EVENT_CONTRACTED, _contracted_cb, priv));
        eo_unref(priv->genlist);
        priv->genlist = NULL;
     }
}


/* --- Efl_Model Callbacks --- */
static Eina_Bool
_efl_model_properties_change_cb(void *data, Eo *obj EINA_UNUSED,
                const Eo_Event_Description *desc EINA_UNUSED, void *event_info)
{
   View_List_ItemData *idata = data;
   Efl_Model_Property_Event *evt = event_info;

   EINA_SAFETY_ON_NULL_RETURN_VAL(idata, EINA_TRUE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(evt, EINA_TRUE);

   if (idata->item)
     elm_genlist_item_update(idata->item);

   return EINA_TRUE;
}

static void
_efl_model_load_children(View_List_ItemData *pdata)
{
   Eo *child;
   Eina_Accessor *accessor = NULL;
   EINA_SAFETY_ON_NULL_RETURN(pdata);

   Elm_View_List_Data *priv = pdata->priv;
   unsigned int i, total = 0;

   eo_do(pdata->model, efl_model_children_count_get(&total));
   if (total == 0)
     return;

   eo_do(pdata->model, efl_model_children_slice_get(0 ,0 ,(Eina_Accessor **)&accessor));

   EINA_SAFETY_ON_NULL_RETURN(accessor);

   EINA_ACCESSOR_FOREACH(accessor, i, child)
     {
        View_List_ItemData *idata = malloc(sizeof(View_List_ItemData));
        memset(idata, 0, sizeof(View_List_ItemData));
        EINA_SAFETY_ON_NULL_RETURN(idata);
        idata->priv = priv;
        idata->parent = pdata;
        idata->model = child;
        eo_ref(child);
        eo_do(child, eo_event_callback_add(EFL_MODEL_BASE_EVENT_PROPERTIES_CHANGED,
                                _efl_model_properties_change_cb, idata));
        eo_do(child, efl_model_properties_load());
        idata->item = elm_genlist_item_append(priv->genlist, priv->itc, idata, pdata->item,
                                                       priv->itype, _item_sel_cb, idata);
     }

   if (pdata->item)
     elm_genlist_item_expanded_set(pdata->item, EINA_TRUE);
}

static Eina_Bool
_efl_model_children_count_change_cb(void *data, Eo *obj EINA_UNUSED,
                const Eo_Event_Description *desc EINA_UNUSED, void *event_info EINA_UNUSED)
{
   View_List_ItemData *idata = data;
   EINA_SAFETY_ON_NULL_RETURN_VAL(idata, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(idata->priv, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(idata->priv->genlist, EINA_FALSE);

   elm_genlist_item_subitems_clear(idata->item);

   _efl_model_load_children(idata);

   return EINA_TRUE;
}

static Eina_Bool
_efl_model_load_status_change_cb(void *data, Eo *obj EINA_UNUSED,
                const Eo_Event_Description *desc EINA_UNUSED, void *event_info)
{
   View_List_ItemData *idata = data;
   Efl_Model_Load *load = event_info;

   if (load->status & EFL_MODEL_LOAD_STATUS_UNLOADED)
     {
        if (idata->item)
          elm_genlist_item_expanded_set(idata->item, EINA_FALSE);
     }

   idata->status = load->status;
   return EINA_TRUE;
}

static void
_priv_model_set(Elm_View_List_Data *priv, Eo *model)
{
   Efl_Model_Load_Status load_st;

   if (priv->model != NULL)
     {
         eo_do(priv->model, eo_event_callback_del(EFL_MODEL_BASE_EVENT_LOAD_STATUS,
                                                 _efl_model_load_status_change_cb, priv->rootdata));
         eo_do(priv->model, eo_event_callback_del(EFL_MODEL_BASE_EVENT_CHILDREN_COUNT_CHANGED,
                                                 _efl_model_children_count_change_cb, priv->rootdata));
         eo_do(priv->genlist, elm_obj_genlist_clear());
         eo_unref(priv->model);
         priv->model = NULL;
     }

   if (model == NULL)
     return;

   priv->model = model;
   priv->rootdata->model = priv->model;
   eo_ref(priv->model);

   eo_do(priv->model, eo_event_callback_add(EFL_MODEL_BASE_EVENT_LOAD_STATUS,
                                                _efl_model_load_status_change_cb, priv->rootdata));
   eo_do(priv->model, eo_event_callback_add(EFL_MODEL_BASE_EVENT_CHILDREN_COUNT_CHANGED,
                                                _efl_model_children_count_change_cb, priv->rootdata));
   eo_do(priv->model, load_st = efl_model_load_status_get());
   if (load_st & EFL_MODEL_LOAD_STATUS_LOADED_CHILDREN)
     {
         _efl_model_load_children(priv->rootdata);
     }
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

   eo_do(priv->genlist,
         eo_event_callback_add(ELM_GENLIST_EVENT_EXPAND_REQUEST, _expand_request_cb, priv),
         eo_event_callback_add(ELM_GENLIST_EVENT_CONTRACT_REQUEST, _contract_request_cb, priv),
         eo_event_callback_add(ELM_GENLIST_EVENT_CONTRACTED, _contracted_cb, priv));
   evas_object_event_callback_add(priv->genlist, EVAS_CALLBACK_DEL, _genlist_deleted, priv);
}


static void
_elm_view_list_eo_base_destructor(Eo *obj, Elm_View_List_Data *priv)
{
   EINA_SAFETY_ON_NULL_RETURN(priv);
   EINA_SAFETY_ON_NULL_RETURN(obj);

   eo_do(priv->model, eo_event_callback_del(EFL_MODEL_BASE_EVENT_LOAD_STATUS,
                                        _efl_model_load_status_change_cb, priv->rootdata));
   eo_do(priv->model, eo_event_callback_del(EFL_MODEL_BASE_EVENT_CHILDREN_COUNT_CHANGED,
                                        _efl_model_children_count_change_cb, priv->rootdata));

   eo_do(priv->genlist, elm_obj_genlist_clear());
   elm_genlist_item_class_free(priv->itc);

   eina_hash_free(priv->prop_con);
   free(priv->rootdata);
   priv->rootdata = NULL;
   if (priv->genlist) {
     evas_object_event_callback_del(priv->genlist, EVAS_CALLBACK_DEL, _genlist_deleted);
     eo_do(priv->genlist,
           eo_event_callback_del(ELM_GENLIST_EVENT_EXPAND_REQUEST, _expand_request_cb, priv),
           eo_event_callback_del(ELM_GENLIST_EVENT_CONTRACT_REQUEST, _contract_request_cb, priv),
           eo_event_callback_del(ELM_GENLIST_EVENT_CONTRACTED, _contracted_cb, priv));
     eo_unref(priv->genlist);
   }

   eo_unref(priv->model);

   eo_do_super(obj, MY_CLASS, eo_destructor());
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
_elm_view_list_model_set(Eo *obj EINA_UNUSED, Elm_View_List_Data *priv, Efl_Model_Base *model)
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
