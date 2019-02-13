#define EFL_CANVAS_OBJECT_PROTECTED
#define EFL_GFX_SIZE_HINT_PROTECTED

#include "evas_common_private.h"
#include "evas_private.h"
#include <Ecore.h>

EVAS_MEMPOOL(_mp_sh);

#define MY_CLASS EFL_CANVAS_OBJECT_CLASS

#define MY_CLASS_NAME "Evas_Object"

/* evas internal stuff */
static const Evas_Object_Proxy_Data default_proxy = {
  NULL, NULL, NULL, 0, 0, NULL, 0, 0, 0, 0
};
static const Evas_Object_Map_Data default_map = {
  { NULL, NULL, 0, 0 }, { NULL, NULL, 0, 0 }, NULL, 0, 0, NULL, NULL
};
static const Evas_Object_Protected_State default_state = {
  NULL, { 0, 0, 0, 0 },
  { { 0, 0, 0, 0, 0, 0, 0, 0, EINA_FALSE, EINA_FALSE } },
  { 255, 255, 255, 255 },
  1.0, 0, EVAS_RENDER_BLEND, EINA_FALSE, EINA_FALSE, EINA_FALSE, EINA_FALSE, EINA_FALSE
};
static const Evas_Object_Mask_Data default_mask = {
  NULL, 0, 0, EINA_FALSE, EINA_FALSE, EINA_FALSE, EINA_FALSE
};
static const Evas_Object_Events_Data default_events = {
  NULL, NULL, NULL, NULL
};

Eina_Cow *evas_object_proxy_cow = NULL;
Eina_Cow *evas_object_map_cow = NULL;
Eina_Cow *evas_object_state_cow = NULL;
Eina_Cow *evas_object_3d_cow = NULL;
Eina_Cow *evas_object_mask_cow = NULL;
Eina_Cow *evas_object_events_cow = NULL;

typedef struct _Event_Animation
{
   EINA_INLIST;

   const Efl_Event_Description *desc;
   Efl_Canvas_Animation        *anim;
} Event_Animation;

static Eina_Bool
_init_cow(void)
{
   if (evas_object_map_cow && evas_object_proxy_cow && evas_object_state_cow && evas_object_3d_cow) return EINA_TRUE;

   evas_object_proxy_cow = eina_cow_add("Evas Object Proxy", sizeof (Evas_Object_Proxy_Data), 8, &default_proxy, EINA_TRUE);
   evas_object_map_cow = eina_cow_add("Evas Object Map", sizeof (Evas_Object_Map_Data), 8, &default_map, EINA_TRUE);
   evas_object_state_cow = eina_cow_add("Evas Object State", sizeof (Evas_Object_Protected_State), 64, &default_state, EINA_FALSE);
   evas_object_3d_cow = eina_cow_add("Evas Object 3D", sizeof (Evas_Object_3D_Data), 8, &default_proxy, EINA_TRUE);
   evas_object_mask_cow = eina_cow_add("Evas Mask Data", sizeof (Evas_Object_Mask_Data), 8, &default_mask, EINA_TRUE);
   evas_object_events_cow = eina_cow_add("Evas Events Data", sizeof (Evas_Object_Events_Data), 8, &default_events, EINA_TRUE);

   if (!(evas_object_map_cow && evas_object_proxy_cow && evas_object_state_cow &&
         evas_object_3d_cow && evas_object_mask_cow && evas_object_events_cow))
     {
        eina_cow_del(evas_object_proxy_cow);
        eina_cow_del(evas_object_map_cow);
        eina_cow_del(evas_object_state_cow);
        eina_cow_del(evas_object_3d_cow);
        eina_cow_del(evas_object_mask_cow);
        eina_cow_del(evas_object_events_cow);

        evas_object_proxy_cow = NULL;
        evas_object_map_cow = NULL;
        evas_object_state_cow = NULL;
        evas_object_3d_cow = NULL;
        evas_object_mask_cow = NULL;
        evas_object_events_cow = NULL;

        return EINA_FALSE;
     }

   return EINA_TRUE;
}

static Evas_Object_Pointer_Data *
_evas_object_pointer_data_find(Evas_Object_Protected_Data *obj,
                               Efl_Input_Device *pointer)
{
   Evas_Object_Pointer_Data *pdata;

   EINA_INLIST_FOREACH(obj->events->pointer_grabs, pdata)
     {
        if (pdata->evas_pdata->pointer == pointer)
          return pdata;
     }
   return NULL;
}

static void
_evas_object_pointer_grab_del(Evas_Object_Protected_Data *obj, Evas_Object_Pointer_Data *pdata);

static void
_evas_device_del_cb(void *data, const Efl_Event *ev)
{
   Evas_Object_Protected_Data *obj;
   Evas_Object_Pointer_Data *pdata;

   obj = efl_data_scope_safe_get(data, MY_CLASS);
   EINA_SAFETY_ON_NULL_RETURN(obj);
   pdata = _evas_object_pointer_data_find(obj, ev->object);
   if (!pdata) return;
   _evas_object_pointer_grab_del(obj, pdata);
}

static void
_evas_object_pointer_grab_del(Evas_Object_Protected_Data *obj,
                              Evas_Object_Pointer_Data *pdata)
{
   if ((pdata->mouse_grabbed > 0) && (obj->layer) && (obj->layer->evas))
     pdata->evas_pdata->seat->mouse_grabbed -= pdata->mouse_grabbed;
   if (((pdata->mouse_in) || (pdata->mouse_grabbed > 0)) &&
       (obj->layer) && (obj->layer->evas))
     pdata->evas_pdata->seat->object.in = eina_list_remove(pdata->evas_pdata->seat->object.in, obj->object);
   efl_event_callback_del(pdata->evas_pdata->pointer, EFL_EVENT_DEL,
                          _evas_device_del_cb, obj->object);
   EINA_COW_WRITE_BEGIN(evas_object_events_cow, obj->events, Evas_Object_Events_Data, events)
     events->pointer_grabs = eina_inlist_remove(events->pointer_grabs, EINA_INLIST_GET(pdata));
   EINA_COW_WRITE_END(evas_object_events_cow, obj->events, events);

   free(pdata);
}

static Evas_Object_Pointer_Data *
_evas_object_pointer_data_add(Evas_Pointer_Data *evas_pdata,
                              Evas_Object_Protected_Data *obj)
{
   Evas_Object_Pointer_Data *pdata;

   pdata = calloc(1, sizeof(Evas_Object_Pointer_Data));
   EINA_SAFETY_ON_NULL_RETURN_VAL(pdata, NULL);
   pdata->pointer_mode = EVAS_OBJECT_POINTER_MODE_AUTOGRAB;
   pdata->evas_pdata = evas_pdata;
   EINA_COW_WRITE_BEGIN(evas_object_events_cow, obj->events, Evas_Object_Events_Data, events)
     events->pointer_grabs = eina_inlist_append(events->pointer_grabs,
                                                EINA_INLIST_GET(pdata));
   EINA_COW_WRITE_END(evas_object_events_cow, obj->events, events);

   efl_event_callback_priority_add(evas_pdata->pointer, EFL_EVENT_DEL,
                                   EFL_CALLBACK_PRIORITY_BEFORE,
                                   _evas_device_del_cb, obj->object);
   return pdata;
}

Evas_Object_Pointer_Data *
_evas_object_pointer_data_get(Evas_Pointer_Data *evas_pdata,
                              Evas_Object_Protected_Data *obj)
{
   Evas_Object_Pointer_Data *pdata;

   pdata = _evas_object_pointer_data_find(obj, evas_pdata->pointer);

   //The pointer does not exist yet - create one.
   if (!pdata)
     return _evas_object_pointer_data_add(evas_pdata, obj);
   return pdata;
}

static void _hide(Evas_Object *eo_obj, Evas_Object_Protected_Data *obj);

static void
_anim_started_cb(void *data, const Efl_Event *ev)
{
   const Efl_Event_Description *desc = data;

   Efl_Canvas_Object_Animation_Event event;
   event.event_desc = desc;

   Eo *eo_obj = efl_animation_player_target_get(ev->object);

   efl_event_callback_call(eo_obj, EFL_CANVAS_OBJECT_EVENT_ANIM_STARTED,
                           &event);
}

static void
_anim_running_cb(void *data, const Efl_Event *ev)
{
   const Efl_Event_Description *desc = data;

   Efl_Canvas_Object_Animation_Event event;
   event.event_desc = desc;

   Eo *eo_obj = efl_animation_player_target_get(ev->object);

   efl_event_callback_call(eo_obj, EFL_CANVAS_OBJECT_EVENT_ANIM_RUNNING,
                           &event);
}

static void
_anim_ended_cb(void *data, const Efl_Event *ev)
{
   const Efl_Event_Description *desc = data;

   efl_event_callback_del(ev->object, EFL_ANIMATION_PLAYER_EVENT_STARTED,
                          _anim_started_cb, desc);
   efl_event_callback_del(ev->object, EFL_ANIMATION_PLAYER_EVENT_RUNNING,
                          _anim_running_cb, desc);
   efl_event_callback_del(ev->object, EFL_ANIMATION_PLAYER_EVENT_ENDED,
                          _anim_ended_cb, desc);

   Efl_Canvas_Object_Animation_Event event;
   event.event_desc = desc;

   Eo *eo_obj = efl_animation_player_target_get(ev->object);

   efl_event_callback_call(eo_obj, EFL_CANVAS_OBJECT_EVENT_ANIM_ENDED,
                           &event);

   if (desc == EFL_GFX_ENTITY_EVENT_HIDE)
     {
        Evas_Object_Protected_Data *obj = EVAS_OBJECT_DATA_SAFE_GET(eo_obj);
        if (!obj) return;

        _hide(eo_obj, obj);
     }
}

static void
_animation_intercept_hide(void *data, Evas_Object *eo_obj)
{
   Event_Animation *event_anim = data;
   Evas_Object_Protected_Data *obj = EVAS_OBJECT_DATA_SAFE_GET(eo_obj);
   if (!obj) return;

   if (event_anim->anim && obj->anim_player &&
       (event_anim->anim != efl_animation_player_animation_get(obj->anim_player)))
     {
        efl_animation_player_animation_set(obj->anim_player, event_anim->anim);

        efl_event_callback_add(obj->anim_player,
                               EFL_ANIMATION_PLAYER_EVENT_STARTED,
                               _anim_started_cb, EFL_GFX_ENTITY_EVENT_HIDE);
        efl_event_callback_add(obj->anim_player,
                               EFL_ANIMATION_PLAYER_EVENT_RUNNING,
                               _anim_running_cb, EFL_GFX_ENTITY_EVENT_HIDE);
        efl_event_callback_add(obj->anim_player,
                               EFL_ANIMATION_PLAYER_EVENT_ENDED,
                               _anim_ended_cb, EFL_GFX_ENTITY_EVENT_HIDE);

        //Start animation
        efl_player_start(obj->anim_player);
     }
}

static void
_event_anim_free(Event_Animation *event_anim, Evas_Object_Protected_Data *obj)
{
   if (event_anim->anim)
     {
        //Unset callbacks for Hide event
        if (event_anim->desc == EFL_GFX_ENTITY_EVENT_HIDE)
          evas_object_intercept_hide_callback_del(obj->object,
                                                  _animation_intercept_hide);

        if (efl_player_play_get(obj->anim_player))
          {
             Efl_Animation *running_anim =
                efl_animation_player_animation_get(obj->anim_player);

             if (running_anim == event_anim->anim)
               efl_player_stop(obj->anim_player);
          }
     }

   free(event_anim);
}

EOLIAN static Eo *
_efl_canvas_object_efl_object_constructor(Eo *eo_obj, Evas_Object_Protected_Data *obj)
{
   const char *class_name;
   Eo *parent = NULL;
   Evas *evas = NULL;

   class_name = efl_class_name_get(eo_obj);

   eo_obj = efl_constructor(efl_super(eo_obj, MY_CLASS));
   if (!eo_obj) goto on_error;
   efl_canvas_object_type_set(eo_obj, MY_CLASS_NAME);
   efl_manual_free_set(eo_obj, EINA_TRUE);

   parent = efl_parent_get(eo_obj);
   evas = evas_object_evas_get(parent);

   if (!obj || !_init_cow() || !evas) goto on_error;

   obj->is_frame = EINA_FALSE;
   obj->object = eo_obj;
   obj->proxy = eina_cow_alloc(evas_object_proxy_cow);
   obj->map = eina_cow_alloc(evas_object_map_cow);
   obj->cur = eina_cow_alloc(evas_object_state_cow);
   obj->prev = eina_cow_alloc(evas_object_state_cow);
   obj->data_3d = eina_cow_alloc(evas_object_3d_cow);
   obj->mask = eina_cow_alloc(evas_object_mask_cow);
   obj->events = eina_cow_alloc(evas_object_events_cow);
   obj->event_anims = NULL;

   evas_object_inject(eo_obj, obj, evas);
   evas_object_callback_init(eo_obj, obj);

   return eo_obj;

 on_error:
   ERR("Failed to create a canvas object (evas: %p) of type '%s'.", evas, class_name);
   return NULL;
}

EOLIAN static Eo *
_efl_canvas_object_efl_object_finalize(Eo *eo_obj, Evas_Object_Protected_Data *obj)
{
   Evas_Public_Data *e;

   if (!EVAS_OBJECT_DATA_ALIVE(obj)) goto end;
   if (obj->legacy.ctor) goto end;

   if (!obj->legacy.weight_set)
     efl_gfx_size_hint_weight_set(eo_obj, 1.0, 1.0);

   if (obj->legacy.visible_set /* && ... */)
     {
        obj->legacy.finalized = EINA_TRUE;
        goto end;
     }

   e = obj->layer->evas;
   e->finalize_objects = eina_list_prepend(e->finalize_objects, eo_obj);

end:
   return efl_finalize(efl_super(eo_obj, MY_CLASS));
}

void
evas_object_change_reset(Evas_Object_Protected_Data *obj)
{
   obj->changed = EINA_FALSE;
   obj->changed_move = EINA_FALSE;
   obj->changed_color = EINA_FALSE;
   obj->changed_pchange = EINA_FALSE;
   obj->changed_src_visible = EINA_FALSE;
   obj->need_surface_clear = EINA_FALSE;
}

typedef struct _Map_Same
{
   void *p1, *p2;
   Eina_Bool b1;
} Map_Same;

