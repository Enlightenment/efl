#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif
#include <Elementary.h>
#include <Elementary_Cursor.h>
#include "elm_priv.h"
#include "elm_widget_gengrid.h"

#include <limits.h>

#define GG_IT(_it) (_it->item)

#define EINA_INLIST_REVERSE_FOREACH_INSIDE(list, it)                         \
   for (it = NULL, it = (list ? _EINA_INLIST_CONTAINER(it, list) : NULL); it; \
        it = (EINA_INLIST_GET(it)->prev                                       \
                  ? _EINA_INLIST_CONTAINER(it, EINA_INLIST_GET(it)->prev)     \
                  : NULL))

#define EINA_LIST_REVERSE_FOREACH_INSIDE(list, l, data) \
   for (l = list, data = eina_list_data_get(l); l;       \
        l = eina_list_prev(l), data = eina_list_data_get(l))

#define WEIGHT_MAX ((double)INT_MAX / (double)1000000)

/**
 * Find the first parent of object.
 * Parent must be type of elm_gengrid.
 *
 * @param base
 *
 * @return
 */
static const Evas_Object *
_find_gengrid_parent_item(const Evas_Object *base)
{
   const Evas_Object *res = NULL;
   if (base)
     {
        const Evas_Object *parent = NULL;
        parent = elm_widget_parent_get(base);
        
        while (parent)
          {
             if (evas_object_smart_type_check(parent, "elm_gengrid"))
               {
                  res = parent;
                  break;
               }
             parent = elm_widget_parent_get(parent);
          }
     }
   return res;
}

static Eina_Bool
_can_focus(const Evas_Object *obj)
{
   if (obj && elm_object_focus_allow_get(obj) && elm_widget_can_focus_get(obj))
     return EINA_TRUE;
   return EINA_FALSE;
}

static Eina_List *
_get_contents(const Evas_Object *obj)
{
   Eina_List *res = NULL;
   if (!obj) return NULL;
   
   if (evas_object_smart_type_check(obj, "elm_layout"))
     {
        Evas_Object *edje = elm_layout_edje_get(obj);
        
        if (edje)
          {
             const Eina_List *l = NULL;
             const char *key;
             const Eina_List *contents =
               elm_widget_stringlist_get(edje_object_data_get(edje, "contents"));
             
             EINA_LIST_FOREACH(contents, l, key)
               {
                  if (key)
                    {
                       const Evas_Object *child = 
                         edje_object_part_swallow_get(edje, key);
                       if (child)
                         {
                            Eina_List *l1 = NULL;
                            res = eina_list_append(res, child);
                            
                            l1 = _get_contents(child);
                            if (l1) res = eina_list_merge(res, l1);
                         }
                    }
               }
          }
     }
   return res;
}

static Eina_List *
_gengrid_item_contents_get(const Elm_Gen_Item *it)
{
   Eina_List *res = NULL;
   Eina_List *l = NULL;
   Evas_Object *child = NULL;
   EINA_LIST_FOREACH(it->content_objs, l, child)
     {
        if (child)
          {
             Eina_List *l1 = NULL;
             res = eina_list_append(res, child);
             
             l1 = _get_contents(child);
             if (l1) res = eina_list_merge(res, l1);
          }
     }
  return res;
}

static const Evas_Object *
_find_focusable_object(const Elm_Gen_Item *it,
                       const Evas_Object *base,
                       Elm_Focus_Direction dir)
{
   const Evas_Object *obj = NULL;
   Eina_List *l = NULL;
   Eina_List *contents = NULL;
   
   if (!it) return NULL;
   
   contents = _gengrid_item_contents_get(it);
   if (contents)
     {
        if (base)
          {
             l = eina_list_data_find_list(contents, base);
             obj = base;
          }
        
        if ((dir == ELM_FOCUS_LEFT) || (dir == ELM_FOCUS_UP))
          {
             if (l) l = eina_list_prev(l);
             else l = eina_list_last(contents);
             
             while (l)
               {
                  obj = eina_list_data_get(l);
                  if (_can_focus(obj)) break;
                  obj = NULL;
                  l = eina_list_prev(l);
               }
          }
        else if ((dir == ELM_FOCUS_RIGHT) || (dir == ELM_FOCUS_DOWN))
          {
             if (l) l = eina_list_next(l);
             else l = contents;
             while (l)
               {
                  obj = eina_list_data_get(l);
                  if (_can_focus(obj)) break;
                  obj = NULL;
                  l = eina_list_next(l);
               }
          }
        eina_list_free(contents);
     }
   return obj;
}



