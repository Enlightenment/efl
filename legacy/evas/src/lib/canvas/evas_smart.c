#include "evas_common.h"
#include "evas_private.h"


static void _evas_smart_class_callbacks_create(Evas_Smart *s);

/* all public */

/**
 * @addtogroup Evas_Smart_Group
 * @{
 */

/**
 * Create an Evas_Smart, which can be used to instantiate new smart objects.
 *
 * This function internally creates an Evas_Smart_Class and sets the
 * provided callbacks. Callbacks that are unneeded (or marked DEPRECATED
 * below) should be set to NULL.
 *
 * Alternatively you can create an Evas_Smart_Class yourself and use
 * evas_smart_class_new().
 *
 * @param name a unique name for the smart
 * @param func_add callback called when smart object is added
 * @param func_del callback called when smart object is deleted
 * @param func_layer_set DEPRECATED
 * @param func_raise DEPRECATED
 * @param func_lower DEPRECATED
 * @param func_stack_above DEPRECATED
 * @param func_stack_below DEPRECATED
 * @param func_move callback called when smart object is moved
 * @param func_resize callback called when smart object is resized
 * @param func_show callback called when smart object is shown
 * @param func_hide callback called when smart object is hidden
 * @param func_color_set callback called when smart object has its color set
 * @param func_clip_set callback called when smart object has its clip set
 * @param func_clip_unset callback called when smart object has its clip unset
 * @param data a pointer to user data for the smart
 * @return an Evas_Smart
 *
 */
EAPI Evas_Smart *
evas_smart_new(const char *name,
	       void      (*func_add) (Evas_Object *o),
	       void      (*func_del) (Evas_Object *o),
	       void      (*func_layer_set) (Evas_Object *o, int l) __UNUSED__,
	       void      (*func_raise) (Evas_Object *o) __UNUSED__,
	       void      (*func_lower) (Evas_Object *o) __UNUSED__,
	       void      (*func_stack_above) (Evas_Object *o, Evas_Object *above) __UNUSED__,
	       void      (*func_stack_below) (Evas_Object *o, Evas_Object *below) __UNUSED__,
	       void      (*func_move) (Evas_Object *o, Evas_Coord x, Evas_Coord y),
	       void      (*func_resize) (Evas_Object *o, Evas_Coord w, Evas_Coord h),
	       void      (*func_show) (Evas_Object *o),
	       void      (*func_hide) (Evas_Object *o),
	       void      (*func_color_set) (Evas_Object *o, int r, int g, int b, int a),
	       void      (*func_clip_set) (Evas_Object *o, Evas_Object *clip),
	       void      (*func_clip_unset) (Evas_Object *o),
	       const void *data)
{
   Evas_Smart *s;
   Evas_Smart_Class *sc;

   WRN("----- WARNING. evas_smart_new() will be deprecated and removed soon"
	  "----- Please use evas_smart_class_new() instead");

   if (!name) return NULL;

   s = evas_mem_calloc(sizeof(Evas_Smart));
   if (!s) return NULL;

   s->magic = MAGIC_SMART;

   s->class_allocated = 1;

   sc = evas_mem_calloc(sizeof(Evas_Smart_Class));
   if (!sc)
     {
	free(s);
	return NULL;
     }
   sc->name = name;
   sc->add = func_add;
   sc->del = func_del;
   sc->move = func_move;
   sc->resize = func_resize;
   sc->show = func_show;
   sc->hide = func_hide;
   sc->color_set = func_color_set;
   sc->clip_set = func_clip_set;
   sc->clip_unset = func_clip_unset;
   sc->data = (void *)data;
   s->smart_class = sc;

   return s;
}

/**
 * Free an Evas_Smart
 *
 * If this smart was created using evas_smart_class_new(), the associated
 * Evas_Smart_Class will not be freed.
 *
 * @param s the Evas_Smart to free
 *
 */
EAPI void
evas_smart_free(Evas_Smart *s)
{
   MAGIC_CHECK(s, Evas_Smart, MAGIC_SMART);
   return;
   MAGIC_CHECK_END();
   s->delete_me = 1;
   if (s->usage > 0) return;
   if (s->class_allocated) free((void *)s->smart_class);
   free(s->callbacks.array);
   free(s);
}

/**
 * Creates an Evas_Smart from an Evas_Smart_Class.
 *
 * @param sc the smart class definition
 * @return an Evas_Smart
 */
EAPI Evas_Smart *
evas_smart_class_new(const Evas_Smart_Class *sc)
{
   Evas_Smart *s;

   if (!sc) return NULL;

   /* api does not match abi! for now refuse as we only have 1 version */
   if (sc->version != EVAS_SMART_CLASS_VERSION) return NULL;

   s = evas_mem_calloc(sizeof(Evas_Smart));
   if (!s) return NULL;

   s->magic = MAGIC_SMART;

   s->smart_class = sc;
   _evas_smart_class_callbacks_create(s);

   return s;
}

