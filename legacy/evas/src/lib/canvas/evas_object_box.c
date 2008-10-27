#include "evas_common.h"

/**
 * @addtogroup Evas_Object_Box
 * @{
 * @ingroup Evas_Smart_Object_Group
 */

#define EVAS_OBJECT_BOX_DATA_GET(o, ptr)			\
  Evas_Object_Box_Data *ptr = evas_object_smart_data_get(o)

#define EVAS_OBJECT_BOX_DATA_GET_OR_RETURN(o, ptr)			\
  EVAS_OBJECT_BOX_DATA_GET(o, ptr);					\
  if (!ptr)								\
    {									\
       fprintf(stderr, "CRITICAL: no widget data for object %p (%s)\n",	\
	       o, evas_object_type_get(o));				\
       fflush(stderr);							\
       abort();								\
       return;								\
}

#define EVAS_OBJECT_BOX_DATA_GET_OR_RETURN_VAL(o, ptr, val)		\
  EVAS_OBJECT_BOX_DATA_GET(o, ptr);					\
  if (!ptr)								\
    {									\
       fprintf(stderr, "CRITICAL: no widget data for object %p (%s)\n",	\
	       o, evas_object_type_get(o));				\
       fflush(stderr);							\
       abort();								\
       return val;							\
    }

static Evas_Smart_Class _parent_sc = {NULL};

static void
_on_child_resize(void *data, Evas *evas, Evas_Object *o, void *einfo)
{
   Evas_Object *box = data;
   evas_object_smart_changed(box);
}

static void
_on_child_del(void *data, Evas *evas, Evas_Object *o, void *einfo)
{
   const Evas_Object_Box_Api *api;
   Evas_Object *box = data;

   EVAS_OBJECT_BOX_DATA_GET(box, priv);
   api = priv->api;

   if ((!api) || (!api->remove))
     {
	fputs("no api->remove\n", stderr);
	return;
     }

   if (!api->remove(box, priv, o))
     fputs("child removal failed\n", stderr);
}

static void
_on_child_hints_changed(void *data, Evas *evas, Evas_Object *o, void *einfo)
{
   Evas_Object *box = data;
   evas_object_smart_changed(box);
}

static Evas_Object_Box_Option *
_evas_object_box_option_new(Evas_Object *o, Evas_Object_Box_Data *priv, Evas_Object *child)
{
   Evas_Object_Box_Option *opt;
   const Evas_Object_Box_Api *api;

   api = priv->api;
   if ((!api) || (!api->option_new))
     {
	fputs("no api->option_new\n", stderr);
	return NULL;
     }

   opt = api->option_new(o, priv, child);
   if (!opt)
     {
	fputs("option_new failed\n", stderr);
	return NULL;
     }

   return opt;
}

static void
_evas_object_box_child_callbacks_unregister(Evas_Object *obj)
{
   evas_object_event_callback_del
     (obj, EVAS_CALLBACK_RESIZE, _on_child_resize);
   evas_object_event_callback_del
     (obj, EVAS_CALLBACK_FREE, _on_child_del);
   evas_object_event_callback_del
     (obj, EVAS_CALLBACK_CHANGED_SIZE_HINTS, _on_child_hints_changed);
}

static Evas_Object_Box_Option *
_evas_object_box_option_callbacks_register(Evas_Object *o, Evas_Object_Box_Data *priv, Evas_Object_Box_Option *opt)
{
   const Evas_Object_Box_Api *api;
   Evas_Object *obj = opt->obj;

   api = priv->api;

   if ((!api) || (!api->option_free))
     {
	fputs("WARNING: api->option_free not set (may cause memory leaks,"
	      " segfaults)\n", stderr);
        return NULL;
     }

   evas_object_event_callback_add
     (obj, EVAS_CALLBACK_RESIZE, _on_child_resize, o);
   evas_object_event_callback_add
     (obj, EVAS_CALLBACK_FREE, _on_child_del, o);
   evas_object_event_callback_add
     (obj, EVAS_CALLBACK_CHANGED_SIZE_HINTS, _on_child_hints_changed, o);

   return opt;
}

static Evas_Object_Box_Option *
_evas_object_box_option_new_default(Evas_Object *o, Evas_Object_Box_Data *priv, Evas_Object *child)
{
   Evas_Object_Box_Option *opt;

   opt = malloc(sizeof(*opt));
   if (!opt)
     return NULL;

   opt->obj = child;

   return opt;
}

static void
_evas_object_box_option_free_default(Evas_Object *o, Evas_Object_Box_Data *priv, Evas_Object_Box_Option *opt)
{
   free(opt);
}

static Evas_Object_Box_Option *
_evas_object_box_append_default(Evas_Object *o, Evas_Object_Box_Data *priv, Evas_Object *child)
{
   Evas_Object_Box_Option *opt;

   opt = _evas_object_box_option_new(o, priv, child);
   if (!opt)
     return NULL;

   priv->children = eina_list_append(priv->children, opt);

   return opt;
}

static Evas_Object_Box_Option *
_evas_object_box_prepend_default(Evas_Object *o, Evas_Object_Box_Data *priv, Evas_Object *child)
{
   Evas_Object_Box_Option *opt;

   opt = _evas_object_box_option_new(o, priv, child);
   if (!opt)
     return NULL;

   priv->children = eina_list_prepend(priv->children, opt);

   return opt;
}

static Evas_Object_Box_Option *
_evas_object_box_insert_before_default(Evas_Object *o, Evas_Object_Box_Data *priv, Evas_Object *child, const Evas_Object *reference)
{
   Eina_List *l;
   Evas_Object_Box_Option *opt;

   EINA_LIST_FOREACH(priv->children, l, opt)
     {
        if (opt->obj == reference)
	  {
	     Evas_Object_Box_Option *new_opt;

	     new_opt = _evas_object_box_option_new(o, priv, child);
	     if (!new_opt)
	       return NULL;

	     priv->children = eina_list_prepend_relative
	       (priv->children, new_opt, opt);
	     return new_opt;
	  }
     }

   return NULL;
}

static Evas_Object_Box_Option *
_evas_object_box_insert_at_default(Evas_Object *o, Evas_Object_Box_Data *priv, Evas_Object *child, unsigned int pos)
{
   Eina_List *l;
   int i;

   if ((pos == 0) && (eina_list_count(priv->children) == 0))
     {
        Evas_Object_Box_Option *new_opt;

        new_opt = _evas_object_box_option_new(o, priv, child);
        if (!new_opt)
	  return NULL;

        priv->children = eina_list_prepend(priv->children, new_opt);
        return new_opt;
     }

   for (l = priv->children, i = 0; l != NULL; l = l->next, i++)
     {
	Evas_Object_Box_Option *opt = l->data;

        if (i == pos)
	  {
	     Evas_Object_Box_Option *new_opt;

	     new_opt = _evas_object_box_option_new(o, priv, child);
	     if (!new_opt)
	       return NULL;

	     priv->children = eina_list_prepend_relative
	       (priv->children, new_opt, opt);
	     return new_opt;
	  }
     }

   return NULL;
}

static Evas_Object *
_evas_object_box_remove_default(Evas_Object *o, Evas_Object_Box_Data *priv, Evas_Object *child)
{
   const Evas_Object_Box_Api *api;
   Evas_Object_Box_Option *opt;
   Eina_List *l;

   api = priv->api;

   if ((!api) || (!api->option_free))
     {
	fputs("WARNING: api->option_free not set (may cause memory leaks,"
	      " segfaults)\n", stderr);
	return NULL;
     }

   EINA_LIST_FOREACH(priv->children, l, opt)
     {
        Evas_Object *obj = opt->obj;

        if (obj == child)
	  {
	     priv->children = eina_list_remove(priv->children, opt);
	     api->option_free(o, priv, opt);

	     return obj;
	  }
     }

   return NULL;
}

