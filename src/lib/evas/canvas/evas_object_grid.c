#include "evas_common_private.h"
#include "evas_private.h"
#include <errno.h>

#define MY_CLASS EVAS_GRID_CLASS

typedef struct _Evas_Grid_Data              Evas_Grid_Data;
typedef struct _Evas_Object_Grid_Option     Evas_Object_Grid_Option;
typedef struct _Evas_Object_Grid_Iterator   Evas_Object_Grid_Iterator;
typedef struct _Evas_Object_Grid_Accessor   Evas_Object_Grid_Accessor;

struct _Evas_Object_Grid_Option
{
   Evas_Object *obj;
   Eina_List *l;
   int x, y, w, h;
};

struct _Evas_Grid_Data
{
   Evas_Object_Smart_Clipped_Data base;
   Eina_List *children;
   struct {
      int w, h;
   } size;
   Eina_Bool is_mirrored : 1;
};

struct _Evas_Object_Grid_Iterator
{
   Eina_Iterator iterator;

   Eina_Iterator *real_iterator;
   const Evas_Object *grid;
};

struct _Evas_Object_Grid_Accessor
{
   Eina_Accessor accessor;

   Eina_Accessor *real_accessor;
   const Evas_Object *grid;
};

#define EVAS_OBJECT_GRID_DATA_GET(o, ptr)			\
  Evas_Grid_Data *ptr = efl_data_scope_get(o, MY_CLASS)

#define EVAS_OBJECT_GRID_DATA_GET_OR_RETURN(o, ptr)			\
  EVAS_OBJECT_GRID_DATA_GET(o, ptr);					\
  if (!ptr)								\
    {									\
      ERR("No widget data for object %p (%s)",				\
	   o, evas_object_type_get(o));					\
       return;								\
    }

#define EVAS_OBJECT_GRID_DATA_GET_OR_RETURN_VAL(o, ptr, val)		\
  EVAS_OBJECT_GRID_DATA_GET(o, ptr);					\
  if (!ptr)								\
    {									\
       ERR("No widget data for object %p (%s)",	                \
	       o, evas_object_type_get(o));				\
       return val;							\
    }

static const char EVAS_OBJECT_GRID_OPTION_KEY[] = "|EvGd";

static Eina_Bool
_evas_object_grid_iterator_next(Evas_Object_Grid_Iterator *it, void **data)
{
   Evas_Object_Grid_Option *opt;

   if (!eina_iterator_next(it->real_iterator, (void **)&opt))
     return EINA_FALSE;
   if (data) *data = opt->obj;
   return EINA_TRUE;
}

static Evas_Object *
_evas_object_grid_iterator_get_container(Evas_Object_Grid_Iterator *it)
{
   return (Evas_Object *)it->grid;
}

static void
_evas_object_grid_iterator_free(Evas_Object_Grid_Iterator *it)
{
   eina_iterator_free(it->real_iterator);
   free(it);
}

static Eina_Bool
_evas_object_grid_accessor_get_at(Evas_Object_Grid_Accessor *it, unsigned int idx, void **data)
{
   Evas_Object_Grid_Option *opt = NULL;

   if (!eina_accessor_data_get(it->real_accessor, idx, (void **)&opt))
     return EINA_FALSE;
   if (data) *data = opt->obj;
   return EINA_TRUE;
}

static Evas_Object *
_evas_object_grid_accessor_get_container(Evas_Object_Grid_Accessor *it)
{
   return (Evas_Object *)it->grid;
}

static void
_evas_object_grid_accessor_free(Evas_Object_Grid_Accessor *it)
{
   eina_accessor_free(it->real_accessor);
   free(it);
}

static Evas_Object_Grid_Option *
_evas_object_grid_option_get(Evas_Object *o)
{
   return evas_object_data_get(o, EVAS_OBJECT_GRID_OPTION_KEY);
}

static void
_evas_object_grid_option_set(Evas_Object *o, const Evas_Object_Grid_Option *opt)
{
   evas_object_data_set(o, EVAS_OBJECT_GRID_OPTION_KEY, opt);
}