static Eina_Bool
_check_item_contains(Elm_Gen_Item *it, const Evas_Object *base)
{
   Eina_Bool res = EINA_FALSE;
   Eina_List *contents = NULL;
   contents = _gengrid_item_contents_get(it);
   
   if (contents)
     {
        const Eina_List *l = NULL;
        Evas_Object *content = NULL;
        
        // loop contents (Evas_Object) of item
        EINA_LIST_FOREACH(contents, l, content)
          {
             if (content == base)
               {
                  res = EINA_TRUE;
                  break;
               }
          }
        eina_list_free(contents);
     }
   return res;
}

static Elm_Gen_Item *
_find_item_for_base(const Evas_Object *obj,
                    const Evas_Object *base)
{
   ELM_GENGRID_CHECK(obj) EINA_FALSE;
   ELM_GENGRID_DATA_GET(obj, sd);
   Elm_Gen_Item *res = NULL;
   Elm_Gen_Item *it = (Elm_Gen_Item *)(sd->last_selected_item);

   if (it && (_check_item_contains(it, base))) res = it;
   else
     {  // try find in all
        EINA_INLIST_FOREACH(sd->items, it)
          {
             if (it && _check_item_contains(it, base))
               {
                  res = it;
                  break;
               }
          }
     }
   return res;
}

static Eina_Bool
_gengrid_self_focus_item_get(const Evas_Object *obj, const Evas_Object *base,
                             // list of Elm_Gen_Items
                             const Eina_List *items,
                             void *(*list_data_get)(const Eina_List *l) EINA_UNUSED,
                             double degree, Evas_Object **direction,
                             double *weight)
{
   Evas_Coord ox, oy;
   Evas_Coord vw, vh;
   const Evas_Object *res_obj = NULL;
   Elm_Focus_Direction dir = ELM_FOCUS_UP;
   unsigned int items_count = 0;
   unsigned int columns = 0, items_visible = 0;
   unsigned int items_row = 0, items_col = 0, rows = 0;
   int new_position = 0;
   int cx = 0;
   int cy = 0;
   int focused_pos = 0;
   Elm_Gen_Item *it = NULL;
   Elm_Gen_Item *it_res = NULL;
   Eina_List *list = NULL;
   Eina_List *l = NULL;
   Elm_Gen_Item *focused_item = NULL;
   ELM_GENGRID_CHECK(obj) EINA_FALSE;
   ELM_GENGRID_DATA_GET(obj, sd);
   
   if ((!direction) || (!weight) || (!base) || (!items)) return EINA_FALSE;

   evas_object_geometry_get(sd->pan_obj, &ox, &oy, &vw, &vh);
   
   focused_item = _find_item_for_base(obj, base);
   if (!focused_item) return EINA_FALSE;
   
   if (degree == 0) dir = ELM_FOCUS_UP;
   else if (degree == 90) dir = ELM_FOCUS_RIGHT;
   else if (degree == 180) dir = ELM_FOCUS_DOWN;
   else if (degree == 270) dir = ELM_FOCUS_LEFT;
   else return EINA_FALSE;
   
   res_obj = _find_focusable_object(focused_item, base, dir);
   if (res_obj && (res_obj != base))
     {
        *direction = (Evas_Object *)res_obj;
        *weight = WEIGHT_MAX;
        return EINA_TRUE;
     }
   
   focused_pos = focused_item->position - 1;
   
   items_count = sd->item_count - 
     eina_list_count(sd->group_items) + sd->items_lost;
   if (sd->horizontal)
     {
        if (sd->item_height > 0) items_visible = vh / sd->item_height;
        if (items_visible < 1) items_visible = 1;
        
        columns = items_count / items_visible;
        if (items_count % items_visible) columns++;
        
        items_row = items_visible;
        if (items_row > sd->item_count) items_row = sd->item_count;
        
        cx = focused_pos / items_row;
        cy = focused_pos % items_row;
     }
   else
     {
        if (sd->item_width > 0) items_visible = vw / sd->item_width;
        if (items_visible < 1) items_visible = 1;
        
        rows = items_count / items_visible;
        if (items_count % items_visible) rows++;
        
        items_col = items_visible;
        if (items_col > sd->item_count) items_col = sd->item_count;
        
        cy = focused_pos / items_col;
        cx = focused_pos % items_col;
     }
   
   
   if (dir == ELM_FOCUS_UP) cy--;
   else if (dir == ELM_FOCUS_RIGHT) cx++;
   else if (dir == ELM_FOCUS_DOWN) cy++;
   else if (dir == ELM_FOCUS_LEFT) cx--;
   
   if (cx < 0 || cy < 0) return EINA_FALSE;
   
   if (sd->horizontal)
     {
        if ((cy > (int)(items_row - 1)) ||
            (cx > (int)(columns - 1))) return EINA_FALSE;
        new_position = items_row * cx + cy;
     }
   else
     {
        if ((cx > (int)(items_col - 1)) ||
            (cy > (int)(rows - 1))) return EINA_FALSE;
        new_position = cx + items_col * cy;
     }
   
   if (new_position > (int)(items_count - 1)) return EINA_FALSE;
   
   focused_pos++;
   new_position++;
   
   list = eina_list_data_find_list(items, focused_item);
   
   if (!list) return EINA_FALSE;
   
   if (new_position > focused_pos)
     {
        /// New position should be after focused
        EINA_LIST_FOREACH(list, l, it)
          {
             if (it->position == new_position)
               {
                  it_res = it;
                  break;
               }
          }
     }
   else if (new_position < focused_pos)
     {
        /// New position should be before focused
        EINA_LIST_REVERSE_FOREACH_INSIDE(list, l, it)
          {
             if (it->position == new_position)
               {
                  it_res = it;
                  break;
               }
          }
     }
   else return EINA_FALSE;
   
   if (it_res)
     {
        res_obj = _find_focusable_object(it_res, base, dir);
        *direction = (Evas_Object *)res_obj;
        *weight = WEIGHT_MAX;
        return EINA_TRUE;
     }
   return EINA_FALSE;
}

