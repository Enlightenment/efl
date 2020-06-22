// Note: @1.23 Initial release has infrastructure to support more mode than homogeneous, but isn't exposed in the API nor supported.

#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#define ELM_LAYOUT_PROTECTED
#define EFL_UI_SCROLL_MANAGER_PROTECTED
#define EFL_UI_SCROLLBAR_PROTECTED
#define EFL_UI_WIDGET_FOCUS_MANAGER_PROTECTED

#include <Efl_Ui.h>
#include <Elementary.h>
#include "elm_widget.h"
#include "elm_priv.h"
#include "inttypes.h"

#include "efl_ui_collection_view_focus_manager.eo.h"

#ifndef VIEWPORT_ENABLE
# undef VIEWPORT_ENABLE
#endif

typedef struct _Efl_Ui_Collection_View_Data Efl_Ui_Collection_View_Data;
typedef struct _Efl_Ui_Collection_Viewport Efl_Ui_Collection_Viewport;
typedef struct _Efl_Ui_Collection_View_Focus_Manager_Data Efl_Ui_Collection_View_Focus_Manager_Data;
typedef struct _Efl_Ui_Collection_Item Efl_Ui_Collection_Item;
typedef struct _Efl_Ui_Collection_Item_Lookup Efl_Ui_Collection_Item_Lookup;
typedef struct _Efl_Ui_Collection_Request Efl_Ui_Collection_Request;

struct _Efl_Ui_Collection_Item
{
   Efl_Gfx_Entity *entity;
   Efl_Model *model;
};

struct _Efl_Ui_Collection_Item_Lookup
{
   EINA_RBTREE;

   unsigned int index;
   Efl_Ui_Collection_Item item;
};

struct _Efl_Ui_Collection_Viewport
{
   Efl_Ui_Collection_Item *items;

   unsigned int offset;
   uint16_t count;
};

struct _Efl_Ui_Collection_Request
{
   Eina_Future *f;

   unsigned int offset;
   unsigned int length;

   Eina_Bool need_size : 1;
   Eina_Bool need_entity : 1;
   Eina_Bool entity_requested : 1;
};

struct _Efl_Ui_Collection_View_Data
{
   Efl_Ui_Factory *factory;
   Efl_Ui_Position_Manager_Entity *manager;
   Efl_Ui_Scroll_Manager *scroller;
   Efl_Ui_Pan *pan;
   Efl_Gfx_Entity *sizer;
   Efl_Model *model;
   Efl_Model *multi_selectable_async_model;

#ifdef VIEWPORT_ENABLE
   Efl_Ui_Collection_Viewport *viewport[3];
#endif
   Eina_Rbtree *cache;

   Eina_List *requests; // Array of Efl_Ui_Collection_Request in progress

   struct {
      Efl_Gfx_Entity *last; // The last item of the collection, so focus can start by the end if necessary.
      Efl_Gfx_Entity *previously; // The previously selected item in the collection, so focus can come back to it.
   } focus;

   unsigned int start_id;
   unsigned int end_id;

   Eina_Size2D content_min_size;

   Efl_Ui_Layout_Orientation direction;
   Efl_Ui_Select_Mode mode;

   struct {
      Eina_Bool w : 1;
      Eina_Bool h : 1;
   } match_content;

   Efl_Ui_Position_Manager_Request_Range current_range;
};

struct _Efl_Ui_Collection_View_Focus_Manager_Data
{
   Efl_Ui_Collection_View *collection;
};

static const char *COLLECTION_VIEW_MANAGED = "_collection_view.managed";
static const char *COLLECTION_VIEW_MANAGED_YES = "yes";

#define MY_CLASS EFL_UI_COLLECTION_VIEW_CLASS

#define MY_DATA_GET(obj, pd)                                            \
  Efl_Ui_Collection_View_Data *pd = efl_data_scope_get(obj, MY_CLASS);

static Eina_Bool _entity_request(Efl_Ui_Collection_View *obj, Efl_Ui_Collection_Request *request);
static void _idle_cb(void *data, const Efl_Event *event);

static int
_cache_tree_lookup(const Eina_Rbtree *node, const void *key,
                   int length EINA_UNUSED, void *data EINA_UNUSED)
{
   const Efl_Ui_Collection_Item_Lookup *n = (Efl_Ui_Collection_Item_Lookup *)node;
   const unsigned int *index = key;

   if (n->index > *index)
     return 1;
   if (n->index < *index)
     return -1;
   return 0;
}

static Eina_Rbtree_Direction
_cache_tree_cmp(const Eina_Rbtree *left, const Eina_Rbtree *right, void *data EINA_UNUSED)
{
   Efl_Ui_Collection_Item_Lookup *l = (Efl_Ui_Collection_Item_Lookup *)left;
   Efl_Ui_Collection_Item_Lookup *r = (Efl_Ui_Collection_Item_Lookup *)right;

   return l->index < r->index ? EINA_RBTREE_LEFT : EINA_RBTREE_RIGHT;
}

static Eina_Value
_undo_item_selected_then(Eo *item, void *data EINA_UNUSED, Eina_Error err)
{
   Eina_Value *get;
   Eina_Bool item_selected = efl_ui_selectable_selected_get(item);
   Eina_Bool model_selected = EINA_FALSE;

   get = efl_model_property_get(efl_ui_view_model_get(item), "self.selected");
   eina_value_bool_get(get, &model_selected);
   eina_value_free(get);

   if ((!!model_selected) != (!!item_selected))
     efl_ui_selectable_selected_set(item, model_selected);

   return eina_value_error_init(err);
}

static void
_selected_item_cb(void *data EINA_UNUSED, const Efl_Event *ev)
{
   // Link back property to model, maybe just trigger event on the item should be enough
   Eina_Value *get;
   Eina_Bool item_selected = efl_ui_selectable_selected_get(ev->object);
   Eina_Bool model_selected = EINA_FALSE;
   Eina_Value set = eina_value_bool_init(!!item_selected);

   get = efl_model_property_get(efl_ui_view_model_get(ev->object), "self.selected");
   eina_value_bool_get(get, &model_selected);
   eina_value_free(get);

   if ((!!model_selected) != (!!item_selected))
     {
        Eina_Future *f;

        f = efl_model_property_set(efl_ui_view_model_get(ev->object), "self.selected", &set);

        // In case the mode is preventing the change, we need to update the UI back. So handle error case
        efl_future_then(ev->object, f,
                        .error = _undo_item_selected_then);
     }

   eina_value_flush(&set);
}

static void
_redirect_item_cb(void *data, const Efl_Event *ev)
{
   Eo *obj = data;

#define REDIRECT_EVT(Desc, Item_Desc)                           \
   if (Desc == ev->desc)                                        \
     {                                                          \
        Efl_Ui_Item_Clickable_Clicked item_clicked;             \
        Efl_Input_Clickable_Clicked *clicked = ev->info;        \
                                                                \
        item_clicked.clicked = *clicked;                        \
        item_clicked.item = ev->object;                         \
                                                                \
        efl_event_callback_call(obj, Item_Desc, &item_clicked); \
     }
#define REDIRECT_EVT_PRESS(Desc, Item_Desc)                           \
   if (Desc == ev->desc)                                        \
     {                                                          \
        Efl_Ui_Item_Clickable_Pressed item_pressed;             \
        int *button = ev->info;        \
                                                                \
        item_pressed.button = *button;                        \
        item_pressed.item = ev->object;                         \
                                                                \
        efl_event_callback_call(obj, Item_Desc, &item_pressed); \
     }

   REDIRECT_EVT_PRESS(EFL_INPUT_EVENT_PRESSED, EFL_UI_EVENT_ITEM_PRESSED);
   REDIRECT_EVT_PRESS(EFL_INPUT_EVENT_UNPRESSED, EFL_UI_EVENT_ITEM_UNPRESSED);
   REDIRECT_EVT_PRESS(EFL_INPUT_EVENT_LONGPRESSED, EFL_UI_EVENT_ITEM_LONGPRESSED);
   REDIRECT_EVT(EFL_INPUT_EVENT_CLICKED_ANY, EFL_UI_EVENT_ITEM_CLICKED_ANY);
   REDIRECT_EVT(EFL_INPUT_EVENT_CLICKED, EFL_UI_EVENT_ITEM_CLICKED);
#undef REDIRECT_EVT
#undef REDIRECT_EVT_PRESS
}

EFL_CALLBACKS_ARRAY_DEFINE(active_item_cbs,
  { EFL_UI_EVENT_SELECTED_CHANGED, _selected_item_cb },
  { EFL_INPUT_EVENT_PRESSED, _redirect_item_cb },
  { EFL_INPUT_EVENT_UNPRESSED, _redirect_item_cb },
  { EFL_INPUT_EVENT_LONGPRESSED, _redirect_item_cb },
  { EFL_INPUT_EVENT_CLICKED, _redirect_item_cb },
  { EFL_INPUT_EVENT_CLICKED_ANY, _redirect_item_cb });

static void
_entity_cleanup(Efl_Ui_Collection_View *obj, Efl_Ui_Factory *factory,
                Efl_Ui_Collection_Item *item, Eina_Array *scheduled_release)
{
   Efl_Gfx_Entity *entities[1];

   entities[0] = item->entity;
   if (!entities[0]) return ;

   efl_event_callback_array_del(entities[0], active_item_cbs(), obj);
   efl_replace(&item->entity, NULL);
   efl_event_callback_call(obj, EFL_UI_COLLECTION_VIEW_EVENT_ITEM_UNREALIZED, entities[0]);
   if (!scheduled_release)
     {
        efl_ui_factory_release(factory, EINA_C_ARRAY_ITERATOR_NEW(entities));
     }
   else
     {
        eina_array_push(scheduled_release, entities[0]);
     }
}

static void
_item_cleanup(Efl_Ui_Collection_View *obj, Efl_Ui_Factory *factory,
              Efl_Ui_Collection_Item *item, Eina_Array *scheduled_release)
{
   efl_replace(&item->model, NULL);

   _entity_cleanup(obj, factory, item, scheduled_release);
}

static void
_cache_item_free(Eina_Rbtree *node, void *data)
{
   Efl_Ui_Collection_Item_Lookup *n = (void*) node;
   MY_DATA_GET(data, pd);

   _item_cleanup(data, pd->factory, &n->item, NULL);
   free(n);
}

static void
_cache_cleanup(Efl_Ui_Collection_View *obj, Efl_Ui_Collection_View_Data *pd)
{
   eina_rbtree_delete(pd->cache, _cache_item_free, obj);
   pd->cache = NULL;
}

