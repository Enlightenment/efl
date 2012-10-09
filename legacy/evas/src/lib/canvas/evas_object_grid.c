#include "evas_common.h"
#include "evas_private.h"
#include <errno.h>

#include <Eo.h>

EAPI Eo_Op EVAS_OBJ_GRID_BASE_ID = EO_NOOP;

#define MY_CLASS EVAS_OBJ_GRID_CLASS

typedef struct _Evas_Object_Grid_Data       Evas_Object_Grid_Data;
typedef struct _Evas_Object_Grid_Option     Evas_Object_Grid_Option;
typedef struct _Evas_Object_Grid_Iterator   Evas_Object_Grid_Iterator;
typedef struct _Evas_Object_Grid_Accessor   Evas_Object_Grid_Accessor;

struct _Evas_Object_Grid_Option
{
   Evas_Object *obj;
   Eina_List *l;
   int x, y, w, h;
};

struct _Evas_Object_Grid_Data
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
  Evas_Object_Grid_Data *ptr = eo_data_get(o, MY_CLASS)

#define EVAS_OBJECT_GRID_DATA_GET_OR_RETURN(o, ptr)			\
  EVAS_OBJECT_GRID_DATA_GET(o, ptr);					\
  if (!ptr)								\
    {									\
      CRIT("no widget data for object %p (%s)",				\
	   o, evas_object_type_get(o));					\
       abort();								\
       return;								\
    }

#define EVAS_OBJECT_GRID_DATA_GET_OR_RETURN_VAL(o, ptr, val)		\
  EVAS_OBJECT_GRID_DATA_GET(o, ptr);					\
  if (!ptr)								\
    {									\
       CRIT("No widget data for object %p (%s)",	                \
	       o, evas_object_type_get(o));				\
       abort();								\
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
   Evas_Object_Grid_Data *priv;

   priv = evas_object_smart_data_get(o);
   if (!priv)
     {
        priv = eo_data_get(o, MY_CLASS);
        evas_object_smart_data_set(o, priv);
     }

   priv->size.w = 100;
   priv->size.h = 100;
   
   _evas_object_grid_parent_sc->add(o);
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
   _evas_object_grid_parent_sc->del(o);
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
   Evas_Coord x, y, w, h, vw, vh, t;
   Eina_Bool mirror;
   
   EVAS_OBJECT_GRID_DATA_GET_OR_RETURN(o, priv);
   if (!priv) return;
   if (!priv->children) return;
   evas_object_geometry_get(o, &x, &y, &w, &h);
   mirror = priv->is_mirrored;
   vw = priv->size.w;
   vh = priv->size.h;
   EINA_LIST_FOREACH(priv->children, l, opt)
     {
        Evas_Coord x1, y1, x2, y2;
        
        x1 = x + ((w * opt->x) / vw);
        y1 = y + ((h * opt->y) / vh);
        x2 = x + ((w * (opt->x + opt->w)) / vw);
        y2 = y + ((h * (opt->y + opt->h)) / vh);
        if (mirror)
          {
             t = x1; x1 = x2; x2 = t;
             t = y1; y1 = y2; y2 = t;
          }
        evas_object_move(opt->obj, x1, y1);
        evas_object_resize(opt->obj, x2 - x1, y2 - y1);
     }
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
   Evas_Object *obj = eo_add(MY_CLASS, evas);
   eo_unref(obj);
   return obj;
}

static void
_constructor(Eo *obj, void *class_data EINA_UNUSED, va_list *list EINA_UNUSED)
{
   eo_do_super(obj, eo_constructor());
   eo_do(obj, evas_obj_smart_attach(_evas_object_grid_smart_class_new()));

//   return evas_object_smart_add(evas, _evas_object_grid_smart_class_new());
}

EAPI Evas_Object *
evas_object_grid_add_to(Evas_Object *parent)
{
   Evas_Object *o = NULL;
   eo_do(parent, evas_obj_grid_add_to(&o));
   return o;
}

static void
_add_to(Eo *parent, void *_pd EINA_UNUSED, va_list *list)
{
   Evas *evas;
   Evas_Object **ret = va_arg(*list, Evas_Object **);
   evas = evas_object_evas_get(parent);
   *ret = evas_object_grid_add(evas);
   evas_object_smart_member_add(*ret, parent);
}

EAPI void
evas_object_grid_size_set(Evas_Object *o, int w, int h)
{
   eo_do(o, evas_obj_grid_size_set(w, h));
}

static void
_size_set(Eo *o, void *_pd, va_list *list)
{
   Evas_Object_Grid_Data *priv = _pd;
   int w = va_arg(*list, int);
   int h = va_arg(*list, int);

   if ((priv->size.w == w) && (priv->size.h == h)) return;
   priv->size.w = w;
   priv->size.h = h;
   evas_object_smart_changed(o);
}

