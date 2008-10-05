/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#include "edje_private.h"

static Evas_Hash *_edje_color_class_hash = NULL;
static Evas_Hash *_edje_color_class_member_hash = NULL;

static Evas_Hash *_edje_text_class_hash = NULL;
static Evas_Hash *_edje_text_class_member_hash = NULL;

char *_edje_fontset_append = NULL;
double _edje_scale = 1.0;
int _edje_freeze_val = 0;
int _edje_freeze_calc_count = 0;

typedef struct _Edje_List_Foreach_Data Edje_List_Foreach_Data;

struct _Edje_List_Foreach_Data
{
   Evas_List *list;
};

static Evas_Bool _edje_color_class_list_foreach(const Evas_Hash *hash, const char *key, void *data, void *fdata);
static Evas_Bool _edje_text_class_list_foreach(const Evas_Hash *hash, const char *key, void *data, void *fdata);

Edje_Real_Part *_edje_real_part_recursive_get_helper(Edje *ed, char **path);

/************************** API Routines **************************/

/* FIXDOC: These all need to be looked over, Verified/Expanded upon.  I just got lazy and stopped putting FIXDOC next to each function in this file. */

//#define FASTFREEZE 1

/** Freeze all Edje objects in the current process.
 *
 * See edje_object_freeze() for more.
 */
EAPI void
edje_freeze(void)
{
#ifdef FASTFREEZE   
   _edje_freeze_val++;
   printf("fr ++ ->%i\n", _edje_freeze_val);
#else   
// FIXME: could just have a global freeze instead of per object
// above i tried.. but this broke some things. notable e17's menus. why?
   Evas_List *l;

   for (l = _edje_edjes; l; l = l->next)
     edje_object_freeze((Evas_Object *)(l->data));
#endif   
}

#ifdef FASTFREEZE   
static void
_edje_thaw_edje(Edje *ed)
{
   int i;
   
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
   if ((ed->recalc) && (ed->freeze <= 0)) _edje_recalc(ed);
}
#endif

/** Thaw all Edje objects in the current process.
 *
 * See edje_object_thaw() for more.
 */
EAPI void
edje_thaw(void)
{
#ifdef FASTFREEZE   
   _edje_freeze_val--;
   printf("fr -- ->%i\n", _edje_freeze_val);
   if ((_edje_freeze_val <= 0) && (_edje_freeze_calc_count > 0))
     {
	Evas_List *l;

	_edje_freeze_calc_count = 0;
	for (l = _edje_edjes; l; l = l->next)
	  {
	     Edje *ed;
	     
	     ed = _edje_fetch(l->data);
	     if (ed) _edje_thaw_edje(ed);
	  }
     }
#else   
// FIXME: could just have a global freeze instead of per object
// comment as above.. why?
   Evas_List *l;

   for (l = _edje_edjes; l; l = l->next)
     edje_object_thaw((Evas_Object *)(l->data));
#endif   
}

/* FIXDOC: Expand */
EAPI void
edje_fontset_append_set(const char *fonts)
{
   if (_edje_fontset_append)
     free(_edje_fontset_append);
   _edje_fontset_append = fonts ? strdup(fonts) : NULL;
}

/* FIXDOC: Expand */
EAPI const char *
edje_fontset_append_get(void)
{
   return _edje_fontset_append;
}

EAPI void
edje_scale_set(double scale)
{
   Evas_List *l;

   if (_edje_scale == scale) return;
   _edje_scale = scale;
   for (l = _edje_edjes; l; l = l->next)
     edje_object_calc_force((Evas_Object *)(l->data));
}

EAPI double
edje_scale_get(void)
{
   return _edje_scale;
}

/* FIXDOC: Verify/Expand */
/** Get Edje object data
 * @param obj A valid Evas_Object handle
 * @param key The data key
 * @return The data string
 *
 * This fetches data specified at the object level.
 *
 * In EDC this comes from a data block within the group block that @a obj
 * was loaded from. E.g.
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
   Evas_List *l;

   ed = _edje_fetch(obj);
   if ((!ed) || (!key))
     return NULL;
   if (!ed->collection) return NULL;
   for (l = ed->collection->data; l; l = l->next)
     {
	Edje_Data *di;

	di = l->data;
	if ((di->key) && (!strcmp(di->key, key)))
	  return (const char *)di->value;
     }
   return NULL;
}

/* FIXDOC: Verify/Expand */
/** Freeze object
 * @param obj A valid Evas_Object handle
 * @return The frozen state\n
 * 0 on Error
 *
 * This puts all changes on hold.  Successive freezes will nest,
 * requiring an equal number of thaws.
 */
