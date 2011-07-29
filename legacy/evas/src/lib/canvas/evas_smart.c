#include "evas_common.h"
#include "evas_private.h"


static void _evas_smart_class_callbacks_create(Evas_Smart *s);

/* all public */

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

EAPI const Evas_Smart_Class *
evas_smart_class_get(const Evas_Smart *s)
{
   MAGIC_CHECK(s, Evas_Smart, MAGIC_SMART);
   return NULL;
   MAGIC_CHECK_END();
   return s->smart_class;
}

EAPI void *
evas_smart_data_get(const Evas_Smart *s)
{
   MAGIC_CHECK(s, Evas_Smart, MAGIC_SMART);
   return NULL;
   MAGIC_CHECK_END();
   return (void *)s->smart_class->data;
}

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

EAPI const Evas_Smart_Cb_Description *
evas_smart_callback_description_find(const Evas_Smart *s, const char *name)
{
   if (!name) return NULL;
   MAGIC_CHECK(s, Evas_Smart, MAGIC_SMART);
   return NULL;
   MAGIC_CHECK_END();
   return evas_smart_cb_description_find(&s->callbacks, name);
}

EAPI Eina_Bool
evas_smart_class_inherit_full(Evas_Smart_Class *sc, const Evas_Smart_Class *parent_sc, unsigned int parent_sc_size)
{
   unsigned int off;

   /* api does not match abi! for now refuse as we only have 1 version */
   if (parent_sc->version != EVAS_SMART_CLASS_VERSION) return EINA_FALSE;

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
   if (parent_sc_size == off) return EINA_TRUE;

   memcpy(((char *)sc) + off, ((char *)parent_sc) + off, parent_sc_size - off);
   return EINA_TRUE;
}

EAPI int
evas_smart_usage_get(const Evas_Smart *s)
{
   MAGIC_CHECK(s, Evas_Smart, MAGIC_SMART);
   return 0;
   MAGIC_CHECK_END();
   return s->usage;
}


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
     return EINA_TRUE;

   if (size == EINA_FALSE)
     {
        free(a->array);
        a->array = NULL;
        a->size = 0;
        return EINA_TRUE;
     }

   tmp = realloc(a->array, (size + 1) * sizeof(Evas_Smart_Cb_Description *));
   if (tmp)
     {
        a->array = tmp;
        a->size = size;
        a->array[size] = NULL;
        return EINA_TRUE;
     }
   else
     {
        ERR("realloc failed!");
        return EINA_FALSE;
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

   if (!a)
     {
        ERR("no array to fix!");
        return;
     }

   qsort(a->array, a->size, sizeof(Evas_Smart_Cb_Description *),
         _evas_smart_cb_description_cmp_sort);

   DBG("%u callbacks", a->size);
   if (a->size)
     DBG("%s [type=%s]", a->array[0]->name, a->array[0]->type);

   for (i = 0, j = 1; j < a->size; j++)
     {
        const Evas_Smart_Cb_Description *cur, *prev;

        cur = a->array[j];
        prev = a->array[i];

        DBG("%s [type=%s]", cur->name, cur->type);

        if (strcmp(cur->name, prev->name) != 0)
          {
             i++;
             if (i != j)
               a->array[i] = a->array[j];
          }
        else
          {
             if (strcmp(cur->type, prev->type) == 0)
               WRN("duplicated smart callback description"
                   " with name '%s' and type '%s'", cur->name, cur->type);
             else
               ERR("callback descriptions named '%s' differ"
                   " in type, keeping '%s', ignoring '%s'",
                   cur->name, prev->type, cur->type);
          }
     }

   evas_smart_cb_descriptions_resize(a, i + 1);
}

static void
_evas_smart_class_callbacks_create(Evas_Smart *s)
{
   const Evas_Smart_Class *sc;
   unsigned int n = 0;

   for (sc = s->smart_class; sc; sc = sc->parent)
     {
        const Evas_Smart_Cb_Description *d;
        for (d = sc->callbacks; d && d->name; d++)
          n++;
     }

   if (n == 0) return;
   if (!evas_smart_cb_descriptions_resize(&s->callbacks, n)) return;
   for (n = 0, sc = s->smart_class; sc; sc = sc->parent)
     {
        const Evas_Smart_Cb_Description *d;
        for (d = sc->callbacks; d && d->name; d++)
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