static void
_all_cleanup(Efl_Ui_Collection_View *obj, Efl_Ui_Collection_View_Data *pd)
{
   Efl_Ui_Collection_Request *request;
   Eina_List *l, *ll;
#ifdef VIEWPORT_ENABLE
   unsigned int i;
#endif

   _cache_cleanup(obj, pd);
#ifdef VIEWPORT_ENABLE
   for (i = 0; i < 3; i++)
     {
        unsigned int j;

        if (!pd->viewport[i]) continue;

        for (j = 0; j < pd->viewport[i]->count; j++)
          _item_cleanup(obj, pd->factory, &(pd->viewport[i]->items[j]));
     }
#endif

   efl_replace(&pd->focus.previously, NULL);
   efl_replace(&pd->focus.last, NULL);

   EINA_LIST_FOREACH_SAFE(pd->requests, l, ll, request)
     eina_future_cancel(request->f);
}

static inline Eina_Bool
_size_from_model(Efl_Model *model, Eina_Size2D *r, const char *width, const char *height)
{
   Eina_Value *vw, *vh;
   Eina_Bool success = EINA_FALSE;

   EINA_SAFETY_ON_NULL_RETURN_VAL(model, EINA_FALSE);

   vw = efl_model_property_get(model, width);
   vh = efl_model_property_get(model, height);

   if (eina_value_type_get(vw) == EINA_VALUE_TYPE_ERROR ||
       eina_value_type_get(vh) == EINA_VALUE_TYPE_ERROR)
     goto on_error;

   if (!eina_value_int_convert(vw, &(r->w))) r->w = 0;
   if (!eina_value_int_convert(vh, &(r->h))) r->h = 0;

   success = EINA_TRUE;

 on_error:
   eina_value_free(vw);
   eina_value_free(vh);

   return success;
}

static inline void
_size_to_model(Efl_Model *model, Eina_Size2D state)
{
   Eina_Value vw, vh;

   vw = eina_value_int_init(state.w);
   vh = eina_value_int_init(state.h);

   efl_model_property_set(model, "self.width", &vw);
   efl_model_property_set(model, "self.height", &vh);

   eina_value_flush(&vw);
   eina_value_flush(&vh);
}

#define ITEM_BASE_SIZE_FROM_MODEL(Model, Size) _size_from_model(Model, &Size, "item.width", "item.height")
#define ITEM_SIZE_FROM_MODEL(Model, Size) _size_from_model(Model, &Size, "self.width", "self.height")

static Eina_List *
_request_add(Eina_List *requests, Efl_Ui_Collection_Request **request,
             unsigned int index, Eina_Bool need_entity)
{
   if (!(*request)) goto create;

   if ((*request)->offset + (*request)->length == index)
     {
        if (need_entity) (*request)->need_entity = EINA_TRUE;
        if (!need_entity) (*request)->need_size = EINA_TRUE;
        (*request)->length += 1;
        return requests;
     }

   requests = eina_list_append(requests, *request);

 create:
   *request = calloc(1, sizeof (Efl_Ui_Collection_Request));
   if (!(*request)) return requests;
   (*request)->offset = index;
   (*request)->length = 1;
   // At this point, we rely on the model caching ability to avoid recreating model
   (*request)->need_entity = !!need_entity;
   (*request)->need_size = EINA_TRUE;

   return requests;
}

static Eina_Value
_model_fetched_cb(Eo *obj, void *data, const Eina_Value v)
{
   MY_DATA_GET(obj, pd);
   Efl_Ui_Collection_Request *request = data;
   Efl_Model *child;
   unsigned int i, len;
   Eina_Bool request_entity = EINA_FALSE;

   EINA_VALUE_ARRAY_FOREACH(&v, len, i, child)
     {
        Efl_Ui_Collection_Item_Lookup *insert;
        Eina_Size2D item_size;
#ifdef VIEWPORT_ENABLE
        unsigned int v;

        for (v = 0; v < 3; ++v)
          {
             if (!pd->viewport[v]) continue;

             if ((pd->viewport[v]->offset <= request->offset + i) &&
                 (request->offset + i < pd->viewport[v]->offset + pd->viewport[v]->count))
               {
                  unsigned int index = request->offset + i - pd->viewport[v]->offset;

                  efl_replace(&pd->viewport[v]->items[index].model, child);
                  child = NULL;
                  break;
               }
          }
#endif

        // When requesting a model, it should not be in the cache prior to the request
        if (!child) continue;

        unsigned int search_index = request->offset + i;

        insert = (void*) eina_rbtree_inline_lookup(pd->cache, &search_index,
                                                   sizeof (search_index), _cache_tree_lookup,
                                                   NULL);
        if (insert)
          {
             if (!insert->item.entity && request->need_entity)
               {
                  //drop the old model here, overwrite with model + view
                  efl_replace(&insert->item.model, child);
               }
             else
               ERR("Inserting a model that was already fetched, dropping new model %u", search_index);
          }
        else
          {
             insert = calloc(1, sizeof (Efl_Ui_Collection_Item_Lookup));
             if (!insert) continue;
             insert->index = request->offset + i;
             insert->item.model = efl_ref(child);
             pd->cache = eina_rbtree_inline_insert(pd->cache, EINA_RBTREE_GET(insert), _cache_tree_cmp, NULL);
          }

        if (!ITEM_SIZE_FROM_MODEL(insert->item.model, item_size))
          request_entity = EINA_TRUE;
     }

   if (request_entity)
     {
        request->need_entity = EINA_TRUE;

        if (!request->entity_requested)
          _entity_request(obj, request);
     }
   else if (request->need_size)
     {
        efl_ui_position_manager_entity_item_size_changed(pd->manager, request->offset,
                                                         request->offset + len);
     }

   return v;
}

static void
_model_free_cb(Eo *o, void *data, const Eina_Future *dead_future EINA_UNUSED)
{
   MY_DATA_GET(o, pd);
   Efl_Ui_Collection_Request *request = data;

   if (!request->entity_requested)
     {
        pd->requests = eina_list_remove(pd->requests, request);
        free(request);
     }
}

static Eina_Value
_entity_fetch_cb(Eo *obj, void *data EINA_UNUSED, const Eina_Value v)
{
   MY_DATA_GET(obj, pd);
   Efl_Model *child;
   Eina_Future *r;
   Eina_Array tmp;
   unsigned int i, len;

   eina_array_step_set(&tmp, sizeof (Eina_Array), 4);

   EINA_VALUE_ARRAY_FOREACH(&v, len, i, child)
     {
        eina_array_push(&tmp, child);
     }

   r = efl_ui_view_factory_create_with_event(pd->factory, eina_array_iterator_new(&tmp));

   eina_array_flush(&tmp);

   return eina_future_as_value(r);
}

static inline unsigned int
_lookup_entity_index(Efl_Gfx_Entity *entity, Efl_Model **model)
{
   Efl_Model *fetch;

   fetch = efl_ui_view_model_get(entity);
   if (model) *model = fetch;
   return efl_composite_model_index_get(fetch);
}

static void
_last_entity_update(Efl_Ui_Collection_View_Data *pd, Efl_Gfx_Entity *entity)
{
   Efl_Model *new_model, *old_model;
   unsigned int new_index, old_index;

   if (!pd->focus.last) goto replace;

   new_index = _lookup_entity_index(entity, &new_model);
   old_index = _lookup_entity_index(pd->focus.last, &old_model);

   if (new_index <= old_index) return;

 replace:
   efl_replace(&pd->focus.last, entity);
}

static inline Eina_Bool
_entity_propagate(Efl_Model *model, Efl_Gfx_Entity *entity)
{
   Eina_Size2D item_size;

   if (efl_key_data_get(entity, "efl.ui.widget.factory.size_set"))
     {
        return EINA_FALSE;
     }

   if (ITEM_SIZE_FROM_MODEL(model, item_size))
     {
        efl_gfx_hint_size_min_set(entity, item_size);
        efl_canvas_group_need_recalculate_set(entity, EINA_FALSE);
        if (efl_isa(entity, EFL_UI_ITEM_CLASS)) efl_ui_item_calc_locked_set(entity, EINA_TRUE);
        return EINA_FALSE;
     }

   efl_canvas_group_calculate(entity);
   item_size = efl_gfx_hint_size_combined_min_get(entity);
   efl_canvas_group_need_recalculate_set(entity, EINA_FALSE);

   _size_to_model(model, item_size);
   return EINA_TRUE;
}

static Eina_Value
_entity_fetched_cb(Eo *obj, void *data, const Eina_Value v)
{
   MY_DATA_GET(obj, pd);
   Efl_Ui_Collection_Request *request = data;
   Efl_Gfx_Entity *child;
   unsigned int i, len;
   unsigned int updated_size_start_id = 0, updated_entity_start_id = 0;
   Eina_Bool updated_size = EINA_FALSE, updated_entity = EINA_FALSE;
   Evas *e;

   e = evas_object_evas_get(obj);
   evas_event_freeze(e);

   EINA_VALUE_ARRAY_FOREACH(&v, len, i, child)
     {
        Efl_Ui_Collection_Item_Lookup *lookup;
        unsigned int search_index;
        //unsigned int v;

        efl_key_data_set(child, COLLECTION_VIEW_MANAGED, COLLECTION_VIEW_MANAGED_YES);
        /* fix eventing in scroller by ensuring collection items are in the scroller hierarchy */
        efl_ui_item_container_set(child, obj);
        efl_ui_widget_sub_object_add(obj, child);
        efl_canvas_group_member_add(pd->pan, child);
        efl_gfx_entity_visible_set(child, EINA_FALSE);

#ifdef VIEWPORT_ENABLE
        for (v = 0; v < 3; ++v)
          {
             if (!pd->viewport[v]) continue;

             if ((pd->viewport[v]->offset <= request->offset + i) &&
                 (request->offset + i < pd->viewport[v]->offset + pd->viewport[v]->count))
               {
                  unsigned int index = request->offset + i - pd->viewport[v]->offset;

                  if (pd->viewport[v]->items[index].entity)
                    {
                       ERR("Entity already existing for id %d", i);
                       efl_unref(pd->viewport[v]->items[index].entity);
                       efl_del(pd->viewport[v]->items[index].entity);
                       pd->viewport[v]->items[index].entity = NULL;
                    }

                  efl_replace(&pd->viewport[v]->items[index].entity, child);
                  if (_entity_propagate(pd->viewport[v]->items[index].model, child))
                    {
                       if (!updated_size)
                         {
                            updated_size = EINA_TRUE;
                            updated_size_start_id = index;
                         }
                    }
                  else
                    {
                       if (updated_size)
                         {
                            efl_ui_position_manager_entity_item_size_changed(pd->manager,
                                                                             updated_size_start_id,
                                                                             index - 1);
                            updated_size = EINA_FALSE;
                         }
                    }
                  child = NULL;
                  break;
               }
          }
#endif
        // When requesting an entity, the model should already be in the cache
        if (!child) continue;

        search_index = request->offset + i;

        lookup = (void*) eina_rbtree_inline_lookup(pd->cache, &search_index,
                                                   sizeof (search_index), _cache_tree_lookup,
                                                   NULL);

        if (!lookup)
          {
             Efl_Gfx_Entity *entities[1] = { child };
             efl_ui_factory_release(pd->factory, EINA_C_ARRAY_ITERATOR_NEW(entities));
             continue;
          }
        if (lookup->item.entity)
          {
             ERR("Entity already existing for id %u", search_index);
             _entity_cleanup(obj, pd->factory, &lookup->item, NULL);
          }

        lookup->item.entity = efl_ref(child);
        efl_event_callback_array_add(child, active_item_cbs(), obj);
        efl_event_callback_call(obj, EFL_UI_COLLECTION_VIEW_EVENT_ITEM_REALIZED, child);

        if (!updated_entity)
          {
             updated_entity = EINA_TRUE;
             updated_entity_start_id = search_index;
          }

        if (_entity_propagate(lookup->item.model, child))
          {
             if (!updated_size)
               {
                  updated_size = EINA_TRUE;
                  updated_size_start_id = search_index;
               }
          }
        else
          {
             if (updated_size)
               {
                  efl_ui_position_manager_entity_item_size_changed(pd->manager,
                                                                   updated_size_start_id,
                                                                   search_index - 1);
                  updated_size = EINA_FALSE;
               }
          }
     }

   evas_event_thaw(e);
   evas_event_thaw_eval(e);

   // Check if the last child is also the list item in the list
   _last_entity_update(pd, child);

   // Currently position manager will flush its entire size cache on update, so only do
   // it when necessary to improve performance.
   if (updated_size || request->need_size)
     {
        efl_ui_position_manager_entity_item_size_changed(pd->manager,
                                                         updated_size_start_id,
                                                         request->offset + i - 1);
        updated_size = EINA_FALSE;
     }

   // Notify the position manager that new entity are ready to display
   if (updated_entity)
     {
        efl_ui_position_manager_entity_entities_ready(pd->manager,
                                                      updated_entity_start_id,
                                                      request->offset + i - 1);

        efl_event_callback_del(efl_main_loop_get(), EFL_LOOP_EVENT_IDLE, _idle_cb, obj);
        efl_event_callback_add(efl_main_loop_get(), EFL_LOOP_EVENT_IDLE, _idle_cb, obj);
     }
   return v;
}