void
evas_object_clip_recalc_do(Evas_Object_Protected_Data *obj, Evas_Object_Protected_Data *clipper)
{
   int cx, cy, cw, ch, cr, cg, cb, ca;
   int nx, ny, nw, nh, nr, ng, nb, na;
   Eina_Bool cvis, nvis;

   evas_object_coords_recalc(obj->object, obj);

   if (EINA_UNLIKELY((!!obj->map) && (obj->map->cur.map) && (obj->map->cur.usemap)))
     {
        cx = obj->map->cur.map->normal_geometry.x;
        cy = obj->map->cur.map->normal_geometry.y;
        cw = obj->map->cur.map->normal_geometry.w;
        ch = obj->map->cur.map->normal_geometry.h;
     }
   else
     {
        if (obj->is_smart)
          {
             Eina_Rectangle bounding_box;

             evas_object_smart_bounding_box_update(obj);
             evas_object_smart_bounding_box_get(obj, &bounding_box, NULL);
             cx = bounding_box.x;
             cy = bounding_box.y;
             cw = bounding_box.w;
             ch = bounding_box.h;
          }
        else
          {
             cx = obj->cur->geometry.x;
             cy = obj->cur->geometry.y;
             cw = obj->cur->geometry.w;
             ch = obj->cur->geometry.h;
          }
     }

   if (EINA_UNLIKELY((obj->cur->color.a == 0) &&
                     (obj->cur->render_op == EVAS_RENDER_BLEND)))
     cvis = EINA_FALSE;
   else cvis = obj->cur->visible;

   cr = obj->cur->color.r; cg = obj->cur->color.g;
   cb = obj->cur->color.b; ca = obj->cur->color.a;

   if (EVAS_OBJECT_DATA_VALID(clipper))
     {
        // this causes problems... hmmm ?????
        evas_object_clip_recalc(clipper);

        // I don't know why this test was here in the first place. As I have
        // no issue showing up due to this, I keep it and move color out of it.
        // breaks cliping of mapped images!!!
        if (EINA_LIKELY(!!clipper->map) &&
            EINA_LIKELY(!!obj->map) &&
            clipper->map->cur.map_parent == obj->map->cur.map_parent)
          {
             nx = clipper->cur->cache.clip.x;
             ny = clipper->cur->cache.clip.y;
             nw = clipper->cur->cache.clip.w;
             nh = clipper->cur->cache.clip.h;
             RECTS_CLIP_TO_RECT(cx, cy, cw, ch, nx, ny, nw, nh);
          }

        obj->clip.prev_mask = NULL;
        if (EINA_UNLIKELY(clipper->mask->is_mask))
          {
             // Set complex masks the object being clipped (parent)
             obj->clip.mask = clipper;

             // Forward any mask from the parents
             if (EINA_LIKELY(obj->smart.parent != NULL))
               {
                  Evas_Object_Protected_Data *parent =
                    efl_data_scope_get(obj->smart.parent, EFL_CANVAS_OBJECT_CLASS);
                  evas_object_clip_recalc(parent);
                  if (parent->clip.mask)
                    {
                       if (parent->clip.mask != obj->clip.mask)
                         obj->clip.prev_mask = parent->clip.mask;
                    }
               }
          }
        else if (EINA_UNLIKELY(clipper->clip.mask != NULL))
          {
             // Pass complex masks to children
             obj->clip.mask = clipper->clip.mask;
          }
        else obj->clip.mask = NULL;

        nvis = clipper->cur->cache.clip.visible;
        nr = clipper->cur->cache.clip.r;
        ng = clipper->cur->cache.clip.g;
        nb = clipper->cur->cache.clip.b;
        na = clipper->cur->cache.clip.a;
        cvis = (cvis & nvis);
        cr = (cr * (nr + 1)) >> 8;
        cg = (cg * (ng + 1)) >> 8;
        cb = (cb * (nb + 1)) >> 8;
        ca = (ca * (na + 1)) >> 8;
     }
   else obj->clip.mask = NULL;
   if (!EVAS_OBJECT_DATA_VALID(obj->clip.mask))
     obj->clip.mask = NULL;

   if (((ca == 0) && (obj->cur->render_op == EVAS_RENDER_BLEND)) ||
       (cw <= 0) || (ch <= 0))
     cvis = EINA_FALSE;

   if ((obj->cur->cache.clip.dirty == EINA_FALSE) &&
       (obj->cur->cache.clip.visible == cvis) &&
       (obj->cur->cache.clip.x == cx) &&
       (obj->cur->cache.clip.y == cy) &&
       (obj->cur->cache.clip.w == cw) &&
       (obj->cur->cache.clip.h == ch) &&
       (obj->cur->cache.clip.r == cr) &&
       (obj->cur->cache.clip.g == cg) &&
       (obj->cur->cache.clip.b == cb) &&
       (obj->cur->cache.clip.a == ca))
     return;

   EINA_COW_STATE_WRITE_BEGIN(obj, state_write, cur)
     {
        state_write->cache.clip.x = cx;
        state_write->cache.clip.y = cy;
        state_write->cache.clip.w = cw;
        state_write->cache.clip.h = ch;
        state_write->cache.clip.visible = cvis;
        state_write->cache.clip.r = cr;
        state_write->cache.clip.g = cg;
        state_write->cache.clip.b = cb;
        state_write->cache.clip.a = ca;
        state_write->cache.clip.dirty = EINA_FALSE;
     }
   EINA_COW_STATE_WRITE_END(obj, state_write, cur);
}

static inline Eina_Bool
_map_same(const void *map1, const void *map2)
{
   const Map_Same *same1 = map1, *same2 = map2;
   return ((same1->p1 == same2->p1) &&
           (same1->p2 == same2->p2) &&
           (same1->b1 == same2->b1));
}

void
evas_object_cur_prev(Evas_Object_Protected_Data *obj)
{
   if (!obj->map->prev.valid_map && obj->map->prev.map)
     {
        EINA_COW_WRITE_BEGIN(evas_object_map_cow, obj->map, Evas_Object_Map_Data, map_write)
          {
             if (map_write->prev.map != map_write->cur.map)
               evas_map_free(map_write->prev.map);
             if (map_write->cache_map == map_write->prev.map)
               map_write->cache_map = NULL;
             map_write->prev.map = NULL;
          }
        EINA_COW_WRITE_END(evas_object_map_cow, obj->map, map_write);
     }

   if (obj->map->cur.map != obj->map->prev.map)
     {
        EINA_COW_WRITE_BEGIN(evas_object_map_cow, obj->map, Evas_Object_Map_Data, map_write)
          {
             if (map_write->cache_map) evas_map_free(map_write->cache_map);
             map_write->cache_map = map_write->prev.map;
          }
        EINA_COW_WRITE_END(evas_object_map_cow, obj->map, map_write);
     }
   if (!_map_same(&obj->map->prev, &obj->map->cur))
     {
        EINA_COW_WRITE_BEGIN(evas_object_map_cow, obj->map, Evas_Object_Map_Data, map_write)
          map_write->prev = map_write->cur;
        EINA_COW_WRITE_END(evas_object_map_cow, obj->map, map_write);
     }
   _efl_canvas_object_clip_prev_reset(obj, EINA_TRUE);
   eina_cow_memcpy(evas_object_state_cow, (const Eina_Cow_Data **) &obj->prev, obj->cur);
}

void
evas_object_free(Evas_Object_Protected_Data *obj, Eina_Bool clean_layer)
{
   Evas_Object *eo_obj;
   int was_smart_child = 0;

   if (!obj) return ;
   eo_obj = obj->object;

   evas_object_callback_shutdown(eo_obj, obj);
   if (efl_isa(eo_obj, EFL_CANVAS_IMAGE_INTERNAL_CLASS))
     _evas_object_image_free(eo_obj);
   evas_object_map_set(eo_obj, NULL);
   if (obj->map->prev.map) evas_map_free(obj->map->prev.map);
   if (obj->map->cache_map) evas_map_free(obj->map->cache_map);
   if (obj->map->surface)
     {
        if (obj->layer)
          {
             obj->layer->evas->engine.func->image_free(ENC, obj->map->surface);
          }
        EINA_COW_WRITE_BEGIN(evas_object_map_cow, obj->map, Evas_Object_Map_Data, map_write)
          map_write->surface = NULL;
        EINA_COW_WRITE_END(evas_object_map_cow, obj->map, map_write);
     }
   if (obj->mask->is_mask)
     {
        EINA_COW_WRITE_BEGIN(evas_object_mask_cow, obj->mask, Evas_Object_Mask_Data, mask)
          mask->is_mask = EINA_FALSE;
          mask->redraw = EINA_FALSE;
          mask->is_alpha = EINA_FALSE;
          mask->w = mask->h = 0;
          if (mask->surface)
            {
               if (obj->layer)
                 {
                    obj->layer->evas->engine.func->image_free(ENC, mask->surface);
                    mask->surface = NULL;
                 }
            }
        EINA_COW_WRITE_END(evas_object_mask_cow, obj->mask, mask);
     }
   if (eo_obj)
     {
        evas_object_grabs_cleanup(eo_obj, obj);
        evas_object_intercept_cleanup(eo_obj);
        if (obj->smart.parent) was_smart_child = 1;
        evas_object_smart_cleanup(eo_obj);
        if (obj->func->free)
          {
             obj->func->free(eo_obj, obj, obj->private_data);
          }
     }
   if (!was_smart_child) evas_object_release(eo_obj, obj, !!clean_layer);
   if (obj->clip.clipees)
     obj->clip.clipees = eina_list_free(obj->clip.clipees);
   obj->clip.cache_clipees_answer = eina_list_free(obj->clip.cache_clipees_answer);
   evas_object_clip_changes_clean(obj);
   evas_object_event_callback_all_del(eo_obj);
   evas_object_event_callback_cleanup(eo_obj);
   if (obj->map->spans)
     {
        EINA_COW_WRITE_BEGIN(evas_object_map_cow, obj->map, Evas_Object_Map_Data, map_write)
          {
             free(map_write->spans);
             map_write->spans = NULL;
          }
        EINA_COW_WRITE_END(evas_object_map_cow, obj->map, map_write);
     }
   if (obj->size_hints)
     {
       EVAS_MEMPOOL_FREE(_mp_sh, obj->size_hints);
     }
   eina_cow_free(evas_object_proxy_cow, (const Eina_Cow_Data**) &obj->proxy);
   eina_cow_free(evas_object_map_cow, (const Eina_Cow_Data**) &obj->map);
   eina_cow_free(evas_object_state_cow, (const Eina_Cow_Data**) &obj->cur);
   eina_cow_free(evas_object_state_cow, (const Eina_Cow_Data**) &obj->prev);
   eina_cow_free(evas_object_3d_cow, (const Eina_Cow_Data**) &obj->data_3d);
   eina_cow_free(evas_object_mask_cow, (const Eina_Cow_Data**) &obj->mask);
   eina_cow_free(evas_object_events_cow, (const Eina_Cow_Data**) &obj->events);

   efl_data_unref(eo_obj, obj->private_data);
   obj->private_data = NULL;

   /* Try to manual free, and if it fails, unset it so the next unref will
    * actually free the object. */
   if (!efl_manual_free(eo_obj))
      efl_manual_free_set(eo_obj, EINA_FALSE);
}

void
evas_object_change(Evas_Object *eo_obj EINA_UNUSED, Evas_Object_Protected_Data *obj)
{
   Eina_List *l;
   Evas_Object_Protected_Data *obj2;
   Evas_Object *eo_obj2;
   Eina_Bool movch = EINA_FALSE;
   Evas_Canvas3D_Texture *texture;

   if ((!obj->layer) || (!obj->layer->evas)) return;
   if (obj->layer->evas->nochange) return;
   obj->layer->evas->changed = EINA_TRUE;

   if (obj->changed_move)
     {
        movch = EINA_TRUE;
        obj->changed_move = EINA_FALSE;
     }
   else obj->need_surface_clear = EINA_TRUE;

   if (obj->changed) return;

   evas_render_object_recalc(obj);
   /* set changed flag on all objects this one clips too */
   if (!((movch) && (obj->is_static_clip)))
     {
        EINA_LIST_FOREACH(obj->clip.clipees, l, obj2)
          {
             evas_object_change(obj2->object, obj2);
          }
     }
   EINA_LIST_FOREACH(obj->proxy->proxies, l, eo_obj2)
     {
        obj2 = efl_data_scope_get(eo_obj2, MY_CLASS);

        if (!obj2) continue;
        evas_object_change(eo_obj2, obj2);
     }
   EINA_LIST_FOREACH(obj->proxy->proxy_textures, l, texture)
     {
        evas_canvas3d_object_change(texture, EVAS_CANVAS3D_STATE_TEXTURE_DATA, NULL);
     }
   if (obj->smart.parent)
     {
        Evas_Object_Protected_Data *smart_parent = efl_data_scope_get(obj->smart.parent, MY_CLASS);
        if (!smart_parent) return;
        evas_object_change(obj->smart.parent, smart_parent);
     }
}

void
evas_object_content_change(Evas_Object *eo_obj, Evas_Object_Protected_Data *obj)
{
   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();

   if ((obj->map) && (obj->map->surface))
     {
        EINA_COW_WRITE_BEGIN(evas_object_map_cow,
                             obj->map, Evas_Object_Map_Data, map_write)
          {
             obj->layer->evas->engine.func->image_free(ENC, map_write->surface);
             map_write->surface = NULL;
          }
        EINA_COW_WRITE_END(evas_object_map_cow, obj->map, map_write);
     }
}

void
evas_object_render_pre_visible_change(Eina_Array *rects, Evas_Object *eo_obj, int is_v, int was_v)
{
   Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, MY_CLASS);

   if (!obj) return;
   if (obj->is_smart) return;
   if (is_v == was_v) return;
   if (is_v)
     {
        evas_add_rect(rects,
                      obj->cur->cache.clip.x,
                      obj->cur->cache.clip.y,
                      obj->cur->cache.clip.w,
                      obj->cur->cache.clip.h);
     }
   else
     {
        evas_add_rect(rects,
                      obj->prev->cache.clip.x,
                      obj->prev->cache.clip.y,
                      obj->prev->cache.clip.w,
                      obj->prev->cache.clip.h);
     }
}