static Evas_Object *
_evas_object_box_remove_at_default(Evas_Object *o, Evas_Object_Box_Data *priv, unsigned int pos)
{
   const Evas_Object_Box_Api *api;
   Eina_List *l;
   int i;

   api = priv->api;

   if ((!api) || (!api->option_free))
     {
	fputs("WARNING: api->option_free not set (may cause memory leaks,"
	      " segfaults)\n", stderr);
        return NULL;
     }

   for (l = priv->children, i = 0; l != NULL; l = l->next, i++)
     {
	Evas_Object_Box_Option *opt;
        Evas_Object *obj = opt->obj;

        if (i == pos)
	  {
	     priv->children = eina_list_remove(priv->children, opt);
	     api->option_free(o, priv, opt);

	     return obj;
	  }
     }

   return NULL;
}

static void
_evas_object_box_smart_add(Evas_Object *o)
{
   Evas_Object_Box_Data *priv;

   priv = evas_object_smart_data_get(o);
   if (!priv)
     {
        const Evas_Smart *smart;
        const Evas_Smart_Class *sc;

	priv = calloc(1, sizeof(*priv));
	if (!priv)
	  {
	     fputs("ERROR: could not allocate object private data.\n", stderr);
	     return;
	  }

        smart = evas_object_smart_smart_get(o);
        sc = evas_smart_class_get(smart);
        priv->api = (const Evas_Object_Box_Api *)sc;

	evas_object_smart_data_set(o, priv);
     }
   _parent_sc.add(o);

   priv->children = NULL;
   priv->align_h = 0.5;
   priv->align_v = 0.5;
   priv->padding_h = 0;
   priv->padding_v = 0;
   priv->layout = evas_object_box_layout_horizontal;
}

static void
_evas_object_box_smart_del(Evas_Object *o)
{
   const Evas_Object_Box_Api *api;
   Eina_List *l;

   EVAS_OBJECT_BOX_DATA_GET(o, priv);

   api = priv->api;
   if ((!api) || (!api->option_free))
     {
	fputs("WARNING: api->option_free not set (may cause memory leaks,"
	      " segfaults)\n", stderr);
        return;
     }

   l = priv->children;
   while (l)
     {
	Evas_Object_Box_Option *opt = l->data;

	_evas_object_box_child_callbacks_unregister(opt->obj);
	api->option_free(o, priv, opt);
	l = eina_list_remove_list(l, l);
     }

   _parent_sc.del(o);
}

static void
_evas_object_box_smart_resize(Evas_Object *o, int w, int h)
{
   evas_object_smart_changed(o);
}

static void
_evas_object_box_smart_calculate(Evas_Object *o)
{
   EVAS_OBJECT_BOX_DATA_GET_OR_RETURN(o, priv);
   if (priv->layout)
     priv->layout(o, priv);
   else
     fprintf(stderr, "ERROR: no layout function set for %p box.\n", o);
}

static Evas_Smart *
_evas_object_box_smart_class_new(void)
{
   static Evas_Object_Box_Api api = {
     {"Evas_Object_Box", EVAS_SMART_CLASS_VERSION},
   };

   if (!_parent_sc.name)
     evas_object_box_smart_set(&api);

   return evas_smart_class_new(&api.base);
}

/**
 * Create a new box.
 *
 * Its layout function must be set via evas_object_box_layout_set()
 * (defaults to evas_object_box_layout_horizontal()).  The other
 * properties of the box must be set/retrieved via
 * evas_object_box_{h,v}_{align,padding}_{get,set)().
 */
Evas_Object *
evas_object_box_add(Evas *evas)
{
   Evas_Object *o;
   Evas_Smart *smart;

   if (!smart)
     smart = _evas_object_box_smart_class_new();

   o = evas_object_smart_add(evas, smart);
   return o;
}

/**
 * Create a box that is child of a given element @a parent.
 *
 * @see evas_object_box_add()
 */
Evas_Object *
evas_object_box_add_to(Evas_Object *parent)
{
   Evas *evas;
   Evas_Object *o;

   evas = evas_object_evas_get(parent);
   o = evas_object_box_add(evas);
   evas_object_smart_member_add(o, parent);
   return o;
}

/**
 * Set the default box @a api struct (Evas_Object_Box_Api)
 * with the default values. May be used to extend that API.
 */
void
evas_object_box_smart_set(Evas_Object_Box_Api *api)
{
   if (!api)
     return;

   if (!_parent_sc.name)
     evas_object_smart_clipped_smart_set(&_parent_sc);

   api->base.add = _evas_object_box_smart_add;
   api->base.del = _evas_object_box_smart_del;
   api->base.move = _parent_sc.move;
   api->base.resize = _evas_object_box_smart_resize;
   api->base.show = _parent_sc.show;
   api->base.hide = _parent_sc.hide;
   api->base.color_set = _parent_sc.color_set;
   api->base.clip_set = _parent_sc.clip_set;
   api->base.clip_unset = _parent_sc.clip_unset;
   api->base.calculate = _evas_object_box_smart_calculate;
   api->base.member_add = _parent_sc.member_add;
   api->base.member_del = _parent_sc.member_del;

   api->append = _evas_object_box_append_default;
   api->prepend = _evas_object_box_prepend_default;
   api->insert_before = _evas_object_box_insert_before_default;
   api->insert_at = _evas_object_box_insert_at_default;
   api->remove = _evas_object_box_remove_default;
   api->remove_at = _evas_object_box_remove_at_default;
   api->property_set = NULL;
   api->property_get = NULL;
   api->property_name_get = NULL;
   api->property_id_get = NULL;
   api->option_new = _evas_object_box_option_new_default;
   api->option_free = _evas_object_box_option_free_default;
}

/**
 * Set a 'calculate' callback (@a cb) to the @a o box's smart class,
 * which here defines its genre (horizontal, vertical, homogeneous,
 * etc.).
 */
void
evas_object_box_layout_set(Evas_Object *o, Evas_Object_Box_Layout cb)
{
   EVAS_OBJECT_BOX_DATA_GET_OR_RETURN(o, priv);
   priv->layout = cb;
   evas_object_smart_changed(o);
}

static void
_fixed_point_divide_and_decompose_integer(int dividend, int divisor, int *int_part, int *frac_part)
{
   int quotient = (long long)(dividend << 16) / divisor;
   *frac_part = quotient & 0xffff;
   *int_part = quotient >> 16;
}

static void
_layout_dimension_change_min_max_cell_bound(int dim, int *new_dim, int min_d, int max_d, int cell_sz)
{
   if (dim > cell_sz)
     {
	if ((min_d != 0) && (cell_sz < min_d))
	  *new_dim = min_d;
	else
	  *new_dim = cell_sz;
     }
   else
     {
	if ((max_d != -1) && (cell_sz > max_d))
	  *new_dim = max_d;
	else
	  *new_dim = cell_sz;
     }
}

static void
_layout_set_offset_and_expand_dimension_space_max_bounded(int dim, int *new_dim, int space_sz, int max_dim, int *offset, double align, int pad_before, int pad_after)
{
   if (align >= 0.0)
     {
	*offset = (space_sz - (dim + pad_before + pad_after)) * align
	  + pad_before;
     }
   else
     {
	if ((max_dim != -1) && (space_sz > max_dim))
	  {
	     *new_dim = max_dim;
	     *offset = (space_sz - (max_dim + pad_before + pad_after)) * 0.5
	       + pad_before;
	  }
	else
	  {
	     *new_dim = space_sz;
	     *offset = 0;
	  }
     }
}

static void
_layout_set_offset_and_change_dimension_min_max_cell_bounded(int dim, int *new_dim, int min_dim, int max_dim, int cell_sz, int *offset, double align, int pad_before, int pad_after)
{
   if (align >= 0.0)
     *offset = (cell_sz - (dim + pad_before + pad_after)) * align
       + pad_before;
   else
     {
        *offset = pad_before;
        _layout_dimension_change_min_max_cell_bound(
						    dim, new_dim, min_dim, max_dim, cell_sz - pad_before - pad_after);
     }
}