/**
 * Get the Evas_Smart_Class of an Evas_Smart
 *
 * @param s the Evas_Smart
 * @return the Evas_Smart_Class
 */
EAPI const Evas_Smart_Class *
evas_smart_class_get(const Evas_Smart *s)
{
   MAGIC_CHECK(s, Evas_Smart, MAGIC_SMART);
   return NULL;
   MAGIC_CHECK_END();
   return s->smart_class;
}

/**
 * @brief Get the data pointer set on an Evas_Smart.
 *
 * @param s Evas_Smart
 *
 * This data pointer is set either as the final parameter to
 * evas_smart_new or as the data field in the Evas_Smart_Class passed
 * in to evas_smart_class_new
 */
EAPI void *
evas_smart_data_get(const Evas_Smart *s)
{
   MAGIC_CHECK(s, Evas_Smart, MAGIC_SMART);
   return NULL;
   MAGIC_CHECK_END();
   return (void *)s->smart_class->data;
}

/**
 * Get the callbacks known by this Evas_Smart.
 *
 * This is likely different from Evas_Smart_Class::callbacks as it
 * will contain the callbacks of all class hierarchy sorted, while the
 * direct smart class member refers only to that specific class and
 * should not include parent's.
 *
 * If no callbacks are known, this function returns @c NULL.
 *
 * The array elements and thus their contents will be reference to
 * original values given to evas_smart_new() as
 * Evas_Smart_Class::callbacks.
 *
 * The array is sorted by name. The last array element is the @c NULL
 * pointer and is not counted in @a count. Loop iterations can check
 * any of these cases.
 *
 * @param s the Evas_Smart.
 * @param count returns the number of elements in returned array.
 * @return the array with callback descriptions known by this class,
 *         its size is returned in @a count parameter. It should not
 *         be modified anyhow. If no callbacks are known, @c NULL is
 *         returned. The array is sorted by name and elements refer to
 *         the original value given to evas_smart_new().
 *
 * @note objects may provide per-instance callbacks, use
 *       evas_object_smart_callbacks_descriptions_get() to get those
 *       as well.
 * @see evas_object_smart_callbacks_descriptions_get()
 */
EAPI const Evas_Smart_Cb_Description **
evas_smart_callbacks_descriptions_get(const Evas_Smart *s, unsigned int *count)
{
   MAGIC_CHECK(s, Evas_Smart, MAGIC_SMART);
   if (count) *count = 0;
   return NULL;
   MAGIC_CHECK_END();

   if (count) *count = s->callbacks.size;
   return s->callbacks.array;
}

/**
 * Find callback description for callback called @a name.
 *
 * @param s the Evas_Smart.
 * @param name name of desired callback, must @b not be @c NULL.  The
 *        search have a special case for @a name being the same
 *        pointer as registered with Evas_Smart_Cb_Description, one
 *        can use it to avoid excessive use of strcmp().
 * @return reference to description if found, @c NULL if not found.
 */
EAPI const Evas_Smart_Cb_Description *
evas_smart_callback_description_find(const Evas_Smart *s, const char *name)
{
   if (!name) return NULL;
   MAGIC_CHECK(s, Evas_Smart, MAGIC_SMART);
   return NULL;
   MAGIC_CHECK_END();
   return evas_smart_cb_description_find(&s->callbacks, name);
}

/**
 * Sets one class to inherit from the other.
 *
 * Copy all function pointers, set @c parent to @a parent_sc and copy
 * everything after sizeof(Evas_Smart_Class) present in @a parent_sc,
 * using @a parent_sc_size as reference.
 *
 * This is recommended instead of a single memcpy() since it will take
 * care to not modify @a sc name, version, callbacks and possible
 * other members.
 *
 * @param sc child class.
 * @param parent_sc parent class, will provide attributes.
 * @param parent_sc_size size of parent_sc structure, child should be at least
 *        this size. Everything after @c Evas_Smart_Class size is copied
 *        using regular memcpy().
 */
EAPI Eina_Bool
evas_smart_class_inherit_full(Evas_Smart_Class *sc, const Evas_Smart_Class *parent_sc, unsigned int parent_sc_size)
{
   unsigned int off;

   /* api does not match abi! for now refuse as we only have 1 version */
   if (parent_sc->version != EVAS_SMART_CLASS_VERSION) return 0;

#define _CP(m) sc->m = parent_sc->m
   _CP(add);
   _CP(del);
   _CP(move);
   _CP(resize);
   _CP(show);
   _CP(hide);
   _CP(color_set);
   _CP(clip_set);
   _CP(clip_unset);
   _CP(calculate);
   _CP(member_add);
   _CP(member_del);
#undef _CP

   sc->parent = parent_sc;

   off = sizeof(Evas_Smart_Class);
   if (parent_sc_size == off) return 1;

   memcpy(((char *)sc) + off, ((char *)parent_sc) + off, parent_sc_size - off);
   return 1;
}