void
evas_object_render_pre_clipper_change(Eina_Array *rects, Evas_Object *eo_obj)
{
   Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, MY_CLASS);

   if (!obj) return;
   if (obj->is_smart) return;
   if (obj->cur->clipper == obj->prev->clipper) return;
   if ((obj->cur->clipper) && (obj->prev->clipper))
     {
        /* get difference rects between clippers */
        evas_rects_return_difference_rects(rects,
                                           obj->cur->clipper->cur->cache.clip.x,
                                           obj->cur->clipper->cur->cache.clip.y,
                                           obj->cur->clipper->cur->cache.clip.w,
                                           obj->cur->clipper->cur->cache.clip.h,
                                           obj->prev->clipper->prev->cache.clip.x,
                                           obj->prev->clipper->prev->cache.clip.y,
                                           obj->prev->clipper->prev->cache.clip.w,
                                           obj->prev->clipper->prev->cache.clip.h);
     }
   else if (obj->cur->clipper)
     {
        evas_rects_return_difference_rects(rects,
                                           obj->cur->geometry.x,
                                           obj->cur->geometry.y,
                                           obj->cur->geometry.w,
                                           obj->cur->geometry.h,
////	rl = evas_rects_return_difference_rects(obj->cur->cache.geometry.x,
////						obj->cur->cache.geometry.y,
////						obj->cur->cache.geometry.w,
////						obj->cur->cache.geometry.h,
                                           obj->cur->clipper->cur->cache.clip.x,
                                           obj->cur->clipper->cur->cache.clip.y,
                                           obj->cur->clipper->cur->cache.clip.w,
                                           obj->cur->clipper->cur->cache.clip.h);
     }
   else if (obj->prev->clipper)
     {
     evas_rects_return_difference_rects(rects,
                                        obj->prev->geometry.x,
                                        obj->prev->geometry.y,
                                        obj->prev->geometry.w,
                                        obj->prev->geometry.h,
////	rl = evas_rects_return_difference_rects(obj->prev->cache.geometry.x,
////						obj->prev->cache.geometry.y,
////						obj->prev->cache.geometry.w,
////						obj->prev->cache.geometry.h,
                                        obj->prev->clipper->prev->cache.clip.x,
                                        obj->prev->clipper->prev->cache.clip.y,
                                        obj->prev->clipper->prev->cache.clip.w,
                                        obj->prev->clipper->prev->cache.clip.h);
     }
}

void
evas_object_render_pre_prev_cur_add(Eina_Array *rects, Evas_Object *eo_obj EINA_UNUSED, Evas_Object_Protected_Data *obj)
{
   if (!obj) return;

   evas_add_rect(rects,
                 obj->cur->cache.clip.x,
                 obj->cur->cache.clip.y,
                 obj->cur->cache.clip.w,
                 obj->cur->cache.clip.h);
   evas_add_rect(rects,
                 obj->prev->cache.clip.x,
                 obj->prev->cache.clip.y,
                 obj->prev->cache.clip.w,
                 obj->prev->cache.clip.h);
/*
        evas_add_rect(rects,
                      obj->cur->geometry.x,
                      obj->cur->geometry.y,
                      obj->cur->geometry.w,
                      obj->cur->geometry.h);
////	    obj->cur->cache.geometry.x,
////	    obj->cur->cache.geometry.y,
////	    obj->cur->cache.geometry.w,
////	    obj->cur->cache.geometry.h);
        evas_add_rect(rects,
                      obj->prev->geometry.x,
                      obj->prev->geometry.y,
                      obj->prev->geometry.w,
                      obj->prev->geometry.h);
////	    obj->prev->cache.geometry.x,
////	    obj->prev->cache.geometry.y,
////	    obj->prev->cache.geometry.w,
////	    obj->prev->cache.geometry.h);
*/
}

void
evas_object_clip_changes_clean(Evas_Object_Protected_Data *obj)
{
   Eina_Rectangle *r;

   EINA_LIST_FREE(obj->clip.changes, r) eina_rectangle_free(r);
}

void
evas_object_render_pre_effect_updates(Eina_Array *rects, Evas_Object *eo_obj, int is_v, int was_v EINA_UNUSED)
{
   Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, MY_CLASS);
   Eina_Rectangle *r;
   Eina_List *l;
   unsigned int i;
   Eina_Array_Iterator it;
   int x, y, w, h;

   if (!obj) return;
   if (!obj->layer) return;

   if (obj->is_smart) goto end;

   if (evas_object_is_on_plane(eo_obj, obj))
     {
        /* We need some damage to occur if only planes are being updated,
           or nothing will provoke a page flip.
         */
        obj->layer->evas->engine.func->output_redraws_rect_add(ENC,
                                                                 0, 0, 1, 1);
        /* Force a pixels get callback for E's benefit :( */
        evas_object_pixels_get_force(eo_obj, obj);
        goto end;
     }
   /* FIXME: was_v isn't used... why? */
   if (!obj->clip.clipees)
     {
        Evas_Public_Data *e;
        e = obj->layer->evas;
        EINA_ARRAY_ITER_NEXT(rects, i, r, it)
          {
             /* get updates and clip to current clip */
             x = r->x;
             y = r->y;
             w = r->w;
             h = r->h;
             RECTS_CLIP_TO_RECT(x, y, w, h,
                                obj->cur->cache.clip.x,
                                obj->cur->cache.clip.y,
                                obj->cur->cache.clip.w,
                                obj->cur->cache.clip.h);
             if ((w > 0) && (h > 0))
               obj->layer->evas->engine.func->output_redraws_rect_add(ENC,
                                                                      x + e->framespace.x,
                                                                      y + e->framespace.y,
                                                                      w, h);
             /* get updates and clip to previous clip */
             x = r->x;
             y = r->y;
             w = r->w;
             h = r->h;
             RECTS_CLIP_TO_RECT(x, y, w, h,
                                obj->prev->cache.clip.x,
                                obj->prev->cache.clip.y,
                                obj->prev->cache.clip.w,
                                obj->prev->cache.clip.h);
             if ((w > 0) && (h > 0))
               obj->layer->evas->engine.func->output_redraws_rect_add(ENC,
                                                                      x + e->framespace.x,
                                                                      y + e->framespace.y,
                                                                      w, h);
          }
        /* if the object is actually visible, take any parent clip changes */
        if (is_v)
          {
             Evas_Object_Protected_Data *clipper;

             clipper = obj->cur->clipper;
             while (clipper)
               {
                  EINA_LIST_FOREACH(clipper->clip.changes, l, r)
                    {
                       /* get updates and clip to current clip */
                       x = r->x; y = r->y; w = r->w; h = r->h;
                       RECTS_CLIP_TO_RECT(x, y, w, h,
                                          obj->cur->cache.clip.x,
                                          obj->cur->cache.clip.y,
                                          obj->cur->cache.clip.w,
                                          obj->cur->cache.clip.h);
                       if ((w > 0) && (h > 0))
                         obj->layer->evas->engine.func->output_redraws_rect_add(ENC,
                                                                                x + e->framespace.x,
                                                                                y + e->framespace.y,
                                                                                w, h);
                       /* get updates and clip to previous clip */
                       x = r->x; y = r->y; w = r->w; h = r->h;
                       RECTS_CLIP_TO_RECT(x, y, w, h,
                                          obj->prev->cache.clip.x,
                                          obj->prev->cache.clip.y,
                                          obj->prev->cache.clip.w,
                                          obj->prev->cache.clip.h);
                       if ((w > 0) && (h > 0))
                         obj->layer->evas->engine.func->output_redraws_rect_add(ENC,
                                                                                x + e->framespace.x,
                                                                                y + e->framespace.y,
                                                                                w, h);
                    }
                  clipper = clipper->cur->clipper;
               }
          }
     }
   else
     {
        /* This is a clipper object: add regions that changed here,
         * See above: EINA_LIST_FOREACH(clipper->clip.changes) */
        evas_object_clip_changes_clean(obj);
        EINA_ARRAY_ITER_NEXT(rects, i, r, it)
           obj->clip.changes = eina_list_append(obj->clip.changes, r);
        eina_array_clean(rects);
     }

 end:
   EINA_ARRAY_ITER_NEXT(rects, i, r, it)
     eina_rectangle_free(r);
   eina_array_clean(rects);
}

int
evas_object_was_in_output_rect(Evas_Object *eo_obj EINA_UNUSED, Evas_Object_Protected_Data *obj, int x, int y, int w, int h)
{
   if (obj->is_smart && !obj->map->prev.map && !obj->map->prev.usemap) return 0;
   /* assumes coords have been recalced */
   if ((RECTS_INTERSECT(x, y, w, h,
                        obj->prev->cache.clip.x,
                        obj->prev->cache.clip.y,
                        obj->prev->cache.clip.w,
                        obj->prev->cache.clip.h)))
     return 1;
   return 0;
}

int
evas_object_was_opaque(Evas_Object *eo_obj, Evas_Object_Protected_Data *obj)
{
   if (obj->is_smart) return 0;
   if (obj->prev->cache.clip.a == 255)
     {
        if (obj->func->was_opaque)
          return obj->func->was_opaque(eo_obj, obj, obj->private_data);
        return 1;
     }
   return 0;
}

int
evas_object_is_inside(Evas_Object *eo_obj, Evas_Object_Protected_Data *obj, Evas_Coord x, Evas_Coord y)
{
   if (obj->is_smart) return 0;
   if (obj->func->is_inside)
     return obj->func->is_inside(eo_obj, obj, obj->private_data, x, y);
   return 0;
}

int
evas_object_was_inside(Evas_Object *eo_obj, Evas_Object_Protected_Data *obj, Evas_Coord x, Evas_Coord y)
{
   if (obj->is_smart) return 0;
   if (obj->func->was_inside)
     return obj->func->was_inside(eo_obj, obj, obj->private_data, x, y);
   return 0;
}
/* routines apps will call */

EAPI void
evas_object_ref(Evas_Object *eo_obj)
{
   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();

   efl_ref(eo_obj);

   Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, MY_CLASS);
   if (!obj) return;
   obj->ref++;
}

EAPI void
evas_object_unref(Evas_Object *eo_obj)
{
   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();

   Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, MY_CLASS);
   if (!obj) return;

   if (obj->ref == 0) return;
   obj->ref--;
   efl_unref(eo_obj);
   if ((obj->del_ref) && (obj->ref == 0)) evas_object_del(eo_obj);

}

EAPI int
evas_object_ref_get(const Evas_Object *eo_obj)
{
   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return 0;
   MAGIC_CHECK_END();

   Evas_Object_Protected_Data *obj = efl_data_scope_get(eo_obj, MY_CLASS);
   if (!obj) return 0;
   return obj->ref;
}

EAPI void
evas_object_del(Evas_Object *obj)
{
   Evas_Object_Protected_Data *pd;

   if (!obj) return;
   if (!efl_isa(obj, MY_CLASS))
     {
        ERR("Called %s on a non-evas object: %s@%p",
            __FUNCTION__, efl_class_name_get(obj), obj);
        return;
     }
   pd = efl_data_scope_get(obj, MY_CLASS);
   if (pd->delete_me || pd->efl_del_called) return;
   if (pd->ref)
     {
        pd->del_ref = EINA_TRUE;
        return;
     }
   efl_del(obj);
}

EOLIAN static Eina_Bool
_efl_canvas_object_efl_input_interface_seat_event_filter_get(const Eo *eo_obj EINA_UNUSED,
                                                             Evas_Object_Protected_Data *obj,
                                                             Efl_Input_Device *seat)
{
   //It means this object accept events from any seat.
   if (!obj->events_filter_enabled)
     return EINA_TRUE;
   return eina_list_data_find(obj->events->events_whitelist, seat) ?
     EINA_TRUE : EINA_FALSE;
}

static void
_whitelist_events_device_remove_cb(void *data, const Efl_Event *event)
{
   Evas_Object_Protected_Data *obj = data;

   EINA_COW_WRITE_BEGIN(evas_object_events_cow, obj->events, Evas_Object_Events_Data, events)
     events->events_whitelist = eina_list_remove(events->events_whitelist,
                                                 event->object);
   EINA_COW_WRITE_END(evas_object_events_cow, obj->events, events);
}

EOLIAN static void
_efl_canvas_object_efl_input_interface_seat_event_filter_set(Eo *eo_obj,
                                                             Evas_Object_Protected_Data *obj,
                                                             Efl_Input_Device *seat,
                                                             Eina_Bool add)
{
   EINA_SAFETY_ON_NULL_RETURN(seat);

   if (efl_input_device_type_get(seat) != EFL_INPUT_DEVICE_TYPE_SEAT) return;

   obj->events_filter_enabled = EINA_TRUE;
   if (add)
     {
        if (eina_list_data_find(obj->events->events_whitelist, seat)) return;
        /* remove all previously focused seats, if any - it may happen
           since there wasn't a whitelist in place (no restrictions) */
        if ((!obj->events->events_whitelist) && (obj->layer) && (obj->layer->evas))
          {
             const Eina_List *devices, *l;
             Efl_Input_Device *dev;

             devices = obj->layer->evas->devices;
             EINA_LIST_FOREACH(devices, l, dev)
               {
                  if ((efl_input_device_type_get(dev) ==
                       EFL_INPUT_DEVICE_TYPE_SEAT) && (dev != seat))
                    efl_canvas_object_seat_focus_del(eo_obj, dev);
               }
          }
        EINA_COW_WRITE_BEGIN(evas_object_events_cow, obj->events, Evas_Object_Events_Data, events)
          events->events_whitelist = eina_list_append(events->events_whitelist, seat);
        EINA_COW_WRITE_END(evas_object_events_cow, obj->events, events);

        efl_event_callback_add(seat, EFL_EVENT_DEL,
                               _whitelist_events_device_remove_cb, obj);
     }
   else
     {
        efl_canvas_object_seat_focus_del(eo_obj, seat);
        EINA_COW_WRITE_BEGIN(evas_object_events_cow, obj->events, Evas_Object_Events_Data, events)
          events->events_whitelist = eina_list_remove(events->events_whitelist, seat);
        EINA_COW_WRITE_END(evas_object_events_cow, obj->events, events);

        efl_event_callback_del(seat, EFL_EVENT_DEL,
                               _whitelist_events_device_remove_cb, obj);
     }
}