static int
_evas_object_box_layout_horizontal_weight_apply(Evas_Object_Box_Data *priv, Evas_Object_Box_Option **objects, int n_objects, int remaining, int weight_total)
{
   int rem_diff = 0;
   int i;

   for (i = 0; i < n_objects; i++)
     {
        Evas_Object_Box_Option *opt = objects[i];
        Evas_Object *o = opt->obj;
        int h;

        evas_object_geometry_get(o, NULL, NULL, NULL, &h);

        if (remaining < 0)
	  {
	     int min_w;

	     evas_object_size_hint_min_get(o, &min_w, NULL);
	     evas_object_resize(o, min_w, h);
	  }
	else
	  {
	     double normal_weight, weight_x;
	     int target_size;
	     int max_w;

	     evas_object_size_hint_weight_get(o, &weight_x, NULL);
	     normal_weight = weight_x / weight_total;
	     target_size = (int)((double)remaining * normal_weight);

	     evas_object_size_hint_max_get(o, &max_w, NULL);
	     if ((max_w != -1) && (target_size > max_w))
	       {
		  evas_object_resize(o, max_w, h);
		  rem_diff += max_w;
		  objects[i] = objects[n_objects - 1];
		  weight_total -= weight_x;
		  n_objects--;
		  return _evas_object_box_layout_horizontal_weight_apply
		    (priv, objects, n_objects, remaining - rem_diff,
		     weight_total);
	       }
	     else
	       {
		  evas_object_resize(o, target_size, h);
		  rem_diff += target_size;
	       }
	  }
     }

   return remaining - rem_diff;
}

/**
 * Layout function which sets the box @a o to a (basic) horizontal
 * box.  @a priv must be the smart data of the box.
 *
 * The object's overall behavior is controlled by its properties,
 * which are set by the evas_object_box_{h,v}_{align,padding}_set()
 * family of functions.  The properties of the elements in the box --
 * set by evas_object_size_hint_{align,padding,weight}_set() functions
 * -- also control the way this function works.
 *
 * \par box's properties:
 * @c align_h controls the horizontal alignment of the child objects
 * relative to the containing box. When set to 0, children are aligned
 * to the left. A value of 1 lets them aligned to the right border.
 * Values in between align them proportionally.  Note that if the size
 * required by the children, which is given by their widths and the @c
 * padding_h property of the box, is bigger than the container width,
 * the children will be displayed out of its bounds.  A negative value
 * of @c align_h makes the box to *justify* its children. The padding
 * between them, in this case, is corrected so that the leftmost one
 * touches the left border and the rightmost one touches the right
 * border (even if they must overlap).  The @c align_v and @c
 * padding_v properties of the box don't contribute to its behaviour
 * when this layout is chosen.
 *
 * \par Child element's properties:
 * @c align_x does not influence the box's behavior.  @c padding_l and
 * @c padding_r sum up to the container's horizontal padding between
 * elements.  The child's @c padding_t, @c padding_b and @c align_y
 * properties apply for padding/positioning relative to the overall
 * height of the box. Finally, there is the @c weight_x property,
 * which, if set to a non-zero value, tells the container that the
 * child width is not pre-defined.  If the container can't accomodate
 * all its children, it sets the widths of the children *with weights*
 * to sizes as small as they can all fit into it.  If the size
 * required by the children is less than the available, the box
 * increases its children's (which have weights) widths as to fit the
 * remaining space.  The @c weight_x property, besides telling the
 * element is resizable, gives a *weight* for the resizing process.
 * The parent box will try to distribute (or take off) widths
 * accordingly to the *normalized* list of weigths: most weighted
 * children remain/get larger in this process than the the least ones.
 * @c weight_y does not influence the layout.
 *
 * If one desires that, besides having weights, child elements must be
 * resized bounded to a minimum or maximum size, their size hint
 * properties must be set (by the
 * evas_object_size_hint_{min,max}_set() functions.
 */
void
evas_object_box_layout_horizontal(Evas_Object *o, Evas_Object_Box_Data *priv)
{
   int pad_inc = 0, sub_pixel = 0;
   int req_w, global_pad, remaining;
   double weight_total = 0.0;
   int weight_use = 0;
   int x, y, w, h;
   int n_children;
   Evas_Object_Box_Option *opt;
   Eina_List *l;

   n_children = eina_list_count(priv->children);
   if (!n_children)
     return;

   Evas_Object_Box_Option *objects[n_children];

   evas_object_geometry_get(o, &x, &y, &w, &h);
   global_pad = priv->padding_h;
   req_w = global_pad * (n_children - 1);

   EINA_LIST_FOREACH(priv->children, l, opt)
     {
        int padding_l, padding_r;
        double weight_x;

        evas_object_size_hint_weight_get(opt->obj, &weight_x, NULL);
        evas_object_size_hint_padding_get
	  (opt->obj, &padding_l, &padding_r, NULL, NULL);
        req_w += padding_l + padding_r;

        if (!weight_x)
	  {
	     int child_w;

	     evas_object_geometry_get(opt->obj, NULL, NULL, &child_w, NULL);
	     req_w += child_w;
	  }
	else
	  {
	     objects[weight_use] = opt;
	     weight_use++;
	     weight_total += weight_x;
	  }
     }

   remaining = w - req_w;

   if (weight_use)
     remaining = _evas_object_box_layout_horizontal_weight_apply
       (priv, objects, weight_use, remaining, weight_total);

   if (priv->align_h >= 0.0)
     x += remaining * priv->align_h;
   else if (n_children == 1)
     x += remaining / 2;
   else
     { /* justified */
        _fixed_point_divide_and_decompose_integer
	  (remaining, n_children - 1, &global_pad, &pad_inc);
        global_pad += priv->padding_h;
     }

   EINA_LIST_FOREACH(priv->children, l, opt)
     {
        int child_w, child_h, max_h, new_h, off_x, off_y;
        int padding_l, padding_r, padding_t, padding_b;
        double align_y;

        evas_object_size_hint_align_get(opt->obj, NULL, &align_y);
        evas_object_size_hint_padding_get
	  (opt->obj, &padding_l, &padding_r, &padding_t, &padding_b);
        evas_object_size_hint_max_get(opt->obj, NULL, &max_h);

        evas_object_geometry_get(opt->obj, NULL, NULL, &child_w, &child_h);

        off_x = padding_l;
        new_h = child_h;

        _layout_set_offset_and_expand_dimension_space_max_bounded
	  (child_h, &new_h, h, max_h, &off_y, align_y, padding_t, padding_b);

        evas_object_resize(opt->obj, child_w, new_h);
        evas_object_move(opt->obj, x + off_x, y + off_y);

        x += child_w + padding_l + padding_r + global_pad;
        sub_pixel += pad_inc;
        if (sub_pixel >= 1 << 16)
	  {
	     x++;
	     sub_pixel -= 1 << 16;
	  }
     }
}

static int
_evas_object_box_layout_vertical_weight_apply(Evas_Object_Box_Data *priv, Evas_Object_Box_Option **objects, int n_objects, int remaining, int weight_total)
{
   int rem_diff = 0;
   int i;

   for (i = 0; i < n_objects; i++)
     {
        Evas_Object_Box_Option *opt = objects[i];
        Evas_Object *o = opt->obj;
        int w;

        evas_object_geometry_get(o, NULL, NULL, &w, NULL);

        if (remaining < 0)
	  {
	     int min_h;

	     evas_object_size_hint_min_get(o, NULL, &min_h);
	     evas_object_resize(o, w, min_h);
	  }
	else
	  {
	     double normal_weight, weight_y;
	     int target_size;
	     int max_h;

	     evas_object_size_hint_weight_get(o, NULL, &weight_y);
	     normal_weight = weight_y / weight_total;
	     target_size = (int)((double)remaining * normal_weight);

	     evas_object_size_hint_max_get(o, NULL, &max_h);
	     if ((max_h != -1) && (target_size > max_h))
	       {
		  evas_object_resize(o, w, max_h);
		  rem_diff += max_h;
		  objects[i] = objects[n_objects - 1];
		  weight_total -= weight_y;
		  n_objects--;
		  return _evas_object_box_layout_vertical_weight_apply
		    (priv, objects, n_objects, remaining - rem_diff,
		     weight_total);
	       }
	     else
	       {
		  evas_object_resize(o, w, target_size);
		  rem_diff += target_size;
	       }
	  }
     }

   return remaining - rem_diff;
}

/**
 * Layout function which sets the box @a o to a (basic) vertical box.
 * @a priv must be the smart data of the box.
 *
 * This function behaves analogously to
 * evas_object_box_layout_horizontal().  The description of its
 * behaviour can be derived from that function's documentation.
 */
