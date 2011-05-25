#include <Elementary.h>
#include "elm_priv.h"

/**
 * @defgroup Grid Grid
 *
 * The grid is a grid layout widget that lays out a series of children as a
 * fixed "grid" of widgets using a given percentage of the grid width and
 * height each using the child object.
 *
 * The Grid uses a "Virtual resolution" that is stretched to fill the grid
 * widgets size itself. The default is 100 x 100, so that means the
 * position and sizes of children will effectively be percentages (0 to 100)
 * of the width or height of the grid widget
 *
 */

typedef struct _Widget_Data Widget_Data;

struct _Widget_Data
{
   Evas_Object *obj, *grd;
};

static const char *widtype = NULL;
static void _del_hook(Evas_Object *obj);
static void _theme_hook(Evas_Object *obj);

static void
_del_hook(Evas_Object *obj)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   free(wd);
}

static Eina_Bool
_elm_grid_focus_next_hook(const Evas_Object *obj, Elm_Focus_Direction dir, Evas_Object **next)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   const Eina_List *items;
   void *(*list_data_get) (const Eina_List *list);
   Eina_List *(*list_free) (Eina_List *list);
   
   if ((!wd) || (!wd->grd))
      return EINA_FALSE;
   
   /* Focus chain */
   /* TODO: Change this to use other chain */
   if ((items = elm_widget_focus_custom_chain_get(obj)))
     {
        list_data_get = eina_list_data_get;
        list_free = NULL;
     }
   else
     {
        items = evas_object_table_children_get(wd->grd);
        list_data_get = eina_list_data_get;
        list_free = eina_list_free;
        
        if (!items) return EINA_FALSE;
     }
   
   Eina_Bool ret = elm_widget_focus_list_next_get(obj, items, list_data_get,
                                                  dir, next);
   
   if (list_free)
      list_free((Eina_List *)items);
   
   return ret;
}

static void
_mirrored_set(Evas_Object *obj, Eina_Bool rtl)
{
   Widget_Data *wd = elm_widget_data_get(obj);
   if ((!wd) || (!wd->grd)) return;
   evas_object_grid_mirrored_set(wd->grd, rtl);
}

static void
_theme_hook(Evas_Object *obj)
{
   _elm_widget_mirrored_reload(obj);
   _mirrored_set(obj, elm_widget_mirrored_get(obj));
}

/**
 * Add a new grid to the parent
 *
 * @param parent The parent object
 * @return The new object or NULL if it cannot be created
 *
 * @ingroup Grid
 */
EAPI Evas_Object *
elm_grid_add(Evas_Object *parent)
{
   Evas_Object *obj;
   Evas *e;
   Widget_Data *wd;

   ELM_WIDGET_STANDARD_SETUP(wd, Widget_Data, parent, e, obj, NULL);

   ELM_SET_WIDTYPE(widtype, "grid");
   wd->obj = obj;
   elm_widget_type_set(obj, "grid");
   elm_widget_sub_object_add(parent, obj);
   elm_widget_data_set(obj, wd);
   elm_widget_del_hook_set(obj, _del_hook);
   elm_widget_focus_next_hook_set(obj, _elm_grid_focus_next_hook);
   elm_widget_can_focus_set(obj, EINA_FALSE);
   elm_widget_theme_hook_set(obj, _theme_hook);

   wd->grd = evas_object_grid_add(e);
   evas_object_grid_size_set(wd->grd, 100, 100);
   elm_widget_resize_object_set(obj, wd->grd);
   
   _mirrored_set(obj, elm_widget_mirrored_get(obj));
   return obj;
}

/**
 * Set the virtual size of the grid
 *
 * @param obj The grid object
 * @param w The virtual width of the grid
 * @param h The virtual height of the grid
 *
 * @ingroup Grid
 */
EAPI void
elm_grid_size_set(Evas_Object *obj, int w, int h)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   evas_object_grid_size_set(wd->grd, w, h);
}

/**
 * Get the virtual size of the grid
 *
 * @param obj The grid object
 * @param w Pointer to integer to store the virtual width of the grid
 * @param h Pointer to integer to store the virtual height of the grid
 *
 * @ingroup Grid
 */
EAPI void
elm_grid_size_get(Evas_Object *obj, int *w, int *h)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   evas_object_grid_size_get(wd->grd, w, h);
}

/**
 * Pack child at given position and size
 *
 * @param obj The grid object
 * @param subobj The child to pack
 * @param x The virtual x coord at which to pack it
 * @param y The virtual y coord at which to pack it
 * @param w The virtual width at which to pack it
 * @param h The virtual height at which to pack it
 *
 * @ingroup Grid
 */
EAPI void
elm_grid_pack(Evas_Object *obj, Evas_Object *subobj, int x, int y, int w, int h)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_widget_sub_object_add(obj, subobj);
   evas_object_grid_pack(wd->grd, subobj, x, y, w, h);
}

/**
 * Unpack a child from a grid object
 *
 * @param obj The grid object
 * @param subobj The child to unpack
 *
 * @ingroup Grid
 */
EAPI void
elm_grid_unpack(Evas_Object *obj, Evas_Object *subobj)
{
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   elm_widget_sub_object_del(obj, subobj);
   evas_object_grid_unpack(wd->grd, subobj);
}

/**
 * Faster way to remove all child objects from a grid object.
 *
 * @param obj The grid object
 * @param clear If true, it will delete just removed children
 *
 * @ingroup Grid
 */
EAPI void
elm_grid_clear(Evas_Object *obj, Eina_Bool clear)
{
   Eina_List *chld;
   Evas_Object *o;
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   chld = evas_object_grid_children_get(wd->grd);
   EINA_LIST_FREE(chld, o) elm_widget_sub_object_del(obj, o);
   evas_object_grid_clear(wd->grd, clear);
}

/**
 * Set packing of an existing child at to position and size
 *
 * @param subobj The child to set packing of
 * @param x The virtual x coord at which to pack it
 * @param y The virtual y coord at which to pack it
 * @param w The virtual width at which to pack it
 * @param h The virtual height at which to pack it
 *
 * @ingroup Grid
 */
EAPI void
elm_grid_pack_set(Evas_Object *subobj, int x, int y, int w, int h)
{
   Evas_Object *obj = elm_widget_parent_widget_get(subobj);
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   evas_object_grid_pack(wd->grd, subobj, x, y, w, h);
}

/**
 * get packing of a child
 *
 * @param subobj The child to query
 * @param x Pointer to integer to store the virtual x coord
 * @param y Pointer to integer to store the virtual y coord
 * @param w Pointer to integer to store the virtual width
 * @param h Pointer to integer to store the virtual height
 *
 * @ingroup Grid
 */
EAPI void
elm_grid_pack_get(Evas_Object *subobj, int *x, int *y, int *w, int *h)
{
   Evas_Object *obj = elm_widget_parent_widget_get(subobj);
   ELM_CHECK_WIDTYPE(obj, widtype);
   Widget_Data *wd = elm_widget_data_get(obj);
   if (!wd) return;
   evas_object_grid_pack_get(wd->grd, subobj, x, y, w, h);
}