static Eina_Bool
_is_event_blocked(Eo *eo_obj, const Efl_Event_Description *desc,
                  void *event_info)
{
   if ((desc == EFL_EVENT_FOCUS_IN) ||
       (desc == EFL_EVENT_FOCUS_OUT) ||
       (desc == EFL_EVENT_KEY_DOWN) ||
       (desc == EFL_EVENT_KEY_UP) ||
       (desc == EFL_EVENT_HOLD) ||
       (desc == EFL_EVENT_POINTER_IN) ||
       (desc == EFL_EVENT_POINTER_OUT) ||
       (desc == EFL_EVENT_POINTER_DOWN) ||
       (desc == EFL_EVENT_POINTER_UP) ||
       (desc == EFL_EVENT_POINTER_MOVE) ||
       (desc == EFL_EVENT_POINTER_WHEEL) ||
       (desc == EFL_EVENT_POINTER_CANCEL) ||
       (desc == EFL_EVENT_POINTER_AXIS) ||
       (desc == EFL_EVENT_FINGER_MOVE) ||
       (desc == EFL_EVENT_FINGER_DOWN) ||
       (desc == EFL_EVENT_FINGER_UP))
     {
        Efl_Input_Device *seat = efl_input_device_seat_get(efl_input_device_get(event_info));
        return !efl_input_seat_event_filter_get(eo_obj, seat);
     }
   return EINA_FALSE;
}

static Event_Animation *
_event_animation_find(Evas_Object_Protected_Data *obj,
                      const Efl_Event_Description *desc)
{
   Event_Animation *event_anim;
   EINA_INLIST_FOREACH(obj->event_anims, event_anim)
     {
        if (event_anim->desc == desc)
          return event_anim;
     }
   return NULL;
}

EOLIAN static Eina_Bool
_efl_canvas_object_efl_object_event_callback_call(Eo *eo_obj,
                                                  Evas_Object_Protected_Data *obj,
                                                  const Efl_Event_Description *desc,
                                                  void *event_info)
{
   if (_is_event_blocked(eo_obj, desc, event_info)) return EINA_FALSE;

   //Start animation corresponding to the current event
   if (desc)
     {
        if ((desc != EFL_GFX_ENTITY_EVENT_HIDE) && desc != (EFL_GFX_ENTITY_EVENT_SHOW))
          {
             Event_Animation *event_anim = _event_animation_find(obj, desc);
             if (event_anim)
               {
                  //Create animation object to start animation
                  efl_animation_player_animation_set(obj->anim_player, event_anim->anim);

                  efl_event_callback_add(obj->anim_player,
                                         EFL_ANIMATION_PLAYER_EVENT_STARTED,
                                         _anim_started_cb, desc);
                  efl_event_callback_add(obj->anim_player,
                                         EFL_ANIMATION_PLAYER_EVENT_RUNNING,
                                         _anim_running_cb, desc);
                  efl_event_callback_add(obj->anim_player,
                                         EFL_ANIMATION_PLAYER_EVENT_ENDED,
                                         _anim_ended_cb, desc);

                  //Start animation
                  efl_player_start(obj->anim_player);
               }
          }
     }

   return efl_event_callback_call(efl_super(eo_obj, MY_CLASS),
                                  desc, event_info);
}

EOLIAN static Eina_Bool
_efl_canvas_object_efl_object_event_callback_legacy_call(Eo *eo_obj,
                                                         Evas_Object_Protected_Data *obj,
                                                         const Efl_Event_Description *desc,
                                                         void *event_info)
{
   if (_is_event_blocked(eo_obj, desc, event_info)) return EINA_FALSE;

   //Start animation corresponding to the current event
   if (desc)
     {
        if ((desc != EFL_GFX_ENTITY_EVENT_HIDE) && desc != (EFL_GFX_ENTITY_EVENT_SHOW))
          {
             Event_Animation *event_anim = _event_animation_find(obj, desc);
             if (event_anim)
               {
                  //Create animation object to start animation
                  efl_animation_player_animation_set(obj->anim_player, event_anim->anim);

                  efl_event_callback_add(obj->anim_player,
                                         EFL_ANIMATION_PLAYER_EVENT_STARTED,
                                         _anim_started_cb, desc);
                  efl_event_callback_add(obj->anim_player,
                                         EFL_ANIMATION_PLAYER_EVENT_RUNNING,
                                         _anim_running_cb, desc);
                  efl_event_callback_add(obj->anim_player,
                                         EFL_ANIMATION_PLAYER_EVENT_ENDED,
                                         _anim_ended_cb, desc);

                  //Start animation
                  efl_player_start(obj->anim_player);
               }
          }
     }

   return efl_event_callback_legacy_call(efl_super(eo_obj, MY_CLASS),
                                         desc, event_info);
}

EOLIAN static void
_efl_canvas_object_efl_object_invalidate(Eo *eo_obj, Evas_Object_Protected_Data *obj)
{
   Efl_Input_Device *dev;
   Evas_Object_Pointer_Data *pdata;
   Evas_Object *proxy;
   Eina_List *l, *l2;
   int event_id;

   evas_object_async_block(obj);

   // Unset callbacks for Hide event before hiding
   evas_object_intercept_hide_callback_del((Eo *)eo_obj,
                                           _animation_intercept_hide);

   efl_gfx_entity_visible_set((Eo *) eo_obj, EINA_FALSE);
   obj->efl_del_called = EINA_TRUE;

   evas_object_hide(eo_obj);

   if (obj->events)
     EINA_COW_WRITE_BEGIN(evas_object_events_cow, obj->events, Evas_Object_Events_Data, events)
       {
          Evas_Public_Data *edata = NULL;

          if (!efl_invalidated_get(evas_object_evas_get(eo_obj)))
            edata = efl_data_scope_get(evas_object_evas_get(eo_obj), EVAS_CANVAS_CLASS);

          EINA_LIST_FREE (events->focused_by_seats, dev)
            {
               event_id = _evas_event_counter;
               efl_event_callback_del(dev, EFL_EVENT_INVALIDATE,
                                      _evas_focus_device_invalidate_cb, obj);
               if (edata) eina_hash_del_by_key(edata->focused_objects, &dev);
               _evas_focus_dispatch_event(obj, dev, EINA_FALSE);
               if ((obj->layer) && (obj->layer->evas))
                 _evas_post_event_callback_call(obj->layer->evas->evas, obj->layer->evas, event_id);
            }
          EINA_INLIST_FREE(events->pointer_grabs, pdata)
            _evas_object_pointer_grab_del(obj, pdata);
          EINA_LIST_FREE(events->events_whitelist, dev)
            efl_event_callback_del(dev, EFL_EVENT_DEL, _whitelist_events_device_remove_cb, obj);
       }
     EINA_COW_WRITE_END(evas_object_events_cow, obj->events, events);

   event_id = _evas_object_event_new();
   evas_object_event_callback_call(eo_obj, obj, EVAS_CALLBACK_DEL, NULL, event_id, NULL);
   if ((obj->layer) && (obj->layer->evas))
     _evas_post_event_callback_call(obj->layer->evas->evas, obj->layer->evas, event_id);

   if (obj->layer)
     {
        if (obj->layer->evas)
          if (obj->layer->evas->pending_default_focus_obj == eo_obj)
            obj->layer->evas->pending_default_focus_obj = NULL;
     }

   evas_object_grabs_cleanup(eo_obj, obj);

   /* FIXME: Proxies should listen to source death */
   if (obj->proxy)
     {
        EINA_LIST_FOREACH_SAFE(obj->proxy->proxies, l, l2, proxy)
          {
             if (efl_isa(proxy, EFL_CANVAS_IMAGE_INTERNAL_CLASS))
               evas_object_image_source_unset(proxy);
             if (efl_isa(proxy, EFL_GFX_FILTER_INTERFACE))
               efl_gfx_filter_source_set(proxy, NULL, eo_obj);
          }

        /* Eina_Cow has no way to know if we are going to really change something
           or not. So before calling the cow, let's check if we want to do something */
        if (obj->proxy->proxy_textures)
          {
             EINA_COW_WRITE_BEGIN(evas_object_proxy_cow, obj->proxy,
                                  Evas_Object_Proxy_Data, proxy_src)
               {
                  Evas_Canvas3D_Texture *texture;

                  EINA_LIST_FREE(proxy_src->proxy_textures, texture)
                    evas_canvas3d_texture_source_set(texture, NULL);
               }
             EINA_COW_WRITE_END(evas_object_proxy_cow, obj->proxy, proxy_src);
          }
     }

   if (obj->cur && obj->cur->clipper) evas_object_clip_unset(eo_obj);
   if (obj->prev) _efl_canvas_object_clip_prev_reset(obj, EINA_FALSE);

   //Free event animations
   while (obj->event_anims)
     {
        Event_Animation *event_anim =
           EINA_INLIST_CONTAINER_GET(obj->event_anims, Event_Animation);

        obj->event_anims =
           eina_inlist_remove(obj->event_anims, obj->event_anims);

        _event_anim_free(event_anim, obj);
     }

   if (obj->map) evas_object_map_set(eo_obj, NULL);

   if (obj->is_smart) evas_object_smart_del(eo_obj);
   evas_object_change(eo_obj, obj);

   efl_invalidate(efl_super(eo_obj, MY_CLASS));
}

EOLIAN static void
_efl_canvas_object_efl_object_destructor(Eo *eo_obj, Evas_Object_Protected_Data *obj)
{
   int event_id;

   if (obj->clip.clipees)
     {
        ERR("object %p of type '%s' still has %d clippees after del callback",
            eo_obj, efl_class_name_get(eo_obj), eina_list_count(obj->clip.clipees));
        /* "while" should be used for null check of obj->clip.clipees,
           because evas_objct_clip_unset can set null to obj->clip.clipees */
        while (obj->clip.clipees)
          {
             Evas_Object_Protected_Data *tmp;
             tmp = eina_list_data_get(obj->clip.clipees);
             evas_object_clip_unset(tmp->object);
          }
     }

   if (obj->name) evas_object_name_set(eo_obj, NULL);
   if (!obj->layer)
     {
        efl_manual_free_set(eo_obj, EINA_FALSE);
        goto end;
     }

   event_id = _evas_object_event_new();
   evas_object_event_callback_call(eo_obj, obj, EVAS_CALLBACK_FREE, NULL, event_id, NULL);
   if ((obj->layer) && (obj->layer->evas))
     _evas_post_event_callback_call(obj->layer->evas->evas, obj->layer->evas, event_id);
   evas_object_smart_cleanup(eo_obj);
   obj->delete_me = 1;
   evas_object_change(eo_obj, obj);
end:
   evas_object_event_callback_all_del(eo_obj);
   evas_object_event_callback_cleanup(eo_obj);

   efl_destructor(efl_super(eo_obj, MY_CLASS));
}


EOLIAN static void
_efl_canvas_object_efl_gfx_entity_geometry_set(Eo *obj, Evas_Object_Protected_Data *pd EINA_UNUSED, Eina_Rect r)
{
   efl_gfx_entity_position_set(obj, r.pos);
   efl_gfx_entity_size_set(obj, EINA_SIZE2D(r.w,  r.h));
}

EAPI void
evas_object_geometry_set(Evas_Object *eo_obj, Evas_Coord x, Evas_Coord y, Evas_Coord w, Evas_Coord h)
{
   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   efl_gfx_entity_geometry_set(eo_obj, EINA_RECT(x, y, w, h));
}

EAPI void
evas_object_move(Evas_Object *obj, Evas_Coord x, Evas_Coord y)
{
   efl_gfx_entity_position_set(obj, EINA_POSITION2D(x, y));
}

EOLIAN static void
_efl_canvas_object_efl_gfx_entity_position_set(Eo *eo_obj, Evas_Object_Protected_Data *obj, Eina_Position2D pos)
{
   Eina_Bool pass = EINA_FALSE, freeze = EINA_FALSE;
   Eina_Bool source_invisible = EINA_FALSE;
   Eina_List *was = NULL;
   Evas_Map *map;
   int x = pos.x;
   int y = pos.y;

   if (_evas_object_intercept_call_evas(obj, EVAS_OBJECT_INTERCEPT_CB_MOVE, 1, x, y))
     return;

   map = obj->map->cur.map;
   if (map && map->move_sync.enabled)
     {
        Evas_Coord diff_x = x - obj->cur->geometry.x;
        Evas_Coord diff_y = y - obj->cur->geometry.y;
        evas_map_object_move_diff_set(map, diff_x, diff_y);
     }

   if (!(obj->layer->evas->is_frozen))
     {
        pass = evas_event_passes_through(eo_obj, obj);
        freeze = evas_event_freezes_through(eo_obj, obj);
        source_invisible = evas_object_is_source_invisible(eo_obj, obj);
        if ((!pass) && (!freeze) && (!source_invisible))
          was = _evas_pointer_list_in_rect_get(obj->layer->evas, eo_obj, obj,
                                               1, 1);
     }
   obj->doing.in_move++;

   if (obj->is_smart && obj->smart.smart && obj->smart.smart->smart_class->move)
     obj->smart.smart->smart_class->move(eo_obj, x, y);

   EINA_COW_STATE_WRITE_BEGIN(obj, state_write, cur)
     {
        state_write->geometry.x = x;
        state_write->geometry.y = y;
     }
   EINA_COW_STATE_WRITE_END(obj, state_write, cur);

   evas_object_update_bounding_box(eo_obj, obj, NULL);

////   obj->cur->cache.geometry.validity = 0;
   obj->changed_move = EINA_TRUE;
   evas_object_change(eo_obj, obj);
   evas_object_clip_dirty(eo_obj, obj);
   obj->doing.in_move--;
   if (!(obj->layer->evas->is_frozen))
     {
        evas_object_recalc_clippees(obj);
        if (!pass && !obj->is_smart && obj->cur->visible)
          _evas_canvas_event_pointer_in_list_mouse_move_feed(obj->layer->evas, was, eo_obj, obj, 1, 1, EINA_TRUE, NULL);
     }
   eina_list_free(was);
   evas_object_inform_call_move(eo_obj, obj);
}

EAPI void
evas_object_resize(Evas_Object *obj, Evas_Coord w, Evas_Coord h)
{
   efl_gfx_entity_size_set((Evas_Object *)obj, EINA_SIZE2D(w,  h));
}