void
evas_object_box_layout_vertical(Evas_Object *o, Evas_Object_Box_Data *priv)
{
   int pad_inc = 0, sub_pixel = 0;
   int req_h, global_pad, remaining;
   double weight_total = 0.0;
   int weight_use = 0;
   int x, y, w, h;
   int n_children;
   Evas_Object_Box_Option *opt;
   Eina_List *l;

   n_children = eina_list_count(priv->children);
   if (!n_children)
     return;

   Evas_Object_Box_Option *objects[n_children];

   evas_object_geometry_get(o, &x, &y, &w, &h);
   global_pad = priv->padding_v;
   req_h = global_pad * (n_children - 1);

   EINA_LIST_FOREACH(priv->children, l, opt)
     {
        int padding_t, padding_b;
        double weight_y;

        evas_object_size_hint_weight_get(opt->obj, NULL, &weight_y);
        evas_object_size_hint_padding_get
	  (opt->obj, NULL, NULL, &padding_t, &padding_b);
        req_h += padding_t + padding_b;

        if (!weight_y)
	  {
	     int child_h;

	     evas_object_geometry_get(opt->obj, NULL, NULL, NULL, &child_h);
	     req_h += child_h;
	  }
	else
	  {
	     objects[weight_use] = opt;
	     weight_use++;
	     weight_total += weight_y;
	  }
     }

   remaining = h - req_h;

   if (weight_use)
     remaining = _evas_object_box_layout_vertical_weight_apply
       (priv, objects, weight_use, remaining, weight_total);

   if (priv->align_v >= 0.0)
     y += remaining * priv->align_v;
   else if (n_children == 1)
     y += remaining / 2;
   else
     { /* justified */
	_fixed_point_divide_and_decompose_integer
	  (remaining, n_children - 1, &global_pad, &pad_inc);
	global_pad += priv->padding_v;
     }

   EINA_LIST_FOREACH(priv->children, l, opt)
     {
        int child_w, child_h, max_w, new_w, off_x, off_y;
        int padding_l, padding_r, padding_t, padding_b;
        double align_x;

        evas_object_size_hint_align_get(opt->obj, &align_x, NULL);
        evas_object_size_hint_padding_get
	  (opt->obj, &padding_l, &padding_r, &padding_t, &padding_b);
        evas_object_size_hint_max_get(opt->obj, &max_w, NULL);

        evas_object_geometry_get(opt->obj, NULL, NULL, &child_w, &child_h);

        off_y = padding_t;
        new_w = child_w;

        _layout_set_offset_and_expand_dimension_space_max_bounded
	  (child_w, &new_w, w, max_w, &off_x, align_x, padding_l, padding_r);

        evas_object_resize(opt->obj, new_w, child_h);
        evas_object_move(opt->obj, x + off_x, y + off_y);

        y += child_h + padding_t + padding_b + global_pad;
        sub_pixel += pad_inc;
        if (sub_pixel >= 1 << 16)
	  {
	     y++;
	     sub_pixel -= 1 << 16;
	  }
     }
}

/**
 * Layout function which sets the box @a o to a *homogeneous*
 * horizontal box.  @a priv must be the smart data of the box.
 *
 * In a homogeneous horizontal box, its width is divided equally
 * between the contained objects.  The box's overall behavior is
 * controlled by its properties, which are set by the
 * evas_object_box_{h,v}_{align,padding}_set() family of functions.
 * The properties of the elements in the box -- set by
 * evas_object_size_hint_{align,padding,weight}_set() functions --
 * also control the way this function works.
 *
 * \par box's properties:
 * @c align_h has no influence on the box for this layout.  @c
 * padding_h tells the box to draw empty spaces of that size, in
 * pixels, between the (still equal) child objects's cells.  The @c
 * align_v and @c padding_v properties of the box don't contribute to
 * its behaviour when this layout is chosen.
 *
 * \par Child element's properties:
 * @c padding_l and @c padding_r sum up to the required width of the
 * child element.  The @c align_x property tells the relative position
 * of this overall child width in its allocated cell (0 to extreme
 * left, 1 to extreme right).  A value of -1.0 to @c align_x makes the
 * box try to resize this child element to the exact width of its cell
 * (respecting the min and max hints on the child's width *and*
 * accounting its horizontal padding properties).  The child's @c
 * padding_t, @c padding_b and @c align_y properties apply for
 * padding/positioning relative to the overall height of the box. A
 * value of -1.0 to @c align_y makes the box try to resize this child
 * element to the exact height of its parent (respecting the max hint
 * on the child's height).
 */
void
evas_object_box_layout_homogeneous_horizontal(Evas_Object *o, Evas_Object_Box_Data *priv)
{
   int cell_sz, share, inc;
   int sub_pixel = 0;
   int x, y, w, h;
   int n_children;
   Evas_Object_Box_Option *opt;
   Eina_List *l;

   n_children = eina_list_count(priv->children);
   if (!n_children)
     return;

   evas_object_geometry_get(o, &x, &y, &w, &h);

   share = w - priv->padding_h * (n_children - 1);
   _fixed_point_divide_and_decompose_integer
     (share, n_children, &cell_sz, &inc);

   EINA_LIST_FOREACH(priv->children, l, opt)
     {
        int child_w, child_h, max_h, min_w, max_w, new_w, new_h, off_x, off_y;
        int padding_l, padding_r, padding_t, padding_b;
        double align_x, align_y;

        evas_object_size_hint_align_get(opt->obj, &align_x, &align_y);
        evas_object_size_hint_padding_get
	  (opt->obj, &padding_l, &padding_r, &padding_t, &padding_b);
        evas_object_size_hint_max_get(opt->obj, &max_w, &max_h);
        evas_object_size_hint_min_get(opt->obj, &min_w, NULL);

        evas_object_geometry_get(opt->obj, NULL, NULL, &child_w, &child_h);

        new_w = child_w;
        new_h = child_h;

        _layout_set_offset_and_expand_dimension_space_max_bounded
	  (child_h, &new_h, h, max_h, &off_y, align_y, padding_t, padding_b);

        _layout_set_offset_and_change_dimension_min_max_cell_bounded
	  (child_w, &new_w, min_w, max_w, cell_sz, &off_x, align_x,
	   padding_l, padding_r);

        evas_object_resize(opt->obj, new_w, new_h);
        evas_object_move(opt->obj, x + off_x, y + off_y);

        x += cell_sz + priv->padding_h;
        sub_pixel += inc;
        if (sub_pixel >= 1 << 16)
	  {
	     x++;
	     sub_pixel -= 1 << 16;
	  }
     }
}

/**
 * Layout function which sets the box @a o to a *homogeneous* vertical
 * box.  @a priv must be the smart data of the box.
 *
 * This function behaves analogously to
 * evas_object_box_layout_homogeneous_horizontal().  The description
 * of its behaviour can be derived from that function's documentation.
 */
void
evas_object_box_layout_homogeneous_vertical(Evas_Object *o, Evas_Object_Box_Data *priv)
{
   int cell_sz, share, inc;
   int sub_pixel = 0;
   int x, y, w, h;
   int n_children;
   Evas_Object_Box_Option *opt;
   Eina_List *l;

   n_children = eina_list_count(priv->children);
   if (!n_children)
     return;

   evas_object_geometry_get(o, &x, &y, &w, &h);

   share = h - priv->padding_v * (n_children - 1);
   _fixed_point_divide_and_decompose_integer
     (share, n_children, &cell_sz, &inc);

   EINA_LIST_FOREACH(priv->children, l, opt)
     {
        int child_w, child_h, max_w, min_h, max_h, new_w, new_h, off_x, off_y;
        int padding_l, padding_r, padding_t, padding_b;
        double align_x, align_y;

        evas_object_size_hint_align_get(opt->obj, &align_x, &align_y);
        evas_object_size_hint_padding_get
	  (opt->obj, &padding_l, &padding_r, &padding_t, &padding_b);
        evas_object_size_hint_max_get(opt->obj, &max_w, &max_h);
        evas_object_size_hint_min_get(opt->obj, NULL, &min_h);

        evas_object_geometry_get(opt->obj, NULL, NULL, &child_w, &child_h);
        new_w = child_w;
        new_h = child_h;

        _layout_set_offset_and_expand_dimension_space_max_bounded
	  (child_w, &new_w, w, max_w, &off_x, align_x, padding_l, padding_r);

        _layout_set_offset_and_change_dimension_min_max_cell_bounded
	  (child_h, &new_h, min_h, max_h, cell_sz, &off_y, align_y,
	   padding_t, padding_b);

        evas_object_resize(opt->obj, new_w, new_h);
        evas_object_move(opt->obj, x + off_x, y + off_y);

        y += cell_sz + priv->padding_v;
        sub_pixel += inc;
        if (sub_pixel >= 1 << 16)
	  {
	     y++;
	     sub_pixel -= 1 << 16;
	  }
     }
}