static Evas_Object_Grid_Option *
_evas_object_grid_option_del(Evas_Object *o)
{
   return evas_object_data_del(o, EVAS_OBJECT_GRID_OPTION_KEY);
}

static void
_on_child_del(void *data, Evas *evas EINA_UNUSED, Evas_Object *child, void *einfo EINA_UNUSED)
{
   Evas_Object *grid = data;
   evas_object_grid_unpack(grid, child);
}

static void
_evas_object_grid_child_connect(Evas_Object *o, Evas_Object *child)
{
   evas_object_event_callback_add
     (child, EVAS_CALLBACK_DEL, _on_child_del, o);
}

static void
_evas_object_grid_child_disconnect(Evas_Object *o, Evas_Object *child)
{
   evas_object_event_callback_del_full
     (child, EVAS_CALLBACK_DEL, _on_child_del, o);
}

EVAS_SMART_SUBCLASS_NEW("Evas_Object_Grid", _evas_object_grid,
			Evas_Smart_Class, Evas_Smart_Class,
			evas_object_smart_clipped_class_get, NULL)

static void
_evas_object_grid_smart_add(Evas_Object *o)
{
   Evas_Object_Smart_Clipped_Data *base;
   Evas_Grid_Data *priv;

   // Grid is an ugly mix of legacy & eo...
   base = evas_object_smart_data_get(o);
   priv = efl_data_scope_get(o, MY_CLASS);
   priv->base = *base;
   evas_object_smart_data_set(o, priv);

   priv->size.w = 100;
   priv->size.h = 100;
}

static void
_evas_object_grid_smart_del(Evas_Object *o)
{
   EVAS_OBJECT_GRID_DATA_GET(o, priv);
   Eina_List *l;

   l = priv->children;
   while (l)
     {
	Evas_Object_Grid_Option *opt = l->data;
	_evas_object_grid_child_disconnect(o, opt->obj);
	_evas_object_grid_option_del(opt->obj);
	free(opt);
	l = eina_list_remove_list(l, l);
     }
}

static void
_evas_object_grid_smart_resize(Evas_Object *o, Evas_Coord w, Evas_Coord h)
{
   Evas_Coord ow, oh;
   evas_object_geometry_get(o, NULL, NULL, &ow, &oh);
   if ((ow == w) && (oh == h)) return;
   evas_object_smart_changed(o);
}

static void
_evas_object_grid_smart_calculate(Evas_Object *o)
{
   Eina_List *l;
   Evas_Object_Grid_Option *opt;
   Evas *e;
   Evas_Coord x, y, w, h;
   long long xl, yl, wl, hl, vwl, vhl;
   Eina_Bool mirror;

   EVAS_OBJECT_GRID_DATA_GET_OR_RETURN(o, priv);
   if (!priv->children) return;

   e = evas_object_evas_get(o);
   evas_event_freeze(e);

   evas_object_geometry_get(o, &x, &y, &w, &h);
   xl = x;
   yl = y;
   wl = w;
   hl = h;
   mirror = priv->is_mirrored;
   vwl = priv->size.w;
   vhl = priv->size.h;
   EINA_LIST_FOREACH(priv->children, l, opt)
     {
        long long x1, y1, x2, y2;

        if (vwl > 0)
          {
             if (!mirror)
               {
                  x1 = xl + ((wl * (long long)opt->x) / vwl);
                  x2 = xl + ((wl * (long long)(opt->x + opt->w)) / vwl);
               }
             else
               {
                  x1 = xl + ((wl * (vwl - (long long)(opt->x + opt->w))) / vwl);
                  x2 = xl + ((wl * (vwl - (long long)opt->x)) / vwl);
               }
          }
        else
          {
             x1 = xl;
             x2 = xl;
          }
        if (vhl > 0)
          {
             y1 = yl + ((hl * (long long)opt->y) / vhl);
             y2 = yl + ((hl * (long long)(opt->y + opt->h)) / vhl);
          }
        else
          {
             y1 = yl;
             y2 = yl;
          }
        evas_object_move(opt->obj, x1, y1);
        evas_object_resize(opt->obj, x2 - x1, y2 - y1);
     }

   evas_event_thaw(e);
}