Eina_Bool
_efl_canvas_object_efl_gfx_entity_size_set_block(Eo *eo_obj, Evas_Object_Protected_Data *obj,
                                          Evas_Coord w, Evas_Coord h, Eina_Bool internal)
{
   if (!internal && (obj->doing.in_resize > 0))
     {
        WRN("evas_object_resize() called on object %p (%s) when in the middle "
            "of resizing the same object", eo_obj, efl_class_name_get(eo_obj));
        return EINA_TRUE;
     }

   if (w < 0) w = 0;
   if (h < 0) h = 0;

   if ((obj->cur->geometry.w == w) && (obj->cur->geometry.h == h))
     return EINA_TRUE;

   return EINA_FALSE;
}

EOLIAN static void
_efl_canvas_object_efl_gfx_entity_size_set(Eo *eo_obj, Evas_Object_Protected_Data *obj,
                                    Eina_Size2D sz)
{
   Eina_Bool pass = EINA_FALSE, freeze = EINA_FALSE;
   Eina_Bool source_invisible = EINA_FALSE;
   Eina_List *was = NULL;

   if (sz.w < 0) sz.w = 0;
   if (sz.h < 0) sz.h = 0;

   if (_evas_object_intercept_call_evas(obj, EVAS_OBJECT_INTERCEPT_CB_RESIZE, 1, sz.w, sz.h))
     return;

   if (!(obj->layer->evas->is_frozen))
     {
        pass = evas_event_passes_through(eo_obj, obj);
        freeze = evas_event_freezes_through(eo_obj, obj);
        source_invisible = evas_object_is_source_invisible(eo_obj, obj);
        if ((!pass) && (!freeze) && (!source_invisible))
          was = _evas_pointer_list_in_rect_get(obj->layer->evas, eo_obj,
                                               obj, 1, 1);
     }
   obj->doing.in_resize++;

   if (obj->is_smart && obj->smart.smart && obj->smart.smart->smart_class->resize)
     obj->smart.smart->smart_class->resize(eo_obj, sz.w, sz.h);

   EINA_COW_STATE_WRITE_BEGIN(obj, state_write, cur)
     {
        state_write->geometry.w = sz.w;
        state_write->geometry.h = sz.h;
     }
   EINA_COW_STATE_WRITE_END(obj, state_write, cur);

   evas_object_update_bounding_box(eo_obj, obj, NULL);

////   obj->cur->cache.geometry.validity = 0;
   evas_object_change(eo_obj, obj);
   evas_object_clip_dirty(eo_obj, obj);
   obj->doing.in_resize--;
   /* NB: evas_object_recalc_clippees was here previously ( < 08/07/2009) */
   if (!(obj->layer->evas->is_frozen))
     {
        /* NB: If this creates glitches on screen then move to above position */
        evas_object_recalc_clippees(obj);

        //   if (obj->func->coords_recalc) obj->func->coords_recalc(eo_obj);
        if (!pass && !obj->is_smart && obj->cur->visible)
          _evas_canvas_event_pointer_in_list_mouse_move_feed(obj->layer->evas, was, eo_obj, obj, 1, 1, EINA_TRUE, NULL);
     }
   eina_list_free(was);
   evas_object_inform_call_resize(eo_obj, obj);
}

EOLIAN Eina_Rect
_efl_canvas_object_efl_gfx_entity_geometry_get(const Eo *eo_obj EINA_UNUSED, Evas_Object_Protected_Data *obj)
{
   return (Eina_Rect) obj->cur->geometry;
}

EAPI void
evas_object_geometry_get(const Evas_Object *eo_obj, Evas_Coord *x, Evas_Coord *y, Evas_Coord *w, Evas_Coord *h)
{
   Eina_Rect r = efl_gfx_entity_geometry_get(eo_obj);
   if (x) *x = r.x;
   if (y) *y = r.y;
   if (w) *w = r.w;
   if (h) *h = r.h;
}

EOLIAN static Eina_Position2D
_efl_canvas_object_efl_gfx_entity_position_get(const Eo *obj EINA_UNUSED,
                                        Evas_Object_Protected_Data *pd)
{
   if ((pd->delete_me) || (!pd->layer))
     return EINA_POSITION2D(0, 0);

   return ((Eina_Rect) pd->cur->geometry).pos;
}

EOLIAN static Eina_Size2D
_efl_canvas_object_efl_gfx_entity_size_get(const Eo *obj EINA_UNUSED,
                                           Evas_Object_Protected_Data *pd)
{
   if (pd->delete_me)
     return EINA_SIZE2D(0, 0);

   return ((Eina_Rect) pd->cur->geometry).size;
}

static void
_evas_object_size_hint_alloc(Evas_Object *eo_obj EINA_UNUSED, Evas_Object_Protected_Data *obj)
{
   if (obj->size_hints) return;

   EVAS_MEMPOOL_INIT(_mp_sh, "evas_size_hints", Evas_Size_Hints, 32, );
   obj->size_hints = EVAS_MEMPOOL_ALLOC(_mp_sh, Evas_Size_Hints);
   if (!obj->size_hints) return;
   EVAS_MEMPOOL_PREP(_mp_sh, obj->size_hints, Evas_Size_Hints);
   obj->size_hints->max.w = -1;
   obj->size_hints->max.h = -1;
   obj->size_hints->align.x = 0.5;
   obj->size_hints->align.y = 0.5;
   obj->size_hints->fill.x = 1;
   obj->size_hints->fill.y = 1;
   obj->size_hints->dispmode = EVAS_DISPLAY_MODE_NONE;
}

/* Legacy only */
EAPI Evas_Display_Mode
evas_object_size_hint_display_mode_get(const Evas_Object *eo_obj)
{
   Evas_Object_Protected_Data *obj = EVAS_OBJECT_DATA_SAFE_GET(eo_obj);
   EVAS_OBJECT_DATA_ALIVE_CHECK(obj, EVAS_DISPLAY_MODE_NONE);
   if (!obj->size_hints) return EVAS_DISPLAY_MODE_NONE;
   return obj->size_hints->dispmode;
}

/* Legacy only */
EAPI void
evas_object_size_hint_display_mode_set(Eo *eo_obj, Evas_Display_Mode dispmode)
{
   Evas_Object_Protected_Data *obj = EVAS_OBJECT_DATA_SAFE_GET(eo_obj);

   EVAS_OBJECT_DATA_ALIVE_CHECK(obj);
   evas_object_async_block(obj);
   if (EINA_UNLIKELY(!obj->size_hints))
     {
        if (dispmode == EVAS_DISPLAY_MODE_NONE) return;
        _evas_object_size_hint_alloc(eo_obj, obj);
     }
   if (obj->size_hints->dispmode == dispmode) return;
   obj->size_hints->dispmode = dispmode;

   evas_object_inform_call_changed_size_hints(eo_obj, obj);
}

EOLIAN static Eina_Size2D
_efl_canvas_object_efl_gfx_size_hint_hint_restricted_min_get(const Eo *eo_obj EINA_UNUSED, Evas_Object_Protected_Data *obj)
{
   if ((!obj->size_hints) || obj->delete_me)
     return EINA_SIZE2D(0, 0);

   return obj->size_hints->min;
}

EOLIAN static void
_efl_canvas_object_efl_gfx_size_hint_hint_restricted_min_set(Eo *eo_obj, Evas_Object_Protected_Data *obj, Eina_Size2D sz)
{
   if (obj->delete_me)
     return;

   EVAS_OBJECT_DATA_VALID_CHECK(obj);
   evas_object_async_block(obj);
   if (EINA_UNLIKELY(!obj->size_hints))
     {
        if (!sz.w && !sz.h) return;
        _evas_object_size_hint_alloc(eo_obj, obj);
     }
   if ((obj->size_hints->min.w == sz.w) && (obj->size_hints->min.h == sz.h)) return;
   obj->size_hints->min = sz;

   evas_object_inform_call_changed_size_hints(eo_obj, obj);
}

EOLIAN static Eina_Size2D
_efl_canvas_object_efl_gfx_size_hint_hint_combined_min_get(const Eo *eo_obj EINA_UNUSED, Evas_Object_Protected_Data *obj)
{
   Eina_Size2D sz = { 0, 0 };

   if ((!obj->size_hints) || obj->delete_me)
     return sz;

   sz.w = MAX(obj->size_hints->min.w, obj->size_hints->user_min.w);
   sz.h = MAX(obj->size_hints->min.h, obj->size_hints->user_min.h);
   return sz;
}

EOLIAN static Eina_Size2D
_efl_canvas_object_efl_gfx_size_hint_hint_max_get(const Eo *eo_obj EINA_UNUSED, Evas_Object_Protected_Data *obj)
{
   if ((!obj->size_hints) || obj->delete_me)
     return EINA_SIZE2D(-1, -1);

   return obj->size_hints->max;
}

EOLIAN static void
_efl_canvas_object_efl_gfx_size_hint_hint_max_set(Eo *eo_obj, Evas_Object_Protected_Data *obj, Eina_Size2D sz)
{
   if (obj->delete_me)
     return;

   EVAS_OBJECT_DATA_VALID_CHECK(obj);
   evas_object_async_block(obj);
   if (EINA_UNLIKELY(!obj->size_hints))
     {
        if ((sz.w == -1) && (sz.h == -1)) return;
        _evas_object_size_hint_alloc(eo_obj, obj);
     }
   if ((obj->size_hints->max.w == sz.w) && (obj->size_hints->max.h == sz.h)) return;
   obj->size_hints->max.w = sz.w;
   obj->size_hints->max.h = sz.h;

   evas_object_inform_call_changed_size_hints(eo_obj, obj);
}

EAPI void
evas_object_size_hint_request_get(const Eo *eo_obj, Evas_Coord *w, Evas_Coord *h)
{
   Evas_Object_Protected_Data *obj = EVAS_OBJECT_DATA_SAFE_GET(eo_obj);
   EVAS_OBJECT_DATA_VALID_CHECK(obj);

   if ((!obj->size_hints) || obj->delete_me)
     {
        if (w) *w = 0;
        if (h) *h = 0;
        return;
     }
   if (w) *w = obj->size_hints->request.w;
   if (h) *h = obj->size_hints->request.h;
}

EAPI void
evas_object_size_hint_request_set(Eo *eo_obj, Evas_Coord w, Evas_Coord h)
{
   Evas_Object_Protected_Data *obj = EVAS_OBJECT_DATA_SAFE_GET(eo_obj);
   EVAS_OBJECT_DATA_ALIVE_CHECK(obj);

   evas_object_async_block(obj);
   if (EINA_UNLIKELY(!obj->size_hints))
     {
        if (!w && !h) return;
        _evas_object_size_hint_alloc(eo_obj, obj);
     }
   if ((obj->size_hints->request.w == w) && (obj->size_hints->request.h == h)) return;
   obj->size_hints->request.w = w;
   obj->size_hints->request.h = h;

   evas_object_inform_call_changed_size_hints(eo_obj, obj);
}

EOLIAN static Eina_Size2D
_efl_canvas_object_efl_gfx_size_hint_hint_min_get(const Eo *eo_obj EINA_UNUSED, Evas_Object_Protected_Data *obj)
{
   if ((!obj->size_hints) || obj->delete_me)
     return EINA_SIZE2D(0, 0);

   return obj->size_hints->user_min;
}

EOLIAN static void
_efl_canvas_object_efl_gfx_size_hint_hint_min_set(Eo *eo_obj, Evas_Object_Protected_Data *obj, Eina_Size2D sz)
{
   if (obj->delete_me)
     return;

   EVAS_OBJECT_DATA_VALID_CHECK(obj);
   evas_object_async_block(obj);
   if (EINA_UNLIKELY(!obj->size_hints))
     {
        if (!sz.w && !sz.h) return;
        _evas_object_size_hint_alloc(eo_obj, obj);
     }
   if ((obj->size_hints->user_min.w == sz.w) && (obj->size_hints->user_min.h == sz.h)) return;
   obj->size_hints->user_min = sz;

   evas_object_inform_call_changed_size_hints(eo_obj, obj);
}

EOLIAN static void
_efl_canvas_object_efl_gfx_size_hint_hint_aspect_get(const Eo *eo_obj EINA_UNUSED, Evas_Object_Protected_Data *obj, Efl_Gfx_Size_Hint_Aspect *aspect, Eina_Size2D *sz)
{
   if ((!obj->size_hints) || obj->delete_me)
     {
        if (aspect) *aspect = EVAS_ASPECT_CONTROL_NONE;
        if (sz) *sz = EINA_SIZE2D(0, 0);
        return;
     }
   if (aspect) *aspect = obj->size_hints->aspect.mode;
   if (sz)
     {
        sz->w = obj->size_hints->aspect.size.w;
        sz->h = obj->size_hints->aspect.size.h;
     }
}

EOLIAN static void
_efl_canvas_object_efl_gfx_size_hint_hint_aspect_set(Eo *eo_obj, Evas_Object_Protected_Data *obj, Efl_Gfx_Size_Hint_Aspect aspect, Eina_Size2D sz)
{
   if (obj->delete_me)
     return;

   EVAS_OBJECT_DATA_VALID_CHECK(obj);
   evas_object_async_block(obj);
   if (EINA_UNLIKELY(!obj->size_hints))
     {
        if (!sz.w && !sz.h) return;
        _evas_object_size_hint_alloc(eo_obj, obj);
     }
   if ((obj->size_hints->aspect.mode == aspect) &&
       (obj->size_hints->aspect.size.w == sz.w) &&
       (obj->size_hints->aspect.size.h == sz.h)) return;
   obj->size_hints->aspect.mode = aspect;
   obj->size_hints->aspect.size = sz;

   evas_object_inform_call_changed_size_hints(eo_obj, obj);
}

EOLIAN static void
_efl_canvas_object_efl_gfx_size_hint_hint_align_get(const Eo *eo_obj EINA_UNUSED, Evas_Object_Protected_Data *obj, double *x, double *y)
{
   if ((!obj->size_hints) || obj->delete_me)
     {
        if (x) *x = 0.5;
        if (y) *y = 0.5;
        return;
     }
   if (x) *x = obj->size_hints->align.x;
   if (y) *y = obj->size_hints->align.y;
}