/**
 * Layout function which sets the box @a o to a *max size-homogeneous*
 * horizontal box.  @a priv must be the smart data of the box.
 *
 * In a max size-homogeneous horizontal box, the equal sized cells
 * reserved for the child objects have the width of the space required
 * by the largest child (in width). The box's overall behavior is
 * controlled by its properties, which are set by the
 * evas_object_box_{h,v}_{align,padding}_set() family of functions.
 * The properties of the elements in the box -- set by
 * evas_object_size_hint_{align,padding,weight}_set() functions --
 * also control the way this function works.
 *
 * \par box's properties:
 * @c padding_h tells the box to draw empty spaces of that size, in
 * pixels, between the child objects's cells.  @c align_h controls the
 * horizontal alignment of the child objects relative to the
 * containing box. When set to 0, children are aligned to the left. A
 * value of 1 lets them aligned to the right border.  Values in
 * between align them proportionally. A negative value of @c align_h
 * makes the box to *justify* its children cells. The padding between
 * them, in this case, is corrected so that the leftmost one touches
 * the left border and the rightmost one touches the right border
 * (even if they must overlap).  The @c align_v and @c padding_v
 * properties of the box don't contribute to its behaviour when this
 * layout is chosen.
 *
 * \par Child element's properties:
 * @c padding_l and @c padding_r sum up to the required width of the
 * child element. The @c align_x property tells the relative position
 * of this overall child width in its allocated cell (0 to extreme
 * left, 1 to extreme right).  A value of -1.0 to @c align_x makes the
 * box try to resize this child element to the exact width of its cell
 * (respecting the min and max hints on the child's width *and*
 * accounting its horizontal padding properties).  The child's @c
 * padding_t, @c padding_b and @c align_y properties apply for
 * padding/positioning relative to the overall height of the box. A
 * value of -1.0 to @c align_y makes the box try to resize this child
 * element to the exact height of its parent (respecting the max hint
 * on the child's height).
 */
void
evas_object_box_layout_homogeneous_max_size_horizontal(Evas_Object *o, Evas_Object_Box_Data *priv)
{
   int remaining, global_pad, pad_inc = 0, sub_pixel = 0;
   int cell_sz = 0;
   int x, y, w, h;
   int n_children;
   Evas_Object_Box_Option *opt;
   Eina_List *l;

   n_children = eina_list_count(priv->children);
   if (!n_children)
     return;

   evas_object_geometry_get(o, &x, &y, &w, &h);

   EINA_LIST_FOREACH(priv->children, l, opt)
     {
        int child_w, padding_l, padding_r;

        evas_object_size_hint_padding_get
	  (opt->obj, &padding_l, &padding_r, NULL, NULL);
        evas_object_geometry_get(opt->obj, NULL, NULL, &child_w, NULL);
        if (child_w + padding_l + padding_r > cell_sz)
	  cell_sz = child_w + padding_l + padding_r;
     }

   global_pad = priv->padding_h;
   remaining = w - n_children * cell_sz - global_pad * (n_children - 1);

   if (priv->align_h >= 0.0)
     x += remaining * priv->align_h;
   else if (n_children == 1)
     x += remaining / 2;
   else
     { /* justified */
        _fixed_point_divide_and_decompose_integer
	  (remaining, n_children - 1, &global_pad, &pad_inc);
        global_pad += priv->padding_h;
     }

   EINA_LIST_FOREACH(priv->children, l, opt)
     {
        int child_w, child_h, min_w, max_w, max_h, new_w, new_h, off_x, off_y;
        int padding_l, padding_r, padding_t, padding_b;
        double align_x, align_y;

        evas_object_size_hint_align_get(opt->obj, &align_x, &align_y);
        evas_object_size_hint_padding_get
	  (opt->obj, &padding_l, &padding_r, &padding_t, &padding_b);
        evas_object_size_hint_max_get(opt->obj, &max_w, &max_h);
        evas_object_size_hint_min_get(opt->obj, &min_w, NULL);

        evas_object_geometry_get(opt->obj, NULL, NULL, &child_w, &child_h);

        new_w = child_w;
        new_h = child_h;

        _layout_set_offset_and_expand_dimension_space_max_bounded
	  (child_h, &new_h, h, max_h, &off_y, align_y, padding_t, padding_b);

        _layout_set_offset_and_change_dimension_min_max_cell_bounded
	  (child_w, &new_w, min_w, max_w, cell_sz, &off_x, align_x,
	   padding_l, padding_r);

        evas_object_resize(opt->obj, new_w, new_h);
        evas_object_move(opt->obj, x + off_x, y + off_y);

        x += cell_sz + global_pad;
        sub_pixel += pad_inc;
        if (sub_pixel >= 1 << 16)
	  {
	     x++;
	     sub_pixel -= 1 << 16;
	  }
     }
}

/**
 * Layout function which sets the box @a o to a *max size-homogeneous*
 * vertical box.  @a priv must be the smart data of the box.
 *
 * This function behaves analogously to
 * evas_object_box_layout_homogeneous_max_size_horizontal().  The
 * description of its behaviour can be derived from that function's
 * documentation.
 */
void
evas_object_box_layout_homogeneous_max_size_vertical(Evas_Object *o, Evas_Object_Box_Data *priv)
{
   int remaining, global_pad, pad_inc = 0, sub_pixel = 0;
   int cell_sz = 0;
   int x, y, w, h;
   int n_children;
   Evas_Object_Box_Option *opt;
   Eina_List *l;

   n_children = eina_list_count(priv->children);
   if (!n_children)
     return;

   evas_object_geometry_get(o, &x, &y, &w, &h);

   EINA_LIST_FOREACH(priv->children, l, opt)
     {
        int child_h, padding_t, padding_b;

        evas_object_size_hint_padding_get
	  (opt->obj, NULL, NULL, &padding_t, &padding_b);
        evas_object_geometry_get(opt->obj, NULL, NULL, NULL, &child_h);
        if (child_h + padding_t + padding_b > cell_sz)
	  cell_sz = child_h + padding_t + padding_b;
     }

   global_pad = priv->padding_v;
   remaining = h - n_children * cell_sz - global_pad * (n_children - 1);

   if (priv->align_v >= 0.0)
     y += remaining * priv->align_v;
   else if (n_children == 1)
     y += remaining / 2;
   else
     { /* justified */
	_fixed_point_divide_and_decompose_integer
	  (remaining, n_children - 1, &global_pad, &pad_inc);
	global_pad += priv->padding_v;
     }

   EINA_LIST_FOREACH(priv->children, l, opt)
     {
        int child_w, child_h, max_h, min_h, max_w, new_w, new_h, off_x, off_y;
        int padding_l, padding_r, padding_t, padding_b;
        double align_x, align_y;

        evas_object_size_hint_align_get(opt->obj, &align_x, &align_y);
        evas_object_size_hint_padding_get
	  (opt->obj, &padding_l, &padding_r, &padding_t, &padding_b);
        evas_object_size_hint_max_get(opt->obj, &max_w, &max_h);
        evas_object_size_hint_min_get(opt->obj, NULL, &min_h);

        evas_object_geometry_get(opt->obj, NULL, NULL, &child_w, &child_h);

        new_w = child_w;
        new_h = child_h;

        _layout_set_offset_and_expand_dimension_space_max_bounded
	  (child_w, &new_w, w, max_w, &off_x, align_x, padding_l, padding_r);

        _layout_set_offset_and_change_dimension_min_max_cell_bounded
	  (child_h, &new_h, min_h, max_h, cell_sz, &off_y, align_y,
	   padding_t, padding_b);

        evas_object_resize(opt->obj, new_w, new_h);
        evas_object_move(opt->obj, x + off_x, y + off_y);

        y += cell_sz + global_pad;
        sub_pixel += pad_inc;
        if (sub_pixel >= 1 << 16)
	  {
	     y++;
	     sub_pixel -= 1 << 16;
	  }

     }
}