static void
_entity_free_cb(Eo *o, void *data, const Eina_Future *dead_future EINA_UNUSED)
{
   MY_DATA_GET(o, pd);
   Efl_Ui_Collection_Request *request = data;

   pd->requests = eina_list_remove(pd->requests, request);
   free(request);
}

static Eina_Bool
_focus_lookup(Efl_Ui_Collection_View_Data *pd, unsigned int search_index,
              Efl_Gfx_Entity **entity, Efl_Model **model)
{
   unsigned int idx;

   if (entity) *entity = pd->focus.last;
   if (pd->focus.last)
     {
        idx = _lookup_entity_index(pd->focus.last, model);
        if (idx == search_index) return EINA_TRUE;
     }
   if (entity) *entity = pd->focus.previously;
   if (pd->focus.previously)
     {
        idx = _lookup_entity_index(pd->focus.previously, model);
        if (idx == search_index) return EINA_TRUE;
     }

   if (entity) *entity = NULL;
   if (model) *model = NULL;
   return EINA_FALSE;
}

static Efl_Ui_Collection_Item_Lookup *
_build_from_focus(Efl_Ui_Collection_View_Data *pd, unsigned int search_index,
                  Efl_Model **model)
{
   Efl_Ui_Collection_Item_Lookup *insert;
   Efl_Gfx_Entity *entity = NULL;

   // Not found in the cache lookup, but just maybe
   if (!_focus_lookup(pd, search_index, &entity, model)) return NULL;

   // Lucky us, let's add it to the cache
   insert = calloc(1, sizeof (Efl_Ui_Collection_Item_Lookup));
   if (!insert) return NULL;

   insert->index = search_index;
   insert->item.model = efl_ref(*model);
   insert->item.entity = efl_ref(entity);

   pd->cache = eina_rbtree_inline_insert(pd->cache, EINA_RBTREE_GET(insert),
                                         _cache_tree_cmp, NULL);

   return insert;
}

static Eina_List *
_cache_size_fetch(Eina_List *requests, Efl_Ui_Collection_Request **request,
                  Efl_Ui_Collection_View_Data *pd,
                  unsigned int search_index,
                  Efl_Ui_Position_Manager_Size_Batch_Entity *target,
                  Eina_Size2D item_base)
{
   Efl_Ui_Collection_Item_Lookup *lookup;
   Efl_Model *model = NULL;
   Eina_Size2D item_size = item_base;

   if (!pd->cache) goto not_found;

   lookup = (void*) eina_rbtree_inline_lookup(pd->cache, &search_index,
                                              sizeof (search_index), _cache_tree_lookup,
                                              NULL);
   // In the cache we should always have model, so no need to check for it
   if (lookup)
     {
        model = lookup->item.model;
     }
   else
     {
        lookup = _build_from_focus(pd, search_index, &model);
        if (!lookup) goto not_found;
     }

   // If we do not know the size
   if (!ITEM_SIZE_FROM_MODEL(model, item_size))
     {
        if (lookup->item.entity)
          {
             ERR("Got a model '%s' and an item '%s', but no size. Recalculating.",
                 efl_debug_name_get(model), efl_debug_name_get(lookup->item.entity));
             _entity_propagate(model, lookup->item.entity);
             if (!ITEM_SIZE_FROM_MODEL(model, item_size))
               {
                  CRI("No size for itme '%s' after recalculating. This is bad.",
                      efl_debug_name_get(lookup->item.entity));
               }
          }
        else if (!ITEM_BASE_SIZE_FROM_MODEL(pd->model, item_size))
          {
             INF("No base size yet available. Making things up.");
             item_size.w = 1;
             item_size.h = 1;
          }
     }

   target->size = item_size;
   target->element_depth = 0;
   target->depth_leader = EINA_FALSE;
   return requests;

 not_found:
   requests = _request_add(requests, request, search_index, EINA_FALSE);

   target->size = item_size;
   target->element_depth = 0;
   target->depth_leader = EINA_FALSE;
   return requests;
}

static Eina_List *
_cache_entity_fetch(Eina_List *requests, Efl_Ui_Collection_Request **request,
                    Efl_Ui_Collection_View_Data *pd,
                    unsigned int search_index,
                    Efl_Ui_Position_Manager_Object_Batch_Entity *target)
{
   Efl_Ui_Collection_Item_Lookup *lookup;
   Efl_Model *model = NULL;

   if (!pd->cache) goto not_found;

   lookup = (void*) eina_rbtree_inline_lookup(pd->cache, &search_index,
                                              sizeof (search_index), _cache_tree_lookup,
                                              NULL);
   if (!lookup) lookup = _build_from_focus(pd, search_index, &model);
   if (!lookup) goto not_found;
   if (!lookup->item.entity) goto not_found;

   if (target) target->entity = lookup->item.entity;
   goto finish;

 not_found:
   requests = _request_add(requests, request, search_index, EINA_TRUE);

   if (target) target->entity = NULL;
 finish:
   if (!target) return requests;

   target->element_depth = 0;
   target->depth_leader = EINA_FALSE;

   return requests;
}

static Eina_Bool
_entity_request(Efl_Ui_Collection_View *obj, Efl_Ui_Collection_Request *request)
{
   if (request->entity_requested) return EINA_TRUE;
   request->f = efl_future_then(obj, request->f,
                                .success_type = EINA_VALUE_TYPE_ARRAY,
                                .success = _entity_fetch_cb);
   request->f = efl_future_then(obj, request->f,
                                .success_type = EINA_VALUE_TYPE_ARRAY,
                                .success = _entity_fetched_cb,
                                .data = request,
                                .free = _entity_free_cb);
   request->entity_requested = EINA_TRUE;
   request->need_entity = EINA_TRUE;

   return EINA_TRUE;
}

static inline Eina_Bool
_entity_inflight_request(Efl_Ui_Collection_View *obj,
                         Efl_Ui_Collection_Request *request,
                         Efl_Ui_Collection_Request *inflight)
{
   inflight->need_size |= request->need_size;
   if (request->need_entity == EINA_FALSE) return EINA_TRUE;

   return _entity_request(obj, inflight);
}

static Eina_List *
_batch_request_flush(Eina_List *requests,
                     Efl_Ui_Collection_View *obj,
                     Efl_Ui_Collection_View_Data *pd)
{
   Efl_Ui_Collection_Request *request;
   Eina_List *ll, *next_list_item;

   EINA_LIST_FOREACH_SAFE(requests, ll, next_list_item, request)
     {
        // Check request intersection with all pending request
        Efl_Ui_Collection_Request *inflight;
        Efl_Model *model;
        Eina_List *l;

        EINA_LIST_FOREACH(pd->requests, l, inflight)
          {
             unsigned int istart = inflight->offset;
             unsigned int iend = inflight->offset + inflight->length;
             unsigned int rstart = request->offset;
             unsigned int rend = request->offset + request->length;

             // Way before
             if (rend < istart) continue;
             // Way after
             if (rstart >= iend) continue;

             // request included in current inflight request
             if (rstart >= istart && rend <= iend)
               {
                  if (!_entity_inflight_request(obj, request, inflight)) continue;

                  // In this case no need to start a request
                  requests = eina_list_remove_list(requests, ll);
                  free(request);
                  request = NULL;
                  break;
               }

             // request overflow left and right
             if (rstart < istart && iend < rend)
               {
                  if (!_entity_inflight_request(obj, request, inflight)) continue;

                  // Remove the center portion of the request by emitting a new one
                  Efl_Ui_Collection_Request *rn;

                  rn = calloc(1, sizeof (Efl_Ui_Collection_Request));
                  if (!rn) break;

                  rn->offset = iend;
                  rn->length = rend - iend;
                  rn->need_entity = request->need_entity;
                  rn->need_size = request->need_size;

                  requests = eina_list_append(requests, rn);

                  request->length = istart - rstart;
                  continue;
               }

             // request overflow left
             if (rstart < istart && rend > istart && rend <= iend)
               {
                  if (!_entity_inflight_request(obj, request, inflight)) continue;
                  request->length = istart - rstart;
                  continue;
               }

             // request overflow right
             if (rstart >= istart && rstart < iend && iend <= rend)
               {
                  if (!_entity_inflight_request(obj, request, inflight)) continue;
                  request->offset = iend;
                  request->length = rend - iend;
                  continue;
               }
          }

        if (!request) continue;

        model = pd->model;
        // Are we ready yet
        if (!model)
          {
             requests = eina_list_remove_list(requests, ll);
             free(request);
             continue;
          }
        // Is the request inside the limit of the model?
        if (request->offset >= efl_model_children_count_get(model))
          {
             requests = eina_list_remove_list(requests, ll);
             free(request);
             continue;
          }
        // Is its limit outside the model limit?
        if (request->offset + request->length >= efl_model_children_count_get(model))
          {
             request->length = efl_model_children_count_get(model) - request->offset;
          }

        // We now have a request, time to trigger a fetch
        // We assume here that we are always fetching the model (model_requested must be true)
        request->f = efl_model_children_slice_get(model, request->offset, request->length);
        request->f = efl_future_then(obj, request->f,
                                     .success = _model_fetched_cb,
                                     .data = request,
                                     .free = _model_free_cb);

        eina_list_move_list(&pd->requests, &requests, ll);
     }
   return eina_list_free(requests);
}