EOLIAN static void
_efl_canvas_object_efl_gfx_size_hint_hint_align_set(Eo *eo_obj, Evas_Object_Protected_Data *obj, double x, double y)
{
   if (obj->delete_me)
     return;

   EVAS_OBJECT_DATA_VALID_CHECK(obj);
   evas_object_async_block(obj);
   if (EINA_UNLIKELY(!obj->size_hints))
     {
        if (EINA_DBL_EQ(x, 0.5) && EINA_DBL_EQ(y, 0.5)) return;
        _evas_object_size_hint_alloc(eo_obj, obj);
     }
   if ((EINA_DBL_EQ(obj->size_hints->align.x, x)) &&
       (EINA_DBL_EQ(obj->size_hints->align.y, y)))
     return;
   obj->size_hints->align.x = x;
   obj->size_hints->align.y = y;

   evas_object_inform_call_changed_size_hints(eo_obj, obj);
}

EOLIAN static void
_efl_canvas_object_efl_gfx_size_hint_hint_weight_get(const Eo *eo_obj EINA_UNUSED, Evas_Object_Protected_Data *obj, double *x, double *y)
{
   if ((!obj->size_hints) || obj->delete_me)
     {
        if (x) *x = 0.0;
        if (y) *y = 0.0;
        return;
     }
   if (x) *x = obj->size_hints->weight.x;
   if (y) *y = obj->size_hints->weight.y;
}

EOLIAN static void
_efl_canvas_object_efl_gfx_size_hint_hint_weight_set(Eo *eo_obj, Evas_Object_Protected_Data *obj, double x, double y)
{
   if (obj->delete_me)
     return;

   EVAS_OBJECT_DATA_VALID_CHECK(obj);
   evas_object_async_block(obj);
   if (!obj->legacy.weight_set) obj->legacy.weight_set = 1;
   if (EINA_UNLIKELY(!obj->size_hints))
     {
        if (EINA_DBL_EQ(x, 0.0) && EINA_DBL_EQ(y, 0.0)) return;
        _evas_object_size_hint_alloc(eo_obj, obj);
     }
   if ((EINA_DBL_EQ(obj->size_hints->weight.x, x)) &&
       (EINA_DBL_EQ(obj->size_hints->weight.y, y)))
     return;
   obj->size_hints->weight.x = x;
   obj->size_hints->weight.y = y;

   evas_object_inform_call_changed_size_hints(eo_obj, obj);
}

EOLIAN static void
_efl_canvas_object_efl_gfx_size_hint_hint_margin_get(const Eo *eo_obj EINA_UNUSED, Evas_Object_Protected_Data *obj, Evas_Coord *l, Evas_Coord *r, Evas_Coord *t, Evas_Coord *b)
{
   if ((!obj->size_hints) || obj->delete_me)
     {
        if (l) *l = 0;
        if (r) *r = 0;
        if (t) *t = 0;
        if (b) *b = 0;
        return;
     }
   if (l) *l = obj->size_hints->padding.l;
   if (r) *r = obj->size_hints->padding.r;
   if (t) *t = obj->size_hints->padding.t;
   if (b) *b = obj->size_hints->padding.b;
}

EOLIAN static void
_efl_canvas_object_efl_gfx_size_hint_hint_margin_set(Eo *eo_obj, Evas_Object_Protected_Data *obj, Evas_Coord l, Evas_Coord r, Evas_Coord t, Evas_Coord b)
{
   if (obj->delete_me)
     return;

   EVAS_OBJECT_DATA_VALID_CHECK(obj);
   evas_object_async_block(obj);
   if (EINA_UNLIKELY(!obj->size_hints))
     {
        if (!l && !r && !t && !b) return;
        _evas_object_size_hint_alloc(eo_obj, obj);
     }
   if ((obj->size_hints->padding.l == l) && (obj->size_hints->padding.r == r) && (obj->size_hints->padding.t == t) && (obj->size_hints->padding.b == b)) return;
   obj->size_hints->padding.l = l;
   obj->size_hints->padding.r = r;
   obj->size_hints->padding.t = t;
   obj->size_hints->padding.b = b;

   evas_object_inform_call_changed_size_hints(eo_obj, obj);
}

EOLIAN static void
_efl_canvas_object_efl_gfx_size_hint_hint_fill_get(const Eo *eo_obj EINA_UNUSED, Evas_Object_Protected_Data *obj, Eina_Bool *x, Eina_Bool *y)
{
   if ((!obj->size_hints) || obj->delete_me)
     {
        if (x) *x = EINA_TRUE;
        if (y) *y = EINA_TRUE;
        return;
     }
   if (x) *x = obj->size_hints->fill.x;
   if (y) *y = obj->size_hints->fill.y;
}

EOLIAN static void
_efl_canvas_object_efl_gfx_size_hint_hint_fill_set(Eo *eo_obj, Evas_Object_Protected_Data *obj, Eina_Bool x, Eina_Bool y)
{
   if (obj->delete_me)
     return;

   if (obj->legacy.ctor)
     {
        ERR("fill_set() not supported. use align_set() with EVAS_HINT_FILL instead.");
        return;
     }

   EVAS_OBJECT_DATA_VALID_CHECK(obj);
   evas_object_async_block(obj);

   x = !!x;
   y = !!y;

   if (EINA_UNLIKELY(!obj->size_hints))
     {
        if (x && y) return;
        _evas_object_size_hint_alloc(eo_obj, obj);
     }
   if ((obj->size_hints->fill.x == x) && (obj->size_hints->fill.y == y)) return;
   obj->size_hints->fill.x = x;
   obj->size_hints->fill.y = y;

   evas_object_inform_call_changed_size_hints(eo_obj, obj);
}

EAPI void
evas_object_show(Evas_Object *eo_obj)
{
   MAGIC_CHECK(eo_obj, Evas_Object, MAGIC_OBJ);
   return;
   MAGIC_CHECK_END();
   efl_gfx_entity_visible_set(eo_obj, EINA_TRUE);
}

EAPI void
evas_object_hide(Evas_Object *eo_obj)
{
   if (!eo_obj) return;
   efl_gfx_entity_visible_set(eo_obj, EINA_FALSE);
}

EAPI Eina_Bool
evas_object_visible_get(const Evas_Object *obj)
{
   return efl_gfx_entity_visible_get((Evas_Object *)obj);
}

static void
_show(Evas_Object *eo_obj, Evas_Object_Protected_Data *obj)
{
   if (obj->anim_player)
     efl_player_stop(obj->anim_player);
   if (obj->is_smart && obj->smart.smart && obj->smart.smart->smart_class->show)
     {
        obj->smart.smart->smart_class->show(eo_obj);
     }
   EINA_COW_STATE_WRITE_BEGIN(obj, state_write, cur)
     {
        state_write->visible = 1;
     }
   EINA_COW_STATE_WRITE_END(obj, state_write, cur);

   evas_object_change(eo_obj, obj);
   evas_object_clip_dirty(eo_obj, obj);
   if (!(obj->layer->evas->is_frozen))
     {
        evas_object_clip_across_clippees_check(eo_obj, obj);
        evas_object_recalc_clippees(obj);
        if ((!evas_event_passes_through(eo_obj, obj)) &&
            (!evas_event_freezes_through(eo_obj, obj)) &&
            (!evas_object_is_source_invisible(eo_obj, obj)))
          {
             if (!obj->is_smart)
               {
                  _evas_canvas_event_pointer_in_rect_mouse_move_feed(obj->layer->evas,
                                                                     eo_obj,
                                                                     obj, 1, 1,
                                                                     EINA_FALSE,
                                                                     NULL);
               }
          }
     }
   evas_object_update_bounding_box(eo_obj, obj, NULL);
   evas_object_inform_call_show(eo_obj, obj);

   Event_Animation *event_anim = _event_animation_find(obj, EFL_GFX_ENTITY_EVENT_SHOW);
   if (event_anim)
     {
        //Create animation object to start animation
        efl_animation_player_animation_set(obj->anim_player, event_anim->anim);

        efl_event_callback_add(obj->anim_player,
                               EFL_ANIMATION_PLAYER_EVENT_STARTED,
                               _anim_started_cb, EFL_GFX_ENTITY_EVENT_SHOW);
        efl_event_callback_add(obj->anim_player,
                               EFL_ANIMATION_PLAYER_EVENT_RUNNING,
                               _anim_running_cb, EFL_GFX_ENTITY_EVENT_SHOW);
        efl_event_callback_add(obj->anim_player,
                               EFL_ANIMATION_PLAYER_EVENT_ENDED,
                               _anim_ended_cb, EFL_GFX_ENTITY_EVENT_SHOW);

        //Start animation
        efl_player_start(obj->anim_player);
     }
}

static void
_hide(Evas_Object *eo_obj, Evas_Object_Protected_Data *obj)
{
   if (obj->is_smart && obj->smart.smart && obj->smart.smart->smart_class->hide)
     {
        obj->smart.smart->smart_class->hide(eo_obj);
     }
   EINA_COW_STATE_WRITE_BEGIN(obj, state_write, cur)
     {
        state_write->visible = 0;
     }
   EINA_COW_STATE_WRITE_END(obj, state_write, cur);

   if (obj->mask->is_mask)
     {
        if (obj->mask->surface ||
            obj->mask->w || obj->mask->h ||
            obj->mask->is_alpha || obj->mask->redraw)
          {
             EINA_COW_WRITE_BEGIN(evas_object_mask_cow, obj->mask,
                                  Evas_Object_Mask_Data, mask)
               mask->redraw = EINA_FALSE;
               mask->is_alpha = EINA_FALSE;
               mask->w = mask->h = 0;
               if (mask->surface)
                 {
                    obj->layer->evas->engine.func->image_free(ENC, mask->surface);
                    mask->surface = NULL;
                 }
             EINA_COW_WRITE_END(evas_object_mask_cow, obj->mask, mask);
          }
     }

   evas_object_change(eo_obj, obj);
   evas_object_clip_dirty(eo_obj, obj);
   if (obj->layer->evas && !(obj->layer->evas->is_frozen))
     {
        evas_object_clip_across_clippees_check(eo_obj, obj);
        evas_object_recalc_clippees(obj);
        if ((!evas_event_passes_through(eo_obj, obj)) &&
            (!evas_event_freezes_through(eo_obj, obj)) &&
            (!evas_object_is_source_invisible(eo_obj, obj)))
          {
             if ((!obj->is_smart) ||
                 ((obj->map->cur.map) && (obj->map->cur.usemap)))
               {
                  Evas_Object_Pointer_Data *obj_pdata;
                  Eina_Bool mouse_grabbed = EINA_FALSE;
                  EINA_INLIST_FOREACH(obj->events->pointer_grabs, obj_pdata)
                    {
                       if (obj_pdata->mouse_grabbed > 0)
                         {
                            mouse_grabbed = EINA_TRUE;
                            break;
                         }
                    }
                  if (!mouse_grabbed)
                    {
                       EINA_INLIST_FOREACH(obj->events->pointer_grabs, obj_pdata)
                         {
                            if (evas_object_is_in_output_rect(eo_obj, obj, obj_pdata->evas_pdata->seat->x,
                                                              obj_pdata->evas_pdata->seat->y,
                                                              1, 1))
                              _evas_canvas_event_pointer_move_event_dispatch(obj->layer->evas, obj_pdata->evas_pdata, NULL);
                         }
                    }
/* this is at odds to handling events when an obj is moved out of the mouse
 * ore resized out or clipped out. if mouse is grabbed - regardless of
 * visibility, mouse move events should keep happening and mouse up.
 * for better or worse it's at least consistent.
                  if (obj->delete_me) return;
                  if (obj->mouse_grabbed > 0)
                    obj->layer->evas->pointer.mouse_grabbed -= obj->mouse_grabbed;
                  if ((obj->mouse_in) || (obj->mouse_grabbed > 0))
                    obj->layer->evas->pointer.object.in = eina_list_remove(obj->layer->evas->pointer.object.in, eo_obj);
                  obj->mouse_grabbed = 0;
                  if (obj->layer->evas->is_frozen)
                    {
                       obj->mouse_in = 0;
                       return;
                    }
                  if (obj->mouse_in)
                    {
                       Evas_Event_Mouse_Out ev;

                       _evas_object_event_new();

                       obj->mouse_in = 0;
                       ev.buttons = obj->layer->evas->pointer.button;
                       ev.output.x = obj->layer->evas->pointer.x;
                       ev.output.y = obj->layer->evas->pointer.y;
                       ev.canvas.x = obj->layer->evas->pointer.x;
                       ev.canvas.y = obj->layer->evas->pointer.y;
                       ev.data = NULL;
                       ev.modifiers = &(obj->layer->evas->modifiers);
                       ev.locks = &(obj->layer->evas->locks);
                       ev.timestamp = obj->layer->evas->last_timestamp;
                       ev.event_flags = EVAS_EVENT_FLAG_NONE;
                       event_id = _evas_object_event_new();
                       evas_object_event_callback_call(eo_obj, obj, EVAS_CALLBACK_MOUSE_OUT, &ev);
                       _evas_post_event_callback_call(obj->layer->evas, obj->layer->evas, event_id);
                    }
 */
               }
          }
     }
   else
     {
/*
        if (obj->mouse_grabbed > 0)
          obj->layer->evas->pointer.mouse_grabbed -= obj->mouse_grabbed;
        if ((obj->mouse_in) || (obj->mouse_grabbed > 0))
          obj->layer->evas->pointer.object.in = eina_list_remove(obj->layer->evas->pointer.object.in, eo_obj);
        obj->mouse_grabbed = 0;
        obj->mouse_in = 0;
 */
     }
   evas_object_update_bounding_box(eo_obj, obj, NULL);
   evas_object_inform_call_hide(eo_obj, obj);
}

EOLIAN static void
_efl_canvas_object_efl_gfx_entity_visible_set(Eo *eo_obj, Evas_Object_Protected_Data *obj,
                                       Eina_Bool vis)
{
   if (!obj->legacy.visible_set)
     obj->legacy.visible_set = EINA_TRUE;
   if (_evas_object_intercept_call_evas(obj, EVAS_OBJECT_INTERCEPT_CB_VISIBLE, 1, vis))
     return;

   if (vis) _show(eo_obj, obj);
   else _hide(eo_obj, obj);
}

EOLIAN static Eina_Bool
_efl_canvas_object_efl_gfx_entity_visible_get(const Eo *eo_obj EINA_UNUSED,
                                              Evas_Object_Protected_Data *obj)
{
   if (!EVAS_OBJECT_DATA_ALIVE(obj)) return EINA_FALSE;
#if 0
   // Try to return TRUE when an object is an EO object (must be finalized,
   // otherwise we can't know for sure if it's legacy or not).
   // Note: This was disabled as it used to lead to render bugs.
   if (obj->legacy.finalized && !obj->legacy.ctor && !obj->legacy.visible_set)
     return EINA_TRUE;
#endif
   return obj->cur->visible;
}