static void
_evas_object_grid_smart_set_user(Evas_Smart_Class *sc)
{
   sc->add = _evas_object_grid_smart_add;
   sc->del = _evas_object_grid_smart_del;
   sc->resize = _evas_object_grid_smart_resize;
   sc->calculate = _evas_object_grid_smart_calculate;
}

EAPI Evas_Object *
evas_object_grid_add(Evas *evas)
{
   evas = evas_find(evas);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(efl_isa(evas, EVAS_CANVAS_CLASS), NULL);
   return efl_add(MY_CLASS, evas, efl_canvas_object_legacy_ctor(efl_added));
}

EOLIAN static Eo *
_evas_grid_efl_object_constructor(Eo *obj, Evas_Grid_Data *class_data EINA_UNUSED)
{
   efl_canvas_group_clipped_set(obj, EINA_TRUE);
   obj = efl_constructor(efl_super(obj, MY_CLASS));
   evas_object_smart_attach(obj, _evas_object_grid_smart_class_new());

   return obj;
}

EOLIAN static Evas_Object*
_evas_grid_add_to(Eo *parent, Evas_Grid_Data *_pd EINA_UNUSED)
{
   Evas *evas;
   Evas_Object *ret;
   evas = evas_object_evas_get(parent);
   ret = evas_object_grid_add(evas);
   evas_object_smart_member_add(ret, parent);

   return ret;
}

EOLIAN static void
_evas_grid_grid_size_set(Eo *o, Evas_Grid_Data *priv, int w, int h)
{
   if ((priv->size.w == w) && (priv->size.h == h)) return;
   priv->size.w = w;
   priv->size.h = h;
   evas_object_smart_changed(o);
}

EOLIAN static void
_evas_grid_grid_size_get(const Eo *o EINA_UNUSED, Evas_Grid_Data *priv, int *w, int *h)
{
   if (w) *w = priv->size.w;
   if (h) *h = priv->size.h;
}

EOLIAN static Eina_Bool
_evas_grid_pack(Eo *o, Evas_Grid_Data *priv, Evas_Object *child, int x, int y, int w, int h)
{
   Evas_Object_Grid_Option *opt;
   Eina_Bool newobj = EINA_FALSE;

   opt = _evas_object_grid_option_get(child);
   if (!opt)
     {
        opt = malloc(sizeof(*opt));
        if (!opt)
          {
             ERR("could not allocate grid option data.");
             return EINA_FALSE;
          }
        newobj = EINA_TRUE;
     }

   opt->x = x;
   opt->y = y;
   opt->w = w;
   opt->h = h;

   if (newobj)
     {
        opt->obj = child;
        priv->children = eina_list_append(priv->children, opt);
        opt->l = eina_list_last(priv->children);
        _evas_object_grid_option_set(child, opt);
        evas_object_smart_member_add(child, o);
        _evas_object_grid_child_connect(o, child);
     }
   // FIXME: we could keep a changed list
   evas_object_smart_changed(o);
  
   return EINA_TRUE;
}

static void
_evas_object_grid_remove_opt(Evas_Grid_Data *priv, Evas_Object_Grid_Option *opt)
{
   priv->children = eina_list_remove_list(priv->children, opt->l);
   opt->l = NULL;
}

EOLIAN static Eina_Bool
_evas_grid_unpack(Eo *o, Evas_Grid_Data *priv, Evas_Object *child)
{
   Evas_Object_Grid_Option *opt;

   if (o != evas_object_smart_parent_get(child))
     {
	ERR("cannot unpack child from incorrect grid!");
        return EINA_FALSE;
     }

   opt = _evas_object_grid_option_del(child);
   if (!opt)
     {
	ERR("cannot unpack child with no packing option!");
        return EINA_FALSE;
     }

   _evas_object_grid_child_disconnect(o, child);
   _evas_object_grid_remove_opt(priv, opt);
   evas_object_smart_member_del(child);
   free(opt);
  
   return EINA_TRUE;
}