EAPI int
edje_object_freeze(Evas_Object *obj)
{
   Edje *ed;
   int i;

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

/** Thaw object
 * @param obj A valid Evas_Object handle
 * @return The frozen state\n
 * 0 on Error
 *
 * This allows frozen changes to occur.
 */
EAPI int
edje_object_thaw(Evas_Object *obj)
{
   Edje *ed;
   int i;

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

/** Set Edje color class
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
 * Sets the color values for a process level color class. This will cause all
 * edje parts in the current process that have the specified color class to
 * have their colors multiplied by these values. (Object level color classes
 * set by edje_object_color_class_set() will override the values set by this
 * function).
 *
 * The first color is the object, the second is the text outline, and the
 * third is the text shadow. (Note that the second two only apply to text parts)
 */
EAPI void
edje_color_class_set(const char *color_class, int r, int g, int b, int a, int r2, int g2, int b2, int a2, int r3, int g3, int b3, int a3)
{
   Evas_List *members;
   Edje_Color_Class *cc;

   if (!color_class) return;

   cc = evas_hash_find(_edje_color_class_hash, color_class);
   if (!cc)
     {
        cc = calloc(1, sizeof(Edje_Color_Class));
	if (!cc) return;
	cc->name = evas_stringshare_add(color_class);
	if (!cc->name)
	  {
	     free(cc);
	     return;
	  }
	_edje_color_class_hash =
          evas_hash_add(_edje_color_class_hash, color_class, cc);
	if (evas_hash_alloc_error())
	  {
	     evas_stringshare_del(cc->name);
	     free(cc);
	     return;
	  }

     }

   if (r < 0)   r = 0;
   if (r > 255) r = 255;
   if (g < 0)   g = 0;
   if (g > 255) g = 255;
   if (b < 0)   b = 0;
   if (b > 255) b = 255;
   if (a < 0)   a = 0;
   if (a > 255) a = 255;
   if ((cc->r == r) && (cc->g == g) &&
       (cc->b == b) && (cc->a == a) &&
       (cc->r2 == r2) && (cc->g2 == g2) &&
       (cc->b2 == b2) && (cc->a2 == a2) &&
       (cc->r3 == r3) && (cc->g3 == g3) &&
       (cc->b3 == b3) && (cc->a3 == a3))
     return;
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

   members = evas_hash_find(_edje_color_class_member_hash, color_class);
   while (members)
     {
	Edje *ed;

	ed = members->data;
	ed->dirty = 1;
	_edje_recalc(ed);
	members = members->next;
     }
}

/**
 * @param color_class
 *
 * Deletes any values at the process level for the specified color class.
 */
void
edje_color_class_del(const char *color_class)
{
   Edje_Color_Class *cc;
   Evas_List *members;

   if (!color_class) return;

   cc = evas_hash_find(_edje_color_class_hash, color_class);
   if (!cc) return;

   _edje_color_class_hash =
     evas_hash_del(_edje_color_class_hash, color_class, cc);
   evas_stringshare_del(cc->name);
   free(cc);

   members = evas_hash_find(_edje_color_class_member_hash, color_class);
   while (members)
     {
	Edje *ed;

	ed = members->data;
	ed->dirty = 1;
	_edje_recalc(ed);
	members = members->next;
     }
}

/**
 * Lists all color classes known about by the current process.
 *
 * @return A list of color class names (strings). These strings and the list
 * must be free()'d by the caller.
 */
Evas_List *
edje_color_class_list(void)
{
   Edje_List_Foreach_Data fdata;

   memset(&fdata, 0, sizeof(Edje_List_Foreach_Data));
   evas_hash_foreach(_edje_color_class_member_hash,
                     _edje_color_class_list_foreach, &fdata);

   return fdata.list;
}

static Evas_Bool
_edje_color_class_list_foreach(const Evas_Hash *hash, const char *key, void *data, void *fdata)
{
   Edje_List_Foreach_Data *fd;

   fd = fdata;
   fd->list = evas_list_append(fd->list, strdup(key));
   return 1;
}

/** Sets the object color class
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
 * Sets the color values for an object level color class. This will cause all
 * edje parts in the specified object that have the specified color class to
 * have their colors multiplied by these values.
 *
 * The first color is the object, the second is the text outline, and the
 * third is the text shadow. (Note that the second two only apply to text parts)
 */
EAPI void
edje_object_color_class_set(Evas_Object *obj, const char *color_class, int r, int g, int b, int a, int r2, int g2, int b2, int a2, int r3, int g3, int b3, int a3)
{
   Edje *ed;
   Evas_List *l;
   Edje_Color_Class *cc;
   int i;

   ed = _edje_fetch(obj);
   if ((!ed) || (!color_class)) return;
   if (r < 0)   r = 0;
   if (r > 255) r = 255;
   if (g < 0)   g = 0;
   if (g > 255) g = 255;
   if (b < 0)   b = 0;
   if (b > 255) b = 255;
   if (a < 0)   a = 0;
   if (a > 255) a = 255;
   for (l = ed->color_classes; l; l = l->next)
     {
	cc = l->data;
	if ((cc->name) && (!strcmp(cc->name, color_class)))
	  {
	     if ((cc->r == r) && (cc->g == g) &&
		 (cc->b == b) && (cc->a == a) &&
		 (cc->r2 == r2) && (cc->g2 == g2) &&
		 (cc->b2 == b2) && (cc->a2 == a2) &&
		 (cc->r3 == r3) && (cc->g3 == g3) &&
		 (cc->b3 == b3) && (cc->a3 == a3))
	       return;
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
	     _edje_recalc(ed);
	     return;
	  }
     }
   cc = malloc(sizeof(Edje_Color_Class));
   if (!cc) return;
   cc->name = evas_stringshare_add(color_class);
   if (!cc->name)
     {
	free(cc);
	return;
     }
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
   ed->color_classes = evas_list_append(ed->color_classes, cc);
   ed->dirty = 1;

   for (i = 0; i < ed->table_parts_size; i++)
     {
	Edje_Real_Part *rp;
	rp = ed->table_parts[i];
	if (rp->part->type == EDJE_PART_TYPE_GROUP && rp->swallowed_object)
	  edje_object_color_class_set(rp->swallowed_object, color_class, r, g, b, a, r2, g2, b2, a2, r3, g3, b3, a3);
     }

   _edje_recalc(ed);
}

/**
 * @param color_class
 *
 * Deletes any values at the object level for the specified object and
 * color class.
 */
void
edje_object_color_class_del(Evas_Object *obj, const char *color_class)
{
   Edje *ed;
   Evas_List *l;
   Edje_Color_Class *cc = NULL;
   int i;

   if (!color_class) return;

   ed = _edje_fetch(obj);
   for (l = ed->color_classes; l; l = l->next)
     {
	cc = l->data;
	if (!strcmp(cc->name, color_class))
	  {
	     ed->color_classes = evas_list_remove(ed->color_classes, cc);
	     evas_stringshare_del(cc->name);
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
   _edje_recalc(ed);
}

/** Set the Edje text class
 * @param text_class The text class name ?!
 * @param font The font name
 * @param size The font size
 *
 * This sets updates all edje members which belong to this text class
 * with the new font attributes.
 */
EAPI void
edje_text_class_set(const char *text_class, const char *font, Evas_Font_Size size)
{
   Evas_List *members;
   Edje_Text_Class *tc;

   if (!text_class) return;
   if (!font) font = "";

   tc = evas_hash_find(_edje_text_class_hash, text_class);
   /* Create new text class */
   if (!tc)
     {
        tc = calloc(1, sizeof(Edje_Text_Class));
	if (!tc) return;
	tc->name = evas_stringshare_add(text_class);
	if (!tc->name)
	  {
	     free(tc);
	     return;
	  }
	_edje_text_class_hash =
          evas_hash_add(_edje_text_class_hash, text_class, tc);
	if (evas_hash_alloc_error())
	  {
	     evas_stringshare_del(tc->name);
	     free(tc);
	     return;
	  }

	tc->font = evas_stringshare_add(font);
	tc->size = size;
	return;
     }

   /* If the class found is the same just return */
   if ((tc->size == size) && (tc->font) && (!strcmp(tc->font, font)))
     return;

   /* Update the class found */
   evas_stringshare_del(tc->font);
   tc->font = evas_stringshare_add(font);
   if (!tc->font)
     {
	_edje_text_class_hash =
          evas_hash_del(_edje_text_class_hash, text_class, tc);
	free(tc);
	return;
     }
   tc->size = size;

   /* Tell all members of the text class to recalc */
   members = evas_hash_find(_edje_text_class_member_hash, text_class);
   while (members)
     {
	Edje *ed;

	ed = members->data;
	ed->dirty = 1;
	_edje_textblock_style_all_update(ed);
	_edje_recalc(ed);
	members = members->next;
     }
}

/**
 * @param text_class
 *
 * Deletes any values at the process level for the specified text class.
 */
void
edje_text_class_del(const char *text_class)
{
   Edje_Text_Class *tc;
   Evas_List *members;

   if (!text_class) return;

   tc = evas_hash_find(_edje_text_class_hash, text_class);
   if (!tc) return;

   _edje_text_class_hash =
     evas_hash_del(_edje_text_class_hash, text_class, tc);
   evas_stringshare_del(tc->name);
   evas_stringshare_del(tc->font);
   free(tc);

   members = evas_hash_find(_edje_text_class_member_hash, text_class);
   while (members)
     {
	Edje *ed;

	ed = members->data;
	ed->dirty = 1;
	_edje_textblock_style_all_update(ed);
	_edje_recalc(ed);
	members = members->next;
     }
}

/**
 * Lists all text classes known about by the current process.
 *
 * @return A list of text class names (strings). These strings are stringshares and
 * the list must be free()'d by the caller.
 */
Evas_List *
edje_text_class_list(void)
{
   Edje_List_Foreach_Data fdata;

   memset(&fdata, 0, sizeof(Edje_List_Foreach_Data));
   evas_hash_foreach(_edje_text_class_member_hash,
                     _edje_text_class_list_foreach, &fdata);

   return fdata.list;
}

static Evas_Bool
_edje_text_class_list_foreach(const Evas_Hash *hash, const char *key, void *data, void *fdata)
{
   Edje_List_Foreach_Data *fd;

   fd = fdata;
   fd->list = evas_list_append(fd->list, evas_stringshare_add(key));
   return 1;
}

/** Sets Edje text class
 * @param obj A valid Evas_Object handle
 * @param text_class The text class name
 * @param font Font name
 * @param size Font Size
 *
 * Sets the text class for the Edje.
 */
EAPI void
edje_object_text_class_set(Evas_Object *obj, const char *text_class, const char *font, Evas_Font_Size size)
{
   Edje *ed;
   Evas_List *l;
   Edje_Text_Class *tc;
   int i;

   ed = _edje_fetch(obj);
   if ((!ed) || (!text_class)) return;

   /* for each text_class in the edje */
   for (l = ed->text_classes; l; l = l->next)
     {
	tc = l->data;
	if ((tc->name) && (!strcmp(tc->name, text_class)))
	  {
	     /* Match and the same, return */
	     if ((tc->font) && (font) && (!strcmp(tc->font, font)) &&
		 (tc->size == size))
	       return;

	     /* No font but size is the same, return */
	     if ((!tc->font) && (!font) && (tc->size == size)) return;

	     /* Update new text class properties */
	     if (tc->font) evas_stringshare_del(tc->font);
	     if (font) tc->font = evas_stringshare_add(font);
	     else tc->font = NULL;
	     tc->size = size;

	     /* Update edje */
	     ed->dirty = 1;
	     _edje_recalc(ed);
	     return;
	  }
     }

   /* No matches, create a new text class */
   tc = calloc(1, sizeof(Edje_Text_Class));
   if (!tc) return;
   tc->name = evas_stringshare_add(text_class);
   if (!tc->name)
     {
	free(tc);
	return;
     }
   if (font) tc->font = evas_stringshare_add(font);
   else tc->font = NULL;
   tc->size = size;

   for (i = 0; i < ed->table_parts_size; i++)
     {
	Edje_Real_Part *rp;
	rp = ed->table_parts[i];
	if (rp->part->type == EDJE_PART_TYPE_GROUP && rp->swallowed_object)
	  edje_object_text_class_set(rp->swallowed_object, text_class, font, size);
     }

   /* Add to edje's text class list */
   ed->text_classes = evas_list_append(ed->text_classes, tc);
   ed->dirty = 1;
   _edje_textblock_style_all_update(ed);
   _edje_recalc(ed);
}

/** Check if Edje part exists
 * @param obj A valid Evas_Object handle
 * @param part The part name to check
 * @return 0 on Error\n
 * 1 if Edje part exists
 */
EAPI int
edje_object_part_exists(const Evas_Object *obj, const char *part)
{
   Edje *ed;
   Edje_Real_Part *rp;

   ed = _edje_fetch(obj);
   if ((!ed) || (!part)) return 0;
   rp = _edje_real_part_recursive_get(ed, (char *)part);
   if (!rp) return 0;
   return 1;
}

/**
 * Gets the Evas_Object corresponding to a given part.
 * You should never modify the state of the returned object
 * (with evas_object_move() or evas_object_hide() for example),
 * but you can safely query info about its current state
 * (with evas_object_visible_get() or evas_object_color_get() for example)
 *
 * @param obj A valid Evas_Object handle
 * @param part The Edje part
 * @return Returns the Evas_Object corresponding to the given part,
 * or NULL on failure (if the part doesn't exist)
 **/
EAPI const Evas_Object *
edje_object_part_object_get(const Evas_Object *obj, const char *part)
{
   Edje *ed;
   Edje_Real_Part *rp;

   ed = _edje_fetch(obj);
   if ((!ed) || (!part)) return NULL;
   rp = _edje_real_part_recursive_get(ed, (char *)part);
   if (!rp) return NULL;
   return rp->object;
}

/** Get the geometry of an Edje part
 * @param obj A valid Evas_Object handle
 * @param part The Edje part
 * @param x The x coordinate pointer
 * @param y The y coordinate pointer
 * @param w The width pointer
 * @param h The height pointer
 *
 * Gets the geometry of an Edje part
 *
 * It is valid to pass NULL as any of @a x, @a y, @a w or @a h, whose
 * values you are uninterested in.
 */
EAPI void
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
	return;
     }
   rp = _edje_real_part_recursive_get(ed, (char *)part);
   if (!rp)
     {
	if (x) *x = 0;
	if (y) *y = 0;
	if (w) *w = 0;
	if (h) *h = 0;
	return;
     }
   if (x) *x = rp->x;
   if (y) *y = rp->y;
   if (w) *w = rp->w;
   if (h) *h = rp->h;
}

/* FIXDOC: New Function */
EAPI void
edje_object_text_change_cb_set(Evas_Object *obj, void (*func) (void *data, Evas_Object *obj, const char *part), void *data)
{
   Edje *ed;
   int i;

   ed = _edje_fetch(obj);
   if (!ed) return;
   ed->text_change.func = func;
   ed->text_change.data = data;

   for (i = 0; i < ed->table_parts_size; i++)
     {
	Edje_Real_Part *rp;
	rp = ed->table_parts[i];
	if (rp->part->type == EDJE_PART_TYPE_GROUP && rp->swallowed_object)
	  edje_object_text_change_cb_set(rp->swallowed_object, func, data);
     }
}

/** Sets the text for an object part
 * @param obj A valid Evas Object handle
 * @param part The part name
 * @param text The text string
 */
EAPI void
edje_object_part_text_set(Evas_Object *obj, const char *part, const char *text)
{
   Edje *ed;
   Edje_Real_Part *rp;

   ed = _edje_fetch(obj);
   if ((!ed) || (!part)) return;
   rp = _edje_real_part_recursive_get(ed, (char *)part);
   if (!rp) return;
   if ((rp->part->type != EDJE_PART_TYPE_TEXT) &&
       (rp->part->type != EDJE_PART_TYPE_TEXTBLOCK)) return;
   if ((!rp->text.text) && (!text))
     return;
   if ((rp->text.text) && (text) &&
       (!strcmp(rp->text.text, text)))
     return;
   if (rp->text.text) evas_stringshare_del(rp->text.text);
   if (text) rp->text.text = evas_stringshare_add(text);
   else rp->text.text = NULL;
   rp->edje->dirty = 1;
   _edje_recalc(rp->edje);
   if (rp->edje->text_change.func)
     rp->edje->text_change.func(rp->edje->text_change.data, obj, part);
}

/** Returns the text of the object part
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @return The text string
 */
EAPI const char *
edje_object_part_text_get(const Evas_Object *obj, const char *part)
{
   Edje *ed;
   Edje_Real_Part *rp;

   ed = _edje_fetch(obj);
   if ((!ed) || (!part)) return NULL;
   rp = _edje_real_part_recursive_get(ed, (char *)part);
   if (!rp) return NULL;
   if (rp->part->type == EDJE_PART_TYPE_TEXT)
     return rp->text.text;
   return NULL;
}

/** Swallows an object into the edje
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param obj_swallow The object to swallow
 *
 * Swallows the object into the edje part so that all geometry changes
 * for the part affect the swallowed object. (e.g. resize, move, show,
 * raise/lower, etc.).
 *
 * If an object has already been swallowed into this part, then it will
 * first be unswallowed before the new object is swallowed.
 */
EAPI void
edje_object_part_swallow(Evas_Object *obj, const char *part, Evas_Object *obj_swallow)
{
   Edje *ed;
   Edje_Real_Part *rp;

   ed = _edje_fetch(obj);
   if ((!ed) || (!part)) return;
   rp = _edje_real_part_recursive_get(ed, (char *)part);
   if (!rp) return;
   if (rp->part->type != EDJE_PART_TYPE_SWALLOW) return;
   _edje_real_part_swallow(rp, obj_swallow);
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

/** Set the object minimum size
 * @param obj A valid Evas_Object handle
 * @param minw The minimum width
 * @param minh The minimum height
 *
 * This sets the minimum size restriction for the object.
 */
EAPI void
edje_extern_object_min_size_set(Evas_Object *obj, Evas_Coord minw, Evas_Coord minh)
{
   int mw, mh;
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

/** Set the object maximum size
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

/** Set the object aspect size
 * @param obj A valid Evas_Object handle
 * @param aspect The aspect control axes
 * @param aw The aspect radio width
 * @param ah The aspect ratio height
 *
 * This sets the desired aspect ratio to keep an object that will be swallowed
 * by Edje. The width and height define a preferred size ASPECT and the
 * object may be scaled to be larger or smaller, but retaining the relative
 * scale of both aspect width and height.
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

/** Unswallow an object
 * @param obj A valid Evas_Object handle
 * @param obj_swallow The swallowed object
 *
 * Causes the edje to regurgitate a previously swallowed object.  :)
 */
EAPI void
edje_object_part_unswallow(Evas_Object *obj, Evas_Object *obj_swallow)
{
   Edje_Real_Part *rp;

   if (!obj_swallow) return;

   rp = (Edje_Real_Part *)evas_object_data_get(obj_swallow, "\377 edje.swallowing_part");
   if (rp && rp->swallowed_object == obj_swallow)
     {
	evas_object_smart_member_del(rp->swallowed_object);
	evas_object_event_callback_del(rp->swallowed_object,
                                       EVAS_CALLBACK_FREE,
                                       _edje_object_part_swallow_free_cb);
	evas_object_clip_unset(rp->swallowed_object);
	evas_object_data_del(rp->swallowed_object, "\377 edje.swallowing_part");

	if (rp->part->mouse_events)
	  _edje_callbacks_del(rp->swallowed_object);

	rp->swallowed_object = NULL;
	rp->swallow_params.min.w = 0;
	rp->swallow_params.min.h = 0;
	rp->swallow_params.max.w = 0;
	rp->swallow_params.max.h = 0;
	rp->edje->dirty = 1;
	_edje_recalc(rp->edje);
	return;
     }
}

/** Get the object currently swallowed by a part
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
   rp = _edje_real_part_recursive_get(ed, (char *)part);
   if (!rp) return NULL;
   return rp->swallowed_object;
}

/** Get the minimum size for an object
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

/** Get the maximum size for an object
 * @param obj A valid Evas_Object handle
 * @param maxw Maximum width pointer
 * @param maxh Maximum height pointer
 *
 * Gets the object's maximum size values from the Edje.  These are set
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

/** Force a Size/Geometry calculation
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
   
   pf2 = _edje_freeze_val;
   pf = ed->freeze;
   
   _edje_freeze_val = 0;
   ed->freeze = 0;
   
   _edje_recalc(ed);
   
   ed->freeze = pf;
   _edje_freeze_val = pf2;
}

/** Calculate minimum size
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

/** Calculate minimum size
 * @param obj A valid Evas_Object handle
 * @param minw Minimum width pointer
 * @param minh Minimum height pointer
 * @param restrictedw Do not allow object min width calc to be less than this
 * @param restrictedh Do not allow object min height calc to be less than this
 *
 * Calculates the object's minimum size ?!
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
	int i;

	ok = 0;
	ed->dirty = 1;
	_edje_recalc(ed);
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
	     if (!((ep->chosen_description) &&
		   (ep->chosen_description->fixed.w)))
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
	     if (!((ep->chosen_description) &&
		   (ep->chosen_description->fixed.h)))
	       {
		  if (!((ep->part->type == EDJE_PART_TYPE_TEXTBLOCK) &&
			(!ep->chosen_description->text.min_x) &&
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
	if (ok)
	  {
	     ed->w += maxw;
	     ed->h += maxh;
	     if (ed->w < restrictedw) ed->w = restrictedw;
	     if (ed->h < restrictedh) ed->h = restrictedh;
	  }
	if ((ed->w > 4000) || (ed->h > 4000))
	  {
	     printf("EDJE ERROR: file %s, group %s has a non-fixed part. add fixed: 1 1; ???\n",
		    ed->path, ed->group);
	     if (pep)
	       printf("  Problem part is: %s\n", pep->part->name);
	     printf("  Will recalc min size not allowing broken parts to affect the result.\n");
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
   _edje_recalc(ed);
   ed->calc_only = 0;
}

/** Returns the state of the Edje part
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
   rp = _edje_real_part_recursive_get(ed, (char *)part);
   if (!rp)
     {
	if (val_ret) *val_ret = 0;
	return "";
     }
   if (!rp->chosen_description)
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

/** Determine dragable directions
 * @param obj A valid Evas_Object handle
 * @param part The part name
 *
 * @return 0: Not dragable\n
 * 1: Dragable in X direction\n
 * 2: Dragable in Y direction\n
 * 3: Dragable in X & Y directions
 */
EAPI int
edje_object_part_drag_dir_get(const Evas_Object *obj, const char *part)
{
   Edje *ed;
   Edje_Real_Part *rp;

   ed = _edje_fetch(obj);
   if ((!ed) || (!part)) return EDJE_DRAG_DIR_NONE;
   rp = _edje_real_part_recursive_get(ed, (char *)part);
   if (!rp) return EDJE_DRAG_DIR_NONE;
   if ((rp->part->dragable.x) && (rp->part->dragable.y)) return EDJE_DRAG_DIR_XY;
   else if (rp->part->dragable.x) return EDJE_DRAG_DIR_X;
   else if (rp->part->dragable.y) return EDJE_DRAG_DIR_Y;
   return EDJE_DRAG_DIR_NONE;
}

/** Set the dragable object location
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param dx The x value
 * @param dy The y value
 *
 * Places the dragable object at the given location.
 */
EAPI void
edje_object_part_drag_value_set(Evas_Object *obj, const char *part, double dx, double dy)
{
   Edje *ed;
   Edje_Real_Part *rp;

   ed = _edje_fetch(obj);
   if ((!ed) || (!part)) return;
   rp = _edje_real_part_recursive_get(ed, (char *)part);
   if (!rp) return;
   if (rp->drag.down.count > 0) return;
   if (rp->part->dragable.confine_id != -1)
     {
	dx = CLAMP(dx, 0.0, 1.0);
	dy = CLAMP(dy, 0.0, 1.0);
     }
   if (rp->part->dragable.x < 0) dx = 1.0 - dx;
   if (rp->part->dragable.y < 0) dy = 1.0 - dy;
   if ((rp->drag.val.x == dx) && (rp->drag.val.y == dy)) return;
   rp->drag.val.x = dx;
   rp->drag.val.y = dy;
   _edje_dragable_pos_set(rp->edje, rp, dx, dy);
   _edje_emit(rp->edje, "drag,set", rp->part->name);
}

/** Get the dragable object location
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param dx The X value pointer
 * @param dy The Y value pointer
 *
 * Gets the drag location values.
 */
/* FIXME: Should this be x and y instead of dx/dy? */
EAPI void
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
	return;
     }
   rp = _edje_real_part_recursive_get(ed, (char *)part);
   if (!rp)
     {
	if (dx) *dx = 0;
	if (dy) *dy = 0;
	return;
     }
   ddx = rp->drag.val.x;
   ddy = rp->drag.val.y;
   if (rp->part->dragable.x < 0) ddx = 1.0 - ddx;
   if (rp->part->dragable.y < 0) ddy = 1.0 - ddy;
   if (dx) *dx = ddx;
   if (dy) *dy = ddy;
}

/** Set the dragable object size
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param dw The drag width
 * @param dh The drag height
 *
 * Sets the size of the dragable object.
 */
EAPI void
edje_object_part_drag_size_set(Evas_Object *obj, const char *part, double dw, double dh)
{
   Edje *ed;
   Edje_Real_Part *rp;

   ed = _edje_fetch(obj);
   if ((!ed) || (!part)) return;
   rp = _edje_real_part_recursive_get(ed, (char *)part);
   if (!rp) return;
   if (dw < 0.0) dw = 0.0;
   else if (dw > 1.0) dw = 1.0;
   if (dh < 0.0) dh = 0.0;
   else if (dh > 1.0) dh = 1.0;
   if ((rp->drag.size.x == dw) && (rp->drag.size.y == dh)) return;
   rp->drag.size.x = dw;
   rp->drag.size.y = dh;
   rp->edje->dirty = 1;
   _edje_recalc(rp->edje);
}

/** Get the dragable object size
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param dw The drag width pointer
 * @param dh The drag height pointer
 *
 * Gets the dragable object size.
 */
EAPI void
edje_object_part_drag_size_get(const Evas_Object *obj, const char *part, double *dw, double *dh)
{
   Edje *ed;
   Edje_Real_Part *rp;

   ed = _edje_fetch(obj);
   if ((!ed) || (!part))
     {
	if (dw) *dw = 0;
	if (dh) *dh = 0;
	return;
     }
   rp = _edje_real_part_recursive_get(ed, (char *)part);
   if (!rp)
     {
	if (dw) *dw = 0;
	if (dh) *dh = 0;
	return;
     }
   if (dw) *dw = rp->drag.size.x;
   if (dh) *dh = rp->drag.size.y;
}

/** Sets the drag step increment
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param dx The x step ammount
 * @param dy The y step ammount
 *
 * Sets the x,y step increments for a dragable object.
 */
EAPI void
edje_object_part_drag_step_set(Evas_Object *obj, const char *part, double dx, double dy)
{
   Edje *ed;
   Edje_Real_Part *rp;

   ed = _edje_fetch(obj);
   if ((!ed) || (!part)) return;
   rp = _edje_real_part_recursive_get(ed, (char *)part);
   if (!rp) return;
   if (dx < 0.0) dx = 0.0;
   else if (dx > 1.0) dx = 1.0;
   if (dy < 0.0) dy = 0.0;
   else if (dy > 1.0) dy = 1.0;
   rp->drag.step.x = dx;
   rp->drag.step.y = dy;
}

/** Gets the drag step increment values.
 * @param obj A valid Evas_Object handle
 * @param part The part
 * @param dx The x step increment pointer
 * @param dy The y step increment pointer
 *
 * Gets the x and y step increments for the dragable object.
 */
EAPI void
edje_object_part_drag_step_get(const Evas_Object *obj, const char *part, double *dx, double *dy)
{
   Edje *ed;
   Edje_Real_Part *rp;

   ed = _edje_fetch(obj);
   if ((!ed) || (!part))
     {
	if (dx) *dx = 0;
	if (dy) *dy = 0;
	return;
     }
   rp = _edje_real_part_recursive_get(ed, (char *)part);
   if (!rp)
     {
	if (dx) *dx = 0;
	if (dy) *dy = 0;
	return;
     }
   if (dx) *dx = rp->drag.step.x;
   if (dy) *dy = rp->drag.step.y;
}

/** Sets the page step increments
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param dx The x page step increment
 * @param dy The y page step increment
 *
 * Sets the x,y page step increment values.
 */
EAPI void
edje_object_part_drag_page_set(Evas_Object *obj, const char *part, double dx, double dy)
{
   Edje *ed;
   Edje_Real_Part *rp;

   ed = _edje_fetch(obj);
   if ((!ed) || (!part)) return;
   rp = _edje_real_part_recursive_get(ed, (char *)part);
   if (!rp) return;
   if (dx < 0.0) dx = 0.0;
   else if (dx > 1.0) dx = 1.0;
   if (dy < 0.0) dy = 0.0;
   else if (dy > 1.0) dy = 1.0;
   rp->drag.page.x = dx;
   rp->drag.page.y = dy;
}

/** Gets the page step increments
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param dx The dx page increment pointer
 * @param dy The dy page increment pointer
 *
 * Gets the x,y page step increments for the dragable object.
 */
EAPI void
edje_object_part_drag_page_get(const Evas_Object *obj, const char *part, double *dx, double *dy)
{
   Edje *ed;
   Edje_Real_Part *rp;

   ed = _edje_fetch(obj);
   if ((!ed) || (!part))
     {
	if (dx) *dx = 0;
	if (dy) *dy = 0;
	return;
     }
   rp = _edje_real_part_recursive_get(ed, (char *)part);
   if (!rp)
     {
	if (dx) *dx = 0;
	if (dy) *dy = 0;
	return;
     }
   if (dx) *dx = rp->drag.page.x;
   if (dy) *dy = rp->drag.page.y;
}

/** Steps the dragable x,y steps
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param dx The x step
 * @param dy The y step
 *
 * Steps x,y where the step increment is the amount set by
 * edje_object_part_drag_step_set.
 */
EAPI void
edje_object_part_drag_step(Evas_Object *obj, const char *part, double dx, double dy)
{
   Edje *ed;
   Edje_Real_Part *rp;
   double px, py;

   ed = _edje_fetch(obj);
   if ((!ed) || (!part)) return;
   rp = _edje_real_part_recursive_get(ed, (char *)part);
   if (!rp) return;
   if (rp->drag.down.count > 0) return;
   px = rp->drag.val.x;
   py = rp->drag.val.y;
   rp->drag.val.x += dx * rp->drag.step.x * rp->part->dragable.x;
   rp->drag.val.y += dy * rp->drag.step.y * rp->part->dragable.y;
   rp->drag.val.x = CLAMP (rp->drag.val.x, 0.0, 1.0);
   rp->drag.val.y = CLAMP (rp->drag.val.y, 0.0, 1.0);
   if ((px == rp->drag.val.x) && (py == rp->drag.val.y)) return;
   _edje_dragable_pos_set(rp->edje, rp, rp->drag.val.x, rp->drag.val.y);
   _edje_emit(rp->edje, "drag,step", rp->part->name);
}

/** Pages x,y steps
 * @param obj A valid Evas_Object handle
 * @param part The part name
 * @param dx The x step
 * @param dy The y step
 *
 * Pages x,y where the increment is defined by edje_object_part_drag_page_set.\n
 * WARNING: Paging is bugged!
 */
EAPI void
edje_object_part_drag_page(Evas_Object *obj, const char *part, double dx, double dy)
{
   Edje *ed;
   Edje_Real_Part *rp;
   double px, py;

   ed = _edje_fetch(obj);
   if ((!ed) || (!part)) return;
   rp = _edje_real_part_recursive_get(ed, (char *)part);
   if (!rp) return;
   if (rp->drag.down.count > 0) return;
   px = rp->drag.val.x;
   py = rp->drag.val.y;
   rp->drag.val.x += dx * rp->drag.page.x * rp->part->dragable.x;
   rp->drag.val.y += dy * rp->drag.page.y * rp->part->dragable.y;
   rp->drag.val.x = CLAMP (rp->drag.val.x, 0.0, 1.0);
   rp->drag.val.y = CLAMP (rp->drag.val.y, 0.0, 1.0);
   if ((px == rp->drag.val.x) && (py == rp->drag.val.y)) return;
   _edje_dragable_pos_set(rp->edje, rp, rp->drag.val.x, rp->drag.val.y);
   _edje_emit(rp->edje, "drag,page", rp->part->name);
}

Edje_Real_Part *
_edje_real_part_recursive_get(Edje *ed, const char *part)
{
   Edje_Real_Part *rp;
   char **path;
   path = ecore_str_split(part, EDJE_PART_PATH_SEPARATOR_STRING, 0);
   if (!path) return NULL;

   //printf("recursive get: %s\n", part);
   rp = _edje_real_part_recursive_get_helper(ed, path);

   free(*path);
   free(path);
   return rp;
}

Edje_Real_Part *
_edje_real_part_recursive_get_helper(Edje *ed, char **path)
{
   Edje_Real_Part *rp;
   //printf("  lookup: %s on %s\n", path[0], ed->parent ? ed->parent : "-");
   rp = _edje_real_part_get(ed, path[0]);
   if (path[1] == NULL) return rp;

   if ((!rp) || (rp->part->type != EDJE_PART_TYPE_GROUP) ||
       (!rp->swallowed_object)) return NULL;

   ed = _edje_fetch(rp->swallowed_object);
   if (!ed) return NULL;

   path++;
   return _edje_real_part_recursive_get_helper(ed, path);
}


/* Private Routines */

Edje_Real_Part *
_edje_real_part_get(Edje *ed, const char *part)
{
   int i;

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
   Evas_List *l;
   Edje_Color_Class *cc = NULL;

   if ((!ed) || (!color_class)) return NULL;

   /* first look through the object scope */
   for (l = ed->color_classes; l; l = l->next)
     {
	cc = l->data;
	if ((cc->name) && (!strcmp(color_class, cc->name))) return cc;
     }

   /* next look through the global scope */
   cc = evas_hash_find(_edje_color_class_hash, color_class);
   if (cc) return cc;

   /* finally, look through the file scope */
   for (l = ed->file->color_classes; l; l = l->next)
     {
	cc = l->data;
	if ((cc->name) && (!strcmp(color_class, cc->name))) return cc;
     }

   return NULL;
}

void
_edje_color_class_member_add(Edje *ed, const char *color_class)
{
   Evas_List *members;

   if ((!ed) || (!color_class)) return;
   members = evas_hash_find(_edje_color_class_member_hash, color_class);
   if (members)
     {
        _edje_color_class_member_hash =
          evas_hash_del(_edje_color_class_member_hash, color_class, members);
     }

   members = evas_list_prepend(members, ed);
   _edje_color_class_member_hash =
     evas_hash_add(_edje_color_class_member_hash, color_class, members);
}

void
_edje_color_class_member_del(Edje *ed, const char *color_class)
{
   Evas_List *members;

   if ((!ed) || (!color_class)) return;
   members = evas_hash_find(_edje_color_class_member_hash, color_class);
   if (!members) return;

   _edje_color_class_member_hash =
     evas_hash_del(_edje_color_class_member_hash, color_class, members);
   members = evas_list_remove(members, ed);
   if (members)
     {
        _edje_color_class_member_hash =
          evas_hash_add(_edje_color_class_member_hash, color_class, members);
     }
}

/**
 * Used to free the member lists that are stored in the text_class
 * and color_class hashtables.
 */
static Evas_Bool
member_list_free(const Evas_Hash *hash, const char *key, void *data, void *fdata)
{
   evas_list_free(data);
   return 1;
}

void
_edje_color_class_members_free(void)
{
   if (!_edje_color_class_member_hash) return;
   evas_hash_foreach(_edje_color_class_member_hash, member_list_free, NULL);
   evas_hash_free(_edje_color_class_member_hash);
   _edje_color_class_member_hash = NULL;
}

static Evas_Bool
color_class_hash_list_free(const Evas_Hash *hash, const char *key, void *data, void *fdata)
{
   Edje_Color_Class *cc;

   cc = data;
   if (cc->name) evas_stringshare_del(cc->name);
   free(cc);
   return 1;
}

void
_edje_color_class_hash_free(void)
{
   if (!_edje_color_class_hash) return;
   evas_hash_foreach(_edje_color_class_hash, color_class_hash_list_free, NULL);
   evas_hash_free(_edje_color_class_hash);
   _edje_color_class_hash = NULL;
}

void
_edje_color_class_on_del(Edje *ed, Edje_Part *ep)
{
   Evas_List *tmp;

   if ((ep->default_desc) && (ep->default_desc->color_class))
     _edje_color_class_member_del(ed, ep->default_desc->color_class);

   for (tmp = ep->other_desc; tmp; tmp = tmp->next)
     {
        Edje_Part_Description *desc;

	desc = tmp->data;
	if (desc->color_class)
          _edje_color_class_member_del(ed, desc->color_class);
     }
}

Edje_Text_Class *
_edje_text_class_find(Edje *ed, const char *text_class)
{
   Evas_List *l;

   if ((!ed) || (!text_class)) return NULL;
   for (l = ed->text_classes; l; l = l->next)
     {
	Edje_Text_Class *tc;

	tc = l->data;
	if ((tc->name) && (!strcmp(text_class, tc->name))) return tc;
     }
   return evas_hash_find(_edje_text_class_hash, text_class);
}

void
_edje_text_class_member_add(Edje *ed, const char *text_class)
{
   Evas_List *members;

   if ((!ed) || (!text_class)) return;

   /* Get members list */
   members = evas_hash_find(_edje_text_class_member_hash, text_class);

   /* Remove members list */
   if (members)
     {
        _edje_text_class_member_hash =
          evas_hash_del(_edje_text_class_member_hash, text_class, members);
     }

   /* Update the member list */
   members = evas_list_prepend(members, ed);

   /* Add the member list back */
   _edje_text_class_member_hash =
     evas_hash_add(_edje_text_class_member_hash, text_class, members);
}

void
_edje_text_class_member_del(Edje *ed, const char *text_class)
{
   Evas_List *members;

   if ((!ed) || (!text_class)) return;
   members = evas_hash_find(_edje_text_class_member_hash, text_class);
   if (!members) return;

   _edje_text_class_member_hash =
     evas_hash_del(_edje_text_class_member_hash, text_class, members);

   members = evas_list_remove(members, ed);
   if (members)
     {
        _edje_text_class_member_hash =
          evas_hash_add(_edje_text_class_member_hash, text_class, members);
     }
}

void
_edje_text_class_members_free(void)
{
   if (!_edje_text_class_member_hash) return;
   evas_hash_foreach(_edje_text_class_member_hash, member_list_free, NULL);
   evas_hash_free(_edje_text_class_member_hash);
   _edje_text_class_member_hash = NULL;
}

static Evas_Bool
text_class_hash_list_free(const Evas_Hash *hash, const char *key, void *data, void *fdata)
{
   Edje_Text_Class *tc;

   tc = data;
   if (tc->name) evas_stringshare_del(tc->name);
   if (tc->font) evas_stringshare_del(tc->font);
   free(tc);
  return 1;
}

void
_edje_text_class_hash_free(void)
{
   if (!_edje_text_class_hash) return;
   evas_hash_foreach(_edje_text_class_hash, text_class_hash_list_free, NULL);
   evas_hash_free(_edje_text_class_hash);
   _edje_text_class_hash = NULL;
}

Edje *
_edje_fetch(const Evas_Object *obj)
{
   Edje *ed;
   char *type;

   type = (char *)evas_object_type_get(obj);
   if (!type) return NULL;
   if (strcmp(type, "edje")) return NULL;
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
_edje_object_part_swallow_free_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
   Evas_Object *edje_obj;

   edje_obj = data;
   edje_object_part_unswallow(edje_obj, obj);
   return;
   e = NULL;
   event_info = NULL;
}

void
_edje_real_part_swallow(Edje_Real_Part *rp, Evas_Object *obj_swallow)
{
   char *type;

   if (rp->swallowed_object)
     {
	evas_object_smart_member_del(rp->swallowed_object);
	evas_object_event_callback_del(rp->swallowed_object,
				       EVAS_CALLBACK_FREE,
				       _edje_object_part_swallow_free_cb);
	evas_object_clip_unset(rp->swallowed_object);
	evas_object_data_del(rp->swallowed_object, "\377 edje.swallowing_part");
        if (rp->part->mouse_events)
          _edje_callbacks_del(rp->swallowed_object);
	rp->swallowed_object = NULL;
     }
   if (!obj_swallow) return;
   rp->swallowed_object = obj_swallow;
   evas_object_smart_member_add(rp->swallowed_object, rp->edje->obj);
   if (rp->clip_to)
     evas_object_clip_set(rp->swallowed_object, rp->clip_to->object);
   else evas_object_clip_set(rp->swallowed_object, rp->edje->clipper);
   evas_object_stack_above(rp->swallowed_object, rp->object);
   evas_object_event_callback_add(rp->swallowed_object, EVAS_CALLBACK_FREE,
				  _edje_object_part_swallow_free_cb,
				  rp->edje->obj);
   type = (char *)evas_object_type_get(obj_swallow);
   rp->swallow_params.min.w = 0;
   rp->swallow_params.min.w = 0;
   rp->swallow_params.max.w = -1;
   rp->swallow_params.max.h = -1;
   if ((type) && (!strcmp(type, "edje")))
     {
	Evas_Coord w, h;

	edje_object_size_min_get(obj_swallow, &w, &h);
	rp->swallow_params.min.w = w;
	rp->swallow_params.min.h = h;
	edje_object_size_max_get(obj_swallow, &w, &h);
	rp->swallow_params.max.w = w;
	rp->swallow_params.max.h = h;
     }
   else if ((type) && ((!strcmp(type, "text")) || (!strcmp(type, "polygon")) ||
		       (!strcmp(type, "line"))))
     {
	Evas_Coord w, h;

	evas_object_geometry_get(obj_swallow, NULL, NULL, &w, &h);
	rp->swallow_params.min.w = w;
	rp->swallow_params.min.h = h;
	rp->swallow_params.max.w = w;
	rp->swallow_params.max.h = h;
     }
     {
	Evas_Coord w1, h1, w2, h2, aw, ah;
	Evas_Aspect_Control am;

	evas_object_size_hint_min_get(obj_swallow, &w1, &h1);
	evas_object_size_hint_max_get(obj_swallow, &w2, &h2);
	evas_object_size_hint_aspect_get(obj_swallow, &am, &aw, &ah);
	rp->swallow_params.min.w = w1;
	rp->swallow_params.min.h = h1;
	if (w2 > 0) rp->swallow_params.max.w = w2;
	if (h2 > 0) rp->swallow_params.max.h = h2;
  	switch (am)
	  {
	   case EVAS_ASPECT_CONTROL_NONE: rp->swallow_params.aspect.mode = EDJE_ASPECT_CONTROL_NONE; break;
	   case EVAS_ASPECT_CONTROL_NEITHER: rp->swallow_params.aspect.mode = EDJE_ASPECT_CONTROL_NEITHER; break;
	   case EVAS_ASPECT_CONTROL_HORIZONTAL: rp->swallow_params.aspect.mode = EDJE_ASPECT_CONTROL_HORIZONTAL; break;
	   case EVAS_ASPECT_CONTROL_VERTICAL: rp->swallow_params.aspect.mode = EDJE_ASPECT_CONTROL_VERTICAL; break;
	   case EVAS_ASPECT_CONTROL_BOTH: rp->swallow_params.aspect.mode = EDJE_ASPECT_CONTROL_BOTH; break;
	   default: break;
	  }
	rp->swallow_params.aspect.w = aw;
	rp->swallow_params.aspect.h = ah;
	evas_object_data_set(rp->swallowed_object, "\377 edje.swallowing_part", rp);
     }

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

   if (rp->part->precise_is_inside)
     evas_object_precise_is_inside_set(obj_swallow, 1);

   rp->edje->dirty = 1;
   _edje_recalc(rp->edje);
}