static Efl_Ui_Position_Manager_Size_Batch_Result
_batch_size_cb(void *data, Efl_Ui_Position_Manager_Size_Call_Config conf, Eina_Rw_Slice memory)
{
   MY_DATA_GET(data, pd);
   Efl_Ui_Position_Manager_Size_Batch_Entity *sizes;
   Efl_Ui_Collection_Request *request = NULL;
   Efl_Ui_Position_Manager_Size_Batch_Result result = {0};
   Efl_Model *parent;
   Eina_List *requests = NULL;
   Eina_Size2D item_base = {0};
   unsigned int limit;
   unsigned int idx = 0;

   // get the approximate value from the tree node
   parent = pd->model;

   sizes = memory.mem;
   //count = efl_model_children_count_get(parent);
   limit = conf.range.end_id - conf.range.start_id;
   ITEM_BASE_SIZE_FROM_MODEL(parent, item_base);

   // Look in the temporary cache now for the beginning of the buffer
#ifdef VIEWPORT_ENABLE
   if (pd->viewport[0] && ((unsigned int)(conf.range.start_id + idx) < pd->viewport[0]->offset))
     {
        while ((unsigned int)(conf.range.start_id + idx) < pd->viewport[0]->offset && idx < limit)
          {
             unsigned int search_index = conf.range.start_id + idx;
             requests = _cache_size_fetch(requests, &request, pd,
                                          search_index, &sizes[idx], item_base);
             idx++;
          }
     }

   // Then look in our buffer view if the needed information can be found there
   for (i = 0; i < 3; ++i)
     {
        if (!pd->viewport[i]) continue;

        while (idx < limit &&
               (pd->viewport[i]->offset <= conf.range.start_id + idx) &&
               (conf.range.start_id + idx < (pd->viewport[i]->offset + pd->viewport[i]->count)))
          {
             unsigned int offset = conf.range.start_id + idx - pd->viewport[i]->offset;
             Efl_Model *model = pd->viewport[i]->items[offset].model;
             Efl_Gfx_Entity *entity = pd->viewport[i]->items[offset].entity;
             Eina_Bool entity_request = EINA_FALSE;

             if (model)
               {
                  Eina_Size2D item_size;
                  Eina_Bool found = EINA_FALSE;

                  if (ITEM_SIZE_FROM_MODEL(model, item_size))
                    found = EINA_TRUE;
                  if (!found && entity)
                    {
                       item_size = efl_gfx_hint_size_combined_min_get(entity);
                       //if the size is 0 here, then we are running into trouble,
                       //fetch size from the parent model, where some fallback is defined
                       if (item_size.h == 0 && item_size.w == 0)
                         {
                            item_size = item_base;
                            found = EINA_TRUE;
                         }
                       else
                         {
                            _size_to_model(model, item_size);
                            found = EINA_TRUE;
                         }

                    }

                  if (found)
                    {
                       sizes[idx].size = item_size;
                       sizes[idx].element_depth = 0;
                       sizes[idx].depth_leader = EINA_FALSE;
                       goto done;
                    }

                  // We will need an entity to calculate this size
                  entity_request = EINA_TRUE;
               }
             // No data, add to the requests
             requests = _request_add(requests, &request, conf.range.start_id + idx, entity_request);

             sizes[idx].size = item_base;
             sizes[idx].element_depth = 0;
             sizes[idx].depth_leader = EINA_FALSE;

          done:
             idx++;
          }
     }

   // Look in the temporary cache now for the end of the buffer
   while (idx < limit)
     {
        unsigned int search_index = conf.range.start_id + idx;
        requests = _cache_size_fetch(requests, &request, pd,
                                     search_index, &sizes[idx], item_base);
        idx++;
     }
#endif

   /* if (conf.cache_request) */
   /*   { */
   /*      printf("CACHING SIZE CALL\n"); */
   /*      while (idx < limit) */
   /*          { */
   /*             sizes[idx].depth_leader = EINA_FALSE; */
   /*             sizes[idx].element_depth = 0; */
   /*             sizes[idx].size = pd->last_base; */
   /*             idx++; */
   /*        } */
   /*      fprintf(stderr, "read with no fetch\n"); */
   /*   } */
   /* else */
     {
        while (idx < limit)
          {
             unsigned int search_index = conf.range.start_id + idx;
             requests = _cache_size_fetch(requests, &request, pd,
                                          search_index, &sizes[idx], item_base);
             idx++;
          }


        // Done, but flush request first
        if (request) requests = eina_list_append(requests, request);

        requests = _batch_request_flush(requests, data, pd);
     }

   // Get the amount of filled item
   result.filled_items = limit;

   return result;
}

static Efl_Ui_Position_Manager_Object_Batch_Result
_batch_entity_cb(void *data, Efl_Ui_Position_Manager_Request_Range range, Eina_Rw_Slice memory)
{
   MY_DATA_GET(data, pd);
   Efl_Ui_Position_Manager_Object_Batch_Entity *entities;
   Efl_Ui_Collection_Request *request = NULL;
   Efl_Ui_Position_Manager_Object_Batch_Result result = {0};
   Eina_List *requests = NULL;
#ifdef VIEWPORT_ENABLE
   Efl_Model *parent;
#endif
   unsigned int limit;
   unsigned int idx = 0;

   //parent = pd->model;

   entities = memory.mem;
   //count = efl_model_children_count_get(parent);
   limit = range.end_id - range.start_id;

   // Look in the temporary cache now for the beginning of the buffer
#ifdef VIEWPORT_ENABLE
   if (pd->viewport[0] && ((unsigned int)(range.start_id + idx) < pd->viewport[0]->offset))
     {
        while (idx < limit && (unsigned int)(range.start_id + idx) < pd->viewport[0]->offset)
          {
             unsigned int search_index = range.start_id + idx;

             requests = _cache_entity_fetch(requests, &request, pd,
                                            search_index, &entities[idx]);

             idx++;
          }
     }

   // Then look in our buffer view if the needed information can be found there
   for (i = 0; i < 3; ++i)
     {
        if (!pd->viewport[i]) continue;

        while (idx < limit &&
               (pd->viewport[i]->offset <= range.start_id + idx) &&
               (range.start_id + idx < (pd->viewport[i]->offset + pd->viewport[i]->count)))
          {
             unsigned int offset = range.start_id + idx - pd->viewport[i]->offset;
             Efl_Gfx_Entity *entity = pd->viewport[i]->items[offset].entity;

             if (!entity)
               {
                  // No data, add to the requests
                  requests = _request_add(requests, &request, range.start_id + idx, EINA_TRUE);

                  entities[idx].entity = NULL;
                  entities[idx].depth_leader = EINA_FALSE;
                  entities[idx].element_depth = 0;
               }
             else
               {
                  entities[idx].entity = entity;
                  entities[idx].depth_leader = EINA_FALSE;
                  entities[idx].element_depth = 0;
               }

             idx++;
          }
     }
#endif

   // Look in the temporary cache now for the end of the buffer
   while (idx < limit)
     {
        unsigned int search_index = range.start_id + idx;

        requests = _cache_entity_fetch(requests, &request, pd,
                                       search_index, &entities[idx]);
        idx++;
     }
   // Done, but flush request first
   if (request)
     {
        requests = eina_list_append(requests, request);
     }

   requests = _batch_request_flush(requests, data, pd);

   // Get the amount of filled item
   result.filled_items = limit;

   return result;
}


#if 0
static void
_batch_free_cb(void *data)
{
   efl_unref(data);
}
#endif

static void
flush_min_size(Eo *obj, Efl_Ui_Collection_View_Data *pd)
{
   Eina_Size2D tmp = pd->content_min_size;

   if (!pd->match_content.w)
     tmp.w = -1;

   if (!pd->match_content.h)
     tmp.h = -1;

   efl_gfx_hint_size_restricted_min_set(obj, tmp);
}

static void
_manager_content_size_changed_cb(void *data, const Efl_Event *ev)
{
   Eina_Size2D *size = ev->info;
   MY_DATA_GET(data, pd);

   efl_gfx_entity_size_set(pd->sizer, *size);
}

static void
_manager_content_min_size_changed_cb(void *data, const Efl_Event *ev)
{
   Eina_Size2D *size = ev->info;
   MY_DATA_GET(data, pd);

   pd->content_min_size = *size;

   flush_min_size(data, pd);
}

#ifdef VIEWPORT_ENABLE
static Eina_List *
_viewport_walk_fill(Eina_List *requests,
                    Efl_Ui_Collection_View *obj,
                    Efl_Ui_Collection_View_Data *pd,
                    Efl_Ui_Collection_Viewport *viewport)
{
   Efl_Ui_Collection_Request *current = NULL;
   unsigned int j;

   for (j = 0; j < viewport->count; j++)
     {
        Efl_Ui_Collection_Item_Lookup *lookup;
        unsigned int index = viewport->offset + j;

        if (viewport->items[j].model) goto check_entity;

        lookup = (void*) eina_rbtree_inline_lookup(pd->cache, &index,
                                                   sizeof (index), _cache_tree_lookup,
                                                   NULL);

        if (lookup)
          {
             efl_replace(&viewport->items[j].model, lookup->item.model);
             efl_replace(&viewport->items[j].entity, lookup->item.entity);
             efl_replace(&lookup->item.entity, NULL); // Necessary to avoid premature release

             pd->cache = eina_rbtree_inline_remove(pd->cache, EINA_RBTREE_GET(lookup),
                                                   _cache_tree_cmp, NULL);
             _cache_item_free(EINA_RBTREE_GET(lookup), obj);
          }

     check_entity:
        if (viewport->items[j].entity) continue ;
        requests = _request_add(requests, &current, index, EINA_TRUE);
     }

   // We do break request per viewport, just in case we generate to big batch at once
   if (current) requests = eina_list_append(requests, current);

   return requests;
}