EOLIAN static void
_evas_grid_clear(Eo *o, Evas_Grid_Data *priv, Eina_Bool clear)
{
   Evas_Object_Grid_Option *opt;

   EINA_LIST_FREE(priv->children, opt)
     {
	_evas_object_grid_child_disconnect(o, opt->obj);
	_evas_object_grid_option_del(opt->obj);
	evas_object_smart_member_del(opt->obj);
	if (clear)
	  evas_object_del(opt->obj);
	free(opt);
     }
}

EOLIAN static Eina_Bool
_evas_grid_pack_get(const Eo *o EINA_UNUSED, Evas_Grid_Data *_pd EINA_UNUSED, Evas_Object *child, int *x, int *y, int *w, int *h)
{
   Evas_Object_Grid_Option *opt;

   if (x) *x = 0;
   if (y) *y = 0;
   if (w) *w = 0;
   if (h) *h = 0;
   opt = _evas_object_grid_option_get(child);
   if (!opt) return 0;
   if (x) *x = opt->x;
   if (y) *y = opt->y;
   if (w) *w = opt->w;
   if (h) *h = opt->h;

   return 1;
}

EOLIAN static Eina_Iterator*
_evas_grid_iterator_new(const Eo *o, Evas_Grid_Data *priv)
{
   Evas_Object_Grid_Iterator *it;

   if (!priv->children) return NULL;

   it = calloc(1, sizeof(Evas_Object_Grid_Iterator));
   if (!it) return NULL;

   EINA_MAGIC_SET(&it->iterator, EINA_MAGIC_ITERATOR);

   it->real_iterator = eina_list_iterator_new(priv->children);
   it->grid = o;

   it->iterator.next = FUNC_ITERATOR_NEXT(_evas_object_grid_iterator_next);
   it->iterator.get_container = FUNC_ITERATOR_GET_CONTAINER(_evas_object_grid_iterator_get_container);
   it->iterator.free = FUNC_ITERATOR_FREE(_evas_object_grid_iterator_free);

   return &it->iterator;
}

EOLIAN static Eina_Accessor*
_evas_grid_accessor_new(const Eo *o, Evas_Grid_Data *priv)
{
   Evas_Object_Grid_Accessor *it;

   if (!priv->children) return NULL;

   it = calloc(1, sizeof(Evas_Object_Grid_Accessor));
   if (!it) return NULL;

   EINA_MAGIC_SET(&it->accessor, EINA_MAGIC_ACCESSOR);

   it->real_accessor = eina_list_accessor_new(priv->children);
   it->grid = o;

   it->accessor.get_at = FUNC_ACCESSOR_GET_AT(_evas_object_grid_accessor_get_at);
   it->accessor.get_container = FUNC_ACCESSOR_GET_CONTAINER(_evas_object_grid_accessor_get_container);
   it->accessor.free = FUNC_ACCESSOR_FREE(_evas_object_grid_accessor_free);

   return &it->accessor;
}

EOLIAN static Eina_List*
_evas_grid_children_get(const Eo *o EINA_UNUSED, Evas_Grid_Data *priv)
{
   Eina_List *new_list = NULL, *l;
   Evas_Object_Grid_Option *opt;

   EINA_LIST_FOREACH(priv->children, l, opt)
      new_list = eina_list_append(new_list, opt->obj);

   return new_list;
}

EOLIAN static Eina_Bool
_evas_grid_efl_ui_base_mirrored_get(const Eo *o EINA_UNUSED, Evas_Grid_Data *priv)
{
   return priv->is_mirrored;
}

EOLIAN static void
_evas_grid_efl_ui_base_mirrored_set(Eo *o EINA_UNUSED, Evas_Grid_Data *priv, Eina_Bool mirrored)
{
   mirrored = !!mirrored;
   if (priv->is_mirrored != mirrored)
     {
        priv->is_mirrored = mirrored;
        _evas_object_grid_smart_calculate(o);
     }
}

EAPI void
evas_object_grid_mirrored_set(Evas_Grid *obj, Eina_Bool mirrored)
{
   efl_ui_mirrored_set(obj, mirrored);
}

EAPI Eina_Bool
evas_object_grid_mirrored_get(const Evas_Grid *obj)
{
   return efl_ui_mirrored_get(obj);
}

#include "canvas/evas_grid.eo.c"