/**
 * @}
 */

/* internal funcs */
void
evas_object_smart_use(Evas_Smart *s)
{
   s->usage++;
}

void
evas_object_smart_unuse(Evas_Smart *s)
{
   s->usage--;
   if ((s->usage <= 0) && (s->delete_me)) evas_smart_free(s);
}

Eina_Bool
evas_smart_cb_descriptions_resize(Evas_Smart_Cb_Description_Array *a, unsigned int size)
{
   void *tmp;

   if (size == a->size)
     return 1;

   if (size == 0)
     {
	free(a->array);
	a->array = NULL;
	a->size = 0;
	return 1;
     }

   tmp = realloc(a->array, (size + 1) * sizeof(Evas_Smart_Cb_Description *));
   if (tmp)
     {
	a->array = tmp;
	a->size = size;
	a->array[size] = NULL;
	return 1;
     }
   else
     {
	fprintf(stderr, "ERROR: realloc failed!\n");
	return 0;
     }
}

static int
_evas_smart_cb_description_cmp_sort(const void *p1, const void *p2)
{
   const Evas_Smart_Cb_Description **a = (const Evas_Smart_Cb_Description **)p1;
   const Evas_Smart_Cb_Description **b = (const Evas_Smart_Cb_Description **)p2;
   return strcmp((*a)->name, (*b)->name);
}

void
evas_smart_cb_descriptions_fix(Evas_Smart_Cb_Description_Array *a)
{
   unsigned int i, j;

   qsort(a->array, a->size, sizeof(Evas_Smart_Cb_Description *),
	 _evas_smart_cb_description_cmp_sort);

   fprintf(stderr, "\nDEBUG: %u callbacks\n", a->size);
   if (a->size)
     fprintf(stderr, "DEBUG:    %20.20s  [%s]\n",
	     a->array[0]->name, a->array[0]->type);

   for (i = 0, j = 1; j < a->size; j++)
     {
	const Evas_Smart_Cb_Description *cur, *prev;

	cur = a->array[j];
	prev = a->array[i];

	fprintf(stderr, "DEBUG:    %20.20s  [%s]\n", cur->name, cur->type);

	if (strcmp(cur->name, prev->name) != 0)
	  {
	     i++;
	     if (i != j)
	       a->array[i] = a->array[j];
	  }
	else
	  {
	     if (strcmp(cur->type, prev->type) == 0)
	       fprintf(stderr, "WARNING: duplicated smart callback description"
		       " with name '%s' and type '%s'\n", cur->name, cur->type);
	     else
	       fprintf(stderr, "ERROR: callback descriptions named '%s' differ"
		       " in type, keeping '%s', ignoring '%s'\n", cur->name,
		       prev->type, cur->type);
	  }
     }

   evas_smart_cb_descriptions_resize(a, a->size - (j - i));
}

static void
_evas_smart_class_callbacks_create(Evas_Smart *s)
{
   const Evas_Smart_Class *sc;
   unsigned int n = 0;

   for (sc = s->smart_class; sc != NULL; sc = sc->parent)
     {
	const Evas_Smart_Cb_Description *d;
	for (d = sc->callbacks; d && d->name != NULL; d++)
	  n++;
     }

   if (n == 0) return;
   if (!evas_smart_cb_descriptions_resize(&s->callbacks, n)) return;
   for (n = 0, sc = s->smart_class; sc != NULL; sc = sc->parent)
     {
	const Evas_Smart_Cb_Description *d;
	for (d = sc->callbacks; d->name != NULL; d++)
	  s->callbacks.array[n++] = d;
     }
   evas_smart_cb_descriptions_fix(&s->callbacks);
}

static int
_evas_smart_cb_description_cmp_search(const void *p1, const void *p2)
{
   const char *name = p1;
   const Evas_Smart_Cb_Description **v = (const Evas_Smart_Cb_Description **)p2;
   /* speed up string shares searches (same pointers) */
   if (name == (*v)->name) return 0;
   return strcmp(name, (*v)->name);
}

const Evas_Smart_Cb_Description *
evas_smart_cb_description_find(const Evas_Smart_Cb_Description_Array *a, const char *name)
{
   if (!a->array) return NULL;
   return bsearch(name, a->array, a->size, sizeof(Evas_Smart_Cb_Description *),
		  _evas_smart_cb_description_cmp_search);
}