EAPI void
evas_object_color_set(Evas_Object *obj, int r, int g, int b, int a)
{
   efl_gfx_color_set((Evas_Object *)obj, r, g, b, a);
}

EOLIAN static void
_efl_canvas_object_efl_gfx_color_color_set(Eo *eo_obj, Evas_Object_Protected_Data *obj,
                                     int r, int g, int b, int a)
{
   int prev_a;

   if (obj->delete_me) return;

   if (EVAS_COLOR_SANITIZE(r, g, b, a))
     ERR("Evas only handles premultiplied colors (0 <= R,G,B <= A <= 255)");

   if (_evas_object_intercept_call_evas(obj, EVAS_OBJECT_INTERCEPT_CB_COLOR_SET, 1, r, g, b, a)) return;
   if ((obj->cur->color.r == r) &&
       (obj->cur->color.g == g) &&
       (obj->cur->color.b == b) &&
       (obj->cur->color.a == a)) return;

   prev_a = obj->cur->color.a;
   EINA_COW_STATE_WRITE_BEGIN(obj, state_write, cur)
     {
        state_write->color.r = r;
        state_write->color.g = g;
        state_write->color.b = b;
        state_write->color.a = a;
     }
   EINA_COW_STATE_WRITE_END(obj, state_write, cur);

   if (obj->is_smart && obj->smart.smart &&
       obj->smart.smart->smart_class &&
       obj->smart.smart->smart_class->color_set)
     {
        obj->smart.smart->smart_class->color_set(eo_obj, r, g, b, a);
     }

   evas_object_clip_dirty(eo_obj, obj);
   if ((prev_a == 0) && (a == 0) && (obj->cur->render_op == EVAS_RENDER_BLEND)) return;

   obj->changed_color = EINA_TRUE;
   evas_object_change(eo_obj, obj);
}

EAPI void
evas_object_color_get(const Evas_Object *obj, int *r, int *g, int *b, int *a)
{
   efl_gfx_color_get((Evas_Object *)obj, r, g, b, a);
}

EOLIAN static void
_efl_canvas_object_efl_gfx_color_color_get(const Eo *eo_obj EINA_UNUSED,
                                    Evas_Object_Protected_Data *obj,
                                    int *r, int *g, int *b, int *a)
{
   if (obj->delete_me)
     {
        if (r) *r = 0;
        if (g) *g = 0;
        if (b) *b = 0;
        if (a) *a = 0;
        return;
     }
   if (r) *r = obj->cur->color.r;
   if (g) *g = obj->cur->color.g;
   if (b) *b = obj->cur->color.b;
   if (a) *a = obj->cur->color.a;
}

EOLIAN static void
_efl_canvas_object_anti_alias_set(Eo *eo_obj, Evas_Object_Protected_Data *obj, Eina_Bool anti_alias)
{
   if (obj->delete_me) return;
   anti_alias = !!anti_alias;
   if (obj->cur->anti_alias == anti_alias)return;

   evas_object_async_block(obj);
   EINA_COW_STATE_WRITE_BEGIN(obj, state_write, cur)
     {
        state_write->anti_alias = anti_alias;
     }
   EINA_COW_STATE_WRITE_END(obj, state_write, cur);

   evas_object_change(eo_obj, obj);
}

EOLIAN static Eina_Bool
_efl_canvas_object_anti_alias_get(const Eo *eo_obj EINA_UNUSED, Evas_Object_Protected_Data *obj)
{
   if (obj->delete_me) return EINA_FALSE;
   return obj->cur->anti_alias;
}

EOLIAN static void
_efl_canvas_object_efl_gfx_entity_scale_set(Eo *eo_obj, Evas_Object_Protected_Data *obj, double scale)
{
   if (obj->delete_me) return;
   if (EINA_DBL_EQ(obj->cur->scale, scale)) return;

   evas_object_async_block(obj);
   EINA_COW_STATE_WRITE_BEGIN(obj, state_write, cur)
     {
        state_write->scale = scale;
     }
   EINA_COW_STATE_WRITE_END(obj, state_write, cur);
   evas_object_change(eo_obj, obj);
}

EOLIAN static double
_efl_canvas_object_efl_gfx_entity_scale_get(const Eo *eo_obj EINA_UNUSED, Evas_Object_Protected_Data *obj)
{
   if (obj->delete_me) return 1.0;
   return obj->cur->scale;
}

static void
_render_op_set(Evas_Object_Protected_Data *obj, Evas_Render_Op render_op)
{
   if (!obj || obj->delete_me) return;
   if (obj->cur->render_op == render_op) return;

   evas_object_async_block(obj);
   EINA_COW_STATE_WRITE_BEGIN(obj, state_write, cur)
     {
        state_write->render_op = render_op;
     }
   EINA_COW_STATE_WRITE_END(obj, state_write, cur);

   evas_object_change(obj->object, obj);
}

EOLIAN static void
_efl_canvas_object_render_op_set(Eo *eo_obj EINA_UNUSED, Evas_Object_Protected_Data *obj, Efl_Gfx_Render_Op rop)
{
   _render_op_set(obj, _gfx_to_evas_render_op(rop));
}

EAPI void
evas_object_render_op_set(Evas_Object *eo_obj, Evas_Render_Op render_op)
{
   Evas_Object_Protected_Data *obj = EVAS_OBJ_GET_OR_RETURN(eo_obj);
   _render_op_set(obj, render_op);
}

EOLIAN static Efl_Gfx_Render_Op
_efl_canvas_object_render_op_get(const Eo *eo_obj EINA_UNUSED, Evas_Object_Protected_Data *obj)
{
   return _evas_to_gfx_render_op(obj->cur->render_op);
}

EAPI Evas_Render_Op
evas_object_render_op_get(const Evas_Object *eo_obj)
{
   return _gfx_to_evas_render_op(efl_canvas_object_render_op_get(eo_obj));
}

EOLIAN static void
_efl_canvas_object_efl_object_dbg_info_get(Eo *eo_obj, Evas_Object_Protected_Data *obj EINA_UNUSED, Efl_Dbg_Info *root)
{
   efl_dbg_info_get(efl_super(eo_obj, MY_CLASS), root);
   Efl_Dbg_Info *group = EFL_DBG_INFO_LIST_APPEND(root, MY_CLASS_NAME);
   Efl_Dbg_Info *node;
   const char *name;
   double dblw, dblh;
   double dblx, dbly;
   double scale;
   unsigned int m;
   int r, g, b, a;
   //int requestw, requesth;
   Eina_Rect geom;
   Eina_Size2D max, min;
   short layer;
   Eina_Bool focus;
   Eina_Bool visible;
   Eina_Bool pass_event;
   Eina_Bool propagate_event;
   Eina_Bool repeat_event;
   Eina_Bool clipees_has;
   Eina_Bool fillx, filly;

   visible = efl_gfx_entity_visible_get(eo_obj);
   layer = efl_gfx_stack_layer_get(eo_obj);
   name = efl_name_get(eo_obj); // evas_object_name_get(eo_obj);
   geom = efl_gfx_entity_geometry_get(eo_obj);
   scale = efl_gfx_entity_scale_get(eo_obj);
   min = efl_gfx_size_hint_restricted_min_get(eo_obj);
   max = efl_gfx_size_hint_max_get(eo_obj);
   //efl_gfx_size_hint_request_get(eo_obj, &requestw, &requesth);
   efl_gfx_size_hint_align_get(eo_obj, &dblx, &dbly);
   efl_gfx_size_hint_weight_get(eo_obj, &dblw, &dblh);
   efl_gfx_size_hint_fill_get(eo_obj, &fillx, &filly);
   efl_gfx_color_get(eo_obj, &r, &g, &b, &a);
   focus = evas_object_focus_get(eo_obj);
   m = efl_canvas_object_pointer_mode_get(eo_obj);
   pass_event = efl_canvas_object_pass_events_get(eo_obj);
   repeat_event = efl_canvas_object_repeat_events_get(eo_obj);
   propagate_event = efl_canvas_object_propagate_events_get(eo_obj);
   clipees_has = efl_canvas_object_clipees_has(eo_obj);

   EFL_DBG_INFO_APPEND(group, "Visibility", EINA_VALUE_TYPE_CHAR, visible);

   if (name)
      EFL_DBG_INFO_APPEND(group, "Name", EINA_VALUE_TYPE_STRING, name);

   EFL_DBG_INFO_APPEND(group, "Layer", EINA_VALUE_TYPE_INT, layer);

   node = EFL_DBG_INFO_LIST_APPEND(group, "Position");
   EFL_DBG_INFO_APPEND(node, "x", EINA_VALUE_TYPE_INT, geom.x);
   EFL_DBG_INFO_APPEND(node, "y", EINA_VALUE_TYPE_INT, geom.y);

   node = EFL_DBG_INFO_LIST_APPEND(group, "Size");
   EFL_DBG_INFO_APPEND(node, "w", EINA_VALUE_TYPE_INT, geom.w);
   EFL_DBG_INFO_APPEND(node, "h", EINA_VALUE_TYPE_INT, geom.h);

   EFL_DBG_INFO_APPEND(group, "Scale", EINA_VALUE_TYPE_DOUBLE, scale);

   node = EFL_DBG_INFO_LIST_APPEND(group, "Min size");
   EFL_DBG_INFO_APPEND(node, "w", EINA_VALUE_TYPE_INT, min.w);
   EFL_DBG_INFO_APPEND(node, "h", EINA_VALUE_TYPE_INT, min.h);

   node = EFL_DBG_INFO_LIST_APPEND(group, "Max size");
   EFL_DBG_INFO_APPEND(node, "w", EINA_VALUE_TYPE_INT, max.w);
   EFL_DBG_INFO_APPEND(node, "h", EINA_VALUE_TYPE_INT, max.h);

   //node = EFL_DBG_INFO_LIST_APPEND(group, "Request size");
   //EFL_DBG_INFO_APPEND(node, "w", EINA_VALUE_TYPE_INT, requestw);
   //EFL_DBG_INFO_APPEND(node, "h", EINA_VALUE_TYPE_INT, requesth);

   node = EFL_DBG_INFO_LIST_APPEND(group, "Align");
   EFL_DBG_INFO_APPEND(node, "x", EINA_VALUE_TYPE_DOUBLE, dblx);
   EFL_DBG_INFO_APPEND(node, "y", EINA_VALUE_TYPE_DOUBLE, dbly);

   node = EFL_DBG_INFO_LIST_APPEND(group, "Fill");
   EFL_DBG_INFO_APPEND(node, "x", EINA_VALUE_TYPE_CHAR, fillx);
   EFL_DBG_INFO_APPEND(node, "y", EINA_VALUE_TYPE_CHAR, filly);

   node = EFL_DBG_INFO_LIST_APPEND(group, "Weight");
   EFL_DBG_INFO_APPEND(node, "w", EINA_VALUE_TYPE_DOUBLE, dblw);
   EFL_DBG_INFO_APPEND(node, "h", EINA_VALUE_TYPE_DOUBLE, dblh);

   node = EFL_DBG_INFO_LIST_APPEND(group, "Color");
   EFL_DBG_INFO_APPEND(node, "r", EINA_VALUE_TYPE_INT, r);
   EFL_DBG_INFO_APPEND(node, "g", EINA_VALUE_TYPE_INT, g);
   EFL_DBG_INFO_APPEND(node, "b", EINA_VALUE_TYPE_INT, b);
   EFL_DBG_INFO_APPEND(node, "a", EINA_VALUE_TYPE_INT, a);

   EFL_DBG_INFO_APPEND(group, "Has focus", EINA_VALUE_TYPE_CHAR, focus);

     {
        const char *text = NULL;
        switch (m)
          {
           case EVAS_OBJECT_POINTER_MODE_AUTOGRAB:
              text = "EVAS_OBJECT_POINTER_MODE_AUTOGRAB";
              break;
           case EVAS_OBJECT_POINTER_MODE_NOGRAB:
              text = "EVAS_OBJECT_POINTER_MODE_NOGRAB";
              break;
           case EVAS_OBJECT_POINTER_MODE_NOGRAB_NO_REPEAT_UPDOWN:
              text = "EVAS_OBJECT_POINTER_MODE_NOGRAB_NO_REPEAT_UPDOWN";
              break;
           default:
              text = NULL;
              break;
          }

        if (text)
           EFL_DBG_INFO_APPEND(group, "Pointer Mode", EINA_VALUE_TYPE_STRING, text);
     }

   EFL_DBG_INFO_APPEND(group, "Pass Events", EINA_VALUE_TYPE_CHAR, pass_event);
   EFL_DBG_INFO_APPEND(group, "Repeat Events", EINA_VALUE_TYPE_CHAR, repeat_event);
   EFL_DBG_INFO_APPEND(group, "Propagate Events", EINA_VALUE_TYPE_CHAR, propagate_event);
   EFL_DBG_INFO_APPEND(group, "Has clipees", EINA_VALUE_TYPE_CHAR, clipees_has);

   Evas_Object *clipper = NULL;
   clipper = efl_canvas_object_clip_get(eo_obj);
   EFL_DBG_INFO_APPEND(group, "Clipper", EINA_VALUE_TYPE_UINT64, (uintptr_t) clipper);

   const Evas_Map *map = evas_object_map_get(eo_obj);
   if (map)
     {  /* Save map coords count info if object has map */
        node = EFL_DBG_INFO_LIST_APPEND(group, "Evas Map");
        int points_count = evas_map_count_get(map);
        Efl_Dbg_Info *points = EFL_DBG_INFO_LIST_APPEND(node, "Points");
        Efl_Dbg_Info *pointsuv = EFL_DBG_INFO_LIST_APPEND(node, "Image UV");
        for (int i = 0 ; i < points_count; i++)
          {
               {
                  Evas_Coord px, py, pz;
                  evas_map_point_coord_get(map, i, &px, &py, &pz);
                  Efl_Dbg_Info *point = EFL_DBG_INFO_LIST_APPEND(points, "Points");
                  EFL_DBG_INFO_APPEND(point, "x", EINA_VALUE_TYPE_INT, px);
                  EFL_DBG_INFO_APPEND(point, "y", EINA_VALUE_TYPE_INT, py);
                  EFL_DBG_INFO_APPEND(point, "z", EINA_VALUE_TYPE_INT, pz);
               }

               {
                  double pu, pv;
                  evas_map_point_image_uv_get(map, i, &pu, &pv);
                  Efl_Dbg_Info *point = EFL_DBG_INFO_LIST_APPEND(pointsuv, "Image UV");
                  EFL_DBG_INFO_APPEND(point, "u", EINA_VALUE_TYPE_DOUBLE, pu);
                  EFL_DBG_INFO_APPEND(point, "v", EINA_VALUE_TYPE_DOUBLE, pv);
               }
          }
     }
}

