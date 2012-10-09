#include "evas_common.h"
#include "evas_private.h"

/* private calls */

static int
evas_key_modifier_number(const Evas_Modifier *m, const char *keyname)
{
   int i;

   for (i = 0; i < m->mod.count; i++)
     {
	if (!strcmp(m->mod.list[i], keyname)) return i;
     }
   return -1;
}

static int
evas_key_lock_number(const Evas_Lock *l, const char *keyname)
{
   int i;

   for (i = 0; i < l->lock.count; i++)
     {
	if (!strcmp(l->lock.list[i], keyname)) return i;
     }
   return -1;
}

/* local calls */

/* public calls */

EAPI const Evas_Modifier *
evas_key_modifier_get(const Evas *eo_e)
{
   MAGIC_CHECK(eo_e, Evas, MAGIC_EVAS);
   return NULL;
   MAGIC_CHECK_END();
   const Evas_Modifier *ret = NULL;
   eo_do((Eo *)eo_e, evas_canvas_key_modifier_get(&ret));
   return ret;
}

void
_canvas_key_modifier_get(Eo *eo_e EINA_UNUSED, void *_pd, va_list *list)
{
   const Evas_Modifier **ret = va_arg(*list, const Evas_Modifier **);
   const Evas_Public_Data *e = _pd;
   *ret = &(e->modifiers);
}

EAPI const Evas_Lock *
evas_key_lock_get(const Evas *eo_e)
{
   MAGIC_CHECK(eo_e, Evas, MAGIC_EVAS);
   return NULL;
   MAGIC_CHECK_END();
   const Evas_Lock *ret = NULL;
   eo_do((Eo *)eo_e, evas_canvas_key_lock_get(&ret));
   return ret;
}

void
_canvas_key_lock_get(Eo *eo_e EINA_UNUSED, void *_pd, va_list *list)
{
   const Evas_Lock **ret = va_arg(*list, const Evas_Lock **);
   const Evas_Public_Data *e = _pd;
   *ret = &(e->locks);
}

EAPI Eina_Bool
evas_key_modifier_is_set(const Evas_Modifier *m, const char *keyname)
{
   Evas_Modifier_Mask num;
   int n;

   if (!m) return 0;
   if (!keyname) return 0;
   n = evas_key_modifier_number(m, keyname);
   if (n < 0) return 0;
   num = (Evas_Modifier_Mask)n;
   num = 1 << num;
   if (m->mask & num) return 1;
   return 0;
}

EAPI Eina_Bool
evas_key_lock_is_set(const Evas_Lock *l, const char *keyname)
{
   Evas_Modifier_Mask num;
   int n;

   if (!l) return 0;
   if (!keyname) return 0;
   n = evas_key_lock_number(l, keyname);
   if (n < 0) return 0;
   num = (Evas_Modifier_Mask)n;
   num = 1 << num;
   if (l->mask & num) return 1;
   return 0;
}