EAPI void
evas_object_grid_size_get(const Evas_Object *o, int *w, int *h)
{
   eo_do((Eo *)o, evas_obj_grid_size_get(w, h));
}

static void
_size_get(Eo *o EINA_UNUSED, void *_pd, va_list *list)
{
   const Evas_Object_Grid_Data *priv = _pd;
   int *w = va_arg(*list, int *);
   int *h = va_arg(*list, int *);
   if (w) *w = priv->size.w;
   if (h) *h = priv->size.h;
}

EAPI Eina_Bool
evas_object_grid_pack(Evas_Object *o, Evas_Object *child, int x, int y, int w, int h)
{
   Eina_Bool ret = EINA_FALSE;
   eo_do(o, evas_obj_grid_pack(child, x, y, w, h, &ret));
   return ret;
}

static void
_pack(Eo *o, void *_pd, va_list *list)
{
   Evas_Object_Grid_Option *opt;
   Eina_Bool newobj = EINA_FALSE;

   Evas_Object_Grid_Data *priv = _pd;

   Evas_Object *child = va_arg(*list, Evas_Object *);
   int x = va_arg(*list, int);
   int y = va_arg(*list, int);
   int w = va_arg(*list, int);
   int h = va_arg(*list, int);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_FALSE;

   opt = _evas_object_grid_option_get(child);
   if (!opt)
     {
        opt = malloc(sizeof(*opt));
        if (!opt)
          {
             ERR("could not allocate grid option data.");
             return;
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
   if (ret) *ret = EINA_TRUE;
}

static void
_evas_object_grid_remove_opt(Evas_Object_Grid_Data *priv, Evas_Object_Grid_Option *opt)
{
   priv->children = eina_list_remove_list(priv->children, opt->l);
   opt->l = NULL;
}

EAPI Eina_Bool
evas_object_grid_unpack(Evas_Object *o, Evas_Object *child)
{
   Eina_Bool ret = EINA_FALSE;
   eo_do(o, evas_obj_grid_unpack(child, &ret));
   return ret;
}

static void
_unpack(Eo *o, void *_pd, va_list *list)
{
   Evas_Object_Grid_Option *opt;

   Evas_Object_Grid_Data *priv = _pd;

   Evas_Object *child = va_arg(*list, Evas_Object *);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   if (ret) *ret = EINA_FALSE;

   if (o != evas_object_smart_parent_get(child))
     {
	ERR("cannot unpack child from incorrect grid!");
        return;
     }

   opt = _evas_object_grid_option_del(child);
   if (!opt)
     {
	ERR("cannot unpack child with no packing option!");
        return;
     }

   _evas_object_grid_child_disconnect(o, child);
   _evas_object_grid_remove_opt(priv, opt);
   evas_object_smart_member_del(child);
   free(opt);
   if (ret) *ret = EINA_TRUE;
}

EAPI void
evas_object_grid_clear(Evas_Object *o, Eina_Bool clear)
{
   eo_do(o, evas_obj_grid_clear(clear));
}

static void
_clear(Eo *o, void *_pd, va_list *list)
{
   Evas_Object_Grid_Option *opt;

   Evas_Object_Grid_Data *priv = _pd;
   Eina_Bool clear = va_arg(*list, int);

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

EAPI Eina_Bool
evas_object_grid_pack_get(const Evas_Object *o, Evas_Object *child, int *x, int *y, int *w, int *h)
{
   Eina_Bool ret = EINA_FALSE;
   eo_do((Eo *)o, evas_obj_grid_pack_get(child, x, y, w, h, &ret));
   return ret;
}

static void
_pack_get(Eo *o EINA_UNUSED, void *_pd EINA_UNUSED, va_list *list)
{
   Evas_Object *child = va_arg(*list, Evas_Object *);
   int *x = va_arg(*list, int *);
   int *y = va_arg(*list, int *);
   int *w = va_arg(*list, int *);
   int *h = va_arg(*list, int *);
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);

   Evas_Object_Grid_Option *opt;

   if (x) *x = 0;
   if (y) *y = 0;
   if (w) *w = 0;
   if (h) *h = 0;
   opt = _evas_object_grid_option_get(child);
   if (!opt)
     {
        *ret = 0;
        return;
     }
   if (x) *x = opt->x;
   if (y) *y = opt->y;
   if (w) *w = opt->w;
   if (h) *h = opt->h;
   *ret = 1;
}

EAPI Eina_Iterator *
evas_object_grid_iterator_new(const Evas_Object *o)
{
   Eina_Iterator *ret = NULL;
   eo_do((Eo *)o, evas_obj_grid_iterator_new(&ret));
   return ret;
}

static void
_iterator_new(Eo *o, void *_pd, va_list *list)
{
   Eina_Iterator **ret = va_arg(*list, Eina_Iterator **);
   Evas_Object_Grid_Iterator *it;

   const Evas_Object_Grid_Data *priv = _pd;

   if (!priv->children)
     {
        *ret = NULL;
        return;
     }

   it = calloc(1, sizeof(Evas_Object_Grid_Iterator));
   if (!it)
     {
        *ret = NULL;
        return;
     }

   EINA_MAGIC_SET(&it->iterator, EINA_MAGIC_ITERATOR);

   it->real_iterator = eina_list_iterator_new(priv->children);
   it->grid = o;

   it->iterator.next = FUNC_ITERATOR_NEXT(_evas_object_grid_iterator_next);
   it->iterator.get_container = FUNC_ITERATOR_GET_CONTAINER(_evas_object_grid_iterator_get_container);
   it->iterator.free = FUNC_ITERATOR_FREE(_evas_object_grid_iterator_free);

   *ret = &it->iterator;
}

EAPI Eina_Accessor *
evas_object_grid_accessor_new(const Evas_Object *o)
{
   Eina_Accessor *ret = NULL;
   eo_do((Eo *)o, evas_obj_grid_accessor_new(&ret));
   return ret;
}

static void
_accessor_new(Eo *o, void *_pd, va_list *list)
{
   Eina_Accessor **ret = va_arg(*list, Eina_Accessor **);
   Evas_Object_Grid_Accessor *it;

   const Evas_Object_Grid_Data *priv = _pd;

   if (!priv->children)
     {
        *ret = NULL;
        return;
     }

   it = calloc(1, sizeof(Evas_Object_Grid_Accessor));
   if (!it)
     {
        *ret = NULL;
        return;
     }

   EINA_MAGIC_SET(&it->accessor, EINA_MAGIC_ACCESSOR);

   it->real_accessor = eina_list_accessor_new(priv->children);
   it->grid = o;

   it->accessor.get_at = FUNC_ACCESSOR_GET_AT(_evas_object_grid_accessor_get_at);
   it->accessor.get_container = FUNC_ACCESSOR_GET_CONTAINER(_evas_object_grid_accessor_get_container);
   it->accessor.free = FUNC_ACCESSOR_FREE(_evas_object_grid_accessor_free);

   *ret = &it->accessor;
}

EAPI Eina_List *
evas_object_grid_children_get(const Evas_Object *o)
{
   Eina_List *ret = NULL;
   eo_do((Eo *)o, evas_obj_grid_children_get(&ret));
   return ret;
}

static void
_children_get(Eo *o EINA_UNUSED, void *_pd, va_list *list)
{
   Eina_List **ret = va_arg(*list, Eina_List **);

   Eina_List *new_list = NULL, *l;
   Evas_Object_Grid_Option *opt;

   const Evas_Object_Grid_Data *priv = _pd;

   EINA_LIST_FOREACH(priv->children, l, opt)
      new_list = eina_list_append(new_list, opt->obj);

   *ret = new_list;
}

EAPI Eina_Bool
evas_object_grid_mirrored_get(const Evas_Object *o)
{
   Eina_Bool ret = EINA_FALSE;
   eo_do((Eo *)o, evas_obj_grid_mirrored_get(&ret));
   return ret;
}

static void
_mirrored_get(Eo *o EINA_UNUSED, void *_pd, va_list *list)
{
   Eina_Bool *ret = va_arg(*list, Eina_Bool *);
   const Evas_Object_Grid_Data *priv = _pd;
   *ret = priv->is_mirrored;
}

EAPI void
evas_object_grid_mirrored_set(Evas_Object *obj, Eina_Bool mirrored)
{
   eo_do(obj, evas_obj_grid_mirrored_set(mirrored));
}

static void
_mirrored_set(Eo *o EINA_UNUSED, void *_pd, va_list *list)
{
   Eina_Bool mirrored = va_arg(*list, int);

   Evas_Object_Grid_Data *priv = _pd;
   mirrored = !!mirrored;
   if (priv->is_mirrored != mirrored)
     {
        priv->is_mirrored = mirrored;
        _evas_object_grid_smart_calculate(o);
     }
}

static void
_class_constructor(Eo_Class *klass)
{
   const Eo_Op_Func_Description func_desc[] = {
        EO_OP_FUNC(EO_BASE_ID(EO_BASE_SUB_ID_CONSTRUCTOR), _constructor),
        EO_OP_FUNC(EVAS_OBJ_GRID_ID(EVAS_OBJ_GRID_SUB_ID_ADD_TO), _add_to),
        EO_OP_FUNC(EVAS_OBJ_GRID_ID(EVAS_OBJ_GRID_SUB_ID_SIZE_SET), _size_set),
        EO_OP_FUNC(EVAS_OBJ_GRID_ID(EVAS_OBJ_GRID_SUB_ID_SIZE_GET), _size_get),
        EO_OP_FUNC(EVAS_OBJ_GRID_ID(EVAS_OBJ_GRID_SUB_ID_PACK), _pack),
        EO_OP_FUNC(EVAS_OBJ_GRID_ID(EVAS_OBJ_GRID_SUB_ID_UNPACK), _unpack),
        EO_OP_FUNC(EVAS_OBJ_GRID_ID(EVAS_OBJ_GRID_SUB_ID_CLEAR), _clear),
        EO_OP_FUNC(EVAS_OBJ_GRID_ID(EVAS_OBJ_GRID_SUB_ID_PACK_GET), _pack_get),
        EO_OP_FUNC(EVAS_OBJ_GRID_ID(EVAS_OBJ_GRID_SUB_ID_ITERATOR_NEW), _iterator_new),
        EO_OP_FUNC(EVAS_OBJ_GRID_ID(EVAS_OBJ_GRID_SUB_ID_ACCESSOR_NEW), _accessor_new),
        EO_OP_FUNC(EVAS_OBJ_GRID_ID(EVAS_OBJ_GRID_SUB_ID_CHILDREN_GET), _children_get),
        EO_OP_FUNC(EVAS_OBJ_GRID_ID(EVAS_OBJ_GRID_SUB_ID_MIRRORED_GET), _mirrored_get),
        EO_OP_FUNC(EVAS_OBJ_GRID_ID(EVAS_OBJ_GRID_SUB_ID_MIRRORED_SET), _mirrored_set),
        EO_OP_FUNC_SENTINEL
   };
   eo_class_funcs_set(klass, func_desc);
}

static const Eo_Op_Description op_desc[] = {
     EO_OP_DESCRIPTION(EVAS_OBJ_GRID_SUB_ID_ADD_TO, "Create a grid that is child of a given element parent."),
     EO_OP_DESCRIPTION(EVAS_OBJ_GRID_SUB_ID_SIZE_SET, "Set the virtual resolution for the grid."),
     EO_OP_DESCRIPTION(EVAS_OBJ_GRID_SUB_ID_SIZE_GET, "Get the current virtual resolution."),
     EO_OP_DESCRIPTION(EVAS_OBJ_GRID_SUB_ID_PACK, "Add a new child to a grid object."),
     EO_OP_DESCRIPTION(EVAS_OBJ_GRID_SUB_ID_UNPACK, "Remove child from grid."),
     EO_OP_DESCRIPTION(EVAS_OBJ_GRID_SUB_ID_CLEAR, "Faster way to remove all child objects from a grid object."),
     EO_OP_DESCRIPTION(EVAS_OBJ_GRID_SUB_ID_PACK_GET, "Get the pack options for a grid child."),
     EO_OP_DESCRIPTION(EVAS_OBJ_GRID_SUB_ID_ITERATOR_NEW, "Get an iterator to walk the list of children for the grid."),
     EO_OP_DESCRIPTION(EVAS_OBJ_GRID_SUB_ID_ACCESSOR_NEW, "Get an accessor to get random access to the list of children for the grid."),
     EO_OP_DESCRIPTION(EVAS_OBJ_GRID_SUB_ID_CHILDREN_GET, "Get the list of children for the grid."),
     EO_OP_DESCRIPTION(EVAS_OBJ_GRID_SUB_ID_MIRRORED_GET, "Gets the mirrored mode of the grid."),
     EO_OP_DESCRIPTION(EVAS_OBJ_GRID_SUB_ID_MIRRORED_SET, "Sets the mirrored mode of the grid."),
     EO_OP_DESCRIPTION_SENTINEL
};

static const Eo_Class_Description class_desc = {
     EO_VERSION,
     "Evas_Object_Grid",
     EO_CLASS_TYPE_REGULAR,
     EO_CLASS_DESCRIPTION_OPS(&EVAS_OBJ_GRID_BASE_ID, op_desc, EVAS_OBJ_GRID_SUB_ID_LAST),
     NULL,
     sizeof(Evas_Object_Grid_Data),
     _class_constructor,
     NULL
};

EO_DEFINE_CLASS(evas_object_grid_class_get, &class_desc, EVAS_OBJ_SMART_CLIPPED_CLASS, NULL);