#endif

// An RbTree has the nice property of sorting content. The smaller than the root being in
// son[1] and the greater than the root in son[0]. Using this we can efficiently walk the
// tree once to take note of all the item that need cleaning.
static void
_mark_lesser(Efl_Ui_Collection_Item_Lookup *root, Eina_Array *mark, const unsigned int lower)
{
   if (!root) return ;

   if (root->index < lower)
     {
        eina_array_push(mark, root);
        _mark_lesser((void*) EINA_RBTREE_GET(root)->son[1], mark, lower);
     }
   else
     {
        _mark_lesser((void*) EINA_RBTREE_GET(root)->son[0], mark, lower);
        _mark_lesser((void*) EINA_RBTREE_GET(root)->son[1], mark, lower);
     }
}

static void
_mark_ge(Efl_Ui_Collection_Item_Lookup *root, Eina_Array *mark, const unsigned int upper)
{
   if (!root) return ;

   if (root->index >= upper)
     {
        eina_array_push(mark, root);
        _mark_ge((void*) EINA_RBTREE_GET(root)->son[0], mark, upper);
        _mark_ge((void*) EINA_RBTREE_GET(root)->son[1], mark, upper);
     }
   else
     {
        _mark_ge((void*) EINA_RBTREE_GET(root)->son[0], mark, upper);
     }
}

// we walk the tree twice, once for everything below the limit and once for everything above
// then we do free each item individually.
static void
_idle_cb(void *data, const Efl_Event *event EINA_UNUSED)
{
   Efl_Ui_Collection_Item_Lookup *lookup;
   Eina_Array mark;
   Eina_Array scheduled_release;
   MY_DATA_GET(data, pd);
   const unsigned int length = pd->current_range.end_id - pd->current_range.start_id;
   const unsigned int lower_end = MAX((long)pd->current_range.start_id - (long)length/2, 0);
   const unsigned int upper_end = pd->current_range.end_id + length/2;
   Eina_Array_Iterator iterator;
   unsigned int i;

   eina_array_step_set(&mark, sizeof (Eina_Array), 16);
   eina_array_step_set(&scheduled_release, sizeof (Eina_Array), 16);

   _mark_lesser((void*) pd->cache, &mark, lower_end);
   _mark_ge((void*) pd->cache, &mark, upper_end);

   EINA_ARRAY_ITER_NEXT(&mark, i, lookup, iterator)
     {
        pd->cache = (void*) eina_rbtree_inline_remove(pd->cache,
                                                      EINA_RBTREE_GET(lookup),
                                                      _cache_tree_cmp, NULL);
        _item_cleanup(data, pd->factory, &lookup->item, &scheduled_release);
        free(lookup);
     }
   eina_array_flush(&mark);

   efl_ui_factory_release(pd->factory, eina_array_iterator_new(&scheduled_release));
   eina_array_flush(&scheduled_release);

   efl_event_callback_del(efl_main_loop_get(), EFL_LOOP_EVENT_IDLE, _idle_cb, data);
}

#ifndef VIEWPORT_ENABLE
static void
_manager_content_visible_range_changed_cb(void *data, const Efl_Event *ev)
{
   Efl_Ui_Position_Manager_Range_Update *event = ev->info;
   unsigned int count;
   unsigned int lower_end;
   unsigned int upper_end;
   long length;
   Efl_Ui_Collection_Request *request = NULL;
   Eina_List *requests = NULL;
   unsigned int idx;
   MY_DATA_GET(data, pd);

   pd->current_range.start_id = event->start_id;
   pd->current_range.end_id = event->end_id;

   count = efl_model_children_count_get(efl_ui_view_model_get(data));

   length = pd->current_range.end_id - pd->current_range.start_id;
   lower_end = MAX((long)pd->current_range.start_id - (length / 2), 0);
   upper_end = MIN(pd->current_range.end_id + (length / 2), count);

   idx = lower_end;
   while (idx < upper_end)
     {
        unsigned int search_index = idx;

        requests = _cache_entity_fetch(requests, &request, pd,
                                       search_index, NULL);

        idx++;
     }
   // Done, but flush request first
   if (request) requests = eina_list_append(requests, request);

   requests = _batch_request_flush(requests, data, pd);
}
#endif

#ifdef VIEWPORT_ENABLE
static void
_manager_content_visible_range_changed_cb(void *data, const Efl_Event *ev)
{
   Efl_Ui_Position_Manager_Range_Update *event = ev->info;
   MY_DATA_GET(data, pd);
   Eina_List *requests = NULL;
   long baseid;
   unsigned int delta, marginup, margindown;
   unsigned int upperlimit_offset, lowerlimit_offset;
   unsigned int i;

   pd->start_id = event->start_id;
   pd->end_id = event->end_id;

   delta = pd->end_id - pd->start_id;

   // First time setting up the viewport, so trigger request as we see fit
   if (!pd->viewport[0])
     {
        baseid = pd->start_id - delta;

        for (i = 0; i < 3; i++)
          {
             pd->viewport[i] = calloc(1, sizeof (Efl_Ui_Collection_Viewport));
             if (!pd->viewport[i]) continue;

             pd->viewport[i]->offset = MAX(baseid + delta * i, 0);
             pd->viewport[i]->count = delta;
             pd->viewport[i]->items = calloc(delta, sizeof (Efl_Ui_Collection_Item));
             if (!pd->viewport[i]->items) continue ;

             requests = _viewport_walk_fill(requests, data, pd, pd->viewport[i]);
          }

        goto flush_requests;
     }

   // Compute limit offset
   upperlimit_offset = delta * 3 + pd->viewport[0]->offset;
   lowerlimit_offset = 0;

   // Adjust the viewport for size or to much offset change in two step

   // Trying to resize first if there size is in bigger/smaller than 25% of the original size
   margindown = delta * 75 / 100;
   marginup = delta * 125 / 100;
   if (margindown < pd->viewport[0]->count &&
       pd->viewport[0]->count < marginup)
     {
        // Trying to do the resize in an optimized way is complex, let's do it simple
        Efl_Ui_Collection_Item *items[3];
        unsigned int j = 0, t = 1;

        for (i = 0; i < 3; i++)
          {
             unsigned int m;

             items[i] = calloc(delta, sizeof (Efl_Ui_Collection_Item));
             if (!items[i]) continue;

             for (m = 0; m < delta && t < 3; m++)
               {
                  items[i][m] = pd->viewport[t]->items[j];

                  j++;
                  if (j < pd->viewport[t]->count) continue;

                  j = 0;
                  t++;
                  if (t == 3) break;
               }

             // Preserve last updated index to later build a request
             if (t == 3)
               {
                  upperlimit_offset = pd->viewport[0]->offset + i * delta + m;

                  t = 4; // So that we never come back here again
               }
          }

        // For now destroy leftover object, could be cached
        for (i = t; i < 3; i++)
          {
             for (; j < pd->viewport[i]->count; j++)
               {
                  _item_cleanup(pd->factory, &pd->viewport[i]->items[j]);
               }
             j = 0;
          }

        // And now define viewport back
        for (i = 0; i < 3; i++)
          {
             free(pd->viewport[i]->items);
             pd->viewport[i]->items = items[i];
             pd->viewport[i]->count = delta;
             pd->viewport[i]->offset = pd->viewport[0]->offset + delta * i;
          }
     }

   // We decided that resizing was unecessary
   delta = pd->viewport[0]->count;

   // Try to keep the visual viewport in between half of the first and last viewport

   // start_id is in the first half of the first viewport, assume upward move
   // start_id + delta is in the second half of the last viewport, assume upward move
   if (pd->viewport[0]->offset + delta / 2 < pd->start_id ||
       pd->start_id + delta > pd->viewport[2]->offset + delta / 2)
     {
        // We could optimize this to actually just move viewport around in most cases
        Efl_Ui_Collection_Item *items[3];
        unsigned int j = 0, t = 0;
        unsigned int target, current;

        // Case where are at the top
        if (pd->start_id < delta && pd->viewport[0]->offset == 0) goto build_request;

        // Trying to adjust the offset to maintain it in the center viewport +/- delta/2
        baseid = (pd->start_id < delta) ? 0 : pd->start_id - delta;

        // Lookup for starting point
        lowerlimit_offset = pd->viewport[0]->offset;
        target = baseid;

        // cleanup before target
        for (current = pd->viewport[t]->offset; current < target; current++)
          {
             _item_cleanup(pd->factory, &pd->viewport[t]->items[j]);

             j++;
             if (j < pd->viewport[t]->count) continue;

             j = 0;
             t++;
             if (t == 3) break;
          }

        // Allocation and copy
        for (i = 0; i < 3; i++)
          {
             unsigned int m;

             items[i] = calloc(delta, sizeof (Efl_Ui_Collection_Item));
             if (!items[i]) continue;

             for (m = 0; m < delta && t < 3; m++, target++)
               {
                  if (target < pd->viewport[t]->offset) continue ;
                  items[i][m] = pd->viewport[t]->items[j];

                  j++;
                  if (j < pd->viewport[t]->count) continue;

                  j = 0;
                  t++;
                  if (t == 3) break;
               }

             // Preserve last updated index to later build a request
             if (t == 3)
               {
                  if (upperlimit_offset > pd->viewport[0]->offset + i * delta + m)
                    {
                       upperlimit_offset = pd->viewport[0]->offset + i * delta + m;
                    }

                  t = 4; // So that we never come back here again
               }
          }

        // For now destroy leftover object, could be cached
        for (i = t; i < 3; i++)
          {
             for (; j < pd->viewport[i]->count; j++)
               {
                  _item_cleanup(pd->factory, &pd->viewport[i]->items[j]);
               }
             j = 0;
          }

        // And now define viewport back
        for (i = 0; i < 3; i++)
          {
             free(pd->viewport[i]->items);
             pd->viewport[i]->items = items[i];
             pd->viewport[i]->offset = baseid + delta * i;
          }
     }

 build_request:
   // Check if the first viewport has all the lower part of it filled with objects
   if (pd->viewport[0]->offset < lowerlimit_offset)
     {
        Efl_Ui_Collection_Request *request;

        request = calloc(1, sizeof (Efl_Ui_Collection_Request));
        if (request) return ;

        request->offset = lowerlimit_offset;
        // This length work over multiple viewport as they are contiguous
        request->length = lowerlimit_offset - pd->viewport[0]->offset;
        request->need_size = EINA_TRUE;
        request->need_entity = EINA_TRUE;

        requests = eina_list_append(requests, request);
     }

   // Check if the last viewport has all the upper part of it filler with objects
   if (pd->viewport[2]->offset + pd->viewport[2]->count > upperlimit_offset)
     {
        Efl_Ui_Collection_Request *request;

        request = calloc(1, sizeof (Efl_Ui_Collection_Request));
        if (request) return ;

        request->offset = upperlimit_offset;
        // This length work over multiple viewport as they are contiguous
        request->length = pd->viewport[2]->offset + pd->viewport[2]->count - upperlimit_offset;
        request->need_size = EINA_TRUE;
        request->need_entity = EINA_TRUE;

        requests = eina_list_append(requests, request);
     }

 flush_requests:
   requests = _batch_request_flush(requests, data, pd);
}
#endif