EAPI void
evas_key_modifier_add(Evas *eo_e, const char *keyname)
{
   MAGIC_CHECK(eo_e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();
   eo_do(eo_e, evas_canvas_key_modifier_add(keyname));
}

void
_canvas_key_modifier_add(Eo *eo_e, void *_pd, va_list *list)
{
   const char *keyname = va_arg(*list, const char *);

   Evas_Public_Data *e = _pd;
   if (!keyname) return;
   if (e->modifiers.mod.count >= 64) return;
   evas_key_modifier_del(eo_e, keyname);
   e->modifiers.mod.count++;
   e->modifiers.mod.list = realloc(e->modifiers.mod.list, e->modifiers.mod.count * sizeof(char *));
   e->modifiers.mod.list[e->modifiers.mod.count - 1] = strdup(keyname);
   e->modifiers.mask = 0;
}

EAPI void
evas_key_modifier_del(Evas *eo_e, const char *keyname)
{
   if (!eo_e) return;
   MAGIC_CHECK(eo_e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();
   eo_do(eo_e, evas_canvas_key_modifier_del(keyname));
}

void
_canvas_key_modifier_del(Eo *eo_e EINA_UNUSED, void *_pd, va_list *list)
{
   const char *keyname = va_arg(*list, const char *);

   Evas_Public_Data *e = _pd;
   int i;

   if (!keyname) return;
   for (i = 0; i < e->modifiers.mod.count; i++)
     {
	if (!strcmp(e->modifiers.mod.list[i], keyname))
	  {
	     int j;

	     free(e->modifiers.mod.list[i]);
	     e->modifiers.mod.count--;
	     for (j = i; j < e->modifiers.mod.count; j++)
	       e->modifiers.mod.list[j] = e->modifiers.mod.list[j + 1];
	     e->modifiers.mask = 0;
	     return;
	  }
     }
}

EAPI void
evas_key_lock_add(Evas *eo_e, const char *keyname)
{
   MAGIC_CHECK(eo_e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();
   eo_do(eo_e, evas_canvas_key_lock_add(keyname));
}

void
_canvas_key_lock_add(Eo *eo_e, void *_pd, va_list *list)
{
   const char *keyname = va_arg(*list, const char *);

   if (!keyname) return;
   Evas_Public_Data *e = _pd;
   if (e->locks.lock.count >= 64) return;
   evas_key_lock_del(eo_e, keyname);
   e->locks.lock.count++;
   e->locks.lock.list = realloc(e->locks.lock.list, e->locks.lock.count * sizeof(char *));
   e->locks.lock.list[e->locks.lock.count - 1] = strdup(keyname);
   e->locks.mask = 0;
}

EAPI void
evas_key_lock_del(Evas *eo_e, const char *keyname)
{
   MAGIC_CHECK(eo_e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();
   eo_do(eo_e, evas_canvas_key_lock_del(keyname));
}

void
_canvas_key_lock_del(Eo *eo_e EINA_UNUSED, void *_pd, va_list *list)
{
   const char *keyname = va_arg(*list, const char *);

   int i;
   Evas_Public_Data *e = _pd;
   if (!keyname) return;
   e->locks.mask = 0;
   for (i = 0; i < e->locks.lock.count; i++)
     {
	if (!strcmp(e->locks.lock.list[i], keyname))
	  {
	     int j;

	     free(e->locks.lock.list[i]);
	     e->locks.lock.count--;
	     for (j = i; j < e->locks.lock.count; j++)
	       e->locks.lock.list[j] = e->locks.lock.list[j + 1];
	     e->locks.mask = 0;
	     return;
	  }
     }
}

EAPI void
evas_key_modifier_on(Evas *eo_e, const char *keyname)
{
   MAGIC_CHECK(eo_e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();
   eo_do(eo_e, evas_canvas_key_modifier_on(keyname));
}

void
_canvas_key_modifier_on(Eo *eo_e EINA_UNUSED, void *_pd, va_list *list)
{
   const char *keyname = va_arg(*list, const char *);

   Evas_Modifier_Mask num;
   int n;

   Evas_Public_Data *e = _pd;
   n = (Evas_Modifier_Mask)evas_key_modifier_number(&(e->modifiers), keyname);
   if (n < 0) return;
   num = (Evas_Modifier_Mask)n;
   num = 1 << num;
   e->modifiers.mask |= num;
}

EAPI void
evas_key_modifier_off(Evas *eo_e, const char *keyname)
{
   MAGIC_CHECK(eo_e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();
   eo_do(eo_e, evas_canvas_key_modifier_off(keyname));
}

void
_canvas_key_modifier_off(Eo *eo_e EINA_UNUSED, void *_pd, va_list *list)
{
   const char *keyname = va_arg(*list, const char *);

   Evas_Modifier_Mask num;
   int n;

   Evas_Public_Data *e = _pd;
   n = evas_key_modifier_number(&(e->modifiers), keyname);
   if (n < 0) return;
   num = (Evas_Modifier_Mask)n;
   num = 1 << num;
   e->modifiers.mask &= ~num;
}

EAPI void
evas_key_lock_on(Evas *eo_e, const char *keyname)
{
   MAGIC_CHECK(eo_e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();
   eo_do(eo_e, evas_canvas_key_lock_on(keyname));
}

void
_canvas_key_lock_on(Eo *eo_e EINA_UNUSED, void *_pd, va_list *list)
{
   const char *keyname = va_arg(*list, const char *);

   Evas_Modifier_Mask num;
   int n;

   Evas_Public_Data *e = _pd;
   n = evas_key_lock_number(&(e->locks), keyname);
   if (n < 0) return;
   num = (Evas_Modifier_Mask)n;
   num = 1 << num;
   e->locks.mask |= num;
}

EAPI void
evas_key_lock_off(Evas *eo_e, const char *keyname)
{
   MAGIC_CHECK(eo_e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();
   eo_do(eo_e, evas_canvas_key_lock_off(keyname));
}

void
_canvas_key_lock_off(Eo *eo_e EINA_UNUSED, void *_pd, va_list *list)
{
   const char *keyname = va_arg(*list, const char *);

   Evas_Modifier_Mask num;
   int n;

   Evas_Public_Data *e = _pd;
   n = evas_key_lock_number(&(e->locks), keyname);
   if (n < 0) return;
   num = (Evas_Modifier_Mask)n;
   num = 1 << num;
   e->locks.mask &= ~num;
}

/* errr need to add key grabbing/ungrabbing calls - missing modifier stuff. */

EAPI Evas_Modifier_Mask
evas_key_modifier_mask_get(const Evas *eo_e, const char *keyname)
{
   MAGIC_CHECK(eo_e, Evas, MAGIC_EVAS);
   return 0;
   MAGIC_CHECK_END();
   Evas_Modifier_Mask ret = 0;
   eo_do((Eo *)eo_e, evas_canvas_key_modifier_mask_get(keyname, &ret));
   return ret;
}

void
_canvas_key_modifier_mask_get(Eo *eo_e EINA_UNUSED, void *_pd, va_list *list)
{
   const char *keyname = va_arg(*list, const char *);
   Evas_Modifier_Mask *ret = va_arg(*list, Evas_Modifier_Mask *);

   Evas_Modifier_Mask num;
   int n;
   *ret = 0;

   if (!keyname) return;
   const Evas_Public_Data *e = _pd;
   n = evas_key_modifier_number(&(e->modifiers), keyname);
   if (n < 0)
     {
        *ret = 0;
        return;
     }
   num = (Evas_Modifier_Mask)n;
   *ret = 1 << num;
}