static Eina_Bool
_gengrid_focus_list_direction_get(const Evas_Object *obj,
                                  const Evas_Object *base,
                                  // list of Elm_Gen_Items
                                  const Eina_List *items,
                                  void *(*list_data_get)(const Eina_List *l),
                                  double degree, Evas_Object **direction,
                                  double *weight)
{
   const Eina_List *l = NULL;
   Evas_Object *current_best = NULL;
   ELM_GENGRID_CHECK(obj) EINA_FALSE;
   
   if ((!direction) || (!weight) || (!base) || (!items)) return EINA_FALSE;
   
   l = items;
   current_best = *direction;
   
   // loop items Elm_Gen_Item
   for (; l; l = eina_list_next(l))
     {
        Eina_List *contents = NULL;
        Elm_Gen_Item *it = list_data_get(l);
        contents = _gengrid_item_contents_get(it);
        if (contents)
          {
             const Eina_List *l2 = NULL;
             Evas_Object *content = NULL;
             
             // loop contents (Evas_Object) of item
             EINA_LIST_FOREACH(contents, l2, content)
               {
                  // if better element than set new sd->focused and sd->selected
                  elm_widget_focus_direction_get(content, base, degree,
                                                 direction, weight);
               }
             eina_list_free(contents);
          }
     }
   if (current_best != *direction) return EINA_TRUE;
   return EINA_FALSE;
}

EAPI void
gen_focus_direction(Eo *obj, void *_pd, va_list *list)
{
   Eina_List *items = NULL;
   const Evas_Object *parent = NULL;
   void *(*list_data_get)(const Eina_List * list);
   Eina_List *(*list_free)(Eina_List * list);
   Elm_Gengrid_Smart_Data *sd = _pd;
   Eina_Bool (*list_direction_get)(const Evas_Object * obj, const Evas_Object * base,
                                   const Eina_List * items, void * (*list_data_get)(const Eina_List * l),
                                   double degree, Evas_Object * *direction, double * weight);
   
   Evas_Object *base = va_arg(*list, Evas_Object *);
   double degree = va_arg(*list, double);
   Evas_Object **direction = va_arg(*list, Evas_Object **);
   double *weight = va_arg(*list, double *);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   
   if (!sd)
     {
        *ret = EINA_FALSE;
        return;
     }
   
   *ret = EINA_FALSE;
   list_data_get = NULL;
   list_direction_get = NULL;
   list_free = NULL;
   parent = _find_gengrid_parent_item(base);
   
   /// If focused is subobject of this gengrid then we selected next
   /// in direction
   if (obj == parent)
     {
        items = elm_gengrid_realized_items_get(obj);
        list_data_get = NULL;
        list_direction_get = _gengrid_self_focus_item_get;
        list_free = eina_list_free;
     }
   
   if (!items)
     {
        items = (Eina_List *)(elm_object_focus_custom_chain_get(obj));
        list_data_get = eina_list_data_get;
        list_direction_get = elm_widget_focus_list_direction_get;
        list_free = NULL;
     }
   
   if (!items)
     {
        items = elm_gengrid_realized_items_get(obj);
        list_data_get = eina_list_data_get;
        list_direction_get = _gengrid_focus_list_direction_get;
        list_free = eina_list_free;
     }
   
   if (!items)
     {
        *ret = EINA_FALSE;
        return;
     }
   
   *ret = list_direction_get(obj, base, items, list_data_get, degree,
                             direction, weight);
   if (list_free) list_free(items);
}

// module api funcs needed
EAPI int
elm_modapi_init(void *m EINA_UNUSED)
{
   return 1; // succeed always
}

EAPI int
elm_modapi_shutdown(void *m EINA_UNUSED)
{
   return 1; // succeed always
}