EFL_CALLBACKS_ARRAY_DEFINE(manager_cbs,
 { EFL_UI_POSITION_MANAGER_ENTITY_EVENT_CONTENT_SIZE_CHANGED, _manager_content_size_changed_cb },
 { EFL_UI_POSITION_MANAGER_ENTITY_EVENT_CONTENT_MIN_SIZE_CHANGED, _manager_content_min_size_changed_cb },
 { EFL_UI_POSITION_MANAGER_ENTITY_EVENT_VISIBLE_RANGE_CHANGED, _manager_content_visible_range_changed_cb }
)

static void
_item_scroll_internal(Eo *obj EINA_UNUSED,
                      Efl_Ui_Collection_View_Data *pd,
                      unsigned int index,
                      double align EINA_UNUSED,
                      Eina_Bool anim)
{
   Eina_Rect ipos, view;
   Eina_Position2D vpos;

   if (!pd->scroller) return;

   ipos = efl_ui_position_manager_entity_position_single_item(pd->manager, index);
   view = efl_ui_scrollable_viewport_geometry_get(pd->scroller);
   vpos = efl_ui_scrollable_content_pos_get(pd->scroller);

   ipos.x = ipos.x + vpos.x - view.x;
   ipos.y = ipos.y + vpos.y - view.y;

   //FIXME scrollable needs some sort of align, the docs do not even garantee to completely move in the element
   efl_ui_scrollable_scroll(pd->scroller, ipos, anim);
}

// Exported function

EOLIAN static void
_efl_ui_collection_view_factory_set(Eo *obj EINA_UNUSED, Efl_Ui_Collection_View_Data *pd,
                                  Efl_Ui_Factory *factory)
{
   if (pd->factory) efl_ui_property_bind(pd->factory, "selected", NULL);
   efl_replace(&pd->factory, factory);
   if (pd->factory) efl_ui_property_bind(pd->factory, "selected", "self.selected");
}

EOLIAN static Efl_Ui_Factory *
_efl_ui_collection_view_factory_get(const Eo *obj EINA_UNUSED, Efl_Ui_Collection_View_Data *pd)
{
   return pd->factory;
}

static void
_unref_cb(void *data)
{
   Eo *obj = data;

   efl_unref(obj);
}

EOLIAN static void
_efl_ui_collection_view_position_manager_set(Eo *obj, Efl_Ui_Collection_View_Data *pd,
                                             Efl_Ui_Position_Manager_Entity *manager)
{
   Efl_Model *model;
   unsigned int count;

   if (manager)
     EINA_SAFETY_ON_FALSE_RETURN(efl_isa(manager, EFL_UI_POSITION_MANAGER_ENTITY_INTERFACE));

   if (pd->manager)
     {
        efl_event_callback_array_del(pd->manager, manager_cbs(), obj);
        efl_del(pd->manager);
     }
   pd->manager = manager;
   if (!pd->manager) return;

   // Start watching change on model from here on
   model = pd->model;
   count = model ? efl_model_children_count_get(model) : 0;

   efl_parent_set(pd->manager, obj);
   efl_event_callback_array_add(pd->manager, manager_cbs(), obj);
        switch(efl_ui_position_manager_entity_version(pd->manager, 1))
          {
            case 1:
              efl_ui_position_manager_data_access_v1_data_access_set(pd->manager,
                efl_provider_find(obj, EFL_UI_WIN_CLASS),
                efl_ref(obj), _batch_entity_cb, _unref_cb,
                efl_ref(obj), _batch_size_cb, _unref_cb,
                count);
            break;
          }

   if (efl_finalized_get(obj))
     efl_ui_position_manager_entity_viewport_set(pd->manager, efl_ui_scrollable_viewport_geometry_get(obj));
   efl_ui_layout_orientation_set(pd->manager, pd->direction);
}

EOLIAN static Efl_Ui_Position_Manager_Entity *
_efl_ui_collection_view_position_manager_get(const Eo *obj EINA_UNUSED,
                                             Efl_Ui_Collection_View_Data *pd)
{
   return pd->manager;
}

static void
_efl_model_count_changed(void *data, const Efl_Event *event EINA_UNUSED)
{
   Efl_Ui_Collection_Request *request = NULL;
   Eina_List *requests = NULL;
   MY_DATA_GET(data, pd);
   unsigned int index;
   unsigned int count = 0;

   count = efl_model_children_count_get(pd->model);
   if (pd->focus.last)
     {
        index = _lookup_entity_index(pd->focus.last, NULL);

        if (index + 1 == count)
          return ;
     }

   // The last item is not the last item anymore
   requests = _request_add(requests, &request, count, EINA_TRUE);
   requests = _batch_request_flush(requests, data, pd);

   // We are not triggering efl_ui_position_manager_entity_data_access_set as it is can
   // only be slow, we rely on child added/removed instead (If we were to not rely on
   // child added/removed we could maybe use count changed)
}

static void
_efl_model_properties_changed(void *data EINA_UNUSED, const Efl_Event *event EINA_UNUSED)
{
   // We could here watch if the global base size item change and notify of a global change
   // But I can not find a proper way to do it for the object that are not visible, which
   // is kind of the point...
}

static void
_cache_cleanup_above(Efl_Ui_Collection_View *obj, Efl_Ui_Collection_View_Data *pd, unsigned int index)
{
   Efl_Ui_Collection_Item_Lookup *lookup;
   Eina_Array scheduled_release;
   Eina_Array mark;
   Eina_Array_Iterator iterator;
   unsigned int i;

   eina_array_step_set(&mark, sizeof (Eina_Array), 16);
   eina_array_step_set(&scheduled_release, sizeof (Eina_Array), 16);

   _mark_ge((void*) pd->cache, &mark, index);

   EINA_ARRAY_ITER_NEXT(&mark, i, lookup, iterator)
     {
        pd->cache = (void*) eina_rbtree_inline_remove(pd->cache,
                                                      EINA_RBTREE_GET(lookup),
                                                      _cache_tree_cmp, NULL);
        _item_cleanup(obj, pd->factory, &lookup->item, &scheduled_release);
        free(lookup);
     }
   eina_array_flush(&mark);

   efl_ui_factory_release(pd->factory, eina_array_iterator_new(&scheduled_release));
   eina_array_flush(&scheduled_release);
}

static void
_efl_model_child_added(void *data, const Efl_Event *event)
{
   // At the moment model only append child, but let's try to handle it theorically correct
   Efl_Model_Children_Event *ev = event->info;
   MY_DATA_GET(data, pd);
#ifdef VIEWPORT_ENABLE
   Eina_List *requests = NULL;
   unsigned int i;
#endif

   _cache_cleanup_above(data, pd, ev->index);

   // Check if we really have something to do
#ifdef VIEWPORT_ENABLE
   if (!pd->viewport[0]) goto notify_manager;

   // Insert the child in the viewport if necessary
   for (i = 0; i < 3; i++)
     {
        Efl_Ui_Collection_Request *request;
        unsigned int o;
        unsigned int j;

        if (ev->index < pd->viewport[i]->offset)
          {
             pd->viewport[i]->offset++;
             continue;
          }
        if (pd->viewport[i]->offset + pd->viewport[i]->count < ev->index)
          {
             continue;
          }

        for (j = 2; j > i; j--)
          {
             _item_cleanup(pd->factory, &pd->viewport[j]->items[pd->viewport[j]->count - 1]);
             memmove(&pd->viewport[j]->items[1],
                     &pd->viewport[j]->items[0],
                     (pd->viewport[j]->count - 1) * sizeof (Efl_Ui_Collection_Item));
             pd->viewport[j]->items[0] = pd->viewport[j - 1]->items[pd->viewport[j - 1]->count - 1];
             pd->viewport[j - 1]->items[pd->viewport[j - 1]->count - 1].entity = NULL;
             pd->viewport[j - 1]->items[pd->viewport[j - 1]->count - 1].model = NULL;
          }
        o = ev->index - pd->viewport[i]->offset;
        memmove(&pd->viewport[j]->items[o],
                &pd->viewport[j]->items[o + 1],
                (pd->viewport[j]->count - 1 - o) * sizeof (Efl_Ui_Collection_Item));
        pd->viewport[j]->items[o].entity = NULL;
        pd->viewport[j]->items[o].model = efl_ref(ev->child);

        request = calloc(1, sizeof (Efl_Ui_Collection_Request));
        if (!request) break;
        request->offset = ev->index;
        request->length = 1;
        request->need_size = EINA_TRUE;
        request->need_entity = EINA_TRUE;

        requests = eina_list_append(requests, request);

        requests = _batch_request_flush(requests, data, pd);

        break;
     }

 notify_manager:
#endif
   efl_ui_position_manager_entity_item_added(pd->manager, ev->index, NULL);
}

static void
_efl_model_child_removed(void *data, const Efl_Event *event)
{
   Efl_Model_Children_Event *ev = event->info;
   MY_DATA_GET(data, pd);
   Eina_List *requests = NULL;
#ifdef VIEWPORT_ENABLE
   unsigned int i;
#endif
   unsigned int upper_end;
   long length;
   unsigned int count;
   unsigned int request_length;

   // FIXME: later optimization, instead of reloading everyone, we could actually track index and self
   // update would be more efficient, but it is also more tricky
   _cache_cleanup_above(data, pd, ev->index);

   count = efl_model_children_count_get(event->object);
   length = pd->current_range.end_id - pd->current_range.start_id;
   upper_end = MIN(pd->current_range.end_id + (length / 2), count);

   // Check if we really have something to do
#ifdef VIEWPORT_ENABLE
   if (!pd->viewport[0]) goto notify_manager;

   // Insert the child in the viewport if necessary
   for (i = 0; i < 3; i++)
     {
        Efl_Ui_Collection_Request *request;
        unsigned int o;

        if (ev->index < pd->viewport[i]->offset)
          {
             pd->viewport[i]->offset--;
             continue;
          }
        if (pd->viewport[i]->offset + pd->viewport[i]->count < ev->index)
          {
             continue;
          }

        o = ev->index - pd->viewport[i]->offset;
        _item_cleanup(pd->factory, &pd->viewport[i]->items[o]);
        for (; i < 3; i++)
          {
             memmove(&pd->viewport[i]->items[o],
                     &pd->viewport[i]->items[o + 1],
                     (pd->viewport[i]->count - 1 - o) * sizeof (Efl_Ui_Collection_Item));
             if (i + 1 < 3)
               {
                  pd->viewport[i]->items[pd->viewport[i]->count - 1] = pd->viewport[i + 1]->items[0];
               }
             else
               {
                  pd->viewport[i]->items[pd->viewport[i]->count - 1].entity = NULL;
                  pd->viewport[i]->items[pd->viewport[i]->count - 1].model = NULL;
               }
             o = 0;
          }

        request = calloc(1, sizeof (Efl_Ui_Collection_Request));
        if (!request) break;
        request->offset = pd->viewport[2]->offset + pd->viewport[i]->count - 1;
        request->length = 1;
        request->need_size = EINA_TRUE;
        request->need_entity = EINA_TRUE;

        requests = eina_list_append(requests, request);

        requests = _batch_request_flush(requests, data, pd);

        break;
     }

 notify_manager:
#endif
   request_length = upper_end - ev->index;

   if (request_length > 0)
     {
        Efl_Ui_Collection_Request *request = NULL;

        requests = _request_add(requests, &request, ev->index, EINA_TRUE);
        request->length = request_length;
        requests = eina_list_append(requests, request);
        requests = _batch_request_flush(requests, data, pd);
     }

   efl_ui_position_manager_entity_item_removed(pd->manager, ev->index, NULL);
}