static void
_evas_object_box_layout_flow_horizontal_row_info_collect(Evas_Object_Box_Data *priv, int box_w, int n_children, int *row_count, int *row_max_h, int *row_break, int *row_width, int *off_y_ret, int *max_h_ret)
{
   int i, remain_w = box_w, start_i = 0;
   int off_y = 0, max_h = 0, n_rows = 0;
   Eina_List *l;

   for (i = 0, l = priv->children; l != NULL; i++, l = l->next)
     {
        Evas_Object_Box_Option *opt = l->data;
        int padding_l, padding_r, padding_t, padding_b;
        int child_w, child_h, off_x = 0;

        evas_object_size_hint_padding_get
	  (opt->obj, &padding_l, &padding_r, &padding_t, &padding_b);

        evas_object_geometry_get(opt->obj, NULL, NULL, &child_w, &child_h);

        child_w += padding_l + padding_r + priv->padding_h;
        child_h += padding_t + padding_b;

        remain_w -= child_w;
        if (remain_w >= 0)
	  { /* continue "line" */
	     if (child_h > max_h)
	       max_h = child_h;

	     off_x += child_w;
	     row_width[n_rows] += child_w;
	  }
	else
	  { /* break line */
	     if (i == start_i)
	       { /* obj goes to actual line */
		  max_h = child_h;
		  row_width[n_rows] = child_w;
	       }
	     else
	       { /* obj goes to next line */
		  row_max_h[n_rows] = max_h;
		  row_break[n_rows] = i - 1;
		  n_rows++;

		  off_x = child_w;
		  off_y += max_h;
		  max_h = child_h;

		  row_width[n_rows] = child_w;
		  start_i = i;

		  remain_w = box_w - off_x;
	       }
	  }
     }

   row_break[n_rows] = i - 1;
   row_max_h[n_rows] = max_h;

   *row_count = n_rows;
   *off_y_ret = off_y;
   *max_h_ret = max_h;
}

/**
 * Layout function which sets the box @a o to a *flow* horizontal box.
 * @a priv must be the smart data of the box.
 *
 * In a flow horizontal box, the box's child elements are placed in
 * rows (think of text as an analogy). A row has as much elements as
 * can fit into the box's width.  The box's overall behavior is
 * controlled by its properties, which are set by the
 * evas_object_box_{h,v}_{align,padding}_set() family of functions.
 * The properties of the elements in the box -- set by
 * evas_object_size_hint_{align,padding,weight}_set() functions --
 * also control the way this function works.
 *
 * \par box's properties:
 * @c padding_h tells the box to draw empty spaces of that size, in
 * pixels, between the child objects's cells.  @c align_h dictates the
 * horizontal alignment of the rows (0 to left align them, 1 to right
 * align).  A value of -1.0 to @c align_h lets the rows *justified*
 * horizontally.  @c align_v controls the vertical alignment of the
 * entire set of rows (0 to top, 1 to bottom).  A value of -1.0 to @c
 * align_v makes the box to *justify* the rows vertically. The padding
 * between them, in this case, is corrected so that the first row
 * touches the top border and the last one touches the bottom border
 * (even if they must overlap). @c padding_v has no influence on the
 * layout.
 *
 * \par Child element's properties:
 * @c padding_l and @c padding_r sum up to the required width of the
 * child element.  The @c align_x property has no influence on the
 * layout. The child's @c padding_t and @c padding_b sum up to the
 * required height of the child element and is the only means (besides
 * row justifying) of setting space between rows.  Note, however, that
 * @c align_y dictates positioning relative to the *largest height*
 * required by a child object in the actual row.
 */
void
evas_object_box_layout_flow_horizontal(Evas_Object *o, Evas_Object_Box_Data *priv)
{
   int n_children, v_justify;
   int r, row_count = 0;
   int max_h, inc_y;
   int remain_y, i;
   int x, y, w, h;
   Eina_List *l;
   int off_y;

   n_children = eina_list_count(priv->children);
   if (!n_children)
     return;

   /* *per row* arrays */
   int row_max_h[n_children];
   int row_break[n_children];
   int row_width[n_children];

   memset(row_width, 0, sizeof(row_width));

   evas_object_geometry_get(o, &x, &y, &w, &h);

   _evas_object_box_layout_flow_horizontal_row_info_collect
     (priv, w, n_children, &row_count, row_max_h, row_break, row_width,
      &off_y, &max_h);

   inc_y = 0;
   v_justify = 0;
   remain_y = h - (off_y + max_h);

   if (priv->align_v >= 0.0)
     inc_y = priv->align_v * remain_y;
   else if (row_count == 0)
     y += remain_y / 2;
   else /* y-justified */
     inc_y = remain_y / row_count;

   for (i = 0, r = 0, l = priv->children; r <= row_count; r++)
     {
        int row_justify = 0, just_inc = 0, sub_pixel = 0;
        int row_size, remain_x;

        row_size = row_break[r] - i;
        remain_x = (w - row_width[r]);

        if (priv->align_h < 0.0)
	  {
	     if (row_size == 0)
	       x += remain_x / 2;
	     else
	       _fixed_point_divide_and_decompose_integer
		 (remain_x, row_size, &row_justify, &just_inc);
	  }

        for (; i <= row_break[r]; i++, l = l->next)
	  {
	     Evas_Object_Box_Option *opt = l->data;
	     int off_x, off_y, y_remain;
	     int padding_l, padding_r;
	     int child_w, child_h;
	     double align_y;

	     evas_object_size_hint_align_get(opt->obj, NULL, &align_y);
	     evas_object_size_hint_padding_get
	       (opt->obj, &padding_l, &padding_r, NULL, NULL);

	     evas_object_geometry_get
	       (opt->obj, NULL, NULL, &child_w, &child_h);

	     y_remain = row_max_h[r] - child_h;

	     off_x = padding_l;
	     if (priv->align_h >= 0.0)
	       off_x += remain_x * priv->align_h;
	     off_y = y_remain * align_y;

	     evas_object_move(opt->obj, x + off_x, y + off_y);

	     x += child_w + padding_l + padding_r + row_justify;

	     sub_pixel += just_inc;
	     if (sub_pixel >= 1 << 16)
	       {
		  x++;
		  sub_pixel -= 1 << 16;
	       }
	  }

        evas_object_geometry_get(o, &x, NULL, NULL, NULL);
        y += row_max_h[r] + inc_y;
     }
}

static void
_evas_object_box_layout_flow_vertical_col_info_collect(Evas_Object_Box_Data *priv, int box_h, int n_children, int *col_count, int *col_max_w, int *col_break, int *col_height, int *off_x_ret, int *max_w_ret)
{
   int i, remain_h = box_h, start_i = 0;
   int off_x = 0, max_w = 0, n_cols = 0;
   Eina_List *l;

   for (i = 0, l = priv->children; l != NULL; i++, l = l->next)
     {
        Evas_Object_Box_Option *opt = l->data;
        int padding_l, padding_r, padding_t, padding_b;
        int child_w, child_h, off_y = 0;

        evas_object_size_hint_padding_get
	  (opt->obj, &padding_l, &padding_r, &padding_t, &padding_b);

        evas_object_geometry_get(opt->obj, NULL, NULL, &child_w, &child_h);

        child_w += padding_l + padding_r;
        child_h += padding_t + padding_b + priv->padding_v;

        remain_h -= child_h;
        if (remain_h >= 0)
	  { /* continue "col" */
	     if (child_w > max_w)
	       max_w = child_w;

	     off_y += child_h;
	     col_height[n_cols] += child_h;
	  }
	else
	  {
	     /* break col */
	     if (i == start_i)
	       { /* obj goes to actual col */
		  max_w = child_w;
		  col_height[n_cols] = child_h;
	       }
	     else
	       { /* obj goes to next col */
		  col_max_w[n_cols] = max_w;
		  col_break[n_cols] = i - 1;
		  n_cols++;

		  off_x += max_w;
		  off_y = child_h;
		  max_w = child_w;

		  col_height[n_cols] = child_h;
		  start_i = i;

		  remain_h = box_h - off_y;
	       }
	  }
     }

   col_break[n_cols] = i - 1;
   col_max_w[n_cols] = max_w;

   *col_count = n_cols;
   *off_x_ret = off_x;
   *max_w_ret = max_w;
}

