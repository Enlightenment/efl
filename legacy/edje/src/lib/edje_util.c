#include "edje_private.h"

typedef struct _Edje_Box_Layout Edje_Box_Layout;
struct _Edje_Box_Layout
{
   EINA_RBTREE;
   Evas_Object_Box_Layout func;
   void *(*layout_data_get)(void *);
   void (*layout_data_free)(void *);
   void *data;
   void (*free_data)(void *);
   char name[];
};

static Eina_Hash *_edje_color_class_hash = NULL;
static Eina_Hash *_edje_color_class_member_hash = NULL;

static Eina_Hash *_edje_text_class_hash = NULL;
static Eina_Hash *_edje_text_class_member_hash = NULL;

static Eina_Rbtree *_edje_box_layout_registry = NULL;

char *_edje_fontset_append = NULL;
FLOAT_T _edje_scale = ZERO;
int _edje_freeze_val = 0;
int _edje_freeze_calc_count = 0;
Eina_List *_edje_freeze_calc_list = NULL;

typedef struct _Edje_List_Foreach_Data Edje_List_Foreach_Data;
struct _Edje_List_Foreach_Data
{
   Eina_List *list;
};

static Eina_Bool _edje_color_class_list_foreach(const Eina_Hash *hash, const void *key, void *data, void *fdata);
static Eina_Bool _edje_text_class_list_foreach(const Eina_Hash *hash, const void *key, void *data, void *fdata);
static void _edje_object_image_preload_cb(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _edje_object_signal_preload_cb(void *data, Evas_Object *obj, const char *emission, const char *source);

Edje_Real_Part *_edje_real_part_recursive_get_helper(const Edje *ed, char **path);

/************************** API Routines **************************/

#define FASTFREEZE 1

/**
 * @brief Freeze Edje objects.
 *
 * This function freezes every edje objects in the current process.
 *
 * See edje_object_freeze().
 *
 */
EAPI void
edje_freeze(void)
{
#ifdef FASTFREEZE
   _edje_freeze_val++;
   INF("fr ++ ->%i", _edje_freeze_val);
#else
// FIXME: could just have a global freeze instead of per object
// above i tried.. but this broke some things. notable e17's menus. why?
   Eina_List *l;
   Evas_Object *data;

   EINA_LIST_FOREACH(_edje_edjes, l, data)
     edje_object_freeze(data);
#endif
}

#ifdef FASTFREEZE
static void
_edje_thaw_edje(Edje *ed)
{
   unsigned int i;

   for (i = 0; i < ed->table_parts_size; i++)
     {
	Edje_Real_Part *rp;

	rp = ed->table_parts[i];
	if (rp->part->type == EDJE_PART_TYPE_GROUP && rp->swallowed_object)
	  {
	     Edje *ed2;

	     ed2 = _edje_fetch(rp->swallowed_object);
	     if (ed2) _edje_thaw_edje(ed2);
	  }
     }
   if ((ed->recalc) && (ed->freeze <= 0)) _edje_recalc_do(ed);
}
#endif

/**
 * @brief Thaw edje objects.
 *
 * This function thaw all edje object in the current process.
 *
 * See edje_object_thaw().
 *
 */
EAPI void
edje_thaw(void)
{
#ifdef FASTFREEZE
   _edje_freeze_val--;
   INF("fr -- ->%i", _edje_freeze_val);
   if ((_edje_freeze_val <= 0) && (_edje_freeze_calc_count > 0))
     {
        Edje *ed;

	_edje_freeze_calc_count = 0;
	EINA_LIST_FREE(_edje_freeze_calc_list, ed)
	  {
	     _edje_thaw_edje(ed);
             ed->freeze_calc = 0;
	  }
     }
#else
  Evas_Object *data;

// FIXME: could just have a global freeze instead of per object
// comment as above.. why?
   Eina_List *l;

   EINA_LIST_FOREACH(_edje_edjes, l, data)
     edje_object_thaw(data);
#endif
}

/**
 * @brief Set the edje append fontset.
 *
 * @param fonts The fontset to append.
 *
 * This function sets the edje append fontset.
 *
 */
EAPI void
edje_fontset_append_set(const char *fonts)
{
   if (_edje_fontset_append)
     free(_edje_fontset_append);
   _edje_fontset_append = fonts ? strdup(fonts) : NULL;
}

/**
 * @brief Get the edje append fontset.
 *
 * @return The edje append fontset.
 *
 * This function returns the edje append fontset set by
 * edje_fontset_append_set() function.
 *
 * @see edje_fontset_append_set().
 *
 */
EAPI const char *
edje_fontset_append_get(void)
{
   return _edje_fontset_append;
}

/**
 * @brief Set edje's global scaling factor.
 *
 * @param scale The edje (global) scale factor. The defaul is 1.0.
 *
 * Edje allows one to build scalable interfaces. Scale factors, which
 * are set to neutral values by default (no scaling, actual sizes),
 * are of two types: global and individual. Edje's global scaling
 * factor will affect all its objects which hadn't their individual
 * scaling factors altered from the default value. If they had it set
 * differently, that factor will override the global one.
 *
 * Scaling affects the values of min/max object sizes, which are
 * multiplied by it. Font sizes are scaled, too.
 *
 * This property can be retrieved with edje_scale_get().
 *
 * @see edje_scale_get().
 *
 */
EAPI void
edje_scale_set(double scale)
{
   Eina_List *l;
   Evas_Object *data;

   if (_edje_scale == FROM_DOUBLE(scale)) return;
   _edje_scale = FROM_DOUBLE(scale);
   EINA_LIST_FOREACH(_edje_edjes, l, data)
     edje_object_calc_force(data);
}

/**
 * @brief Get edje's global scaling factor.
 *
 * @return The edje (global) scale factor. The defaul is 1.0.
 *
 * This function returns edje's global scale factor, which can be set
 * by edje_scale_set().
 *
 * @see edje_scale_set().
 *
 */
EAPI double
edje_scale_get(void)
{
  return TO_DOUBLE(_edje_scale);
}

/**
 * @brief Set the edje object's scaling factor.
 *
 * @param obj The edje object's reference.
 * @param scale The edje object scale factor. The defaul is 1.0.
 *
 * This function sets the individual scale factor of the @a obj edje
 * object. This property (or edje's global scale factor, when
 * applicable), will affect this object's parts. However, only parts
 * which, at the EDC language level, were declared which the "scale"
 * attribute set to 1 (default is zero) will be affected.
 *
 * This scale factor can be retrieved with edje_object_scale_get().
 * @see edje_object_scale_get().
 *
 */
EAPI Eina_Bool
edje_object_scale_set(Evas_Object *obj, double scale)
{
   Edje *ed;

   ed = _edje_fetch(obj);
   if (!ed) return EINA_FALSE;
   if (ed->scale == scale) return EINA_TRUE;
   ed->scale = FROM_DOUBLE(scale);
   edje_object_calc_force(obj);
   return EINA_TRUE;
}

/**
 * @brief Get the edje object's scaling factor.
 *
 * @param obj The edje object's reference.
 *
 * This function returns the individual scale factor of the @a obj
 * edje object, which can be set by edje_object_scale_set().
 *
 * @see edje_object_scale_set().
 *
 */
EAPI double
edje_object_scale_get(const Evas_Object *obj)
{
   Edje *ed;

   ed = _edje_fetch(obj);
   if (!ed) return 0.0;
   return TO_DOUBLE(ed->scale);
}

/**
 * Get the RTL orientation for this object.
 *
 * You can RTL orientation explicitly with edje_object_mirrored_set.
 *
 * @param obj the smart object
 * @return if flag is set or not.
 * @since 1.1.0
 */
EAPI Eina_Bool
edje_object_mirrored_get(const Evas_Object *obj)
{
   Edje *ed;

   ed = _edje_fetch(obj);
   if (!ed) return EINA_FALSE;

   return ed->is_rtl;
}

void
_edje_object_orientation_inform(Evas_Object *obj)
{
   if (edje_object_mirrored_get(obj))
     edje_object_signal_emit(obj, "edje,state,rtl", "edje");
   else
     edje_object_signal_emit(obj, "edje,state,ltr", "edje");
}

/**
 * Set the RTL orientation for this object.
 *
 * @param obj the smart object
 * @rtl new value of flag EINA_TRUE/EINA_FALSE
 * @since 1.1.0
 */
EAPI void
edje_object_mirrored_set(Evas_Object *obj, Eina_Bool rtl)
{
   Edje *ed;
   unsigned int i;

   ed = _edje_fetch(obj);
   if (!ed) return;
   if (ed->is_rtl == rtl) return;

   ed->is_rtl = rtl;

   for (i = 0 ; i < ed->table_parts_size ; i++)
     {
	Edje_Real_Part *ep;
        const char *s;
        double v;

	ep = ed->table_parts[i];
	s = ep->param1.description->state.name,
	v = ep->param1.description->state.value;
        _edje_part_description_apply(ed, ep, s, v , NULL, 0.0);
        ep->chosen_description = ep->param1.description;
     }
   _edje_recalc_do(ed);

   _edje_object_orientation_inform(obj);

   return;
}

/**
 * @brief Get Edje object data.
 *
 * @param obj A valid Evas_Object handle
 * @param key The data key
 * @return The data string
 *
 * This function fetches data specified at the object level.
 *
 * In EDC this comes from a data block within the group block that @a
 * obj was loaded from. E.g.
 *
 * @code
 * collections {
 *   group {
 *     name: "a_group";
 *     data {
 *	 item: "key1" "value1";
 *	 item: "key2" "value2";
 *     }
 *   }
 * }
 * @endcode
 */
EAPI const char *
edje_object_data_get(const Evas_Object *obj, const char *key)
{
   Edje *ed;

   ed = _edje_fetch(obj);
   if ((!ed) || (!key))
     return NULL;
   if (!ed->collection) return NULL;
   if (!ed->collection->data) return NULL;
   return edje_string_get(eina_hash_find(ed->collection->data, key));
}

/**
 * @brief Freeze object.
 *
 * @param obj A valid Evas_Object handle
 * @return The frozen state or 0 on Error
 *
 * This function puts all changes on hold. Successive freezes will
 * nest, requiring an equal number of thaws.
 *
 */
EAPI int
edje_object_freeze(Evas_Object *obj)
{
   Edje *ed;
   unsigned int i;

   ed = _edje_fetch(obj);
   if (!ed) return 0;
   for (i = 0; i < ed->table_parts_size; i++)
     {
	Edje_Real_Part *rp;
	rp = ed->table_parts[i];
	if (rp->part->type == EDJE_PART_TYPE_GROUP && rp->swallowed_object)
	  edje_object_freeze(rp->swallowed_object);
     }
   return _edje_freeze(ed);
}

/**
 * @brief Thaw object.
 *
 * @param obj A valid Evas_Object handle
 * @return The frozen state or 0 on Error
 *
 * This allows frozen changes to occur.
 *
 */
EAPI int
edje_object_thaw(Evas_Object *obj)
{
   Edje *ed;
   unsigned int i;

   ed = _edje_fetch(obj);
   if (!ed) return 0;
   for (i = 0; i < ed->table_parts_size; i++)
     {
	Edje_Real_Part *rp;

	rp = ed->table_parts[i];
	if (rp->part->type == EDJE_PART_TYPE_GROUP && rp->swallowed_object)
	  edje_object_thaw(rp->swallowed_object);
     }
   return _edje_thaw(ed);
}

/**
 * @brief Set Edje color class.
 *
 * @param color_class
 * @param r Object Red value
 * @param g Object Green value
 * @param b Object Blue value
 * @param a Object Alpha value
 * @param r2 Outline Red value
 * @param g2 Outline Green value
 * @param b2 Outline Blue value
 * @param a2 Outline Alpha value
 * @param r3 Shadow Red value
 * @param g3 Shadow Green value
 * @param b3 Shadow Blue value
 * @param a3 Shadow Alpha value
 *
 * This function sets the color values for a process level color
 * class.  This will cause all edje parts in the current process that
 * have the specified color class to have their colors multiplied by
 * these values.  (Object level color classes set by
 * edje_object_color_class_set() will override the values set by this
 * function).
 *
 * The first color is the object, the second is the text outline, and
 * the third is the text shadow. (Note that the second two only apply
 * to text parts).
 *
 * Setting color emits a signal "color_class,set" with source being
 * the given color class in all objects.
 *
 * @see edje_color_class_set().
 *
 * @note unlike Evas, Edje colors are @b not pre-multiplied. That is,
 *       half-transparent white is 255 255 255 128.
 */
EAPI Eina_Bool
edje_color_class_set(const char *color_class, int r, int g, int b, int a, int r2, int g2, int b2, int a2, int r3, int g3, int b3, int a3)
{
   Eina_List *members;
   Edje_Color_Class *cc;

   if (!color_class) return EINA_FALSE;

   cc = eina_hash_find(_edje_color_class_hash, color_class);
   if (!cc)
     {
        cc = calloc(1, sizeof(Edje_Color_Class));
	if (!cc) return EINA_FALSE;
	cc->name = eina_stringshare_add(color_class);
	if (!cc->name)
	  {
	     free(cc);
	     return EINA_FALSE;
	  }
	if (!_edje_color_class_hash)
          _edje_color_class_hash = eina_hash_string_superfast_new(NULL);
        eina_hash_add(_edje_color_class_hash, color_class, cc);
     }

   if (r < 0)        r = 0;
   else if (r > 255) r = 255;
   if (g < 0)        g = 0;
   else if (g > 255) g = 255;
   if (b < 0)        b = 0;
   else if (b > 255) b = 255;
   if (a < 0)        a = 0;
   else if (a > 255) a = 255;
   if ((cc->r == r) && (cc->g == g) &&
       (cc->b == b) && (cc->a == a) &&
       (cc->r2 == r2) && (cc->g2 == g2) &&
       (cc->b2 == b2) && (cc->a2 == a2) &&
       (cc->r3 == r3) && (cc->g3 == g3) &&
       (cc->b3 == b3) && (cc->a3 == a3))
     return EINA_TRUE;
   cc->r = r;
   cc->g = g;
   cc->b = b;
   cc->a = a;
   cc->r2 = r2;
   cc->g2 = g2;
   cc->b2 = b2;
   cc->a2 = a2;
   cc->r3 = r3;
   cc->g3 = g3;
   cc->b3 = b3;
   cc->a3 = a3;

   members = eina_hash_find(_edje_color_class_member_hash, color_class);
   while (members)
     {
	Edje *ed;

	ed = eina_list_data_get(members);
	ed->dirty = 1;
#ifdef EDJE_CALC_CACHE
	ed->all_part_change = 1;
#endif
	_edje_recalc(ed);
	_edje_emit(ed, "color_class,set", color_class);
	members = eina_list_next(members);
     }
   return EINA_TRUE;
}

/**
 * @brief Get Edje color class.
 *
 * @param color_class
 * @param r Object Red value
 * @param g Object Green value
 * @param b Object Blue value
 * @param a Object Alpha value
 * @param r2 Outline Red value
 * @param g2 Outline Green value
 * @param b2 Outline Blue value
 * @param a2 Outline Alpha value
 * @param r3 Shadow Red value
 * @param g3 Shadow Green value
 * @param b3 Shadow Blue value
 * @param a3 Shadow Alpha value
 *
 * @return EINA_TRUE if found or EINA_FALSE if not found and all
 *         values are zeroed.
 *
 * This function gets the color values for a process level color
 * class. This value is the globally set and not per-object, that is,
 * the value that would be used by objects if they did not override with
 * edje_object_color_class_set().
 *
 * The first color is the object, the second is the text outline, and
 * the third is the text shadow. (Note that the second two only apply
 * to text parts).
 *
 * @see edje_color_class_set().
 *
 * @note unlike Evas, Edje colors are @b not pre-multiplied. That is,
 *       half-transparent white is 255 255 255 128.
 */
EAPI Eina_Bool
edje_color_class_get(const char *color_class, int *r, int *g, int *b, int *a, int *r2, int *g2, int *b2, int *a2, int *r3, int *g3, int *b3, int *a3)
{
   Edje_Color_Class *cc;

   if (!color_class)
     cc = NULL;
   else
     cc = eina_hash_find(_edje_color_class_hash, color_class);

   if (cc)
     {
#define X(C) if (C) *C = cc->C
#define S(_r, _g, _b, _a) X(_r); X(_g); X(_b); X(_a)
	S(r, g, b, a);
	S(r2, g2, b2, a2);
	S(r3, g3, b3, a3);
#undef S
#undef X
	return EINA_TRUE;
     }
   else
     {
#define X(C) if (C) *C = 0
#define S(_r, _g, _b, _a) X(_r); X(_g); X(_b); X(_a)
	S(r, g, b, a);
	S(r2, g2, b2, a2);
	S(r3, g3, b3, a3);
#undef S
#undef X
	return EINA_FALSE;
     }
}

/**
 * @brief Delete edje color class.
 *
 * @param color_class
 *
 * This function deletes any values at the process level for the
 * specified color class.
 *
 * Deleting color emits a signal "color_class,del" with source being
 * the given color class in all objects.
 */
void
edje_color_class_del(const char *color_class)
{
   Edje_Color_Class *cc;
   Eina_List *members;

   if (!color_class) return;

   cc = eina_hash_find(_edje_color_class_hash, color_class);
   if (!cc) return;

   eina_hash_del(_edje_color_class_hash, color_class, cc);
   eina_stringshare_del(cc->name);
   free(cc);

   members = eina_hash_find(_edje_color_class_member_hash, color_class);
   while (members)
     {
	Edje *ed;

	ed = eina_list_data_get(members);
	ed->dirty = 1;
#ifdef EDJE_CALC_CACHE
	ed->all_part_change = 1;
#endif
	_edje_recalc(ed);
	_edje_emit(ed, "color_class,del", color_class);
	members = eina_list_next(members);
     }
}

/**
 * @brief Lists color classes.
 *
 * @return A list of color class names (strings). These strings and
 * the list must be free()'d by the caller.
 *
 * This function lists all color classes known about by the current
 * process.
 *
 */
Eina_List *
edje_color_class_list(void)
{
   Edje_List_Foreach_Data fdata;

   memset(&fdata, 0, sizeof(Edje_List_Foreach_Data));
   eina_hash_foreach(_edje_color_class_member_hash,
                     _edje_color_class_list_foreach, &fdata);

   return fdata.list;
}

static Eina_Bool
_edje_color_class_list_foreach(const Eina_Hash *hash __UNUSED__, const void *key, void *data __UNUSED__, void *fdata)
{
   Edje_List_Foreach_Data *fd;

   fd = fdata;
   fd->list = eina_list_append(fd->list, strdup(key));
   return EINA_TRUE;
}

/**
 * @brief Sets the object color class.
 *
 * @param obj A valid Evas_Object handle
 * @param color_class
 * @param r Object Red value
 * @param g Object Green value
 * @param b Object Blue value
 * @param a Object Alpha value
 * @param r2 Outline Red value
 * @param g2 Outline Green value
 * @param b2 Outline Blue value
 * @param a2 Outline Alpha value
 * @param r3 Shadow Red value
 * @param g3 Shadow Green value
 * @param b3 Shadow Blue value
 * @param a3 Shadow Alpha value
 *
 * This function sets the color values for an object level color
 * class. This will cause all edje parts in the specified object that
 * have the specified color class to have their colors multiplied by
 * these values.
 *
 * The first color is the object, the second is the text outline, and
 * the third is the text shadow. (Note that the second two only apply
 * to text parts).
 *
 * Setting color emits a signal "color_class,set" with source being
 * the given color.
 *
 * @note unlike Evas, Edje colors are @b not pre-multiplied. That is,
 *       half-transparent white is 255 255 255 128.
 */
EAPI Eina_Bool
edje_object_color_class_set(Evas_Object *obj, const char *color_class, int r, int g, int b, int a, int r2, int g2, int b2, int a2, int r3, int g3, int b3, int a3)
{
   Edje *ed;
   Eina_List *l;
   Edje_Color_Class *cc;
   unsigned int i;

   ed = _edje_fetch(obj);
   if ((!ed) || (!color_class)) return EINA_FALSE;
   if (r < 0)        r = 0;
   else if (r > 255) r = 255;
   if (g < 0)        g = 0;
   else if (g > 255) g = 255;
   if (b < 0)        b = 0;
   else if (b > 255) b = 255;
   if (a < 0)        a = 0;
   else if (a > 255) a = 255;
   color_class = eina_stringshare_add(color_class);
   if (!color_class) return EINA_FALSE;
   EINA_LIST_FOREACH(ed->color_classes, l, cc)
     {
	if (cc->name == color_class)
	  {
	     eina_stringshare_del(color_class);

	     if ((cc->r == r) && (cc->g == g) &&
		 (cc->b == b) && (cc->a == a) &&
		 (cc->r2 == r2) && (cc->g2 == g2) &&
		 (cc->b2 == b2) && (cc->a2 == a2) &&
		 (cc->r3 == r3) && (cc->g3 == g3) &&
		 (cc->b3 == b3) && (cc->a3 == a3))
	       return EINA_TRUE;
	     cc->r = r;
	     cc->g = g;
	     cc->b = b;
	     cc->a = a;
	     cc->r2 = r2;
	     cc->g2 = g2;
	     cc->b2 = b2;
	     cc->a2 = a2;
	     cc->r3 = r3;
	     cc->g3 = g3;
	     cc->b3 = b3;
	     cc->a3 = a3;
	     ed->dirty = 1;
#ifdef EDJE_CALC_CACHE
	     ed->all_part_change = 1;
#endif
	     _edje_recalc(ed);
	     return EINA_TRUE;
	  }
     }
   cc = malloc(sizeof(Edje_Color_Class));
   if (!cc)
     {
	eina_stringshare_del(color_class);
	return EINA_FALSE;
     }
   cc->name = color_class;
   cc->r = r;
   cc->g = g;
   cc->b = b;
   cc->a = a;
   cc->r2 = r2;
   cc->g2 = g2;
   cc->b2 = b2;
   cc->a2 = a2;
   cc->r3 = r3;
   cc->g3 = g3;
   cc->b3 = b3;
   cc->a3 = a3;
   ed->color_classes = eina_list_append(ed->color_classes, cc);
   ed->dirty = 1;
#ifdef EDJE_CALC_CACHE
   ed->all_part_change = 1;
#endif

   for (i = 0; i < ed->table_parts_size; i++)
     {
	Edje_Real_Part *rp;

	rp = ed->table_parts[i];
	if (rp->part->type == EDJE_PART_TYPE_GROUP && rp->swallowed_object)
	  edje_object_color_class_set(rp->swallowed_object, color_class,
                                      r, g, b, a, r2, g2, b2, a2, r3, g3, b3,
                                      a3);
     }

   _edje_recalc(ed);
   _edje_emit(ed, "color_class,set", color_class);
   return EINA_TRUE;
}

/**
 * @brief Gets the object color class.
 *
 * @param obj A valid Evas_Object handle
 * @param color_class
 * @param r Object Red value
 * @param g Object Green value
 * @param b Object Blue value
 * @param a Object Alpha value
 * @param r2 Outline Red value
 * @param g2 Outline Green value
 * @param b2 Outline Blue value
 * @param a2 Outline Alpha value
 * @param r3 Shadow Red value
 * @param g3 Shadow Green value
 * @param b3 Shadow Blue value
 * @param a3 Shadow Alpha value
 *
 * @return EINA_TRUE if found or EINA_FALSE if not found and all
 *         values are zeroed.
 *
 * This function gets the color values for an object level color
 * class. If no explicit object color is set, then global values will
 * be used.
 *
 * The first color is the object, the second is the text outline, and
 * the third is the text shadow. (Note that the second two only apply
 * to text parts).
 *
 * @note unlike Evas, Edje colors are @b not pre-multiplied. That is,
 *       half-transparent white is 255 255 255 128.
 */
EAPI Eina_Bool
edje_object_color_class_get(const Evas_Object *obj, const char *color_class, int *r, int *g, int *b, int *a, int *r2, int *g2, int *b2, int *a2, int *r3, int *g3, int *b3, int *a3)
{
   Edje *ed = _edje_fetch(obj);
   Edje_Color_Class *cc = _edje_color_class_find(ed, color_class);

   if (cc)
     {
#define X(C) if (C) *C = cc->C
#define S(_r, _g, _b, _a) X(_r); X(_g); X(_b); X(_a)
	S(r, g, b, a);
	S(r2, g2, b2, a2);
	S(r3, g3, b3, a3);
#undef S
#undef X
	return EINA_TRUE;
     }
   else
     {
#define X(C) if (C) *C = 0
#define S(_r, _g, _b, _a) X(_r); X(_g); X(_b); X(_a)
	S(r, g, b, a);
	S(r2, g2, b2, a2);
	S(r3, g3, b3, a3);
#undef S
#undef X
	return EINA_FALSE;
     }
}

/**
 * @brief Delete the object color class.
 *
 * @param obj The edje object's reference.
 * @param color_class The color class to be deleted.
 *
 * This function deletes any values at the object level for the
 * specified object and color class.
 *
 * Deleting color emits a signal "color_class,del" with source being
 * the given color.
 */
void
edje_object_color_class_del(Evas_Object *obj, const char *color_class)
{
   Edje *ed;
   Eina_List *l;
   Edje_Color_Class *cc = NULL;
   unsigned int i;

   if (!color_class) return;

   ed = _edje_fetch(obj);
   EINA_LIST_FOREACH(ed->color_classes, l, cc)
     {
	if (!strcmp(cc->name, color_class))
	  {
	     ed->color_classes = eina_list_remove(ed->color_classes, cc);
	     eina_stringshare_del(cc->name);
	     free(cc);
	     return;
	  }
     }

   for (i = 0; i < ed->table_parts_size; i++)
     {
	Edje_Real_Part *rp;

	rp = ed->table_parts[i];
	if (rp->part->type == EDJE_PART_TYPE_GROUP && rp->swallowed_object)
	  edje_object_color_class_del(rp->swallowed_object, color_class);
     }

   ed->dirty = 1;
#ifdef EDJE_CALC_CACHE
   ed->all_part_change = 1;
#endif
   _edje_recalc(ed);
   _edje_emit(ed, "color_class,del", color_class);
}

/**
 * @brief Set the Edje text class.
 *
 * @param text_class The text class name
 * @param font The font name
 * @param size The font size
 *
 * This function sets updates all edje members which belong to this
 * text class with the new font attributes.
 *
 * @see edje_text_class_get().
 *
 */
EAPI Eina_Bool
edje_text_class_set(const char *text_class, const char *font, Evas_Font_Size size)
{
   Eina_List *members;
   Edje_Text_Class *tc;

   if (!text_class) return EINA_FALSE;
   if (!font) font = "";

   tc = eina_hash_find(_edje_text_class_hash, text_class);
   /* Create new text class */
   if (!tc)
     {
        tc = calloc(1, sizeof(Edje_Text_Class));
	if (!tc) return EINA_FALSE;
	tc->name = eina_stringshare_add(text_class);
	if (!tc->name)
	  {
	     free(tc);
	     return EINA_FALSE;
	  }
	if (!_edje_text_class_hash) _edje_text_class_hash = eina_hash_string_superfast_new(NULL);
        eina_hash_add(_edje_text_class_hash, text_class, tc);

	tc->font = eina_stringshare_add(font);
	tc->size = size;
	return EINA_FALSE;
     }

   /* If the class found is the same just return */
   if ((tc->size == size) && (tc->font) && (!strcmp(tc->font, font)))
     return EINA_TRUE;

   /* Update the class found */
   eina_stringshare_del(tc->font);
   tc->font = eina_stringshare_add(font);
   if (!tc->font)
     {
        eina_hash_del(_edje_text_class_hash, text_class, tc);
	free(tc);
	return EINA_FALSE;
     }
   tc->size = size;

   /* Tell all members of the text class to recalc */
   members = eina_hash_find(_edje_text_class_member_hash, text_class);
   while (members)
     {
	Edje *ed;

	ed = eina_list_data_get(members);
	ed->dirty = 1;
	_edje_textblock_style_all_update(ed);
#ifdef EDJE_CALC_CACHE
	ed->text_part_change = 1;
#endif
	_edje_recalc(ed);
	members = eina_list_next(members);
     }
   return EINA_TRUE;
}

/**
 * @brief Delete the text class.
 *
 * @param text_class The text class name string
 *
 * This function deletes any values at the process level for the
 * specified text class.
 *
 */
void
edje_text_class_del(const char *text_class)
{
   Edje_Text_Class *tc;
   Eina_List *members;

   if (!text_class) return;

   tc = eina_hash_find(_edje_text_class_hash, text_class);
   if (!tc) return;

   eina_hash_del(_edje_text_class_hash, text_class, tc);
   eina_stringshare_del(tc->name);
   eina_stringshare_del(tc->font);
   free(tc);

   members = eina_hash_find(_edje_text_class_member_hash, text_class);
   while (members)
     {
	Edje *ed;

	ed = eina_list_data_get(members);
	ed->dirty = 1;
	_edje_textblock_style_all_update(ed);
#ifdef EDJE_CALC_CACHE
	ed->text_part_change = 1;
#endif
	_edje_recalc(ed);
	members = eina_list_next(members);
     }
}

/**
 * @brief List text classes.
 *
 * @return A list of text class names (strings). These strings are
 * stringshares and the list must be free()'d by the caller.
 *
 * This function lists all text classes known about by the current
 * process.
 *
 */
Eina_List *
edje_text_class_list(void)
{
   Edje_List_Foreach_Data fdata;

   memset(&fdata, 0, sizeof(Edje_List_Foreach_Data));
   eina_hash_foreach(_edje_text_class_member_hash,
                     _edje_text_class_list_foreach, &fdata);
   return fdata.list;
}

static Eina_Bool
_edje_text_class_list_foreach(const Eina_Hash *hash __UNUSED__, const void *key, void *data __UNUSED__, void *fdata)
{
   Edje_List_Foreach_Data *fd;

   fd = fdata;
   fd->list = eina_list_append(fd->list, eina_stringshare_add(key));
   return EINA_TRUE;
}

/**
 * @brief Sets Edje text class.
 *
 * @param obj A valid Evas_Object handle
 * @param text_class The text class name
 * @param font Font name
 * @param size Font Size
 *
 * This function sets the text class for the Edje.
 *
 */
EAPI Eina_Bool
edje_object_text_class_set(Evas_Object *obj, const char *text_class, const char *font, Evas_Font_Size size)
{
   Edje *ed;
   Eina_List *l;
   Edje_Text_Class *tc;
   unsigned int i;

   ed = _edje_fetch(obj);
   if ((!ed) || (!text_class)) return EINA_FALSE;

   /* for each text_class in the edje */
   EINA_LIST_FOREACH(ed->text_classes, l, tc)
     {
	if ((tc->name) && (!strcmp(tc->name, text_class)))
	  {
	     /* Match and the same, return */
	     if ((tc->font) && (font) && (!strcmp(tc->font, font)) &&
		 (tc->size == size))
	       return EINA_TRUE;

	     /* No font but size is the same, return */
	     if ((!tc->font) && (!font) && (tc->size == size)) return EINA_TRUE;

	     /* Update new text class properties */
	     if (tc->font) eina_stringshare_del(tc->font);
	     if (font) tc->font = eina_stringshare_add(font);
	     else tc->font = NULL;
	     tc->size = size;

	     /* Update edje */
	     ed->dirty = 1;
#ifdef EDJE_CALC_CACHE
	     ed->text_part_change = 1;
#endif
	     _edje_recalc(ed);
	     return EINA_TRUE;
	  }
     }

   /* No matches, create a new text class */
   tc = calloc(1, sizeof(Edje_Text_Class));
   if (!tc) return EINA_FALSE;
   tc->name = eina_stringshare_add(text_class);
   if (!tc->name)
     {
	free(tc);
	return EINA_FALSE;
     }
   if (font) tc->font = eina_stringshare_add(font);
   else tc->font = NULL;
   tc->size = size;

   for (i = 0; i < ed->table_parts_size; i++)
     {
	Edje_Real_Part *rp;

	rp = ed->table_parts[i];
	if (rp->part->type == EDJE_PART_TYPE_GROUP && rp->swallowed_object)
	  edje_object_text_class_set(rp->swallowed_object, text_class,
                                     font, size);
     }

   /* Add to edje's text class list */
   ed->text_classes = eina_list_append(ed->text_classes, tc);
   ed->dirty = 1;
#ifdef EDJE_CALC_CACHE
   ed->text_part_change = 1;
#endif
   _edje_textblock_style_all_update(ed);
   _edje_recalc(ed);
   return EINA_TRUE;
}

/**
 * @brief Check if Edje part exists.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name to check
 *
 * @return 0 on Error, 1 if Edje part exists.
 *
 * This function returns if a part exists in the edje.
 *
 */
EAPI Eina_Bool
edje_object_part_exists(const Evas_Object *obj, const char *part)
{
   Edje *ed;
   Edje_Real_Part *rp;

   ed = _edje_fetch(obj);
   if ((!ed) || (!part)) return EINA_FALSE;
   rp = _edje_real_part_recursive_get(ed, (char *)part);
   if (!rp) return EINA_FALSE;
   return EINA_TRUE;
}

/**
 * @brief Gets the evas object from a part.
 *
 * @param obj A valid Evas_Object handle
 * @param part The Edje part
 * @return Returns the Evas_Object corresponding to the given part, or
 * NULL on failure (if the part doesn't exist)
 *
 * This functio gets the Evas_Object corresponding to a given part.
 *
 * You should never modify the state of the returned object (with
 * evas_object_move() or evas_object_hide() for example), but you can
 * safely query info about its current state (with
 * evas_object_visible_get() or evas_object_color_get() for example)
 *
 **/
EAPI const Evas_Object *
edje_object_part_object_get(const Evas_Object *obj, const char *part)
{
   Edje *ed;
   Edje_Real_Part *rp;

   ed = _edje_fetch(obj);
   if ((!ed) || (!part)) return NULL;

   /* Need to recalc before providing the object. */
   _edje_recalc_do(ed);

   rp = _edje_real_part_recursive_get(ed, (char *)part);
   if (!rp) return NULL;
   return rp->object;
}

/**
 * @brief Get the geometry of an Edje part.
 *
 * @param obj A valid Evas_Object handle
 * @param part The Edje part
 * @param x The x coordinate pointer
 * @param y The y coordinate pointer
 * @param w The width pointer
 * @param h The height pointer
 *
 * This function gets the geometry of an Edje part.
 *
 * It is valid to pass NULL as any of @a x, @a y, @a w or @a h, whose
 * values you are uninterested in.
 */
EAPI Eina_Bool
edje_object_part_geometry_get(const Evas_Object *obj, const char *part, Evas_Coord *x, Evas_Coord *y, Evas_Coord *w, Evas_Coord *h )
{
   Edje *ed;
   Edje_Real_Part *rp;

   ed = _edje_fetch(obj);
   if ((!ed) || (!part))
     {
	if (x) *x = 0;
	if (y) *y = 0;
	if (w) *w = 0;
	if (h) *h = 0;
	return EINA_FALSE;
     }

   /* Need to recalc before providing the object. */
   _edje_recalc_do(ed);

   rp = _edje_real_part_recursive_get(ed, (char *)part);
   if (!rp)
     {
	if (x) *x = 0;
	if (y) *y = 0;
	if (w) *w = 0;
	if (h) *h = 0;
	return EINA_FALSE;
     }
   if (x) *x = rp->x;
   if (y) *y = rp->y;
   if (w) *w = rp->w;
   if (h) *h = rp->h;
   return EINA_TRUE;
}

/**
 * @brief Set the function that provides item objects for named items in an edje entry text
 *
 * @param obj A valid Evas Object handle
 * @param func The function to call (or NULL to disable) to get item objects
 * @param data The data pointer to pass to the @p func callback
 *
 * Item objects may be deleted any time by Edje, and will be deleted when the
 * Edje object is deleted (or file is set to a new file).
 */
EAPI void
edje_object_item_provider_set(Evas_Object *obj, Edje_Item_Provider_Cb func, void *data)
{
   Edje *ed;

   ed = _edje_fetch(obj);
   if (!ed) return;
   ed->item_provider.func = func;
   ed->item_provider.data = data;
}

/* FIXDOC: New Function */
/**
 * @brief Set the object text callback.
 *
 * @param obj A valid Evas_Object handle
 * @param func The callback function to handle the text change
 * @param data The data associated to the callback function.
 *
 * This function gets the geometry of an Edje part
 *
 * It is valid to pass NULL as any of @a x, @a y, @a w or @a h, whose
 * values you are uninterested in.
 *
 */
EAPI void
edje_object_text_change_cb_set(Evas_Object *obj, Edje_Text_Change_Cb func, void *data)
{
   Edje *ed;
   unsigned int i;

   ed = _edje_fetch(obj);
   if (!ed) return;
   ed->text_change.func = func;
   ed->text_change.data = data;

   for (i = 0; i < ed->table_parts_size; i++)
     {
	Edje_Real_Part *rp;

	rp = ed->table_parts[i];
	if ((rp->part->type == EDJE_PART_TYPE_GROUP) && (rp->swallowed_object))
           edje_object_text_change_cb_set(rp->swallowed_object, func, data);
     }
}

Eina_Bool
_edje_object_part_text_raw_set(Evas_Object *obj, Edje_Real_Part *rp, const char *part, const char *text)
{
   if ((!rp->text.text) && (!text))
     return EINA_FALSE;
   if ((rp->text.text) && (text) &&
       (!strcmp(rp->text.text, text)))
     return EINA_FALSE;
   if (rp->text.text)
     {
	eina_stringshare_del(rp->text.text);
	rp->text.text = NULL;
     }
   if (rp->part->entry_mode > EDJE_ENTRY_EDIT_MODE_NONE)
     _edje_entry_text_markup_set(rp, text);
   else
     if (text) rp->text.text = eina_stringshare_add(text);
   rp->edje->dirty = 1;
#ifdef EDJE_CALC_CACHE
   rp->invalidate = 1;
#endif
   _edje_recalc(rp->edje);
   if (rp->edje->text_change.func)
     rp->edje->text_change.func(rp->edje->text_change.data, obj, part);
   return EINA_TRUE;
}

/** Sets the text for an object part
 * @param obj A valid Evas Object handle
 * @param part The part name
 * @param text The text string
 */
EAPI Eina_Bool
edje_object_part_text_set(Evas_Object *obj, const char *part, const char *text)
{
   Edje *ed;
   Edje_Real_Part *rp;

   ed = _edje_fetch(obj);
   if ((!ed) || (!part)) return EINA_FALSE;
   rp = _edje_real_part_recursive_get(ed, (char *)part);
   if (!rp) return EINA_FALSE;
   if ((rp->part->type != EDJE_PART_TYPE_TEXT) &&
       (rp->part->type != EDJE_PART_TYPE_TEXTBLOCK)) return EINA_FALSE;
   return _edje_object_part_text_raw_set(obj, rp, part, text);
}

/**
 * @brief Return the text of the object part.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 *
 * @return The text string
 *
 * This function returns the text associated to the object part.
 *
 */
EAPI const char *
edje_object_part_text_get(const Evas_Object *obj, const char *part)
{
   Edje *ed;
   Edje_Real_Part *rp;

   ed = _edje_fetch(obj);
   if ((!ed) || (!part)) return NULL;

   /* Need to recalc before providing the object. */
   _edje_recalc_do(ed);

   rp = _edje_real_part_recursive_get(ed, (char *)part);
   if (!rp) return NULL;
   if (rp->part->entry_mode > EDJE_ENTRY_EDIT_MODE_NONE)
     return _edje_entry_text_get(rp);
   else
     {
	if (rp->part->type == EDJE_PART_TYPE_TEXT) return rp->text.text;
	if (rp->part->type == EDJE_PART_TYPE_TEXTBLOCK)
	  return evas_object_textblock_text_markup_get(rp->object);
     }
   return NULL;
}

char *
_edje_text_escape(const char *text)
{
   Eina_Strbuf *txt;
   char *ret;
   const char *text_end;
   size_t text_len;

   if (!text) return NULL;

   txt = eina_strbuf_new();
   text_len = strlen(text);

   text_end = text + text_len;
   while (text < text_end)
     {
	int advance;
	const char *escaped = evas_textblock_string_escape_get(text, &advance);
	if (!escaped)
	  {
	     eina_strbuf_append_char(txt, text[0]);
	     advance = 1;
	  }
	else
	  eina_strbuf_append(txt, escaped);

	text += advance;
     }

   ret = eina_strbuf_string_steal(txt);
   eina_strbuf_free(txt);
   return ret;
}

char *
_edje_text_unescape(const char *text)
{
   Eina_Strbuf *txt;
   char *ret;
   const char *text_end, *last, *escape_start;
   size_t text_len;

   if (!text) return NULL;

   txt = eina_strbuf_new();
   text_len = strlen(text);

   text_end = text + text_len;
   last = text;
   escape_start = NULL;
   for (; text < text_end; text++)
     {
	if (*text == '&')
	  {
	     size_t len;
	     const char *str;

	     if (last)
	       {
		  len = text - last;
		  str = last;
	       }
	     else
	       {
		  len = text - escape_start;
		  str = escape_start;
	       }

	     if (len > 0)
	       eina_strbuf_append_n(txt, str, len);

	     escape_start = text;
	     last = NULL;
	  }
	else if ((*text == ';') && (escape_start))
	  {
	     size_t len;
	     const char *str = evas_textblock_escape_string_range_get(escape_start, text);

	     if (str)
	       len = strlen(str);
	     else
	       {
		  str = escape_start;
		  len = text + 1 - escape_start;
	       }

	     eina_strbuf_append_n(txt, str, len);

	     escape_start = NULL;
	     last = text + 1;
	  }
     }

   if (!last && escape_start)
     last = escape_start;

   if (last && (text > last))
     {
	size_t len = text - last;
	eina_strbuf_append_n(txt, last, len);
     }

   ret = eina_strbuf_string_steal(txt);
   eina_strbuf_free(txt);
   return ret;
}

/**
 * @brief Sets the raw (non escaped) text for an object part.
 *
 * @param obj A valid Evas Object handle
 * @param part The part name
 * @param text_to_escape The text string
 *
 * This funciton will do escape for you if it is a TEXTBLOCK part,
 * that is, if text contain tags, these tags will not be
 * interpreted/parsed by TEXTBLOCK.
 *
 * @see edje_object_part_text_unescaped_get().
 *
 */
EAPI Eina_Bool
edje_object_part_text_unescaped_set(Evas_Object *obj, const char *part, const char *text_to_escape)
{
   Edje *ed;
   Edje_Real_Part *rp;
   Eina_Bool ret = EINA_FALSE;

   ed = _edje_fetch(obj);
   if ((!ed) || (!part)) return ret;
   rp = _edje_real_part_recursive_get(ed, part);
   if (!rp) return ret;
   if (rp->part->type == EDJE_PART_TYPE_TEXT)
     ret = _edje_object_part_text_raw_set(obj, rp, part, text_to_escape);
   else if (rp->part->type == EDJE_PART_TYPE_TEXTBLOCK)
     {
	char *text = _edje_text_escape(text_to_escape);

	ret = _edje_object_part_text_raw_set(obj, rp, part, text);
	free(text);
     }
   return ret;
}

/**
 * @brief Returns the text of the object part, without escaping.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @return The @b allocated text string without escaping, or NULL on
 * problems.
 *
 * This function is the counterpart of
 * edje_object_part_text_unescaped_set(). Please notice that the
 * result is newly allocated memory and should be released with free()
 * when done.
 *
 * @see edje_object_part_text_unescaped_set().
 *
 */
EAPI char *
edje_object_part_text_unescaped_get(const Evas_Object *obj, const char *part)
{
   Edje *ed;
   Edje_Real_Part *rp;

   ed = _edje_fetch(obj);
   if ((!ed) || (!part)) return NULL;

   /* Need to recalc before providing the object. */
   _edje_recalc_do(ed);

   rp = _edje_real_part_recursive_get(ed, (char *)part);
   if (!rp) return NULL;
   if (rp->part->entry_mode > EDJE_ENTRY_EDIT_MODE_NONE)
     {
	const char *t = _edje_entry_text_get(rp);
	return _edje_text_unescape(t);
     }
   else
     {
	if (rp->part->type == EDJE_PART_TYPE_TEXT) return strdup(rp->text.text);
	if (rp->part->type == EDJE_PART_TYPE_TEXTBLOCK)
	  {
	     const char *t = evas_object_textblock_text_markup_get(rp->object);
	     return _edje_text_unescape(t);
	  }
     }
   return NULL;
}

/**
 * @brief Return the selection text of the object part.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @return The text string
 *
 * This function returns selection text of the object part.
 *
 */
EAPI const char *
edje_object_part_text_selection_get(const Evas_Object *obj, const char *part)
{
   Edje *ed;
   Edje_Real_Part *rp;

   ed = _edje_fetch(obj);
   if ((!ed) || (!part)) return NULL;
   rp = _edje_real_part_recursive_get(ed, (char *)part);
   if (!rp) return NULL;
   if (rp->part->entry_mode > EDJE_ENTRY_EDIT_MODE_NONE)
     return _edje_entry_selection_get(rp);
   return NULL;
}

/**
 * @brief Set the selection to be none.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 *
 * This function sets the selection text to be none.
 *
 */
EAPI void
edje_object_part_text_select_none(const Evas_Object *obj, const char *part)
{
   Edje *ed;
   Edje_Real_Part *rp;

   ed = _edje_fetch(obj);
   if ((!ed) || (!part)) return;
   rp = _edje_real_part_recursive_get(ed, (char *)part);
   if (!rp) return;
   if (rp->part->entry_mode > EDJE_ENTRY_EDIT_MODE_NONE)
     _edje_entry_select_none(rp);
}

/**
 * @brief Set the selection to be everything.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 *
 * This function selects all text of the object of the part.
 *
 */
EAPI void
edje_object_part_text_select_all(const Evas_Object *obj, const char *part)
{
   Edje *ed;
   Edje_Real_Part *rp;

   ed = _edje_fetch(obj);
   if ((!ed) || (!part)) return;
   rp = _edje_real_part_recursive_get(ed, (char *)part);
   if (!rp) return;
   if (rp->part->entry_mode > EDJE_ENTRY_EDIT_MODE_NONE)
     _edje_entry_select_all(rp);
}

/**
 * @brief Insert text for an object part.
 *
 * @param obj A valid Evas Object handle
 * @param part The part name
 * @param text The text string
 *
 * This function inserts the text for an object part just before the
 * cursor position.
 *
 */
EAPI void
edje_object_part_text_insert(Evas_Object *obj, const char *part, const char *text)
{
   Edje *ed;
   Edje_Real_Part *rp;

   ed = _edje_fetch(obj);
   if ((!ed) || (!part)) return;
   rp = _edje_real_part_recursive_get(ed, (char *)part);
   if (!rp) return;
   if ((rp->part->type != EDJE_PART_TYPE_TEXTBLOCK)) return;
   if (rp->part->entry_mode <= EDJE_ENTRY_EDIT_MODE_NONE) return;
   _edje_entry_text_markup_insert(rp, text);
   rp->edje->dirty = 1;
#ifdef EDJE_CALC_CACHE
   rp->invalidate = 1;
#endif
   _edje_recalc(rp->edje);
   if (rp->edje->text_change.func)
     rp->edje->text_change.func(rp->edje->text_change.data, obj, part);
}

/**
 * @brief Return a list of char anchor names.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 *
 * @return The list of anchors (const char *), do not modify!
 *
 * This function returns a list of char anchor names.
 *
 */
EAPI const Eina_List *
edje_object_part_text_anchor_list_get(const Evas_Object *obj, const char *part)
{
   Edje *ed;
   Edje_Real_Part *rp;

   ed = _edje_fetch(obj);
   if ((!ed) || (!part)) return NULL;
   rp = _edje_real_part_recursive_get(ed, (char *)part);
   if (!rp) return NULL;
   if (rp->part->entry_mode > EDJE_ENTRY_EDIT_MODE_NONE)
     return _edje_entry_anchors_list(rp);
   return NULL;
}

/**
 * @brief Return a list of Evas_Textblock_Rectangle anchor rectangles.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param anchor The anchor name
 *
 * @return The list of anchor rects (const Evas_Textblock_Rectangle
 * *), do not modify! Geometry is relative to entry part.
 *
 * This function return a list of Evas_Textblock_Rectangle anchor
 * rectangles.
 *
 */
EAPI const Eina_List *
edje_object_part_text_anchor_geometry_get(const Evas_Object *obj, const char *part, const char *anchor)
{
   Edje *ed;
   Edje_Real_Part *rp;

   ed = _edje_fetch(obj);
   if ((!ed) || (!part)) return NULL;
   rp = _edje_real_part_recursive_get(ed, (char *)part);
   if (!rp) return NULL;
   if (rp->part->entry_mode > EDJE_ENTRY_EDIT_MODE_NONE)
     return _edje_entry_anchor_geometry_get(rp, anchor);
   return NULL;
}

/**
 * @brief Return a list of char item names.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 *
 * @return The list of items (const char *), do not modify!
 *
 * This function returns a list of char item names.
 *
 */
EAPI const Eina_List *
edje_object_part_text_item_list_get(const Evas_Object *obj, const char *part)
{
   Edje *ed;
   Edje_Real_Part *rp;

   ed = _edje_fetch(obj);
   if ((!ed) || (!part)) return NULL;
   rp = _edje_real_part_recursive_get(ed, (char *)part);
   if (!rp) return NULL;
   if (rp->part->entry_mode > EDJE_ENTRY_EDIT_MODE_NONE)
     return _edje_entry_items_list(rp);
   return NULL;
}

/**
 * @brief Return item geometry.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param item The item name
 * @param cx Item x return (relative to entry part)
 * @param cy Item y return (relative to entry part)
 * @param cw Item width return
 * @param ch Item height return
 *
 * @return 1 if item exists, 0 if not
 *
 * This function return a list of Evas_Textblock_Rectangle item
 * rectangles.
 *
 */
EAPI Eina_Bool
edje_object_part_text_item_geometry_get(const Evas_Object *obj, const char *part, const char *item, Evas_Coord *cx, Evas_Coord *cy, Evas_Coord *cw, Evas_Coord *ch)
{
   Edje *ed;
   Edje_Real_Part *rp;

   ed = _edje_fetch(obj);
   if ((!ed) || (!part)) return EINA_FALSE;
   rp = _edje_real_part_recursive_get(ed, (char *)part);
   if (!rp) return EINA_FALSE;
   if (rp->part->entry_mode > EDJE_ENTRY_EDIT_MODE_NONE)
     return _edje_entry_item_geometry_get(rp, item, cx, cy, cw, ch);
   return EINA_FALSE;
}

/**
 * @brief Returns the cursor geometry of the part relative to the edje
 * object.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param x Cursor X position
 * @param y Cursor Y position
 * @param w Cursor width
 * @param h Cursor height
 *
 */
EAPI void
edje_object_part_text_cursor_geometry_get(const Evas_Object *obj, const char *part, Evas_Coord *x, Evas_Coord *y, Evas_Coord *w, Evas_Coord *h)
{
   Edje *ed;
   Edje_Real_Part *rp;

   ed = _edje_fetch(obj);
   if (x) *x = 0;
   if (y) *y = 0;
   if (w) *w = 0;
   if (h) *h = 0;
   if ((!ed) || (!part)) return;
   rp = _edje_real_part_recursive_get(ed, (char *)part);
   if (!rp) return;
   if (rp->part->entry_mode > EDJE_ENTRY_EDIT_MODE_NONE)
     {
	_edje_entry_cursor_geometry_get(rp, x, y, w, h);
	if (x) *x -= rp->edje->x;
	if (y) *y -= rp->edje->y;
     }
   return;
}

/**
 * @brief Enables selection if the entry is an EXPLICIT selection mode
 * type.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param allow EINA_TRUE to enable, EINA_FALSE otherwise
 */
EAPI void
edje_object_part_text_select_allow_set(const Evas_Object *obj, const char *part, Eina_Bool allow)
{
   Edje *ed;
   Edje_Real_Part *rp;

   ed = _edje_fetch(obj);
   if ((!ed) || (!part)) return;
   rp = _edje_real_part_recursive_get(ed, (char *)part);
   if (!rp) return;
   if (rp->part->entry_mode > EDJE_ENTRY_EDIT_MODE_NONE)
     _edje_entry_select_allow_set(rp, allow);
}

/**
 * @brief Aborts any selection action on a part.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 */
EAPI void
edje_object_part_text_select_abort(const Evas_Object *obj, const char *part)
{
   Edje *ed;
   Edje_Real_Part *rp;

   ed = _edje_fetch(obj);
   if ((!ed) || (!part)) return;
   rp = _edje_real_part_recursive_get(ed, (char *)part);
   if (!rp) return;
   if (rp->part->entry_mode > EDJE_ENTRY_EDIT_MODE_NONE)
     _edje_entry_select_abort(rp);
}

/**
 * @brief Starts selecting at current cursor position
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 */
EAPI void
edje_object_part_text_select_begin(const Evas_Object *obj, const char *part)
{
   Edje *ed;
   Edje_Real_Part *rp;

   ed = _edje_fetch(obj);
   if ((!ed) || (!part)) return;
   rp = _edje_real_part_recursive_get(ed, (char *)part);
   if (!rp) return;
   if (rp->part->entry_mode > EDJE_ENTRY_EDIT_MODE_NONE)
     _edje_entry_select_begin(rp);
}

/**
 * @brief Extends the current selection to the current cursor position
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 */
EAPI void
edje_object_part_text_select_extend(const Evas_Object *obj, const char *part)
{
   Edje *ed;
   Edje_Real_Part *rp;

   ed = _edje_fetch(obj);
   if ((!ed) || (!part)) return;
   rp = _edje_real_part_recursive_get(ed, (char *)part);
   if (!rp) return;
   if (rp->part->entry_mode > EDJE_ENTRY_EDIT_MODE_NONE)
     _edje_entry_select_extend(rp);
}

/**
 * @brief XX
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 */
EAPI Eina_Bool
edje_object_part_text_cursor_next(Evas_Object *obj, const char *part, Edje_Cursor cur)
{
   Edje *ed;
   Edje_Real_Part *rp;

   ed = _edje_fetch(obj);
   if ((!ed) || (!part)) return EINA_FALSE;
   rp = _edje_real_part_recursive_get(ed, (char *)part);
   if (!rp) return EINA_FALSE;
   if (rp->part->entry_mode > EDJE_ENTRY_EDIT_MODE_NONE)
     {
        return _edje_entry_cursor_next(rp, cur);
     }
   return EINA_FALSE;
}

/**
 * @brief XX
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 */
EAPI Eina_Bool
edje_object_part_text_cursor_prev(Evas_Object *obj, const char *part, Edje_Cursor cur)
{
   Edje *ed;
   Edje_Real_Part *rp;

   ed = _edje_fetch(obj);
   if ((!ed) || (!part)) return EINA_FALSE;
   rp = _edje_real_part_recursive_get(ed, (char *)part);
   if (!rp) return EINA_FALSE;
   if (rp->part->entry_mode > EDJE_ENTRY_EDIT_MODE_NONE)
     {
        return _edje_entry_cursor_prev(rp, cur);
     }
   return EINA_FALSE;
}

/**
 * @brief XX
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 */
EAPI Eina_Bool
edje_object_part_text_cursor_up(Evas_Object *obj, const char *part, Edje_Cursor cur)
{
   Edje *ed;
   Edje_Real_Part *rp;

   ed = _edje_fetch(obj);
   if ((!ed) || (!part)) return EINA_FALSE;
   rp = _edje_real_part_recursive_get(ed, (char *)part);
   if (!rp) return EINA_FALSE;
   if (rp->part->entry_mode > EDJE_ENTRY_EDIT_MODE_NONE)
     {
        return _edje_entry_cursor_up(rp, cur);
     }
   return EINA_FALSE;
}

/**
 * @brief XX
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 */
EAPI Eina_Bool
edje_object_part_text_cursor_down(Evas_Object *obj, const char *part, Edje_Cursor cur)
{
   Edje *ed;
   Edje_Real_Part *rp;

   ed = _edje_fetch(obj);
   if ((!ed) || (!part)) return EINA_FALSE;
   rp = _edje_real_part_recursive_get(ed, (char *)part);
   if (!rp) return EINA_FALSE;
   if (rp->part->entry_mode > EDJE_ENTRY_EDIT_MODE_NONE)
     {
        return _edje_entry_cursor_down(rp, cur);
     }
   return EINA_FALSE;
}

/**
 * @brief XX
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 */
EAPI void
edje_object_part_text_cursor_begin_set(Evas_Object *obj, const char *part, Edje_Cursor cur)
{
   Edje *ed;
   Edje_Real_Part *rp;

   ed = _edje_fetch(obj);
   if ((!ed) || (!part)) return;
   rp = _edje_real_part_recursive_get(ed, (char *)part);
   if (!rp) return;
   if (rp->part->entry_mode > EDJE_ENTRY_EDIT_MODE_NONE)
     {
        _edje_entry_cursor_begin(rp, cur);
     }
}

/**
 * @brief XX
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 */
EAPI void
edje_object_part_text_cursor_end_set(Evas_Object *obj, const char *part, Edje_Cursor cur)
{
   Edje *ed;
   Edje_Real_Part *rp;

   ed = _edje_fetch(obj);
   if ((!ed) || (!part)) return;
   rp = _edje_real_part_recursive_get(ed, (char *)part);
   if (!rp) return;
   if (rp->part->entry_mode > EDJE_ENTRY_EDIT_MODE_NONE)
     {
        _edje_entry_cursor_end(rp, cur);
     }
}

/**
 * @brief XX
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 */
EAPI void
edje_object_part_text_cursor_copy(Evas_Object *obj, const char *part, Edje_Cursor src, Edje_Cursor dst)
{
   Edje *ed;
   Edje_Real_Part *rp;

   ed = _edje_fetch(obj);
   if ((!ed) || (!part)) return;
   rp = _edje_real_part_recursive_get(ed, (char *)part);
   if (!rp) return;
   if (rp->part->entry_mode > EDJE_ENTRY_EDIT_MODE_NONE)
     {
        _edje_entry_cursor_copy(rp, src, dst);
     }
}

/**
 * @brief XX
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 */
EAPI void
edje_object_part_text_cursor_line_begin_set(Evas_Object *obj, const char *part, Edje_Cursor cur)
{
   Edje *ed;
   Edje_Real_Part *rp;

   ed = _edje_fetch(obj);
   if ((!ed) || (!part)) return;
   rp = _edje_real_part_recursive_get(ed, (char *)part);
   if (!rp) return;
   if (rp->part->entry_mode > EDJE_ENTRY_EDIT_MODE_NONE)
     {
        _edje_entry_cursor_line_begin(rp, cur);
     }
}

/**
 * @brief XX
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 */
EAPI void
edje_object_part_text_cursor_line_end_set(Evas_Object *obj, const char *part, Edje_Cursor cur)
{
   Edje *ed;
   Edje_Real_Part *rp;

   ed = _edje_fetch(obj);
   if ((!ed) || (!part)) return;
   rp = _edje_real_part_recursive_get(ed, (char *)part);
   if (!rp) return;
   if (rp->part->entry_mode > EDJE_ENTRY_EDIT_MODE_NONE)
     {
        _edje_entry_cursor_line_end(rp, cur);
     }
}

/**
 * Position the given cursor to a X,Y position.
 *
 * This is frequently used with the user cursor.
 *
 * @param obj An Edje object.
 * @param part The part containing the object.
 * @param cur The cursor to adjust.
 * @param x X Coordinate.
 * @param y Y Coordinate.
 * @return True on success, false on error.
 */
EAPI Eina_Bool
edje_object_part_text_cursor_coord_set(Evas_Object *obj, const char *part,
		Edje_Cursor cur, Evas_Coord x, Evas_Coord y)
{
 Edje *ed;
   Edje_Real_Part *rp;

   ed = _edje_fetch(obj);
   if ((!ed) || (!part)) return EINA_FALSE;
   rp = _edje_real_part_recursive_get(ed, (char *)part);
   if (!rp) return EINA_FALSE;
   if (rp->part->entry_mode > EDJE_ENTRY_EDIT_MODE_NONE)
     {
        return _edje_entry_cursor_coord_set(rp, cur, x, y);
     }
   return EINA_FALSE;
}

/**
 * @brief XX
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 */
EAPI Eina_Bool
edje_object_part_text_cursor_is_format_get(const Evas_Object *obj, const char *part, Edje_Cursor cur)
{
   Edje *ed;
   Edje_Real_Part *rp;

   ed = _edje_fetch(obj);
   if ((!ed) || (!part)) return EINA_FALSE;
   rp = _edje_real_part_recursive_get(ed, (char *)part);
   if (!rp) return EINA_FALSE;
   if (rp->part->entry_mode > EDJE_ENTRY_EDIT_MODE_NONE)
     {
        return _edje_entry_cursor_is_format_get(rp, cur);
     }
   return EINA_FALSE;
}

/**
 * @brief XX
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 */
EAPI Eina_Bool
edje_object_part_text_cursor_is_visible_format_get(const Evas_Object *obj, const char *part, Edje_Cursor cur)
{
   Edje *ed;
   Edje_Real_Part *rp;

   ed = _edje_fetch(obj);
   if ((!ed) || (!part)) return 0;
   rp = _edje_real_part_recursive_get(ed, (char *)part);
   if (!rp) return 0;
   if (rp->part->entry_mode > EDJE_ENTRY_EDIT_MODE_NONE)
     {
        return _edje_entry_cursor_is_visible_format_get(rp, cur);
     }
   return 0;
}

/**
 * @brief XX
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 */
EAPI const char *
edje_object_part_text_cursor_content_get(const Evas_Object *obj, const char *part, Edje_Cursor cur)
{
   Edje *ed;
   Edje_Real_Part *rp;

   ed = _edje_fetch(obj);
   if ((!ed) || (!part)) return NULL;
   rp = _edje_real_part_recursive_get(ed, part);
   if (!rp) return NULL;
   if (rp->part->entry_mode > EDJE_ENTRY_EDIT_MODE_NONE)
     {
        return _edje_entry_cursor_content_get(rp, cur);
     }
   return NULL;
}

/**
 * @brief Sets the cursor position to the given value
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param cur The cursor to move
 * @param pos the position of the cursor
 * @since 1.1.0
 */
EAPI void
edje_object_part_text_cursor_pos_set(Evas_Object *obj, const char *part, Edje_Cursor cur, int pos)
{
   Edje *ed;
   Edje_Real_Part *rp;

   ed = _edje_fetch(obj);
   if ((!ed) || (!part)) return;
   rp = _edje_real_part_recursive_get(ed, part);
   if (!rp) return;
   if (rp->part->entry_mode > EDJE_ENTRY_EDIT_MODE_NONE)
     {
        _edje_entry_cursor_pos_set(rp, cur, pos);
     }
}

/**
 * @brief Retrieves the current position of the cursor
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param cur The cursor to get the position
 * @return The cursor position
 * @since 1.1.0
 */
EAPI int
edje_object_part_text_cursor_pos_get(const Evas_Object *obj, const char *part, Edje_Cursor cur)
{
   Edje *ed;
   Edje_Real_Part *rp;

   ed = _edje_fetch(obj);
   if ((!ed) || (!part)) return 0;
   rp = _edje_real_part_recursive_get(ed, part);
   if (!rp) return 0;
   if (rp->part->entry_mode > EDJE_ENTRY_EDIT_MODE_NONE)
     {
        return _edje_entry_cursor_pos_get(rp, cur);
     }
   return 0;
}

/**
 * Add a filter function for newly inserted text.
 *
 * Whenever text is inserted (not the same as set) into the given @p part,
 * the list of filter functions will be called to decide if and how the new
 * text will be accepted.
 * There are three types of filters, EDJE_TEXT_FILTER_TEXT,
 * EDJE_TEXT_FILTER_FORMAT and EDJE_TEXT_FILTER_MARKUP.
 * The text parameter in the @p func filter can be modified by the user and
 * it's up to him to free the one passed if he's to change the pointer. If
 * doing so, the newly set text should be malloc'ed, as once all the filters
 * are called Edje will free it.
 * If the text is to be rejected, freeing it and setting the pointer to NULL
 * will make Edje break out of the filter cycle and reject the inserted
 * text.
 *
 * @see edje_object_text_insert_filter_callback_del
 * @see edje_object_text_insert_filter_callback_del_full
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param func The callback function that will act as filter
 * @param data User provided data to pass to the filter function
 */
EAPI void
edje_object_text_insert_filter_callback_add(Evas_Object *obj, const char *part, Edje_Text_Filter_Cb func, void *data)
{
   Edje *ed;
   Edje_Text_Insert_Filter_Callback *cb;

   ed = _edje_fetch(obj);
   if ((!ed) || (!part)) return;
   cb = calloc(1, sizeof(Edje_Text_Insert_Filter_Callback));
   cb->part = eina_stringshare_add(part);
   cb->func = func;
   cb->data = (void *)data;
   ed->text_insert_filter_callbacks =
     eina_list_append(ed->text_insert_filter_callbacks, cb);
}

/**
 * Delete a function from the filter list.
 *
 * Delete the given @p func filter from the list in @p part. Returns
 * the user data pointer given when added.
 *
 * @see edje_object_text_insert_filter_callback_add
 * @see edje_object_text_insert_filter_callback_del_full
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param func The function callback to remove
 *
 * @return The user data pointer if succesful, or NULL otherwise
 */
EAPI void *
edje_object_text_insert_filter_callback_del(Evas_Object *obj, const char *part, Edje_Text_Filter_Cb func)
{
   Edje *ed;
   Edje_Text_Insert_Filter_Callback *cb;
   Eina_List *l;

   ed = _edje_fetch(obj);
   if ((!ed) || (!part)) return NULL;
   EINA_LIST_FOREACH(ed->text_insert_filter_callbacks, l, cb)
     {
        if ((!strcmp(cb->part, part)) && (cb->func == func))
          {
             void *data = cb->data;
             ed->text_insert_filter_callbacks =
                eina_list_remove_list(ed->text_insert_filter_callbacks, l);
             eina_stringshare_del(cb->part);
             free(cb);
             return data;
          }
     }
   return NULL;
}

/**
 * Delete a function and matching user data from the filter list.
 *
 * Delete the given @p func filter and @p data user data from the list
 * in @p part.
 * Returns the user data pointer given when added.
 *
 * @see edje_object_text_insert_filter_callback_add
 * @see edje_object_text_insert_filter_callback_del
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param func The function callback to remove
 * @param data The data passed to the callback function
 *
 * @return The same data pointer if succesful, or NULL otherwise
 */
EAPI void *
edje_object_text_insert_filter_callback_del_full(Evas_Object *obj, const char *part, Edje_Text_Filter_Cb func, void *data)
{
   Edje *ed;
   Edje_Text_Insert_Filter_Callback *cb;
   Eina_List *l;

   ed = _edje_fetch(obj);
   if ((!ed) || (!part)) return NULL;
   EINA_LIST_FOREACH(ed->text_insert_filter_callbacks, l, cb)
     {
        if ((!strcmp(cb->part, part)) && (cb->func == func) &&
            (cb->data == data))
          {
             void *data = cb->data;
             ed->text_insert_filter_callbacks =
                eina_list_remove_list(ed->text_insert_filter_callbacks, l);
             eina_stringshare_del(cb->part);
             free(cb);
             return data;
          }
     }
   return NULL;
}

/**
 * @brief Swallows an object into the edje.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param obj_swallow The object to swallow
 *
 * Swallows the object into the edje part so that all geometry changes
 * for the part affect the swallowed object. (e.g. resize, move, show,
 * raise/lower, etc.).
 *
 * If an object has already been swallowed into this part, then it
 * will first be unswallowed before the new object is swallowed.
 */
EAPI Eina_Bool
edje_object_part_swallow(Evas_Object *obj, const char *part, Evas_Object *obj_swallow)
{
   Edje *ed;
   Edje_Real_Part *rp;

   ed = _edje_fetch(obj);
   if ((!ed) || (!part)) return EINA_FALSE;

   /* Need to recalc before providing the object. */
   // XXX: I guess this is not required, removing for testing purposes
   // XXX: uncomment if you see glitches in e17 or others.
   // XXX: by Gustavo, January 21th 2009.
   // XXX: I got a backtrace with over 30000 calls without this,
   // XXX: only with 32px shelves. The problem is probably somewhere else,
   // XXX: but until it's found, leave this here.
   // XXX: by Sachiel, January 21th 2009, 19:30 UTC
   _edje_recalc_do(ed);

   rp = _edje_real_part_recursive_get(ed, (char *)part);
   if (!rp) return EINA_FALSE;
   if (rp->part->type != EDJE_PART_TYPE_SWALLOW)
     {
	ERR("cannot unswallow part %s: not swallow type!", rp->part->name);
	return EINA_FALSE;
     }
   _edje_real_part_swallow(rp, obj_swallow);
   return EINA_TRUE;
}

static void
_recalc_extern_parent(Evas_Object *obj)
{
   Evas_Object *parent;
   Edje *ed;

   parent = evas_object_smart_parent_get(obj);
   ed = _edje_fetch(parent);

   ed->dirty = 1;
   _edje_recalc(ed);
}

/**
 * @brief Set the object minimum size.
 *
 * @param obj A valid Evas_Object handle
 * @param minw The minimum width
 * @param minh The minimum height
 *
 * This sets the minimum size restriction for the object.
 */
EAPI void
edje_extern_object_min_size_set(Evas_Object *obj, Evas_Coord minw, Evas_Coord minh)
{
   Edje_Real_Part *rp;

   evas_object_size_hint_min_set(obj, minw, minh);
   rp = evas_object_data_get(obj, "\377 edje.swallowing_part");
   if (rp)
     {
	rp->swallow_params.min.w = minw;
	rp->swallow_params.min.h = minh;

	_recalc_extern_parent(obj);
     }
}

/**
 * @brief Set the object maximum size.
 *
 * @param obj A valid Evas_Object handle
 * @param maxw The maximum width
 * @param maxh The maximum height
 *
 * This sets the maximum size restriction for the object.
 */
EAPI void
edje_extern_object_max_size_set(Evas_Object *obj, Evas_Coord maxw, Evas_Coord maxh)
{
   Edje_Real_Part *rp;

   evas_object_size_hint_max_set(obj, maxw, maxh);
   rp = evas_object_data_get(obj, "\377 edje.swallowing_part");
   if (rp)
     {
	rp->swallow_params.max.w = maxw;
	rp->swallow_params.max.h = maxh;

	_recalc_extern_parent(obj);
     }
}

/**
 * @brief Set the object aspect size.
 *
 * @param obj A valid Evas_Object handle
 * @param aspect The aspect control axes
 * @param aw The aspect radio width
 * @param ah The aspect ratio height
 *
 * This sets the desired aspect ratio to keep an object that will be
 * swallowed by Edje. The width and height define a preferred size
 * ASPECT and the object may be scaled to be larger or smaller, but
 * retaining the relative scale of both aspect width and height.
 */
EAPI void
edje_extern_object_aspect_set(Evas_Object *obj, Edje_Aspect_Control aspect, Evas_Coord aw, Evas_Coord ah)
{
   Edje_Real_Part *rp;
   Evas_Aspect_Control asp;

   asp = EVAS_ASPECT_CONTROL_NONE;
   switch (aspect)
     {
      case EDJE_ASPECT_CONTROL_NONE: asp = EVAS_ASPECT_CONTROL_NONE; break;
      case EDJE_ASPECT_CONTROL_NEITHER: asp = EVAS_ASPECT_CONTROL_NEITHER; break;
      case EDJE_ASPECT_CONTROL_HORIZONTAL: asp = EVAS_ASPECT_CONTROL_HORIZONTAL; break;
      case EDJE_ASPECT_CONTROL_VERTICAL: asp = EVAS_ASPECT_CONTROL_VERTICAL; break;
      case EDJE_ASPECT_CONTROL_BOTH: asp = EVAS_ASPECT_CONTROL_BOTH; break;
      default: break;
     }
   if (aw < 1) aw = 1;
   if (ah < 1) ah = 1;
   evas_object_size_hint_aspect_set(obj, asp, aw, ah);
   rp = evas_object_data_get(obj, "\377 edje.swallowing_part");
   if (rp)
     {
	rp->swallow_params.aspect.mode = aspect;
	rp->swallow_params.aspect.w = aw;
	rp->swallow_params.aspect.h = ah;
        _recalc_extern_parent(obj);
     }
}

struct edje_box_layout_builtin {
   const char *name;
   Evas_Object_Box_Layout cb;
};

static Evas_Object_Box_Layout
_edje_box_layout_builtin_find(const char *name)
{
   const struct edje_box_layout_builtin _edje_box_layout_builtin[] = {
     {"horizontal", evas_object_box_layout_horizontal},
     {"horizontal_flow", evas_object_box_layout_flow_horizontal},
     {"horizontal_homogeneous", evas_object_box_layout_homogeneous_horizontal},
     {"horizontal_max", evas_object_box_layout_homogeneous_max_size_horizontal},
     {"stack", evas_object_box_layout_stack},
     {"vertical", evas_object_box_layout_vertical},
     {"vertical_flow", evas_object_box_layout_flow_vertical},
     {"vertical_homogeneous", evas_object_box_layout_homogeneous_vertical},
     {"vertical_max", evas_object_box_layout_homogeneous_max_size_vertical},
     {NULL, NULL}
   };
   const struct edje_box_layout_builtin *base;

   switch (name[0])
     {
      case 'h':
	 base = _edje_box_layout_builtin + 0;
	 break;
      case 's':
	 base = _edje_box_layout_builtin + 4;
	 break;
      case 'v':
	 base = _edje_box_layout_builtin + 5;
	 break;
      default:
	 return NULL;
     }

   for (; (base->name) && (base->name[0] == name[0]); base++)
     if (strcmp(base->name, name) == 0)
       return base->cb;

   return NULL;
}

static Eina_Rbtree_Direction
_edje_box_layout_external_node_cmp(const Eina_Rbtree *left, const Eina_Rbtree *right, __UNUSED__ void *data)
{
   Edje_Box_Layout *l = (Edje_Box_Layout *)left;
   Edje_Box_Layout *r = (Edje_Box_Layout *)right;

   if (strcmp(l->name, r->name) < 0)
     return EINA_RBTREE_RIGHT;
   else
     return EINA_RBTREE_LEFT;
}

static int
_edje_box_layout_external_find_cmp(const Eina_Rbtree *node, const void *key, __UNUSED__ int length, __UNUSED__ void *data)
{
   Edje_Box_Layout *l = (Edje_Box_Layout *)node;
   return strcmp(key, l->name);
}

static Edje_Box_Layout *
_edje_box_layout_external_find(const char *name)
{
   return (Edje_Box_Layout *)eina_rbtree_inline_lookup
     (_edje_box_layout_registry, name, 0, _edje_box_layout_external_find_cmp,
      NULL);
}

Eina_Bool
_edje_box_layout_find(const char *name, Evas_Object_Box_Layout *cb, void **data, void (**free_data)(void *data))
{
   const Edje_Box_Layout *l;

   if (!name) return EINA_FALSE;

   *cb = _edje_box_layout_builtin_find(name);
   if (*cb)
     {
	*free_data = NULL;
	*data = NULL;
	return EINA_TRUE;
     }

   l = _edje_box_layout_external_find(name);
   if (!l) return EINA_FALSE;

   *cb = l->func;
   *free_data = l->layout_data_free;
   if (l->layout_data_get)
     *data = l->layout_data_get(l->data);
   else
     *data = NULL;

   return EINA_TRUE;
}

static void
_edje_box_layout_external_free(Eina_Rbtree *node, __UNUSED__ void *data)
{
   Edje_Box_Layout *l = (Edje_Box_Layout *)node;

   if (l->data && l->free_data)
     l->free_data(l->data);
   free(l);
}

static Edje_Box_Layout *
_edje_box_layout_external_new(const char *name, Evas_Object_Box_Layout func, void *(*layout_data_get)(void *), void (*layout_data_free)(void *), void (*free_data)(void *), void *data)
{
   Edje_Box_Layout *l;
   size_t name_len;

   name_len = strlen(name) + 1;
   l = malloc(sizeof(Edje_Box_Layout) + name_len);
   if (!l)
     {
	perror("malloc");
	return NULL;
     }

   l->func = func;
   l->layout_data_get = layout_data_get;
   l->layout_data_free = layout_data_free;
   l->free_data = free_data;
   l->data = data;

   memcpy(l->name, name, name_len);

   return l;
}

/**
 * @brief Registers a custom layout to be used in edje boxes.
 *
 * @param name The name of the layout
 * @param func The function defining the layout
 * @param layout_data_get This function gets the custom data pointer
 * for func
 * @param layout_data_free Passed to func to free its private data
 * when needed
 * @param free_data Frees data
 * @param data Private pointer passed to layout_data_get
 *
 * This function registers custom layouts that can be referred from
 * themes by the registered name. The Evas_Object_Box_Layout
 * functions receive two pointers for internal use, one being private
 * data, and the other the function to free that data when it's not
 * longer needed. From Edje, this private data will be retrieved by
 * calling layout_data_get, and layout_data_free will be the free
 * function passed to func. layout_data_get will be called with data
 * as its parameter, and this one will be freed by free_data whenever
 * the layout is unregistered from Edje.
 */
EAPI void
edje_box_layout_register(const char *name, Evas_Object_Box_Layout func, void *(*layout_data_get)(void *), void (*layout_data_free)(void *), void (*free_data)(void *), void *data)
{
   Edje_Box_Layout *l;

   if (!name) return;

   if (_edje_box_layout_builtin_find(name))
     {
	ERR("Cannot register layout '%s': would override builtin!",
	    name);

	if (data && free_data) free_data(data);
	return;
     }

   l = _edje_box_layout_external_find(name);
   if (!l)
     {
	if (!func)
	  {
	     if (data && free_data) free_data(data);
	     return;
	  }

	l = _edje_box_layout_external_new
	  (name, func, layout_data_get, layout_data_free, free_data, data);
	if (!l)
	  return;

	_edje_box_layout_registry = eina_rbtree_inline_insert
	  (_edje_box_layout_registry, (Eina_Rbtree *)l,
	   _edje_box_layout_external_node_cmp, NULL);
     }
   else
     {
	if (func)
	  {
	     if (l->data && l->free_data) l->free_data(l->data);

	     l->func = func;
	     l->layout_data_get = layout_data_get;
	     l->layout_data_free = layout_data_free;
	     l->free_data = free_data;
	     l->data = data;
	  }
	else
	  {
	     if (data && free_data) free_data(data);

	     _edje_box_layout_registry = eina_rbtree_inline_remove
	       (_edje_box_layout_registry, (Eina_Rbtree *)l,
		_edje_box_layout_external_node_cmp, NULL);
	     _edje_box_layout_external_free((Eina_Rbtree *)l, NULL);
	  }
     }
}

/**
 * @brief Unswallow an object.
 *
 * @param obj A valid Evas_Object handle
 * @param obj_swallow The swallowed object
 *
 * Causes the edje to regurgitate a previously swallowed object.  :)
 */
EAPI void
edje_object_part_unswallow(Evas_Object *obj __UNUSED__, Evas_Object *obj_swallow)
{
   Edje_Real_Part *rp;

   if (!obj_swallow) return;

   rp = (Edje_Real_Part *)evas_object_data_get(obj_swallow, "\377 edje.swallowing_part");
   if (!rp)
     return;
   if (rp->part->type != EDJE_PART_TYPE_SWALLOW)
     {
	ERR("cannot unswallow part %s: not swallow type!", rp->part->name);
	return;
     }
   if (rp->swallowed_object == obj_swallow)
     {
	evas_object_smart_member_del(rp->swallowed_object);
	evas_object_event_callback_del_full(rp->swallowed_object,
                                            EVAS_CALLBACK_FREE,
                                            _edje_object_part_swallow_free_cb,
                                            rp->edje->obj);
	evas_object_event_callback_del_full(rp->swallowed_object,
                                            EVAS_CALLBACK_CHANGED_SIZE_HINTS,
                                            _edje_object_part_swallow_changed_hints_cb,
                                            rp);
	evas_object_clip_unset(rp->swallowed_object);
	evas_object_data_del(rp->swallowed_object, "\377 edje.swallowing_part");

	if (rp->part->mouse_events)
	  _edje_callbacks_del(rp->swallowed_object, rp->edje);
	_edje_callbacks_focus_del(rp->swallowed_object, rp->edje);

	rp->swallowed_object = NULL;
	rp->swallow_params.min.w = 0;
	rp->swallow_params.min.h = 0;
	rp->swallow_params.max.w = 0;
	rp->swallow_params.max.h = 0;
	rp->edje->dirty = 1;
#ifdef EDJE_CALC_CACHE
	rp->invalidate = 1;
#endif
	_edje_recalc_do(rp->edje);
	return;
     }
}

/**
 * @brief Get the object currently swallowed by a part.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @return The swallowed object, or NULL if there is none.
 */
EAPI Evas_Object *
edje_object_part_swallow_get(const Evas_Object *obj, const char *part)
{
   Edje *ed;
   Edje_Real_Part *rp;

   ed = _edje_fetch(obj);
   if ((!ed) || (!part)) return NULL;

   /* Need to recalc before providing the object. */
   _edje_recalc_do(ed);

   rp = _edje_real_part_recursive_get(ed, (char *)part);
   if (!rp) return NULL;
   return rp->swallowed_object;
}

/**
 * @brief Get the minimum size for an object.
 *
 * @param obj A valid Evas_Object handle
 * @param minw Minimum width pointer
 * @param minh Minimum height pointer
 *
 * Gets the object's minimum size values from the Edje. These are set
 * to zero if no Edje is connected to the Evas Object.
 */
EAPI void
edje_object_size_min_get(const Evas_Object *obj, Evas_Coord *minw, Evas_Coord *minh)
{
   Edje *ed;

   ed = _edje_fetch(obj);
   if ((!ed) || (!ed->collection))
     {
	if (minw) *minw = 0;
	if (minh) *minh = 0;
	return;
     }
   if (minw) *minw = ed->collection->prop.min.w;
   if (minh) *minh = ed->collection->prop.min.h;
}

/**
 * @brief Get the maximum size for an object.
 *
 * @param obj A valid Evas_Object handle
 * @param maxw Maximum width pointer
 * @param maxh Maximum height pointer
 *
 * Gets the object's maximum size values from the Edje. These are set
 * to zero if no Edje is connected to the Evas Object.
 */
EAPI void
edje_object_size_max_get(const Evas_Object *obj, Evas_Coord *maxw, Evas_Coord *maxh)
{
   Edje *ed;

   ed = _edje_fetch(obj);
   if ((!ed) || (!ed->collection))
     {
	if (maxw) *maxw = 0;
	if (maxh) *maxh = 0;
	return;
     }

   /* Need to recalc before providing the object. */
   _edje_recalc_do(ed);

   if (ed->collection->prop.max.w == 0)
     {
	/* XXX TODO: convert maxw to 0, fix things that break. */
	if (maxw) *maxw = EDJE_INF_MAX_W;
     }
   else
     {
	if (maxw) *maxw = ed->collection->prop.max.w;
     }
   if (ed->collection->prop.max.h == 0)
     {
	/* XXX TODO: convert maxh to 0, fix things that break. */
	if (maxh) *maxh = EDJE_INF_MAX_H;
     }
   else
     {
	if (maxh) *maxh = ed->collection->prop.max.h;
     }
}

/**
 * @brief Force a Size/Geometry calculation.
 *
 * @param obj A valid Evas_Object handle
 *
 * Forces the object @p obj to recalculation layout regardless of
 * freeze/thaw.
 */
EAPI void
edje_object_calc_force(Evas_Object *obj)
{
   Edje *ed;
   int pf, pf2;

   ed = _edje_fetch(obj);
   if (!ed) return;
   ed->dirty = 1;
#ifdef EDJE_CALC_CACHE
   ed->all_part_change = 1;
#endif

   pf2 = _edje_freeze_val;
   pf = ed->freeze;

   _edje_freeze_val = 0;
   ed->freeze = 0;

   _edje_recalc_do(ed);

   ed->freeze = pf;
   _edje_freeze_val = pf2;
}

/**
 * @brief Calculate minimum size.
 *
 * @param obj A valid Evas_Object handle
 * @param minw Minimum width pointer
 * @param minh Minimum height pointer
 *
 * Calculates the object's minimum size ?!
 */
EAPI void
edje_object_size_min_calc(Evas_Object *obj, Evas_Coord *minw, Evas_Coord *minh)
{
   edje_object_size_min_restricted_calc(obj, minw, minh, 0, 0);
}

/** Calculate the geometry used by all parts
 * @param obj A valid Evas_Object handle
 * @param x The x coordinate pointer
 * @param y The y coordinate pointer
 * @param w The width pointer
 * @param h The height pointer
 *
 * Calculates the geometry used by all object parts. Including out of bounds parts.
 */
EAPI Eina_Bool
edje_object_parts_extends_calc(Evas_Object *obj, Evas_Coord *x, Evas_Coord *y, Evas_Coord *w, Evas_Coord *h)
{
   Edje *ed;
   Evas_Coord x1 = INT_MAX, y1 = INT_MAX;
   Evas_Coord x2 = 0, y2 = 0;
   unsigned int i;

   ed = _edje_fetch(obj);
   if (!ed)
     {
	if (x) *x = 0;
	if (y) *y = 0;
	if (w) *w = 0;
	if (h) *h = 0;
	return EINA_FALSE;
     }

   ed->calc_only = 1;

   /* Need to recalc before providing the object. */
   ed->dirty = 1;
   _edje_recalc_do(ed);

   for (i = 0; i < ed->table_parts_size; i++)
     {
        Edje_Real_Part *rp;
        Evas_Coord rpx1, rpy1;
        Evas_Coord rpx2, rpy2;

        rp = ed->table_parts[i];

	rpx1 = rp->x;
	rpy1 = rp->y;
	rpx2 = rpx1 + rp->w;
	rpy2 = rpy1 + rp->h;

	if (x1 > rpx1) x1 = rpx1;
	if (y1 > rpy1) y1 = rpy1;
	if (x2 < rpx2) x2 = rpx2;
	if (y2 < rpy2) y2 = rpy2;
     }

   ed->calc_only = 0;

   *x = x1;
   *y = y1;
   *w = x2 - x1;
   *h = y2 - y1;

   return EINA_TRUE;
}

/** Calculate minimum size
 * @param obj A valid Evas_Object handle
 * @param minw Minimum width pointer
 * @param minh Minimum height pointer
 * @param restrictedw Do not allow object min width calc to be less than this
 * @param restrictedh Do not allow object min height calc to be less than this
 *
 * Calculates the object's minimum size ?! Be careful the behaviour of this
 * fonction is not really defined when a TEXTBLOCK part is present in the
 * Edje_Object. This may change in futur implementation.
 */
EAPI void
edje_object_size_min_restricted_calc(Evas_Object *obj, Evas_Coord *minw, Evas_Coord *minh, Evas_Coord restrictedw, Evas_Coord restrictedh)
{
   Edje *ed;
   Evas_Coord pw, ph;
   int maxw, maxh;
   int ok;
   int reset_maxwh;
   Edje_Real_Part *pep = NULL;

   ed = _edje_fetch(obj);
   if ((!ed) || (!ed->collection))
     {
	if (minw) *minw = restrictedw;
	if (minh) *minh = restrictedh;
	return;
     }
   reset_maxwh = 1;
   ed->calc_only = 1;
   pw = ed->w;
   ph = ed->h;

   again:
   ed->w = restrictedw;
   ed->h = restrictedh;

   maxw = 0;
   maxh = 0;

   ok = 1;
   while (ok)
     {
	unsigned int i;

	ok = 0;
	ed->dirty = 1;
#ifdef EDJE_CALC_CACHE
	ed->all_part_change = 1;
#endif
	_edje_recalc_do(ed);
	if (reset_maxwh)
	  {
	     maxw = 0;
	     maxh = 0;
	  }
	pep = NULL;
	for (i = 0; i < ed->table_parts_size; i++)
	  {
	     Edje_Real_Part *ep;
	     int w, h;
	     int didw;

	     ep = ed->table_parts[i];
	     w = ep->w - ep->req.w;
	     h = ep->h - ep->req.h;
	     didw = 0;
	     if (ep->chosen_description)
	       {
		  if (!ep->chosen_description->fixed.w)
		    {
		       if (w > maxw)
			 {
			    maxw = w;
			    ok = 1;
			    pep = ep;
			    didw = 1;
			 }
		       if ((ep->part->type == EDJE_PART_TYPE_TEXTBLOCK))
			 {
			    /* FIXME: do something */
			 }
		    }
		  if (!ep->chosen_description->fixed.h)
		    {
		       if (!((ep->part->type == EDJE_PART_TYPE_TEXTBLOCK) &&
			     (!((Edje_Part_Description_Text *)ep->chosen_description)->text.min_x) &&
			     (didw)))
			 {
			    if (h > maxh)
			      {
				 maxh = h;
				 ok = 1;
				 pep = ep;
			      }
			 }
		    }
	       }
	  }
	if (ok)
	  {
	     ed->w += maxw;
	     ed->h += maxh;
	     if (ed->w < restrictedw) ed->w = restrictedw;
	     if (ed->h < restrictedh) ed->h = restrictedh;
	  }
	if ((ed->w > 4000) || (ed->h > 4000))
	  {
             if (pep)
               ERR("file %s, group %s has a non-fixed part '%s'. Adding 'fixed: 1 1;' to source EDC may help. Continuing discarding faulty part.",
                   ed->path, ed->group, pep->part->name);
             else
               ERR("file %s, group %s overflowed 4000x4000 with minimum size of %dx%d. Continuing discarding faulty parts.",
                   ed->path, ed->group, ed->w, ed->h);

	     if (reset_maxwh)
	       {
		  reset_maxwh = 0;
		  goto again;
	       }
	  }
     }
   ed->min.w = ed->w;
   ed->min.h = ed->h;

   if (minw) *minw = ed->min.w;
   if (minh) *minh = ed->min.h;

   ed->w = pw;
   ed->h = ph;
   ed->dirty = 1;
#ifdef EDJE_CALC_CACHE
   ed->all_part_change = 1;
#endif
   _edje_recalc(ed);
   ed->calc_only = 0;
}

/**
 * @brief Returns the state of the Edje part.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param val_ret
 *
 * @return The part state:\n
 * "default" for the default state\n
 * "" for other states
 */
/* FIXME: Correctly return other states */
EAPI const char *
edje_object_part_state_get(const Evas_Object *obj, const char *part, double *val_ret)
{
   Edje *ed;
   Edje_Real_Part *rp;

   ed = _edje_fetch(obj);
   if ((!ed) || (!part))
     {
	if (val_ret) *val_ret = 0;
	return "";
     }

   /* Need to recalc before providing the object. */
   _edje_recalc_do(ed);

   rp = _edje_real_part_recursive_get(ed, (char *)part);
   if (!rp)
     {
	if (val_ret) *val_ret = 0;
	INF("part not found");
	return "";
     }
   if (rp->chosen_description)
     {
	if (val_ret) *val_ret = rp->chosen_description->state.value;
	if (rp->chosen_description->state.name)
	  return rp->chosen_description->state.name;
	return "default";
     }
   else
     {
	if (rp->param1.description)
	  {
	     if (val_ret) *val_ret = rp->param1.description->state.value;
	     if (rp->param1.description->state.name)
	       return rp->param1.description->state.name;
	     return "default";
	  }
     }
   if (val_ret) *val_ret = 0;
   return "";
}

/**
 * @brief Determine dragable directions.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 *
 * @return 0: Not dragable\n
 * 1: Dragable in X direction\n
 * 2: Dragable in Y direction\n
 * 3: Dragable in X & Y directions
 */
EAPI Edje_Drag_Dir
edje_object_part_drag_dir_get(const Evas_Object *obj, const char *part)
{
   Edje *ed;
   Edje_Real_Part *rp;

   ed = _edje_fetch(obj);
   if ((!ed) || (!part)) return EDJE_DRAG_DIR_NONE;

   /* Need to recalc before providing the object. */
   _edje_recalc_do(ed);

   rp = _edje_real_part_recursive_get(ed, (char *)part);
   if (!rp) return EDJE_DRAG_DIR_NONE;
   if ((rp->part->dragable.x) && (rp->part->dragable.y)) return EDJE_DRAG_DIR_XY;
   else if (rp->part->dragable.x) return EDJE_DRAG_DIR_X;
   else if (rp->part->dragable.y) return EDJE_DRAG_DIR_Y;
   return EDJE_DRAG_DIR_NONE;
}

/**
 * @brief Set the dragable object location.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param dx The x value
 * @param dy The y value
 *
 * Places the dragable object at the given location.
 */
EAPI Eina_Bool
edje_object_part_drag_value_set(Evas_Object *obj, const char *part, double dx, double dy)
{
   Edje *ed;
   Edje_Real_Part *rp;

   ed = _edje_fetch(obj);
   if ((!ed) || (!part)) return EINA_FALSE;
   rp = _edje_real_part_recursive_get(ed, (char *)part);
   if (!rp) return EINA_FALSE;
   if (!rp->drag) return EINA_FALSE;
   if (rp->drag->down.count > 0) return EINA_FALSE;
   if (rp->part->dragable.confine_id != -1)
     {
	dx = CLAMP(dx, 0.0, 1.0);
	dy = CLAMP(dy, 0.0, 1.0);
     }
   if (rp->part->dragable.x < 0) dx = 1.0 - dx;
   if (rp->part->dragable.y < 0) dy = 1.0 - dy;
   if ((rp->drag->val.x == FROM_DOUBLE(dx)) && (rp->drag->val.y == FROM_DOUBLE(dy))) return EINA_TRUE;
   rp->drag->val.x = FROM_DOUBLE(dx);
   rp->drag->val.y = FROM_DOUBLE(dy);
#ifdef EDJE_CALC_CACHE
   rp->invalidate = 1;
#endif
   _edje_dragable_pos_set(rp->edje, rp, rp->drag->val.x, rp->drag->val.y);
   _edje_emit(rp->edje, "drag,set", rp->part->name);
   return EINA_TRUE;
}

/**
 * @brief Get the dragable object location.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param dx The X value pointer
 * @param dy The Y value pointer
 *
 * Gets the drag location values.
 */
/* FIXME: Should this be x and y instead of dx/dy? */
EAPI Eina_Bool
edje_object_part_drag_value_get(const Evas_Object *obj, const char *part, double *dx, double *dy)
{
   Edje *ed;
   Edje_Real_Part *rp;
   double ddx, ddy;

   ed = _edje_fetch(obj);
   if ((!ed) || (!part))
     {
	if (dx) *dx = 0;
	if (dy) *dy = 0;
	return EINA_FALSE;
     }

   /* Need to recalc before providing the object. */
   _edje_recalc_do(ed);

   rp = _edje_real_part_recursive_get(ed, (char *)part);
   if (!rp || !rp->drag)
     {
	if (dx) *dx = 0;
	if (dy) *dy = 0;
	return EINA_FALSE;
     }
   ddx = TO_DOUBLE(rp->drag->val.x);
   ddy = TO_DOUBLE(rp->drag->val.y);
   if (rp->part->dragable.x < 0) ddx = 1.0 - ddx;
   if (rp->part->dragable.y < 0) ddy = 1.0 - ddy;
   if (dx) *dx = ddx;
   if (dy) *dy = ddy;
   return EINA_TRUE;
}

/**
 * @brief Set the dragable object size.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param dw The drag width
 * @param dh The drag height
 *
 * Sets the size of the dragable object.
 */
EAPI Eina_Bool
edje_object_part_drag_size_set(Evas_Object *obj, const char *part, double dw, double dh)
{
   Edje *ed;
   Edje_Real_Part *rp;

   ed = _edje_fetch(obj);
   if ((!ed) || (!part)) return EINA_FALSE;
   rp = _edje_real_part_recursive_get(ed, (char *)part);
   if (!rp) return EINA_FALSE;
   if (!rp->drag) return EINA_FALSE;
   if (dw < 0.0) dw = 0.0;
   else if (dw > 1.0) dw = 1.0;
   if (dh < 0.0) dh = 0.0;
   else if (dh > 1.0) dh = 1.0;
   if ((rp->drag->size.x == FROM_DOUBLE(dw)) && (rp->drag->size.y == FROM_DOUBLE(dh))) return EINA_TRUE;
   rp->drag->size.x = FROM_DOUBLE(dw);
   rp->drag->size.y = FROM_DOUBLE(dh);
   rp->edje->dirty = 1;
#ifdef EDJE_CALC_CACHE
   rp->invalidate = 1;
#endif
   _edje_recalc(rp->edje);
   return EINA_TRUE;
}

/**
 * @brief Get the dragable object size.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param dw The drag width pointer
 * @param dh The drag height pointer
 *
 * Gets the dragable object size.
 */
EAPI Eina_Bool
edje_object_part_drag_size_get(const Evas_Object *obj, const char *part, double *dw, double *dh)
{
   Edje *ed;
   Edje_Real_Part *rp;

   ed = _edje_fetch(obj);
   if ((!ed) || (!part))
     {
	if (dw) *dw = 0;
	if (dh) *dh = 0;
	return EINA_FALSE;
     }

   /* Need to recalc before providing the object. */
   _edje_recalc_do(ed);

   rp = _edje_real_part_recursive_get(ed, (char *)part);
   if (!rp || !rp->drag)
     {
	if (dw) *dw = 0;
	if (dh) *dh = 0;
	return EINA_FALSE;
     }
   if (dw) *dw = TO_DOUBLE(rp->drag->size.x);
   if (dh) *dh = TO_DOUBLE(rp->drag->size.y);
   return EINA_TRUE;
}

/**
 * @brief Sets the drag step increment.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param dx The x step amount
 * @param dy The y step amount
 *
 * Sets the x,y step increments for a dragable object.
 */
EAPI Eina_Bool
edje_object_part_drag_step_set(Evas_Object *obj, const char *part, double dx, double dy)
{
   Edje *ed;
   Edje_Real_Part *rp;

   ed = _edje_fetch(obj);
   if ((!ed) || (!part)) return EINA_FALSE;
   rp = _edje_real_part_recursive_get(ed, (char *)part);
   if (!rp) return EINA_FALSE;
   if (!rp->drag) return EINA_FALSE;
   if (dx < 0.0) dx = 0.0;
   else if (dx > 1.0) dx = 1.0;
   if (dy < 0.0) dy = 0.0;
   else if (dy > 1.0) dy = 1.0;
   rp->drag->step.x = FROM_DOUBLE(dx);
   rp->drag->step.y = FROM_DOUBLE(dy);
#ifdef EDJE_CALC_CACHE
   rp->invalidate = 1;
#endif
   return EINA_TRUE;
}

/**
 * @brief Gets the drag step increment values.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part
 * @param dx The x step increment pointer
 * @param dy The y step increment pointer
 *
 * Gets the x and y step increments for the dragable object.
 */
EAPI Eina_Bool
edje_object_part_drag_step_get(const Evas_Object *obj, const char *part, double *dx, double *dy)
{
   Edje *ed;
   Edje_Real_Part *rp;

   ed = _edje_fetch(obj);
   if ((!ed) || (!part))
     {
	if (dx) *dx = 0;
	if (dy) *dy = 0;
	return EINA_FALSE;
     }

   /* Need to recalc before providing the object. */
   _edje_recalc_do(ed);

   rp = _edje_real_part_recursive_get(ed, (char *)part);
   if (!rp || !rp->drag)
     {
	if (dx) *dx = 0;
	if (dy) *dy = 0;
	return EINA_FALSE;
     }
   if (dx) *dx = TO_DOUBLE(rp->drag->step.x);
   if (dy) *dy = TO_DOUBLE(rp->drag->step.y);
   return EINA_TRUE;
}

/**
 * @brief Sets the page step increments.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param dx The x page step increment
 * @param dy The y page step increment
 *
 * Sets the x,y page step increment values.
 */
EAPI Eina_Bool
edje_object_part_drag_page_set(Evas_Object *obj, const char *part, double dx, double dy)
{
   Edje *ed;
   Edje_Real_Part *rp;

   ed = _edje_fetch(obj);
   if ((!ed) || (!part)) return EINA_FALSE;
   rp = _edje_real_part_recursive_get(ed, (char *)part);
   if (!rp) return EINA_FALSE;
   if (!rp->drag) return EINA_FALSE;
   if (dx < 0.0) dx = 0.0;
   else if (dx > 1.0) dx = 1.0;
   if (dy < 0.0) dy = 0.0;
   else if (dy > 1.0) dy = 1.0;
   rp->drag->page.x = FROM_DOUBLE(dx);
   rp->drag->page.y = FROM_DOUBLE(dy);
#ifdef EDJE_CALC_CACHE
   rp->invalidate = 1;
#endif
   return EINA_TRUE;
}

/**
 * @brief Gets the page step increments.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param dx The dx page increment pointer
 * @param dy The dy page increment pointer
 *
 * Gets the x,y page step increments for the dragable object.
 */
EAPI Eina_Bool
edje_object_part_drag_page_get(const Evas_Object *obj, const char *part, double *dx, double *dy)
{
   Edje *ed;
   Edje_Real_Part *rp;

   ed = _edje_fetch(obj);
   if ((!ed) || (!part))
     {
	if (dx) *dx = 0;
	if (dy) *dy = 0;
	return EINA_FALSE;
     }

   /* Need to recalc before providing the object. */
   _edje_recalc_do(ed);

   rp = _edje_real_part_recursive_get(ed, (char *)part);
   if (!rp || !rp->drag)
     {
	if (dx) *dx = 0;
	if (dy) *dy = 0;
	return EINA_FALSE;
     }
   if (dx) *dx = TO_DOUBLE(rp->drag->page.x);
   if (dy) *dy = TO_DOUBLE(rp->drag->page.y);
   return EINA_TRUE;
}

/**
 * @brief Steps the dragable x,y steps.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param dx The x step
 * @param dy The y step
 *
 * Steps x,y where the step increment is the amount set by
 * edje_object_part_drag_step_set.
 */
EAPI Eina_Bool
edje_object_part_drag_step(Evas_Object *obj, const char *part, double dx, double dy)
{
   Edje *ed;
   Edje_Real_Part *rp;
   FLOAT_T px, py;

   ed = _edje_fetch(obj);
   if ((!ed) || (!part)) return EINA_FALSE;
   rp = _edje_real_part_recursive_get(ed, (char *)part);
   if (!rp) return EINA_FALSE;
   if (!rp->drag) return EINA_FALSE;
   if (rp->drag->down.count > 0) return EINA_FALSE;
   px = rp->drag->val.x;
   py = rp->drag->val.y;
   rp->drag->val.x = ADD(px, MUL(FROM_DOUBLE(dx),
				 MUL(rp->drag->step.x, rp->part->dragable.x)));
   rp->drag->val.y = ADD(py, MUL(FROM_DOUBLE(dy),
				 MUL(rp->drag->step.y, rp->part->dragable.y)));
   rp->drag->val.x = CLAMP (rp->drag->val.x, ZERO, FROM_DOUBLE(1.0));
   rp->drag->val.y = CLAMP (rp->drag->val.y, ZERO, FROM_DOUBLE(1.0));
   if ((px == rp->drag->val.x) && (py == rp->drag->val.y)) return EINA_TRUE;
#ifdef EDJE_CALC_CACHE
   rp->invalidate = 1;
#endif
   _edje_dragable_pos_set(rp->edje, rp, rp->drag->val.x, rp->drag->val.y);
   _edje_emit(rp->edje, "drag,step", rp->part->name);
   return EINA_TRUE;
}

/**
 * @brief Pages x,y steps.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param dx The x step
 * @param dy The y step
 *
 * Pages x,y where the increment is defined by
 * edje_object_part_drag_page_set.\n WARNING: Paging is bugged!
 */
EAPI Eina_Bool
edje_object_part_drag_page(Evas_Object *obj, const char *part, double dx, double dy)
{
   Edje *ed;
   Edje_Real_Part *rp;
   FLOAT_T px, py;

   ed = _edje_fetch(obj);
   if ((!ed) || (!part)) return EINA_FALSE;
   rp = _edje_real_part_recursive_get(ed, (char *)part);
   if (!rp) return EINA_FALSE;
   if (!rp->drag) return EINA_FALSE;
   if (rp->drag->down.count > 0) return EINA_FALSE;
   px = rp->drag->val.x;
   py = rp->drag->val.y;
   rp->drag->val.x = ADD(px, MUL(FROM_DOUBLE(dx), MUL(rp->drag->page.x, rp->part->dragable.x)));
   rp->drag->val.y = ADD(py, MUL(FROM_DOUBLE(dy), MUL(rp->drag->page.y, rp->part->dragable.y)));
   rp->drag->val.x = CLAMP (rp->drag->val.x, ZERO, FROM_DOUBLE(1.0));
   rp->drag->val.y = CLAMP (rp->drag->val.y, ZERO, FROM_DOUBLE(1.0));
   if ((px == rp->drag->val.x) && (py == rp->drag->val.y)) return EINA_TRUE;
#ifdef EDJE_CALC_CACHE
   rp->invalidate = 1;
#endif
   _edje_dragable_pos_set(rp->edje, rp, rp->drag->val.x, rp->drag->val.y);
   _edje_emit(rp->edje, "drag,page", rp->part->name);
   return EINA_TRUE;
}

void
_edje_box_init(void)
{

}

void
_edje_box_shutdown(void)
{
   if (!_edje_box_layout_registry)
     return;

   eina_rbtree_delete
     (_edje_box_layout_registry, _edje_box_layout_external_free, NULL);
   _edje_box_layout_registry = NULL;
}

/**
 * @brief Appends an object to the box.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param child The object to append
 *
 * @return 1: Successfully added.\n
 * 0: An error occurred.
 *
 * Appends child to the box indicated by part.
 */
EAPI Eina_Bool
edje_object_part_box_append(Evas_Object *obj, const char *part, Evas_Object *child)
{
   Edje *ed;
   Edje_Real_Part *rp;

   ed = _edje_fetch(obj);
   if ((!ed) || (!part) || (!child)) return EINA_FALSE;

   rp = _edje_real_part_recursive_get(ed, part);
   if (!rp) return EINA_FALSE;
   if (rp->part->type != EDJE_PART_TYPE_BOX) return EINA_FALSE;

   return _edje_real_part_box_append(rp, child);
}

/**
 * @brief Prepends an object to the box.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param child The object to prepend
 *
 * @return 1: Successfully added.\n
 * 0: An error occurred.
 *
 * Prepends child to the box indicated by part.
 */
EAPI Eina_Bool
edje_object_part_box_prepend(Evas_Object *obj, const char *part, Evas_Object *child)
{
   Edje *ed;
   Edje_Real_Part *rp;

   ed = _edje_fetch(obj);
   if ((!ed) || (!part)) return EINA_FALSE;

   rp = _edje_real_part_recursive_get(ed, part);
   if (!rp) return EINA_FALSE;
   if (rp->part->type != EDJE_PART_TYPE_BOX) return EINA_FALSE;

   return _edje_real_part_box_prepend(rp, child);
}

/**
 * @brief Adds an object to the box.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param child The object to insert
 * @param reference The object to be used as reference
 *
 * @return 1: Successfully added.\n
 * 0: An error occurred.
 *
 * Inserts child in the box given by part, in the position marked by
 * reference.
 */
EAPI Eina_Bool
edje_object_part_box_insert_before(Evas_Object *obj, const char *part, Evas_Object *child, const Evas_Object *reference)
{
   Edje *ed;
   Edje_Real_Part *rp;

   ed = _edje_fetch(obj);
   if ((!ed) || (!part)) return EINA_FALSE;

   rp = _edje_real_part_recursive_get(ed, part);
   if (!rp) return EINA_FALSE;
   if (rp->part->type != EDJE_PART_TYPE_BOX) return EINA_FALSE;

   return _edje_real_part_box_insert_before(rp, child, reference);
}

/**
 * @brief Inserts an object to the box.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param child The object to insert
 * @param pos The position where to insert child
 *
 * @return 1: Successfully added.\n
 * 0: An error occurred.
 *
 * Adds child to the box indicated by part, in the position given by
 * pos.
 */
EAPI Eina_Bool
edje_object_part_box_insert_at(Evas_Object *obj, const char *part, Evas_Object *child, unsigned int pos)
{
   Edje *ed;
   Edje_Real_Part *rp;

   ed = _edje_fetch(obj);
   if ((!ed) || (!part)) return EINA_FALSE;

   rp = _edje_real_part_recursive_get(ed, part);
   if (!rp) return EINA_FALSE;
   if (rp->part->type != EDJE_PART_TYPE_BOX) return EINA_FALSE;

   return _edje_real_part_box_insert_at(rp, child, pos);
}

/**
 * @brief Removes an object from the box.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param child The object to remove
 *
 * @return Pointer to the object removed, or NULL.
 *
 * Removes child from the box indicated by part.
 */
EAPI Evas_Object *
edje_object_part_box_remove(Evas_Object *obj, const char *part, Evas_Object *child)
{
   Edje *ed;
   Edje_Real_Part *rp;

   ed = _edje_fetch(obj);
   if ((!ed) || (!part)) return NULL;

   rp = _edje_real_part_recursive_get(ed, part);
   if (!rp) return NULL;
   if (rp->part->type != EDJE_PART_TYPE_BOX) return NULL;

   return _edje_real_part_box_remove(rp, child);
}

/**
 * @brief Removes an object from the box.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param pos
 *
 * @return Pointer to the object removed, or NULL.
 *
 * Removes from the box indicated by part, the object in the position
 * pos.
 */
EAPI Evas_Object *
edje_object_part_box_remove_at(Evas_Object *obj, const char *part, unsigned int pos)
{
   Edje *ed;
   Edje_Real_Part *rp;

   ed = _edje_fetch(obj);
   if ((!ed) || (!part)) return NULL;

   rp = _edje_real_part_recursive_get(ed, part);
   if (!rp) return NULL;
   if (rp->part->type != EDJE_PART_TYPE_BOX) return NULL;

   return _edje_real_part_box_remove_at(rp, pos);
}

/**
 * @brief Removes all elements from the box.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param clear Delete objects on removal
 *
 * @return 1: Successfully cleared.\n
 * 0: An error occurred.
 *
 * Removes all the external objects from the box indicated by part.
 * Elements created from the theme will not be removed.
 */
EAPI Eina_Bool
edje_object_part_box_remove_all(Evas_Object *obj, const char *part, Eina_Bool clear)
{
   Edje *ed;
   Edje_Real_Part *rp;

   ed = _edje_fetch(obj);
   if ((!ed) || (!part)) return EINA_FALSE;

   rp = _edje_real_part_recursive_get(ed, part);
   if (!rp) return EINA_FALSE;
   if (rp->part->type != EDJE_PART_TYPE_BOX) return EINA_FALSE;

   return _edje_real_part_box_remove_all(rp, clear);

}

static void
_edje_box_child_del_cb(void *data, Evas *e __UNUSED__, Evas_Object *child __UNUSED__, void *einfo __UNUSED__)
{
   Edje_Real_Part *rp = data;

   rp->edje->dirty = 1;
#ifdef EDJE_CALC_CACHE
   rp->invalidate = 1;
#endif
   _edje_recalc(rp->edje);
}

static void
_edje_box_child_add(Edje_Real_Part *rp, Evas_Object *child)
{
   evas_object_event_callback_add
     (child, EVAS_CALLBACK_DEL, _edje_box_child_del_cb, rp);

   rp->edje->dirty = 1;
#ifdef EDJE_CALC_CACHE
   rp->invalidate = 1;
#endif
   _edje_recalc(rp->edje);
}

static void
_edje_box_child_remove(Edje_Real_Part *rp, Evas_Object *child)
{
   evas_object_event_callback_del_full
     (child, EVAS_CALLBACK_DEL, _edje_box_child_del_cb, rp);

   rp->edje->dirty = 1;
#ifdef EDJE_CALC_CACHE
   rp->invalidate = 1;
#endif
   _edje_recalc(rp->edje);
}

Eina_Bool
_edje_real_part_box_append(Edje_Real_Part *rp, Evas_Object *child_obj)
{
   Evas_Object_Box_Option *opt;

   opt = evas_object_box_append(rp->object, child_obj);
   if (!opt) return EINA_FALSE;

   if (!_edje_box_layout_add_child(rp, child_obj))
     {
        evas_object_box_remove(rp->object, child_obj);
        return EINA_FALSE;
     }

   _edje_box_child_add(rp, child_obj);

   return EINA_TRUE;
}

Eina_Bool
_edje_real_part_box_prepend(Edje_Real_Part *rp, Evas_Object *child_obj)
{
   Evas_Object_Box_Option *opt;

   opt = evas_object_box_prepend(rp->object, child_obj);
   if (!opt) return EINA_FALSE;

   if (!_edje_box_layout_add_child(rp, child_obj))
     {
        evas_object_box_remove(rp->object, child_obj);
        return EINA_FALSE;
     }

   _edje_box_child_add(rp, child_obj);

   return EINA_TRUE;
}

Eina_Bool
_edje_real_part_box_insert_before(Edje_Real_Part *rp, Evas_Object *child_obj, const Evas_Object *ref)
{
   Evas_Object_Box_Option *opt;

   opt = evas_object_box_insert_before(rp->object, child_obj, ref);
   if (!opt) return EINA_FALSE;

   if (!_edje_box_layout_add_child(rp, child_obj))
     {
        evas_object_box_remove(rp->object, child_obj);
        return EINA_FALSE;
     }

   _edje_box_child_add(rp, child_obj);

   return EINA_TRUE;
}

Eina_Bool
_edje_real_part_box_insert_at(Edje_Real_Part *rp, Evas_Object *child_obj, unsigned int pos)
{
   Evas_Object_Box_Option *opt;

   opt = evas_object_box_insert_at(rp->object, child_obj, pos);
   if (!opt) return EINA_FALSE;

   if (!_edje_box_layout_add_child(rp, child_obj))
     {
        evas_object_box_remove(rp->object, child_obj);
        return EINA_FALSE;
     }

   _edje_box_child_add(rp, child_obj);

   return EINA_TRUE;
}

Evas_Object *
_edje_real_part_box_remove(Edje_Real_Part *rp, Evas_Object *child_obj)
{
   if (evas_object_data_get(child_obj, "\377 edje.box_item")) return NULL;
   if (!evas_object_box_remove(rp->object, child_obj)) return NULL;
   _edje_box_layout_remove_child(rp, child_obj);
   _edje_box_child_remove(rp, child_obj);
   return child_obj;
}

Evas_Object *
_edje_real_part_box_remove_at(Edje_Real_Part *rp, unsigned int pos)
{
   Evas_Object_Box_Option *opt;
   Evas_Object_Box_Data *priv;
   Evas_Object *child_obj;

   priv = evas_object_smart_data_get(rp->object);
   opt = eina_list_nth(priv->children, pos);
   if (!opt) return NULL;
   child_obj = opt->obj;
   if (evas_object_data_get(child_obj, "\377 edje.box_item")) return NULL;
   if (!evas_object_box_remove_at(rp->object, pos)) return NULL;
   _edje_box_layout_remove_child(rp, child_obj);
   _edje_box_child_remove(rp, child_obj);
   return child_obj;
}

Eina_Bool
_edje_real_part_box_remove_all(Edje_Real_Part *rp, Eina_Bool clear)
{
   Eina_List *children;
   int i = 0;

   children = evas_object_box_children_get(rp->object);
   while (children)
     {
	Evas_Object *child_obj = children->data;
	if (evas_object_data_get(child_obj, "\377 edje.box_item"))
	  i++;
	else
	  {
             _edje_box_layout_remove_child(rp, child_obj);
	     _edje_box_child_remove(rp, child_obj);
	     if (!evas_object_box_remove_at(rp->object, i))
	       return EINA_FALSE;
	     if (clear)
	       evas_object_del(child_obj);
	  }
	children = eina_list_remove_list(children, children);
     }
   return EINA_TRUE;
}

static void
_edje_table_child_del_cb(void *data, Evas *e __UNUSED__, Evas_Object *child __UNUSED__, void *einfo __UNUSED__)
{
   Edje_Real_Part *rp = data;

   rp->edje->dirty = 1;
#ifdef EDJE_CALC_CACHE
   rp->invalidate = 1;
#endif
   _edje_recalc(rp->edje);
}

static void
_edje_table_child_add(Edje_Real_Part *rp, Evas_Object *child)
{
   evas_object_event_callback_add
     (child, EVAS_CALLBACK_DEL, _edje_table_child_del_cb, rp);

   rp->edje->dirty = 1;
#ifdef EDJE_CALC_CACHE
   rp->invalidate = 1;
#endif
   _edje_recalc(rp->edje);
}

static void
_edje_table_child_remove(Edje_Real_Part *rp, Evas_Object *child)
{
   evas_object_event_callback_del_full
     (child, EVAS_CALLBACK_DEL, _edje_table_child_del_cb, rp);

   rp->edje->dirty = 1;
#ifdef EDJE_CALC_CACHE
   rp->invalidate = 1;
#endif
   _edje_recalc(rp->edje);
}

/**
 * @brief Retrieve a child from a table
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param col The column of the child to get
 * @param row The row of the child to get
 * @return The child Evas_Object
 */
EAPI Evas_Object *
edje_object_part_table_child_get(Evas_Object *obj, const char *part, unsigned int col, unsigned int row)
{
   Edje *ed;
   Edje_Real_Part *rp;

   ed = _edje_fetch(obj);
   if ((!ed) || (!part)) return NULL;

   rp = _edje_real_part_recursive_get(ed, part);
   if (!rp) return NULL;
   if (rp->part->type != EDJE_PART_TYPE_TABLE) return NULL;

   return evas_object_table_child_get(rp->object, col, row);
}

/**
 * @brief Packs an object into the table.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param child_obj The object to pack in
 * @param col The column to place it in
 * @param row The row to place it in
 * @param colspan Columns the child will take
 * @param rowspan Rows the child will take
 *
 * @return 1: Successfully added.\n
 * 0: An error occurred.
 *
 * Packs an object into the table indicated by part.
 */
EAPI Eina_Bool
edje_object_part_table_pack(Evas_Object *obj, const char *part, Evas_Object *child_obj, unsigned short col, unsigned short row, unsigned short colspan, unsigned short rowspan)
{
   Edje *ed;
   Edje_Real_Part *rp;

   ed = _edje_fetch(obj);
   if ((!ed) || (!part)) return EINA_FALSE;

   rp = _edje_real_part_recursive_get(ed, part);
   if (!rp) return EINA_FALSE;
   if (rp->part->type != EDJE_PART_TYPE_TABLE) return EINA_FALSE;

   return _edje_real_part_table_pack(rp, child_obj, col, row, colspan, rowspan);
}

/**
 * @brief Removes an object from the table.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param child_obj The object to pack in
 *
 * @return 1: Successfully removed.\n
 * 0: An error occurred.
 *
 * Removes an object from the table indicated by part.
 */
EAPI Eina_Bool
edje_object_part_table_unpack(Evas_Object *obj, const char *part, Evas_Object *child_obj)
{
   Edje *ed;
   Edje_Real_Part *rp;

   ed = _edje_fetch(obj);
   if ((!ed) || (!part)) return EINA_FALSE;

   rp = _edje_real_part_recursive_get(ed, part);
   if (!rp) return EINA_FALSE;
   if (rp->part->type != EDJE_PART_TYPE_TABLE) return EINA_FALSE;

   return _edje_real_part_table_unpack(rp, child_obj);
}

/**
 * @brief Gets the number of columns and rows the table has.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param cols Pointer where to store number of columns (can be NULL)
 * @param rows Pointer where to store number of rows (can be NULL)
 *
 * @return 1: Successfully get some data.\n
 * 0: An error occurred.
 *
 * Retrieves the size of the table in number of columns and rows.
 */
EAPI Eina_Bool
edje_object_part_table_col_row_size_get(const Evas_Object *obj, const char *part, int *cols, int *rows)
{
   Edje *ed;
   Edje_Real_Part *rp;

   ed = _edje_fetch(obj);
   if ((!ed) || (!part)) return EINA_FALSE;

   rp = _edje_real_part_recursive_get(ed, part);
   if (!rp) return EINA_FALSE;
   if (rp->part->type != EDJE_PART_TYPE_TABLE) return EINA_FALSE;

   evas_object_table_col_row_size_get(rp->object, cols, rows);
   return EINA_TRUE;
}

/**
 * @brief Removes all object from the table.
 *
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param clear If set, will delete subobjs on remove
 *
 * @return 1: Successfully clear table.\n
 * 0: An error occurred.
 *
 * Removes all object from the table indicated by part, except the
 * internal ones set from the theme.
 */
EAPI Eina_Bool
edje_object_part_table_clear(Evas_Object *obj, const char *part, Eina_Bool clear)
{
   Edje *ed;
   Edje_Real_Part *rp;

   ed = _edje_fetch(obj);
   if ((!ed) || (!part)) return EINA_FALSE;

   rp = _edje_real_part_recursive_get(ed, part);
   if (!rp) return EINA_FALSE;
   if (rp->part->type != EDJE_PART_TYPE_TABLE) return EINA_FALSE;

   _edje_real_part_table_clear(rp, clear);
   return EINA_TRUE;
}

static void
_edje_perspective_obj_del(void *data, __UNUSED__ Evas *e, __UNUSED__ Evas_Object *obj, __UNUSED__ void *event_info)
{
   Edje_Perspective *ps = data;
   Evas_Object *o;

   EINA_LIST_FREE(ps->users, o)
     {
        Edje *ed;

        ed = evas_object_smart_data_get(o);
        if (!ed) continue;
        ed->persp = NULL;
        ed->dirty = 1;
        _edje_recalc_do(ed);
     }
   free(ps);
}

EAPI Edje_Perspective *
edje_perspective_new(Evas *e)
{
   Edje_Perspective *ps;
   Evas_Coord vx, vy, vw, vh;

   if (!e) return NULL;
   ps = calloc(1, sizeof(Edje_Perspective));
   ps->obj = evas_object_rectangle_add(e);
   evas_object_data_set(ps->obj, "_edje_perspective", ps);
   evas_object_event_callback_add(ps->obj, EVAS_CALLBACK_DEL, _edje_perspective_obj_del, ps);
   evas_output_viewport_get(e, &vx, &vy, &vw, &vh);
   ps->e = e;
   ps->px = vx + (vw / 2);
   ps->py = vy + (vh / 2);
   ps->z0 = 0;
   ps->foc = 1000;
   return ps;
}

EAPI void
edje_perspective_free(Edje_Perspective *ps)
{
   if (!ps) return;
   evas_object_del(ps->obj);
}

EAPI void
edje_perspective_set(Edje_Perspective *ps, Evas_Coord px, Evas_Coord py, Evas_Coord z0, Evas_Coord foc)
{
   Eina_List *l;
   Evas_Object *o;

   if (!ps) return;
   if ((ps->px == px) && (ps->py == py) && (ps->z0 == z0) && (ps->foc == foc)) return;
   ps->px = px;
   ps->py = py;
   ps->z0 = z0;
   ps->foc = foc;
   EINA_LIST_FOREACH(ps->users, l, o)
     {
        Edje *ed;

        ed = evas_object_smart_data_get(o);
        if (!ed) continue;
        if (!ed->persp)
          {
             ed->dirty = 1;
             _edje_recalc_do(ed);
          }
     }
   if (ps->global)
     {
        EINA_LIST_FOREACH(_edje_edjes, l, o)
          {
             Edje *ed;

             ed = evas_object_smart_data_get(o);
             if (!ed) continue;
             if (!ed->persp)
               {
                  ed->dirty = 1;
                  _edje_recalc_do(ed);
               }
          }
     }
}

EAPI void
edje_perspective_global_set(Edje_Perspective *ps, Eina_Bool global)
{
   Evas_Object *o;
   Eina_List *l;

   if (!ps) return;
   if (ps->global == global) return;
   if (global)
     {
        o = evas_object_name_find(evas_object_evas_get(ps->obj),
                                  "_edje_perspective");
        if (o) evas_object_name_set(o, NULL);
        evas_object_name_set(ps->obj, "_edje_perspective");
     }
   else
     evas_object_name_set(ps->obj, NULL);
   ps->global = global;
   EINA_LIST_FOREACH(_edje_edjes, l, o)
     {
        Edje *ed;

        ed = evas_object_smart_data_get(o);
        if (!ed) continue;
        if (!ed->persp)
          {
             ed->dirty = 1;
             _edje_recalc_do(ed);
          }
     }
}

EAPI Eina_Bool
edje_perspective_global_get(const Edje_Perspective *ps)
{
   if (!ps) return EINA_FALSE;
   return ps->global;
}

EAPI const Edje_Perspective *
edje_evas_global_perspective_get(const Evas *e)
{
   Evas_Object *obj;

   if (!e) return NULL;
   obj = evas_object_name_find(e, "_edje_perspective");
   if (!obj) return NULL;
   return evas_object_data_get(obj, "_edje_perspective");
}

EAPI void
edje_object_perspective_set(Evas_Object *obj, Edje_Perspective *ps)
{
   Edje *ed;

   ed = evas_object_smart_data_get(obj);
   if (!ed) return;
   if (ed->persp == ps) return;
   if (ed->persp != ps)
     {
        if (ed->persp)
          ed->persp->users = eina_list_remove(ed->persp->users, obj);
     }
   ed->persp = ps;
   if (ps) ps->users = eina_list_append(ps->users, obj);
   ed->dirty = 1;
   _edje_recalc_do(ed);
}

EAPI const Edje_Perspective *
edje_object_perspective_get(const Evas_Object *obj)
{
   Edje *ed;

   ed = evas_object_smart_data_get(obj);
   if (!ed) return NULL;
   return ed->persp;
}

#define EDJE_PRELOAD_EMISSION "preload,done"
#define EDJE_PRELOAD_SOURCE NULL

EAPI Eina_Bool
edje_object_preload(Evas_Object *obj, Eina_Bool cancel)
{
   Edje *ed;
   int count;
   unsigned int i;

   ed = _edje_fetch(obj);
   if (!ed) return EINA_FALSE;

   _edje_recalc_do(ed);

   for (i = 0, count = 0; i < ed->table_parts_size; i++)
     {
	Edje_Real_Part *rp;
	Edje_Part *ep;

	rp = ed->table_parts[i];
	ep = rp->part;

	if (ep->type == EDJE_PART_TYPE_IMAGE ||
	    (ep->type == EDJE_PART_TYPE_GROUP && rp->swallowed_object))
	  count++;
     }

   ed->preload_count = count;

   if (count > 0)
     {
	for (i = 0; i < ed->table_parts_size; i++)
	  {
	     Edje_Real_Part *rp;
	     Edje_Part *ep;

	     rp = ed->table_parts[i];
	     ep = rp->part;

	     if (ep->type == EDJE_PART_TYPE_IMAGE)
	       {
		  const char *file = NULL;
		  const char *key = NULL;

		  evas_object_event_callback_del_full(rp->object, EVAS_CALLBACK_IMAGE_PRELOADED, _edje_object_image_preload_cb, ed);

		  evas_object_image_file_get(rp->object, &file, &key);
		  if (!file && !key)
		    {
		       ed->preload_count--;
		    }
		  else
		    {
		       evas_object_event_callback_add(rp->object, EVAS_CALLBACK_IMAGE_PRELOADED, _edje_object_image_preload_cb, ed);
		       evas_object_image_preload(rp->object, cancel);
		    }
		  count--;
	       }
	     else if (ep->type == EDJE_PART_TYPE_GROUP)
	       {
		  if (rp->swallowed_object) {
		     edje_object_signal_callback_del(rp->swallowed_object, EDJE_PRELOAD_EMISSION, EDJE_PRELOAD_SOURCE, _edje_object_signal_preload_cb);
		     edje_object_signal_callback_add(rp->swallowed_object, EDJE_PRELOAD_EMISSION, EDJE_PRELOAD_SOURCE, _edje_object_signal_preload_cb, ed);
		     edje_object_preload(rp->swallowed_object, cancel);

		     count--;
		  }
	       }
	  }
     }
   else
     {
	_edje_emit(ed, EDJE_PRELOAD_EMISSION, EDJE_PRELOAD_SOURCE);
     }

   return EINA_TRUE;
}

Eina_Bool
_edje_real_part_table_pack(Edje_Real_Part *rp, Evas_Object *child_obj, unsigned short col, unsigned short row, unsigned short colspan, unsigned short rowspan)
{
   Eina_Bool ret =
     evas_object_table_pack(rp->object, child_obj, col, row, colspan, rowspan);

   _edje_table_child_add(rp, child_obj);

   return ret;
}

Eina_Bool
_edje_real_part_table_unpack(Edje_Real_Part *rp, Evas_Object *child_obj)
{
   Eina_Bool ret = evas_object_table_unpack(rp->object, child_obj);

   if (ret)
     _edje_table_child_remove(rp, child_obj);

   return ret;
}

void
_edje_real_part_table_clear(Edje_Real_Part *rp, Eina_Bool clear)
{
   Eina_List *children;

   children = evas_object_table_children_get(rp->object);
   while (children)
     {
	Evas_Object *child_obj = children->data;

	_edje_table_child_remove(rp, child_obj);
	if (!evas_object_data_get(child_obj, "\377 edje.table_item"))
	  {
	     evas_object_table_unpack(rp->object, child_obj);
	     if (clear)
	       evas_object_del(child_obj);
	  }
	children = eina_list_remove_list(children, children);
     }
}

Edje_Real_Part *
_edje_real_part_recursive_get(const Edje *ed, const char *part)
{
   Edje_Real_Part *rp;
   char **path;

   path = eina_str_split(part, EDJE_PART_PATH_SEPARATOR_STRING, 0);
   if (!path) return NULL;

   rp = _edje_real_part_recursive_get_helper(ed, path);

   free(*path);
   free(path);
   return rp;
}

Evas_Object *
_edje_children_get(Edje_Real_Part *rp, const char *partid)
{
   Evas_Object *child;
   Eina_List *l;
   long int v;
   char *p;

   if (!partid) return NULL;

   switch (rp->part->type)
     {
      case EDJE_PART_TYPE_EXTERNAL:
         return _edje_external_content_get(rp->swallowed_object, partid);
      case EDJE_PART_TYPE_BOX:
         l = evas_object_box_children_get(rp->object);
         break;
      case EDJE_PART_TYPE_TABLE:
         l = evas_object_table_children_get(rp->object);
         break;
      default:
         return NULL;
     }

   v = strtol(partid, &p, 10);
   if ((*p == '\0') && (v >= 0))
     {
        child = eina_list_nth(l, v);
     }
   else
     {
        Evas_Object *cur;
        child = NULL;
        EINA_LIST_FREE(l, cur)
          {
             const char *name = evas_object_name_get(cur);
             if ((name) && (!strcmp(name, partid)))
               {
                  child = cur;
                  break;
               }
          }
     }
   eina_list_free(l);

   return child;
}

/* rebuild alternative path */
char *
_edje_merge_path(const char *alias, char * const *path)
{
   char *tmp;
   unsigned int length = 1;
   unsigned int alias_length;
   unsigned int i;

   if (!alias) return NULL;

   alias_length = strlen(alias);

   for (i = 0; path[i]; i++)
     length += strlen(path[i]) + 1;

   tmp = malloc(sizeof (char) * (length + alias_length + 2));
   memcpy(tmp, alias, alias_length);
   tmp[alias_length] = '\0';

   for (i = 0; path[i]; i++)
     {
        strcat(tmp, EDJE_PART_PATH_SEPARATOR_STRING);
        strcat(tmp, path[i]);
     }

   return tmp;
}


Edje_Real_Part *
_edje_real_part_recursive_get_helper(const Edje *ed, char **path)
{
   Edje_Real_Part *rp;
   Evas_Object *child;
   char *idx = NULL;

   if (!path[0])
     return NULL;

   if (ed->collection && ed->collection->alias)
     {
        char *alias;

        alias = _edje_merge_path(eina_hash_find(ed->collection->alias, path[0]), path + 1);
        if (alias) {
           rp = _edje_real_part_recursive_get(ed, alias);
           free(alias);
           return rp;
        }
     }

   //printf("  lookup: %s on %s\n", path[0], ed->parent ? ed->parent : "-");
   idx = strchr(path[0], EDJE_PART_PATH_SEPARATOR_INDEXL);
   if (idx)
     {
	char *end;

	end = strchr(idx + 1, EDJE_PART_PATH_SEPARATOR_INDEXR);
	if (end)
	  {
	     *end = '\0';
	     *idx = '\0';
	     idx++;
	  }
     }

   rp = _edje_real_part_get(ed, path[0]);
   if (!path[1] && !idx) return rp;
   if (!rp) return NULL;

   switch (rp->part->type)
     {
      case EDJE_PART_TYPE_GROUP:
	 if (!rp->swallowed_object) return NULL;
	 ed = _edje_fetch(rp->swallowed_object);
	 if (!ed) return NULL;
	 path++;
	 return _edje_real_part_recursive_get_helper(ed, path);
      case EDJE_PART_TYPE_BOX:
      case EDJE_PART_TYPE_TABLE:
      case EDJE_PART_TYPE_EXTERNAL:
	 if (!idx) return rp;
	 path++;

	 child = _edje_children_get(rp, idx);

         ed = _edje_fetch(child);

	 if (!ed) return NULL;
	 return _edje_real_part_recursive_get_helper(ed, path);
      default:
	 return NULL;
     }
}

/* Private Routines */
Edje_Real_Part *
_edje_real_part_get(const Edje *ed, const char *part)
{
   unsigned int i;

   if (!part) return NULL;

   for (i = 0; i < ed->table_parts_size; i++)
     {
	Edje_Real_Part *rp;

	rp = ed->table_parts[i];
	if ((rp->part->name) && (!strcmp(rp->part->name, part))) return rp;
     }
   return NULL;
}

Edje_Color_Class *
_edje_color_class_find(Edje *ed, const char *color_class)
{
   Eina_List *l;
   Edje_Color_Class *cc = NULL;

   if ((!ed) || (!color_class)) return NULL;

   /* first look through the object scope */
   EINA_LIST_FOREACH(ed->color_classes, l, cc)
     if ((cc->name) && (!strcmp(color_class, cc->name))) return cc;

   /* next look through the global scope */
   cc = eina_hash_find(_edje_color_class_hash, color_class);
   if (cc) return cc;

   /* finally, look through the file scope */
   EINA_LIST_FOREACH(ed->file->color_classes, l, cc)
     if ((cc->name) && (!strcmp(color_class, cc->name))) return cc;

   return NULL;
}

void
_edje_color_class_member_add(Edje *ed, const char *color_class)
{
   Eina_List *members;

   if ((!ed) || (!color_class)) return;
   members = eina_hash_find(_edje_color_class_member_hash, color_class);
   if (members)
     eina_hash_del(_edje_color_class_member_hash, color_class, members);

   members = eina_list_prepend(members, ed);
   if (!_edje_color_class_member_hash) _edje_color_class_member_hash = eina_hash_string_superfast_new(NULL);
   eina_hash_add(_edje_color_class_member_hash, color_class, members);
}

void
_edje_color_class_member_del(Edje *ed, const char *color_class)
{
   Eina_List *members;

   if ((!ed) || (!color_class)) return;
   members = eina_hash_find(_edje_color_class_member_hash, color_class);
   if (!members) return;

   eina_hash_del(_edje_color_class_member_hash, color_class, members);
   members = eina_list_remove(members, ed);
   if (members)
     eina_hash_add(_edje_color_class_member_hash, color_class, members);
}

/**
 * Used to free the member lists that are stored in the text_class and
 * color_class hashtables.
 */
static Eina_Bool
member_list_free(const Eina_Hash *hash __UNUSED__, const void *key __UNUSED__, void *data, void *fdata __UNUSED__)
{
   eina_list_free(data);
   return EINA_TRUE;
}

void
_edje_color_class_members_free(void)
{
   if (!_edje_color_class_member_hash) return;
   eina_hash_foreach(_edje_color_class_member_hash, member_list_free, NULL);
   eina_hash_free(_edje_color_class_member_hash);
   _edje_color_class_member_hash = NULL;
}

static Eina_Bool
color_class_hash_list_free(const Eina_Hash *hash __UNUSED__, const void *key __UNUSED__, void *data, void *fdata __UNUSED__)
{
   Edje_Color_Class *cc;

   cc = data;
   if (cc->name) eina_stringshare_del(cc->name);
   free(cc);
   return EINA_TRUE;
}

void
_edje_color_class_hash_free(void)
{
   if (!_edje_color_class_hash) return;
   eina_hash_foreach(_edje_color_class_hash, color_class_hash_list_free, NULL);
   eina_hash_free(_edje_color_class_hash);
   _edje_color_class_hash = NULL;
}

void
_edje_color_class_on_del(Edje *ed, Edje_Part *ep)
{
   unsigned int i;

   if ((ep->default_desc) && (ep->default_desc->color_class))
     _edje_color_class_member_del(ed, ep->default_desc->color_class);

   for (i = 0; i < ep->other.desc_count; ++i)
     if (ep->other.desc[i]->color_class)
       _edje_color_class_member_del(ed, ep->other.desc[i]->color_class);
}

Edje_Text_Class *
_edje_text_class_find(Edje *ed, const char *text_class)
{
   Eina_List *l;
   Edje_Text_Class *tc;

   if ((!ed) || (!text_class)) return NULL;
   EINA_LIST_FOREACH(ed->text_classes, l, tc)
     if ((tc->name) && (!strcmp(text_class, tc->name))) return tc;
   return eina_hash_find(_edje_text_class_hash, text_class);
}

void
_edje_text_class_member_add(Edje *ed, const char *text_class)
{
   Eina_List *members;

   if ((!ed) || (!text_class)) return;

   /* Get members list */
   members = eina_hash_find(_edje_text_class_member_hash, text_class);

   /* Remove members list */
   if (members)
     eina_hash_del(_edje_text_class_member_hash, text_class, members);

   /* Update the member list */
   members = eina_list_prepend(members, ed);

   /* Add the member list back */
   if (!_edje_text_class_member_hash)
     _edje_text_class_member_hash = eina_hash_string_superfast_new(NULL);
   eina_hash_add(_edje_text_class_member_hash, text_class, members);
}

void
_edje_text_class_member_del(Edje *ed, const char *text_class)
{
   Eina_List *members;

   if ((!ed) || (!text_class)) return;
   members = eina_hash_find(_edje_text_class_member_hash, text_class);
   if (!members) return;

   eina_hash_del(_edje_text_class_member_hash, text_class, members);

   members = eina_list_remove(members, ed);
   if (members)
     eina_hash_add(_edje_text_class_member_hash, text_class, members);
}

void
_edje_text_class_members_free(void)
{
   if (!_edje_text_class_member_hash) return;
   eina_hash_foreach(_edje_text_class_member_hash, member_list_free, NULL);
   eina_hash_free(_edje_text_class_member_hash);
   _edje_text_class_member_hash = NULL;
}

static Eina_Bool
text_class_hash_list_free(const Eina_Hash *hash __UNUSED__, const void *key __UNUSED__, void *data, void *fdata __UNUSED__)
{
   Edje_Text_Class *tc;

   tc = data;
   if (tc->name) eina_stringshare_del(tc->name);
   if (tc->font) eina_stringshare_del(tc->font);
   free(tc);
   return EINA_TRUE;
}

void
_edje_text_class_hash_free(void)
{
   if (!_edje_text_class_hash) return;
   eina_hash_foreach(_edje_text_class_hash, text_class_hash_list_free, NULL);
   eina_hash_free(_edje_text_class_hash);
   _edje_text_class_hash = NULL;
}

Edje *
_edje_fetch(const Evas_Object *obj)
{
   Edje *ed;

   if (!evas_object_smart_type_check(obj, "edje"))
     return NULL;
   ed = evas_object_smart_data_get(obj);
   if ((ed) && (ed->delete_me)) return NULL;
   return ed;
}

int
_edje_freeze(Edje *ed)
{
   ed->freeze++;
//   printf("FREEZE %i\n", ed->freeze);
   return ed->freeze;
}

int
_edje_thaw(Edje *ed)
{
   ed->freeze--;
   if (ed->freeze < 0)
     {
//	printf("-------------########### OVER THAW\n");
	ed->freeze = 0;
     }
   if ((ed->freeze == 0) && (ed->recalc))
     {
//	printf("thaw recalc\n");
	_edje_recalc(ed);
     }
   return ed->freeze;
}

int
_edje_block(Edje *ed)
{
   _edje_ref(ed);
   ed->block++;
   return ed->block;
}

int
_edje_unblock(Edje *ed)
{
   int ret = 0;

   if (!ed) return ret;

   ed->block--;
   if (ed->block == 0) ed->block_break = 0;
   ret = ed->block;
   _edje_unref(ed);
   return ret;
}

int
_edje_block_break(Edje *ed)
{
   if (ed->block_break) return 1;
   return 0;
}

void
_edje_block_violate(Edje *ed)
{
   if (ed->block > 0) ed->block_break = 1;
}

void
_edje_object_part_swallow_free_cb(void *data, Evas *e __UNUSED__, Evas_Object *obj, void *event_info __UNUSED__)
{
   Evas_Object *edje_obj;

   edje_obj = data;
   edje_object_part_unswallow(edje_obj, obj);
   return;
}

static void
_edje_real_part_swallow_hints_update(Edje_Real_Part *rp)
{
   char *type;

   type = (char *)evas_object_type_get(rp->swallowed_object);

   rp->swallow_params.min.w = 0;
   rp->swallow_params.min.h = 0;
   rp->swallow_params.max.w = -1;
   rp->swallow_params.max.h = -1;
   if ((type) && (!strcmp(type, "edje")))
     {
	Evas_Coord w, h;

	edje_object_size_min_get(rp->swallowed_object, &w, &h);
	rp->swallow_params.min.w = w;
	rp->swallow_params.min.h = h;
	edje_object_size_max_get(rp->swallowed_object, &w, &h);
	rp->swallow_params.max.w = w;
	rp->swallow_params.max.h = h;
     }
   else if ((type) && ((!strcmp(type, "text")) || (!strcmp(type, "polygon")) ||
		       (!strcmp(type, "line"))))
     {
	Evas_Coord w, h;

	evas_object_geometry_get(rp->swallowed_object, NULL, NULL, &w, &h);
	rp->swallow_params.min.w = w;
	rp->swallow_params.min.h = h;
	rp->swallow_params.max.w = w;
	rp->swallow_params.max.h = h;
     }
     {
	Evas_Coord w1, h1, w2, h2, aw, ah;
	Evas_Aspect_Control am;

	evas_object_size_hint_min_get(rp->swallowed_object, &w1, &h1);
	evas_object_size_hint_max_get(rp->swallowed_object, &w2, &h2);
	evas_object_size_hint_aspect_get(rp->swallowed_object, &am, &aw, &ah);
	rp->swallow_params.min.w = w1;
	rp->swallow_params.min.h = h1;
	if (w2 > 0) rp->swallow_params.max.w = w2;
	if (h2 > 0) rp->swallow_params.max.h = h2;
  	switch (am)
	  {
	   case EVAS_ASPECT_CONTROL_NONE:
             rp->swallow_params.aspect.mode = EDJE_ASPECT_CONTROL_NONE;
             break;
	   case EVAS_ASPECT_CONTROL_NEITHER:
             rp->swallow_params.aspect.mode = EDJE_ASPECT_CONTROL_NEITHER;
             break;
	   case EVAS_ASPECT_CONTROL_HORIZONTAL:
             rp->swallow_params.aspect.mode = EDJE_ASPECT_CONTROL_HORIZONTAL;
             break;
	   case EVAS_ASPECT_CONTROL_VERTICAL:
             rp->swallow_params.aspect.mode = EDJE_ASPECT_CONTROL_VERTICAL;
             break;
	   case EVAS_ASPECT_CONTROL_BOTH:
             rp->swallow_params.aspect.mode = EDJE_ASPECT_CONTROL_BOTH;
             break;
	   default:
             break;
	  }
	rp->swallow_params.aspect.w = aw;
	rp->swallow_params.aspect.h = ah;
	evas_object_data_set(rp->swallowed_object, "\377 edje.swallowing_part", rp);
     }

#ifdef EDJE_CALC_CACHE
     rp->invalidate = 1;
#endif
}

void
_edje_object_part_swallow_changed_hints_cb(void *data, __UNUSED__ Evas *e, __UNUSED__ Evas_Object *obj, __UNUSED__ void *event_info)
{
   Edje_Real_Part *rp;

   rp = data;
   _edje_real_part_swallow_hints_update(rp);
   rp->edje->dirty = 1;
   _edje_recalc(rp->edje);
   return;
}

void
_edje_real_part_swallow(Edje_Real_Part *rp, Evas_Object *obj_swallow)
{
   if (rp->swallowed_object)
     {
        if (rp->swallowed_object != obj_swallow)
          {
             _edje_real_part_swallow_clear(rp);
             rp->swallowed_object = NULL;
          }
        else
          {
             _edje_real_part_swallow_hints_update(rp);
             rp->edje->dirty = 1;
             _edje_recalc(rp->edje);
             return;
          }
     }
#ifdef EDJE_CALC_CACHE
   rp->invalidate = 1;
#endif
   if (!obj_swallow) return;
   rp->swallowed_object = obj_swallow;
   evas_object_smart_member_add(rp->swallowed_object, rp->edje->obj);
   if (rp->clip_to)
     evas_object_clip_set(rp->swallowed_object, rp->clip_to->object);
   else evas_object_clip_set(rp->swallowed_object, rp->edje->clipper);
   evas_object_stack_above(rp->swallowed_object, rp->object);
   evas_object_event_callback_add(rp->swallowed_object,
                                  EVAS_CALLBACK_FREE,
				  _edje_object_part_swallow_free_cb,
				  rp->edje->obj);
   evas_object_event_callback_add(rp->swallowed_object,
                                  EVAS_CALLBACK_CHANGED_SIZE_HINTS,
				  _edje_object_part_swallow_changed_hints_cb,
				  rp);

   _edje_real_part_swallow_hints_update(rp);

   if (rp->part->mouse_events)
     {
        _edje_callbacks_add(obj_swallow, rp->edje, rp);
	if (rp->part->repeat_events)
           evas_object_repeat_events_set(obj_swallow, 1);
	if (rp->part->pointer_mode != EVAS_OBJECT_POINTER_MODE_AUTOGRAB)
	  evas_object_pointer_mode_set(obj_swallow, rp->part->pointer_mode);
	evas_object_pass_events_set(obj_swallow, 0);
     }
   else
     evas_object_pass_events_set(obj_swallow, 1);
   _edje_callbacks_focus_add(rp->swallowed_object, rp->edje, rp);

   if (rp->part->precise_is_inside)
     evas_object_precise_is_inside_set(obj_swallow, 1);

   rp->edje->dirty = 1;
   _edje_recalc(rp->edje);
}

void
_edje_real_part_swallow_clear(Edje_Real_Part *rp)
{
   evas_object_smart_member_del(rp->swallowed_object);
   evas_object_event_callback_del_full(rp->swallowed_object,
                                       EVAS_CALLBACK_FREE,
                                       _edje_object_part_swallow_free_cb,
                                       rp->edje->obj);
   evas_object_event_callback_del_full(rp->swallowed_object,
                                       EVAS_CALLBACK_CHANGED_SIZE_HINTS,
                                       _edje_object_part_swallow_changed_hints_cb,
                                       rp);
   evas_object_clip_unset(rp->swallowed_object);
   evas_object_data_del(rp->swallowed_object, "\377 edje.swallowing_part");
   if (rp->part->mouse_events)
     _edje_callbacks_del(rp->swallowed_object, rp->edje);
   _edje_callbacks_focus_del(rp->swallowed_object, rp->edje);
}

static void
_edje_object_preload(Edje *ed)
{
   ed->preload_count--;
   if (!ed->preload_count)
     _edje_emit(ed, EDJE_PRELOAD_EMISSION, EDJE_PRELOAD_SOURCE);
}

static void
_edje_object_image_preload_cb(void *data, __UNUSED__ Evas *e, Evas_Object *obj, __UNUSED__ void *event_info)
{
   Edje *ed = data;

   evas_object_event_callback_del_full(obj, EVAS_CALLBACK_IMAGE_PRELOADED, _edje_object_image_preload_cb, ed);
   _edje_object_preload(ed);
}

static void
_edje_object_signal_preload_cb(void *data, Evas_Object *obj, __UNUSED__ const char *emission, __UNUSED__ const char *source)
{
   Edje *ed = data;

   edje_object_signal_callback_del(obj, EDJE_PRELOAD_EMISSION, EDJE_PRELOAD_SOURCE, _edje_object_signal_preload_cb);
   _edje_object_preload(ed);
}

EAPI void
_edje_program_remove(Edje_Part_Collection *edc, Edje_Program *p)
{
   Edje_Program ***array;
   unsigned int *count;
   unsigned int i;

   if (!p->signal && !p->source)
     {
	array = &edc->programs.nocmp;
	count = &edc->programs.nocmp_count;
     }
   else if (p->signal && !strpbrk(p->signal, "*?[\\")
	    && p->source && !strpbrk(p->source, "*?[\\"))
     {
	array = &edc->programs.strcmp;
	count = &edc->programs.strcmp_count;
     }
   else if (p->signal && edje_program_is_strncmp(p->signal)
	    && p->source && edje_program_is_strncmp(p->source))
     {
	array = &edc->programs.strncmp;
	count = &edc->programs.strncmp_count;
     }
   else if (p->signal && edje_program_is_strrncmp(p->signal)
	    && p->source && edje_program_is_strrncmp(p->source))
     {
	array = &edc->programs.strrncmp;
	count = &edc->programs.strrncmp_count;
     }
   else
     {
	array = &edc->programs.fnmatch;
	count = &edc->programs.fnmatch_count;
     }

   for (i = 0; i < *count; ++i)
     if ((*array)[i] == p)
       {
	  memmove(*array + i, *array + i + 1, sizeof (Edje_Program *) * (*count - i -1));
	  (*count)--;
	  break;
       }
}

EAPI void
_edje_program_insert(Edje_Part_Collection *edc, Edje_Program *p)
{
   Edje_Program ***array;
   unsigned int *count;

   if (!p->signal && !p->source)
     {
	array = &edc->programs.nocmp;
	count = &edc->programs.nocmp_count;
     }
   else if (p->signal && !strpbrk(p->signal, "*?[\\")
	    && p->source && !strpbrk(p->source, "*?[\\"))
     {
	array = &edc->programs.strcmp;
	count = &edc->programs.strcmp_count;
     }
   else if (p->signal && edje_program_is_strncmp(p->signal)
	    && p->source && edje_program_is_strncmp(p->source))
     {
	array = &edc->programs.strncmp;
	count = &edc->programs.strncmp_count;
     }
   else if (p->signal && edje_program_is_strrncmp(p->signal)
	    && p->source && edje_program_is_strrncmp(p->source))
     {
	array = &edc->programs.strrncmp;
	count = &edc->programs.strrncmp_count;
     }
   else
     {
	array = &edc->programs.fnmatch;
	count = &edc->programs.fnmatch_count;
     }

   *array = realloc(*array, sizeof (Edje_Program *) * (*count + 1));
   (*array)[(*count)++] = p;
}

const char *
edje_string_get(const Edje_String *es)
{
   /* FIXME: Handle localization here */
   if (!es) return NULL;
   return es->str;
}

const char *
edje_string_id_get(const Edje_String *es)
{
   /* FIXME: Handle localization here */
   if (!es) return NULL;
   return es->str;
}


/* vim:set ts=8 sw=3 sts=3 expandtab cino=>5n-2f0^-2{2(0W1st0 :*/