EFL_CALLBACKS_ARRAY_DEFINE(model_cbs,
                           { EFL_MODEL_EVENT_CHILDREN_COUNT_CHANGED, _efl_model_count_changed },
                           { EFL_MODEL_EVENT_PROPERTIES_CHANGED, _efl_model_properties_changed },
                           { EFL_MODEL_EVENT_CHILD_ADDED, _efl_model_child_added },
                           { EFL_MODEL_EVENT_CHILD_REMOVED, _efl_model_child_removed })

static void
_efl_ui_collection_view_model_changed(void *data, const Efl_Event *event)
{
   Efl_Model_Changed_Event *ev = event->info;
   Eina_List *requests = NULL;
   MY_DATA_GET(data, pd);
   Eina_Iterator *it;
   const char *property;
   Efl_Model *model = NULL;
   unsigned int count;
   Efl_Model *mselect = NULL;
   Eina_Bool selection = EINA_FALSE, sizing = EINA_FALSE;

   // Cleanup all object, pending request to prepare refetching everything
   _all_cleanup(data, pd);
   if (pd->model) efl_event_callback_array_del(pd->model, model_cbs(), data);
   if (pd->multi_selectable_async_model)
     {
        efl_event_callback_forwarder_del(pd->multi_selectable_async_model,
                                         EFL_UI_SELECTABLE_EVENT_SELECTION_CHANGED,
                                         data);
        efl_composite_detach(data, pd->multi_selectable_async_model);
     }

   if (!ev->current)
     {
        efl_replace(&pd->model, NULL);
        efl_replace(&pd->multi_selectable_async_model, NULL);
        return ;
     }

   it = efl_model_properties_get(ev->current);
   EINA_ITERATOR_FOREACH(it, property)
     {
        // Check if the model provide selection
        if (eina_streq(property, "child.selected"))
          selection = EINA_TRUE;
        // Check if the model provide sizing logic
        else if (eina_streq(property, _efl_model_property_itemw) ||
                 eina_streq(property, _efl_model_property_itemh))
          sizing = EINA_TRUE;
     }
   eina_iterator_free(it);

   if (selection)
     {
        // Search the composition of model for the one providing MULTI_SELECTABLE_ASYNC
        mselect = ev->current;
        while (mselect &&
               !efl_isa(mselect, EFL_UI_MULTI_SELECTABLE_INDEX_RANGE_INTERFACE) &&
               efl_isa(mselect, EFL_COMPOSITE_MODEL_CLASS))
          mselect = efl_ui_view_model_get(mselect);

        if (!efl_isa(mselect, EFL_UI_MULTI_SELECTABLE_INDEX_RANGE_INTERFACE))
          {
             mselect = NULL;
             selection = EINA_FALSE;
          }
     }

   // Try to build the minimal chain of necessary model for collection view
   model = ev->current;

   // Build and connect the selection model properly
   if (!mselect)
     {
        mselect = model = efl_add_ref(EFL_UI_SELECT_MODEL_CLASS, data,
                                      efl_ui_view_model_set(efl_added, model),
                                      efl_loop_model_volatile_make(efl_added));
     }
   efl_replace(&pd->multi_selectable_async_model, mselect);
   efl_composite_attach(data, pd->multi_selectable_async_model);
   efl_event_callback_forwarder_add(pd->multi_selectable_async_model,
                                    EFL_UI_SELECTABLE_EVENT_SELECTION_CHANGED,
                                    data);

   if (!sizing) model = efl_add_ref(EFL_UI_HOMOGENEOUS_MODEL_CLASS, data,
                                    efl_ui_view_model_set(efl_added, model),
                                    efl_loop_model_volatile_make(efl_added));

   efl_replace(&pd->model, model);
   efl_event_callback_array_add(pd->model, model_cbs(), data);

   if (mselect) efl_unref(mselect);
   if (!sizing) efl_unref(model);

   count = efl_model_children_count_get(model);

#ifdef VIEWPORT_ENABLE
   for (i = 0; i < 3; i++)
     {

        if (!pd->viewport[i]) continue ;
        if (pd->viewport[i]->count == 0) continue ;

        request = calloc(1, sizeof (Efl_Ui_Collection_Request));
        if (!request) continue ;

        request->offset = pd->viewport[i]->offset;
        request->length = pd->viewport[i]->count;
        request->need_size = EINA_TRUE;
        request->need_entity = EINA_TRUE;

        requests = eina_list_append(requests, request);
     }
#endif

   // Fetch last item if necessary for later focus
   if (efl_model_children_count_get(model))
     {
        Efl_Ui_Collection_Request *request = NULL;
        uint64_t index = efl_model_children_count_get(model) - 1;

        requests = _request_add(requests, &request, index, EINA_TRUE);
     }

   // Flush all pending request
   requests = _batch_request_flush(requests, data, pd);

   switch (efl_ui_position_manager_entity_version(pd->manager, 1))
     {
       case 1:
         efl_ui_position_manager_data_access_v1_data_access_set(pd->manager,
           efl_provider_find(data, EFL_UI_WIN_CLASS),
           efl_ref(data), _batch_entity_cb, _unref_cb,
           efl_ref(data), _batch_size_cb, _unref_cb,
           count);
       break;
     }
   efl_ui_position_manager_entity_item_size_changed(pd->manager, 0, count - 1);
}

static void
_pan_viewport_changed_cb(void *data, const Efl_Event *ev EINA_UNUSED)
{
   MY_DATA_GET(data, pd);
   Eina_Rect rect = efl_ui_scrollable_viewport_geometry_get(data);

   efl_ui_position_manager_entity_viewport_set(pd->manager, rect);
}

static void
_pan_position_changed_cb(void *data, const Efl_Event *ev EINA_UNUSED)
{
   MY_DATA_GET(data, pd);
   Eina_Position2D pos = efl_ui_pan_position_get(pd->pan);
   Eina_Position2D max = efl_ui_pan_position_max_get(pd->pan);
   Eina_Vector2 rpos = {0.0, 0.0};

   if (max.x > 0.0)
     rpos.x = (double)pos.x/(double)max.x;
   if (max.y > 0.0)
     rpos.y = (double)pos.y/(double)max.y;

   efl_ui_position_manager_entity_scroll_position_set(pd->manager, rpos.x, rpos.y);
}

EFL_CALLBACKS_ARRAY_DEFINE(pan_events_cb,
  {EFL_UI_PAN_EVENT_PAN_CONTENT_POSITION_CHANGED, _pan_position_changed_cb},
  {EFL_GFX_ENTITY_EVENT_SIZE_CHANGED, _pan_viewport_changed_cb},
  {EFL_GFX_ENTITY_EVENT_POSITION_CHANGED, _pan_viewport_changed_cb},
)

EOLIAN static Efl_Object *
_efl_ui_collection_view_efl_object_constructor(Eo *obj, Efl_Ui_Collection_View_Data *pd)
{
   pd->direction = EFL_UI_LAYOUT_ORIENTATION_VERTICAL;
   obj = efl_constructor(efl_super(obj, EFL_UI_COLLECTION_VIEW_CLASS));

   if (!elm_widget_theme_klass_get(obj))
     elm_widget_theme_klass_set(obj, "collection");

   efl_wref_add(efl_add(EFL_CANVAS_RECTANGLE_CLASS, evas_object_evas_get(obj)), &pd->sizer);
   efl_gfx_color_set(pd->sizer, 0, 0, 0, 0);

   efl_wref_add(efl_add(EFL_UI_PAN_CLASS, obj), &pd->pan);
   efl_content_set(pd->pan, pd->sizer);
   efl_event_callback_array_add(pd->pan, pan_events_cb(), obj);

   efl_wref_add(efl_add(EFL_UI_SCROLL_MANAGER_CLASS, obj), &pd->scroller);
   efl_composite_attach(obj, pd->scroller);
   efl_ui_mirrored_set(pd->scroller, efl_ui_mirrored_get(obj));
   efl_ui_scroll_manager_pan_set(pd->scroller, pd->pan);

   efl_ui_scroll_connector_bind(obj, pd->scroller);

   efl_event_callback_add(obj, EFL_UI_VIEW_EVENT_MODEL_CHANGED,
                          _efl_ui_collection_view_model_changed, obj);

   return obj;
}

EOLIAN static void
_efl_ui_collection_view_efl_object_invalidate(Eo *obj,
                                              Efl_Ui_Collection_View_Data *pd)
{
   efl_ui_collection_view_position_manager_set(obj, NULL);
   efl_event_callback_del(obj, EFL_UI_VIEW_EVENT_MODEL_CHANGED,
                          _efl_ui_collection_view_model_changed, obj);

   _all_cleanup(obj, pd);

   //pd pan is given to edje, which reparents it, which forces us to manually deleting it
   if (pd->pan)
     efl_del(pd->pan);

   efl_invalidate(efl_super(obj, EFL_UI_COLLECTION_VIEW_CLASS));
}

EOLIAN static void
_efl_ui_collection_view_efl_ui_layout_orientable_orientation_set(Eo *obj EINA_UNUSED,
                                                                 Efl_Ui_Collection_View_Data *pd,
                                                                 Efl_Ui_Layout_Orientation dir)
{
   if (pd->direction == dir) return;

   pd->direction = dir;
   if (pd->manager) efl_ui_layout_orientation_set(pd->manager, dir);
}