/**
 * Layout function which sets the box @a o to a *flow* vertical box.
 * @a priv must be the smart data of the box.
 *
 * This function behaves analogously to
 * evas_object_box_layout_flow_horizontal().  The description of its
 * behaviour can be derived from that function's documentation.
 */
void
evas_object_box_layout_flow_vertical(Evas_Object *o, Evas_Object_Box_Data *priv)
{
   int n_children, h_justify;
   int c, col_count;
   int max_w, inc_x;
   int remain_x, i;
   int x, y, w, h;
   Eina_List *l;
   int off_x;

   n_children = eina_list_count(priv->children);
   if (!n_children)
     return;

   /* *per col* arrays */
   int col_max_w[n_children];
   int col_break[n_children];
   int col_height[n_children];

   memset(col_height, 0, sizeof(col_height));

   evas_object_geometry_get(o, &x, &y, &w, &h);

   _evas_object_box_layout_flow_vertical_col_info_collect
     (priv, h, n_children, &col_count, col_max_w, col_break, col_height,
      &off_x, &max_w);

   inc_x = 0;
   h_justify = 0;
   remain_x = w - (off_x + max_w);

   if (priv->align_h >= 0)
     inc_x = priv->align_h * remain_x;
   else if (col_count == 0)
     x += remain_x / 2;
   else /* x-justified */
     inc_x = remain_x / col_count;

   for (i = 0, c = 0, l = priv->children; c <= col_count; c++)
     {
        int col_justify = 0, just_inc = 0, sub_pixel = 0;
        int col_size, remain_y;

        col_size = col_break[c] - i;
        remain_y = (h - col_height[c]);

        if (priv->align_v < 0.0)
	  {
	     if (col_size == 0)
	       y += remain_y / 2;
	     else
	       _fixed_point_divide_and_decompose_integer
		 (remain_y, col_size, &col_justify, &just_inc);
	  }

        for (; i <= col_break[c]; i++, l = l->next)
	  {
	     Evas_Object_Box_Option *opt = l->data;
	     int off_x, off_y, x_remain;
	     int padding_t, padding_b;
	     int child_w, child_h;
	     double align_x;

	     evas_object_size_hint_align_get(opt->obj, &align_x, NULL);
	     evas_object_size_hint_padding_get
	       (opt->obj, NULL, NULL, &padding_t, &padding_b);

	     evas_object_geometry_get
	       (opt->obj, NULL, NULL, &child_w, &child_h);

	     x_remain = col_max_w[c] - child_w;

	     off_x = x_remain * align_x;
	     off_y = padding_t;
	     if (priv->align_v >= 0.0)
	       off_y += remain_y * priv->align_v;

	     evas_object_move(opt->obj, x + off_x, y + off_y);

	     y += child_h + padding_t + padding_b + col_justify;

	     sub_pixel += just_inc;
	     if (sub_pixel >= 1 << 16)
	       {
		  y++;
		  sub_pixel -= 1 << 16;
	       }
	  }

        evas_object_geometry_get(o, NULL, &y, NULL, NULL);
        x += col_max_w[c] + inc_x;
     }
}

/**
 * Layout function which sets the box @a o to set all children to the
 * size of the object.  @a priv must be the smart data of the box.
 *
 * In a stack box, all children will be given the same size and they
 * will be stacked on above the other, so the first object will be the
 * bottom most.
 *
 * \par box's properties:
 * No box option is used.
 *
 * \par Child  element's   properties:
 * @c padding_l and @c padding_r sum up to the required width of the
 * child element.  The @c align_x property tells the relative position
 * of this overall child width in its allocated cell (0 to extreme
 * left, 1 to extreme right).  A value of -1.0 to @c align_x makes the
 * box try to resize this child element to the exact width of its cell
 * (respecting the min and max hints on the child's width *and*
 * accounting its horizontal padding properties).  Same applies to
 * vertical axis.
 */
void
evas_object_box_layout_stack(Evas_Object *o, Evas_Object_Box_Data *priv)
{
   Eina_List *l;
   Evas_Coord ox, oy, ow, oh;
   Evas_Object_Box_Option *opt;
   Evas_Object *old_child = NULL;

   evas_object_geometry_get(o, &ox, &oy, &ow, &oh);

   EINA_LIST_FOREACH(priv->children, l, opt)
     {
        Evas_Object *child = opt->obj;
        Evas_Coord max_w, max_h, min_w, min_h, pad_l, pad_r, pad_t, pad_b,
	  child_w, child_h, new_w, new_h, off_x, off_y;
        double align_x, align_y;

        evas_object_size_hint_align_get(child, &align_x, &align_y);
        evas_object_size_hint_padding_get
	  (child, &pad_l, &pad_r, &pad_t, &pad_b);
        evas_object_size_hint_max_get(child, &max_w, &max_h);
        evas_object_size_hint_min_get(child, &min_w, &min_h);

        evas_object_geometry_get(child, NULL, NULL, &child_w, &child_h);
        new_w = child_w;
        new_h = child_h;

        _layout_set_offset_and_change_dimension_min_max_cell_bounded
	  (child_w, &new_w, min_w, max_w, ow, &off_x, align_x, pad_l, pad_r);
        _layout_set_offset_and_change_dimension_min_max_cell_bounded
	  (child_h, &new_h, min_h, max_h, oh, &off_y, align_y, pad_t, pad_b);

        evas_object_resize(child, new_w, new_h);
        evas_object_move(child, ox + off_x, oy + off_y);

        if (old_child)
	  evas_object_stack_above(child, old_child);
        old_child = child;
     }
}

/**
 * Get horizontal alignment of the box @a o.
 */
double
evas_object_box_align_h_get(Evas_Object *o)
{
   EVAS_OBJECT_BOX_DATA_GET_OR_RETURN_VAL(o, priv, 0.0);
   return priv->align_h;
}

/**
 * Get horizontal padding of the box @a o.
 */
int
evas_object_box_padding_h_get(Evas_Object *o)
{
   EVAS_OBJECT_BOX_DATA_GET_OR_RETURN_VAL(o, priv, 0);
   return priv->padding_h;
}

/**
 * Get vertical alignment of the box @a o.
 */
double
evas_object_box_align_v_get(Evas_Object *o)
{
   EVAS_OBJECT_BOX_DATA_GET_OR_RETURN_VAL(o, priv, 0.0);
   return priv->align_v;
}

/**
 * Get vertical padding of the box @a o.
 */
int
evas_object_box_padding_v_get(Evas_Object *o)
{
   EVAS_OBJECT_BOX_DATA_GET_OR_RETURN_VAL(o, priv, 0);
   return priv->padding_v;
}

/**
 * Set horizontal alignment of the box @a o.
 */
void
evas_object_box_align_h_set(Evas_Object *o, double align_h)
{
   EVAS_OBJECT_BOX_DATA_GET_OR_RETURN(o, priv);
   priv->align_h = align_h;
   evas_object_smart_changed(o);
}

/**
 * Set horizontal padding of the box @a o.
 */
void
evas_object_box_padding_h_set(Evas_Object *o, int padding_h)
{
   EVAS_OBJECT_BOX_DATA_GET_OR_RETURN(o, priv);
   priv->padding_h = padding_h;
   evas_object_smart_changed(o);
}

/**
 * Set vertical alignment of the box @a o.
 */
void
evas_object_box_align_v_set(Evas_Object *o, double align_v)
{
   EVAS_OBJECT_BOX_DATA_GET_OR_RETURN(o, priv);
   priv->align_v = align_v;
   evas_object_smart_changed(o);
}

/**
 * Set vertical padding of the box @a o.
 */
void
evas_object_box_padding_v_set(Evas_Object *o, int padding_v)
{
   EVAS_OBJECT_BOX_DATA_GET_OR_RETURN(o, priv);
   priv->padding_v = padding_v;
   evas_object_smart_changed(o);
}