EOLIAN static Eo *
_efl_canvas_object_efl_object_provider_find(const Eo *eo_obj EINA_UNUSED, Evas_Object_Protected_Data *obj, const Efl_Class *klass)
{
   if (klass == EVAS_CANVAS_CLASS)
     {
        if ((obj->delete_me) || (!obj->layer)) goto fallback;
        return obj->layer->evas->evas;
     }
   else if (klass == EFL_LOOP_CLASS)
     return efl_main_loop_get();
fallback:
   return efl_provider_find(efl_super(eo_obj, MY_CLASS), klass);
}

EOLIAN static Efl_Loop *
_efl_canvas_object_efl_loop_consumer_loop_get(const Eo *eo_obj EINA_UNUSED, Evas_Object_Protected_Data *obj EINA_UNUSED)
{
   return efl_main_loop_get();
}

EOLIAN static void
_efl_canvas_object_type_set(Eo *eo_obj, Evas_Object_Protected_Data *obj, const char *type)
{
   if (efl_finalized_get(eo_obj))
     {
        ERR("This function is only allowed during construction.");
        return;
     }
   obj->type = type; // Store it as the top type of this class
}

EOLIAN static void
_efl_canvas_object_precise_is_inside_set(Eo *eo_obj EINA_UNUSED, Evas_Object_Protected_Data *obj, Eina_Bool precise)
{
   evas_object_async_block(obj);
   obj->precise_is_inside = precise;
}

EOLIAN static Eina_Bool
_efl_canvas_object_precise_is_inside_get(const Eo *eo_obj EINA_UNUSED, Evas_Object_Protected_Data *obj)
{
   return obj->precise_is_inside;
}

EOLIAN static Eina_Bool
_efl_canvas_object_coords_inside_get(const Eo *eo_obj EINA_UNUSED, Evas_Object_Protected_Data *obj, Eina_Position2D pos)
{
   Eina_Rectangle c;

   if (obj->is_smart)
     {
        Eina_Rectangle bounding_box = { 0, };

        evas_object_smart_bounding_box_update(obj);
        evas_object_smart_bounding_box_get(obj, &bounding_box, NULL);
        c = bounding_box;
     }
   else
     {
        if (obj->clip.clipees) return EINA_FALSE;
        c = obj->cur->geometry;
     }
   return RECTS_INTERSECT(pos.x, pos.y, 1, 1, c.x, c.y, c.w, c.h);
}

static void
_is_frame_flag_set(Evas_Object_Protected_Data *obj, Eina_Bool is_frame)
{
   const Eina_Inlist *l;
   Evas_Object_Protected_Data *child;

   /* Small hack here:
    * The main frame object (ie. the frame edje object itself) will set
    * a value of 2 here (Eina_Bool is an unsigned char). That way we can
    * safely propagate the frame flag inside smart_member_add. */
   if (is_frame > 1)
     {
        obj->is_frame_top = EINA_TRUE;
        is_frame = EINA_TRUE;
     }

   obj->is_frame = is_frame;

   if (!obj->is_smart) return;
   l = evas_object_smart_members_get_direct(obj->object);

   EINA_INLIST_FOREACH(l, child)
     _is_frame_flag_set(child, is_frame);
}

EOLIAN static void
_efl_canvas_object_is_frame_object_set(Eo *eo_obj, Evas_Object_Protected_Data *obj, Eina_Bool is_frame)
{
   Evas_Coord x, y;

   evas_object_async_block(obj);
   evas_object_geometry_get(eo_obj, &x, &y, NULL, NULL);

   _is_frame_flag_set(obj, is_frame);

   evas_object_move(eo_obj, x, y);
}

EOLIAN static Eina_Bool
_efl_canvas_object_is_frame_object_get(const Eo *eo_obj  EINA_UNUSED, Evas_Object_Protected_Data *obj)
{
   return obj->is_frame;
}

EOLIAN static Evas_Object *
_efl_canvas_object_render_parent_get(const Eo *eo_obj EINA_UNUSED, Evas_Object_Protected_Data *obj)
{
   if (!obj) return NULL;
   return obj->smart.parent;
}

EOLIAN static void
_efl_canvas_object_paragraph_direction_set(Eo *eo_obj EINA_UNUSED, Evas_Object_Protected_Data *obj EINA_UNUSED, Evas_BiDi_Direction dir EINA_UNUSED)
{
   return;
}

EOLIAN static Evas_BiDi_Direction
_efl_canvas_object_paragraph_direction_get(const Eo *eo_obj EINA_UNUSED, Evas_Object_Protected_Data *obj EINA_UNUSED)
{
   return EVAS_BIDI_DIRECTION_NEUTRAL;
}

EOLIAN static void
_efl_canvas_object_legacy_ctor(Eo *eo_obj, Evas_Object_Protected_Data *obj)
{
   EINA_SAFETY_ON_FALSE_RETURN(!efl_finalized_get(eo_obj));
   obj->legacy.ctor = EINA_TRUE;
}

EAPI Eo *
evas_find(const Eo *obj)
{
   if (efl_isa(obj, EVAS_CANVAS_CLASS))
     return (Eo*) obj;
   if (efl_isa(obj, EFL_CANVAS_OBJECT_CLASS))
     return evas_object_evas_get(obj);
   return efl_provider_find(obj, EVAS_CANVAS_CLASS);
}

EOLIAN void
_efl_canvas_object_event_animation_set(Eo *eo_obj,
                                       Evas_Object_Protected_Data *pd,
                                       const Efl_Event_Description *desc,
                                       Efl_Canvas_Animation *animation)
{
   Event_Animation *event_anim = _event_animation_find(pd, desc);

   if (!pd->anim_player)
     {
        pd->anim_player = efl_add(EFL_CANVAS_ANIMATION_PLAYER_CLASS, eo_obj,
                                  efl_animation_player_target_set(efl_added, eo_obj));
     }

   if (event_anim)
     {
        if (event_anim->anim == animation)
          return;

        pd->event_anims =
           eina_inlist_remove(pd->event_anims, EINA_INLIST_GET(event_anim));

        _event_anim_free(event_anim, pd);
     }

   if (!animation) return;

   event_anim = calloc(1, sizeof(Event_Animation));

   //Set callback for Hide event
   if (desc == EFL_GFX_ENTITY_EVENT_HIDE)
     {
        evas_object_intercept_hide_callback_add(eo_obj,
                                                _animation_intercept_hide,
                                                event_anim);
     }

   event_anim->desc = desc;
   event_anim->anim = animation;

   pd->event_anims =
      eina_inlist_append(pd->event_anims, EINA_INLIST_GET(event_anim));
}

EOLIAN Efl_Canvas_Animation *
_efl_canvas_object_event_animation_get(const Eo *eo_obj EINA_UNUSED,
                                       Evas_Object_Protected_Data *pd,
                                       const Efl_Event_Description *desc)
{
   Event_Animation *event_anim = _event_animation_find(pd, desc);
   if (event_anim)
     return event_anim->anim;

   return NULL;
}

void
_efl_canvas_object_event_animation_cancel(Eo *eo_obj)
{
   Evas_Object_Protected_Data *obj = EVAS_OBJECT_DATA_SAFE_GET(eo_obj);

   if (obj)
     efl_player_stop(obj->anim_player);
}

/* legacy */

EAPI const char *
evas_object_type_get(const Evas_Object *eo_obj)
{
   Evas_Object_Protected_Data *obj = EVAS_OBJ_GET_OR_RETURN(eo_obj, NULL);
   if (obj->delete_me) return "";
   return obj->type;
}

EAPI void
evas_object_static_clip_set(Evas_Object *eo_obj, Eina_Bool is_static_clip)
{
   Evas_Object_Protected_Data *obj = EVAS_OBJ_GET_OR_RETURN(eo_obj);
   evas_object_async_block(obj);
   obj->is_static_clip = is_static_clip;
}

EAPI Eina_Bool
evas_object_static_clip_get(const Evas_Object *eo_obj)
{
   Evas_Object_Protected_Data *obj = EVAS_OBJ_GET_OR_RETURN(eo_obj, EINA_FALSE);
   return obj->is_static_clip;
}

EAPI void
evas_object_size_hint_aspect_set(Evas_Object *obj, Evas_Aspect_Control aspect, Evas_Coord w, Evas_Coord h)
{
   efl_gfx_size_hint_aspect_set(obj, aspect, EINA_SIZE2D(w, h));
}

EAPI void
evas_object_size_hint_aspect_get(const Evas_Object *obj, Evas_Aspect_Control *aspect, Evas_Coord *w, Evas_Coord *h)
{
   Eina_Size2D sz = { 0, 0 };
   efl_gfx_size_hint_aspect_get(obj, aspect, &sz);
   if (w) *w = sz.w;
   if (h) *h = sz.h;
}

EAPI void
evas_object_size_hint_max_set(Evas_Object *obj, Evas_Coord w, Evas_Coord h)
{
   efl_gfx_size_hint_max_set(obj, EINA_SIZE2D(w, h));
}

EAPI void
evas_object_size_hint_max_get(const Evas_Object *obj, Evas_Coord *w, Evas_Coord *h)
{
   Eina_Size2D sz;
   sz = efl_gfx_size_hint_max_get(obj);
   if (w) *w = sz.w;
   if (h) *h = sz.h;
}

EAPI void
evas_object_size_hint_min_set(Evas_Object *obj, Evas_Coord w, Evas_Coord h)
{
   efl_gfx_size_hint_restricted_min_set(obj, EINA_SIZE2D(w, h));
}

EAPI void
evas_object_size_hint_min_get(const Evas_Object *obj, Evas_Coord *w, Evas_Coord *h)
{
   Eina_Size2D sz;
   sz = efl_gfx_size_hint_restricted_min_get(obj);
   if (w) *w = sz.w;
   if (h) *h = sz.h;
}

EAPI void
evas_object_size_hint_padding_set(Evas_Object *obj, Evas_Coord l, Evas_Coord r, Evas_Coord t, Evas_Coord b)
{
   efl_gfx_size_hint_margin_set(obj, l, r, t, b);
}

EAPI void
evas_object_size_hint_padding_get(const Evas_Object *obj, Evas_Coord *l, Evas_Coord *r, Evas_Coord *t, Evas_Coord *b)
{
   efl_gfx_size_hint_margin_get(obj, l, r, t, b);
}

EAPI void
evas_object_size_hint_weight_set(Evas_Object *obj, double x, double y)
{
   efl_gfx_size_hint_weight_set(obj, x, y);
}

EAPI void
evas_object_size_hint_weight_get(const Evas_Object *obj, double *x, double *y)
{
   efl_gfx_size_hint_weight_get(obj, x, y);
}

EAPI void
evas_object_size_hint_align_set(Evas_Object *obj, double x, double y)
{
   efl_gfx_size_hint_align_set(obj, x, y);
}

EAPI void
evas_object_size_hint_align_get(const Evas_Object *obj, double *x, double *y)
{
   efl_gfx_size_hint_align_get(obj, x, y);
}

EAPI Evas *
evas_object_evas_get(const Eo *eo_obj)
{
   if (efl_isa(eo_obj, EFL_CANVAS_OBJECT_CLASS))
     {
        Evas_Object_Protected_Data *obj = EVAS_OBJ_GET_OR_RETURN(eo_obj, NULL);

        if (!obj->layer || !obj->layer->evas) return NULL;
        return obj->layer->evas->evas;
     }
   return efl_provider_find((Eo *) eo_obj, EVAS_CANVAS_CLASS);
}

EAPI void
evas_object_scale_set(Evas_Object *obj, double scale)
{
   efl_gfx_entity_scale_set(obj, scale);
}

EAPI double
evas_object_scale_get(const Evas_Object *obj)
{
   return efl_gfx_entity_scale_get(obj);
}

EAPI Eina_Bool
evas_object_pointer_inside_by_device_get(const Evas_Object *eo_obj, Efl_Input_Device *dev)
{
   Evas_Object_Protected_Data *obj = EVAS_OBJ_GET_OR_RETURN(eo_obj, EINA_FALSE);

   obj->is_pointer_inside_legacy = EINA_TRUE;
   return efl_canvas_pointer_inside_get(eo_obj, dev);
}

EAPI Eina_Bool
evas_object_pointer_coords_inside_get(const Evas_Object *eo_obj, int x, int y)
{
   Eina_Position2D pos = EINA_POSITION2D(x, y);

   return efl_canvas_object_coords_inside_get(eo_obj, pos);
}

EAPI Eina_Bool
evas_object_pointer_inside_get(const Evas_Object *eo_obj)
{
   return evas_object_pointer_inside_by_device_get(eo_obj, NULL);
}

/* Internal EO APIs and hidden overrides */

EOAPI EFL_VOID_FUNC_BODY(efl_canvas_object_legacy_ctor)
EOAPI EFL_VOID_FUNC_BODYV(efl_canvas_object_type_set, EFL_FUNC_CALL(type), const char *type)

#define EFL_CANVAS_OBJECT_EXTRA_OPS \
   EFL_OBJECT_OP_FUNC(efl_dbg_info_get, _efl_canvas_object_efl_object_dbg_info_get), \
   EFL_OBJECT_OP_FUNC(efl_event_callback_legacy_call, _efl_canvas_object_efl_object_event_callback_legacy_call), \
   EFL_OBJECT_OP_FUNC(efl_event_callback_call, _efl_canvas_object_efl_object_event_callback_call), \
   EFL_OBJECT_OP_FUNC(efl_canvas_object_legacy_ctor, _efl_canvas_object_legacy_ctor), \
   EFL_OBJECT_OP_FUNC(efl_canvas_object_type_set, _efl_canvas_object_type_set)

#include "canvas/efl_canvas_object.eo.c"