EOLIAN static Efl_Ui_Layout_Orientation
_efl_ui_collection_view_efl_ui_layout_orientable_orientation_get(const Eo *obj EINA_UNUSED,
                                                                 Efl_Ui_Collection_View_Data *pd)
{
   return pd->direction;
}

EOLIAN static Eina_Error
_efl_ui_collection_view_efl_ui_widget_theme_apply(Eo *obj, Efl_Ui_Collection_View_Data *pd)
{
   Eina_Error res;

   ELM_WIDGET_DATA_GET_OR_RETURN(obj, wd, EFL_UI_THEME_APPLY_ERROR_GENERIC);
   res = efl_ui_widget_theme_apply(efl_super(obj, MY_CLASS));
   if (res == EFL_UI_THEME_APPLY_ERROR_GENERIC) return res;
   efl_ui_mirrored_set(pd->scroller, efl_ui_mirrored_get(obj));
   efl_content_set(efl_part(wd->resize_obj, "efl.content"), pd->pan);

   return res;
}

EOLIAN static void
_efl_ui_collection_view_efl_ui_scrollable_match_content_set(Eo *obj, Efl_Ui_Collection_View_Data *pd, Eina_Bool w, Eina_Bool h)
{
   if (pd->match_content.w == w && pd->match_content.h == h)
     return;

   pd->match_content.w = w;
   pd->match_content.h = h;

   efl_ui_scrollable_match_content_set(pd->scroller, w, h);
   flush_min_size(obj, pd);
}

EOLIAN static Efl_Ui_Focus_Manager *
_efl_ui_collection_view_efl_ui_widget_focus_manager_focus_manager_create(Eo *obj, Efl_Ui_Collection_View_Data *pd EINA_UNUSED, Efl_Ui_Focus_Object *root)
{
   Efl_Ui_Collection_View_Focus_Manager_Data *mpd;
   Eo *manager = efl_add(EFL_UI_COLLECTION_VIEW_FOCUS_MANAGER_CLASS, obj,
                         efl_ui_focus_manager_root_set(efl_added, root));

   mpd = efl_data_scope_get(manager, EFL_UI_COLLECTION_VIEW_FOCUS_MANAGER_CLASS);
   mpd->collection = obj;

   return manager;
}

EOLIAN static Efl_Ui_Focus_Object *
_efl_ui_collection_view_efl_ui_focus_manager_move(Eo *obj, Efl_Ui_Collection_View_Data *pd, Efl_Ui_Focus_Direction direction)
{
   Eo *new_obj, *focus;
   Eina_Size2D step;

   new_obj = efl_ui_focus_manager_move(efl_super(obj, MY_CLASS), direction);
   focus = efl_ui_focus_manager_focus_get(obj);
   step = efl_gfx_hint_size_combined_min_get(focus);
   if (!new_obj)
     {
        Eina_Rect pos = efl_gfx_entity_geometry_get(focus);
        Eina_Rect view = efl_ui_scrollable_viewport_geometry_get(pd->scroller);
        Eina_Position2D vpos = efl_ui_scrollable_content_pos_get(pd->scroller);

        pos.x = pos.x + vpos.x - view.x;
        pos.y = pos.y + vpos.y - view.y;
        Eina_Position2D max = efl_ui_pan_position_max_get(pd->pan);

        if (direction == EFL_UI_FOCUS_DIRECTION_RIGHT)
          {
             if (pos.x < max.x)
               {
                  pos.x = MIN(max.x, pos.x + step.w);
                  efl_ui_scrollable_scroll(obj, pos, EINA_TRUE);
                  new_obj = focus;
               }
          }
        else if (direction == EFL_UI_FOCUS_DIRECTION_LEFT)
          {
             if (pos.x > 0)
               {
                  pos.x = MAX(0, pos.x - step.w);
                  efl_ui_scrollable_scroll(obj, pos, EINA_TRUE);
                  new_obj = focus;
               }
          }
        else if (direction == EFL_UI_FOCUS_DIRECTION_UP)
          {
             if (pos.y > 0)
               {
                  pos.y = MAX(0, pos.y - step.h);
                  efl_ui_scrollable_scroll(obj, pos, EINA_TRUE);
                  new_obj = focus;
               }
          }
        else if (direction == EFL_UI_FOCUS_DIRECTION_DOWN)
          {
             if (pos.y < max.y)
               {
                  pos.y = MAX(0, pos.y + step.h);
                  efl_ui_scrollable_scroll(obj, pos, EINA_TRUE);
                  new_obj = focus;
               }
          }
     }
   else
     {
        Efl_Model *model;
        Eina_Value *vindex;
        unsigned int index;

        model = efl_ui_view_model_get(new_obj);
        vindex = efl_model_property_get(model, "child.index");
        if (eina_value_uint_convert(vindex, &index))
          _item_scroll_internal(obj, pd, index, .0, EINA_TRUE);
        eina_value_free(vindex);
     }

   return new_obj;
}

EOLIAN static Eina_Bool
_efl_ui_collection_view_efl_ui_widget_focus_state_apply(Eo *obj, Efl_Ui_Collection_View_Data *pd EINA_UNUSED, Efl_Ui_Widget_Focus_State current_state, Efl_Ui_Widget_Focus_State *configured_state, Efl_Ui_Widget *redirect EINA_UNUSED)
{
   return efl_ui_widget_focus_state_apply(efl_super(obj, MY_CLASS), current_state, configured_state, obj);
}

#include "efl_ui_collection_view.eo.c"

#define ITEM_IS_OUTSIDE_VISIBLE(id) id < cpd->start_id || id > cpd->end_id

static Efl_Ui_Item *
_find_item(Eo *obj EINA_UNUSED, Efl_Ui_Collection_View_Data *pd EINA_UNUSED, Eo *focused_element)
{
   if (!focused_element) return NULL;

   while (focused_element &&
          efl_key_data_get(focused_element, COLLECTION_VIEW_MANAGED) != COLLECTION_VIEW_MANAGED_YES)
     {
        focused_element = efl_ui_widget_parent_get(focused_element);
     }

   return focused_element;
}

static inline void
_assert_item_available(Eo *item, int new_id, Efl_Ui_Collection_View_Data *pd)
{
   efl_gfx_entity_visible_set(item, EINA_TRUE);
   efl_gfx_entity_geometry_set(item, efl_ui_position_manager_entity_position_single_item(pd->manager, new_id));
}
EOLIAN static void
_efl_ui_collection_view_focus_manager_efl_ui_focus_manager_manager_focus_set(Eo *obj, Efl_Ui_Collection_View_Focus_Manager_Data *pd, Efl_Ui_Focus_Object *focus)
{
   MY_DATA_GET(pd->collection, cpd);
   Efl_Ui_Item *item = NULL;
   unsigned int item_id;

   if (focus == efl_ui_focus_manager_root_get(obj))
     {
        // Find last item
        item = cpd->focus.previously;
        if (!item) item = cpd->focus.last;
        if (item) item_id = _lookup_entity_index(item, NULL);
        else item_id = efl_model_children_count_get(cpd->model) - 1;
     }
   else
     {
        item = _find_item(obj, cpd, focus);
        if (!item) return ;

        item_id = _lookup_entity_index(item, NULL);
     }

   // If this is NULL then we are before finalize, we cannot serve any sane value here
   if (!cpd->manager) return ;

   if (ITEM_IS_OUTSIDE_VISIBLE(item_id))
     {
        _assert_item_available(item, item_id, cpd);
     }
   efl_ui_focus_manager_focus_set(efl_super(obj, EFL_UI_COLLECTION_VIEW_FOCUS_MANAGER_CLASS), focus);
}

static int
_id_from_item(Efl_Ui_Item *item, unsigned int *index)
{
   Eina_Value *vindex;
   Efl_Model *model;

   model = efl_ui_view_model_get(item);

   vindex = efl_model_property_get(model, "child.index");
   EINA_SAFETY_ON_FALSE_RETURN_VAL(eina_value_uint_convert(vindex, index), EINA_FALSE);
   eina_value_free(vindex);
   return EINA_TRUE;
}

EOLIAN static Efl_Ui_Focus_Object *
_efl_ui_collection_view_focus_manager_efl_ui_focus_manager_request_move(Eo *obj, Efl_Ui_Collection_View_Focus_Manager_Data *pd, Efl_Ui_Focus_Direction direction, Efl_Ui_Focus_Object *child, Eina_Bool logical)
{
   MY_DATA_GET(pd->collection, cpd);
   Efl_Ui_Item *new_item = NULL;
   Efl_Ui_Item *item;
   unsigned int item_id;

   if (!child)
     child = efl_ui_focus_manager_focus_get(obj);

   item = _find_item(obj, cpd, child);

   //if this is NULL then we are before finalize, we cannot serve any sane value here
   if (!cpd->manager) goto end;
   if (!item) goto end;

   if (!_id_from_item(item, &item_id))
     goto end;

   if (ITEM_IS_OUTSIDE_VISIBLE(item_id))
     {
        unsigned int new_id;

        if (!efl_ui_position_manager_entity_relative_item(cpd->manager,
                                                          item_id,
                                                          direction,
                                                          &new_id))
          {
             new_item = NULL;
          }
        else
          {
             Efl_Ui_Collection_Item_Lookup *lookup;
#ifdef VIEWPORT_ENABLE
             unsigned int i;

             for (i = 0; i < 3; i++)
               {
                  if (!cpd->viewport[i]) continue;

                  if (!((cpd->viewport[i]->offset <= (unsigned int) new_id) &&
                        ((unsigned int) new_id < cpd->viewport[i]->offset + cpd->viewport[i]->count)))
                    continue;

                  new_item = cpd->viewport[i]->items[new_id - cpd->viewport[i]->offset].entity;
                  // We shouldn't get in a case where the available item is NULL
                  if (!new_item) break; // Just in case
                  _assert_item_available(new_item, new_id, cpd);
               }
#else
               unsigned int search_index = new_id;
               lookup = (void*) eina_rbtree_inline_lookup(cpd->cache, &search_index,
                                           sizeof (search_index), _cache_tree_lookup,
                                           NULL);
               if (lookup)
                 {
                    _assert_item_available(lookup->item.entity, new_id, cpd);
                    new_item = lookup->item.entity;
                 }
               else
                 {
                    ERR("This item cannot get focus right now. It should be visible first.");
                    new_item = NULL;
                 }
#endif
          }
     }
   else
     {
        new_item = efl_ui_focus_manager_request_move(efl_super(obj, EFL_UI_COLLECTION_VIEW_FOCUS_MANAGER_CLASS), direction, child, logical);
     }

 end:
   efl_replace(&cpd->focus.previously, new_item);
   return new_item;
}

#include "efl_ui_collection_view_focus_manager.eo.c"