/**
 * Append a new object @a child to the box @a o. On error, @c NULL is
 * returned.
 */
Evas_Object_Box_Option *
evas_object_box_append(Evas_Object *o, Evas_Object *child)
{
   Evas_Object_Box_Option *opt;
   const Evas_Object_Box_Api *api;

   EVAS_OBJECT_BOX_DATA_GET_OR_RETURN_VAL(o, priv, 0);
   if (!child)
     return NULL;

   api = priv->api;
   if ((!api) || (!api->append))
     return NULL;

   opt = api->append(o, priv, child);

   if (opt)
     {
        evas_object_smart_member_add(child, o);
        evas_object_smart_changed(o);
        return _evas_object_box_option_callbacks_register(o, priv, opt);
     }

   return NULL;
}

/**
 * Prepend a new object @a child to the box @a o. On error, @c NULL is
 * returned.
 */
Evas_Object_Box_Option *
evas_object_box_prepend(Evas_Object *o, Evas_Object *child)
{
   Evas_Object_Box_Option *opt;
   const Evas_Object_Box_Api *api;

   EVAS_OBJECT_BOX_DATA_GET_OR_RETURN_VAL(o, priv, 0);
   if (!child)
     return NULL;

   api = priv->api;
   if ((!api) || (!api->prepend))
     return NULL;

   opt = api->prepend(o, priv, child);

   if (opt)
     {
        evas_object_smart_member_add(child, o);
        evas_object_smart_changed(o);
        return _evas_object_box_option_callbacks_register(o, priv, opt);
     }

   return NULL;
}

/**
 * Prepend a new object @child to the box @o relative to element @a
 * reference. If @a reference is not contained in the box or any other
 * error occurs, @c NULL is returned.
 */
Evas_Object_Box_Option *
evas_object_box_insert_before(Evas_Object *o, Evas_Object *child, const Evas_Object *reference)
{
   Evas_Object_Box_Option *opt;
   const Evas_Object_Box_Api *api;

   EVAS_OBJECT_BOX_DATA_GET_OR_RETURN_VAL(o, priv, 0);
   if (!child)
     return NULL;

   api = priv->api;
   if ((!api) || (!api->insert_before))
     return NULL;

   opt = api->insert_before(o, priv, child, reference);

   if (opt)
     {
        evas_object_smart_member_add(child, o);
        evas_object_smart_changed(o);
        return _evas_object_box_option_callbacks_register(o, priv, opt);
     }

   return NULL;
}

/**
 * Insert a new object @a child to the box @a o at position @a pos. On
 * error, @c NULL is returned.
 */
Evas_Object_Box_Option *
evas_object_box_insert_at(Evas_Object *o, Evas_Object *child, unsigned int pos)
{
   Evas_Object_Box_Option *opt;
   const Evas_Object_Box_Api *api;

   EVAS_OBJECT_BOX_DATA_GET_OR_RETURN_VAL(o, priv, 0);
   if (!child)
     return NULL;

   api = priv->api;
   if ((!api) || (!api->insert_at))
     return NULL;

   opt = api->insert_at(o, priv, child, pos);

   if (opt)
     {
        evas_object_smart_member_add(child, o);
        evas_object_smart_changed(o);
        return _evas_object_box_option_callbacks_register(o, priv, opt);
     }

   return NULL;
}

/**
 * Remove an object @a child from the box @a o. On error, @c 0 is
 * returned.
 */
Evas_Bool
evas_object_box_remove(Evas_Object *o, Evas_Object *child)
{
   const Evas_Object_Box_Api *api;
   Evas_Object *obj;

   EVAS_OBJECT_BOX_DATA_GET_OR_RETURN_VAL(o, priv, 0);
   if (!child)
     return 0;

   api = priv->api;
   if ((!api) || (!api->remove))
     return 0;

   obj = api->remove(o, priv, child);

   if (obj)
     {
        _evas_object_box_child_callbacks_unregister(obj);
        evas_object_smart_member_del(obj);
        return 1;
     }

   return 0;
}

/**
 * Remove an object from the box @a o which occupies position @a
 * pos. On error, @c 0 is returned.
 */
Evas_Bool
evas_object_box_remove_at(Evas_Object *o, unsigned int pos)
{
   const Evas_Object_Box_Api *api;
   Evas_Object *obj;

   EVAS_OBJECT_BOX_DATA_GET_OR_RETURN_VAL(o, priv, 0);
   if (pos < 0)
     return 0;

   api = priv->api;
   if ((!api) || (!api->remove_at))
     return 0;

   obj = api->remove_at(o, priv, pos);

   if (obj)
     {
        _evas_object_box_child_callbacks_unregister(obj);
        evas_object_smart_member_del(obj);
        return 1;
     }

   return 0;
}

/**
 * Get the name of the property of the child elements of the box @a o
 * whose id is @a property. On error, @c NULL is returned.
 */
const char *
evas_object_box_option_property_name_get(Evas_Object *o, int property)
{
   EVAS_OBJECT_BOX_DATA_GET_OR_RETURN_VAL(o, priv, NULL);
   const Evas_Object_Box_Api *api;

   if (property < 0)
     return NULL;

   api = priv->api;
   if ((!api) || (!api->property_name_get))
     return NULL;

   return api->property_name_get(o, property);
}

/**
 * Get the id of the property of the child elements of the box @a o
 * whose name is @a name. On error, @c -1 is returned.
 */
int
evas_object_box_option_property_id_get(Evas_Object *o, const char *name)
{
   EVAS_OBJECT_BOX_DATA_GET_OR_RETURN_VAL(o, priv, -1);
   const Evas_Object_Box_Api *api;

   if (!name)
     return -1;

   api = priv->api;
   if ((!api) || (!api->property_id_get))
     return -1;

   return api->property_id_get(o, name);
}

/**
 * Set the property (with id @a property) of the child element of the
 * box @a o whose property struct is @a opt. The property's values
 * must be the last arguments and their type *must* match that of the
 * property itself. On error, @c 0 is returned.
 */
Evas_Bool
evas_object_box_option_property_set(Evas_Object *o, Evas_Object_Box_Option *opt, int property, ...)
{
   Evas_Bool ret;
   va_list args;

   va_start(args, property);
   ret = evas_object_box_option_property_vset(o, opt, property, args);
   va_end(args);

   return ret;
}

/**
 * Set the property (with id @a property) of the child element of the
 * box @a o whose property struct is @a opt. The property's values
 * must be the args which the va_list @a args is initialized with and
 * their type *must* match that of the property itself. On error, @c 0
 * is returned.
 */

Evas_Bool
evas_object_box_option_property_vset(Evas_Object *o, Evas_Object_Box_Option *opt, int property, va_list args)
{
   EVAS_OBJECT_BOX_DATA_GET_OR_RETURN_VAL(o, priv, 0);
   const Evas_Object_Box_Api *api;

   if (!opt)
     return 0;

   api = priv->api;
   if ((!api) || (!api->property_set))
     return 0;

   return api->property_set(o, opt, property, args);
}

/**
 * Get the property (with id @a property) of the child element of the
 * box @a o whose property struct is @a opt. The last arguments must
 * be addresses of variables with the same type of that property. On
 * error, @c 0 is returned.
 */
Evas_Bool
evas_object_box_option_property_get(Evas_Object *o, Evas_Object_Box_Option *opt, int property, ...)
{
   Evas_Bool ret;
   va_list args;

   va_start(args, property);
   ret = evas_object_box_option_property_vget(o, opt, property, args);
   va_end(args);

   return ret;
}

/**
 * Get the property (with id @a property) of the child element of the
 * box @a o whose property struct is @a opt. The args which the
 * va_list @a args is initialized with must be addresses of variables
 * with the same type of that property. On error, @c 0 is returned.
 */
Evas_Bool
evas_object_box_option_property_vget(Evas_Object *o, Evas_Object_Box_Option *opt, int property, va_list args)
{
   EVAS_OBJECT_BOX_DATA_GET_OR_RETURN_VAL(o, priv, 0);
   const Evas_Object_Box_Api *api;

   if (!opt)
     return 0;

   api = priv->api;
   if ((!api) || (!api->property_get))
     return 0;

   return api->property_get(o, opt, property, args);
}

/**
 * @}
 */
