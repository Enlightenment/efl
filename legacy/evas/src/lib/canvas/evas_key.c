#include "evas_common.h"
#include "evas_private.h"
#include "Evas.h"

/* private calls */

static int evas_key_modifier_number(Evas_Modifier *m, const char *keyname);
static int evas_key_lock_number(Evas_Lock *l, const char *keyname);

static int
evas_key_modifier_number(Evas_Modifier *m, const char *keyname)
{
   int i;
   
   for (i = 0; i < m->mod.count; i++)
     {
	if (!strcmp(m->mod.list[i], keyname)) return i;
     }
   return -1;
}

static int
evas_key_lock_number(Evas_Lock *l, const char *keyname)
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

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 * 
 */
Evas_Modifier *
evas_key_modifier_get(Evas *e)
{
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return NULL;
   MAGIC_CHECK_END();   
   return &(e->modifiers);
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 * 
 */
Evas_Lock *
evas_key_lock_get(Evas *e)
{
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return NULL;
   MAGIC_CHECK_END();
   return &(e->locks);
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 * 
 */
Evas_Bool
evas_key_modifier_is_set_get(Evas_Modifier *m, const char *keyname)
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

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 * 
 */
Evas_Bool
evas_key_lock_is_set_get(Evas_Lock *l, const char *keyname)
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

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 * 
 */
void
evas_key_modifier_add(Evas *e, const char *keyname)
{
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();
   if (!keyname) return;
   if (e->modifiers.mod.count >= 64) return;
   evas_key_modifier_del(e, keyname);
   e->modifiers.mod.count++;
   e->modifiers.mod.list = realloc(e->modifiers.mod.list, e->modifiers.mod.count * sizeof(char *));
   e->modifiers.mod.list[e->modifiers.mod.count - 1] = strdup(keyname);
   e->modifiers.mask = 0;
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 * 
 */
void
evas_key_modifier_del(Evas *e, const char *keyname)
{
   int i;
   
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();
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

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 * 
 */
void
evas_key_lock_add(Evas *e, const char *keyname)
{
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();   
   if (!keyname) return;
   if (e->locks.lock.count >= 64) return;
   evas_key_lock_del(e, keyname);
   e->locks.lock.count++;
   e->locks.lock.list = realloc(e->locks.lock.list, e->locks.lock.count * sizeof(char *));
   e->locks.lock.list[e->locks.lock.count - 1] = strdup(keyname);
   e->locks.mask = 0;
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 * 
 */
void
evas_key_lock_del(Evas *e, const char *keyname)
{
   int i;
   
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();   
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

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 * 
 */
void
evas_key_modifier_on(Evas *e, const char *keyname)
{
   Evas_Modifier_Mask num;
   int n;
   
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();
   n = (Evas_Modifier_Mask)evas_key_modifier_number(&(e->modifiers), keyname);
   if (n < 0) return;
   num = (Evas_Modifier_Mask)n;
   num = 1 << num;
   e->modifiers.mask |= num;
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 * 
 */
void
evas_key_modifier_off(Evas *e, const char *keyname)
{
   Evas_Modifier_Mask num;
   int n;
   
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();   
   n = evas_key_modifier_number(&(e->modifiers), keyname);
   if (n < 0) return;
   num = (Evas_Modifier_Mask)n;
   num = 1 << num;
   e->modifiers.mask &= ~num;
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 * 
 */
void
evas_key_lock_on(Evas *e, const char *keyname)
{
   Evas_Modifier_Mask num;
   int n;
   
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();   
   n = evas_key_lock_number(&(e->locks), keyname);
   if (n < 0) return;
   num = (Evas_Modifier_Mask)n;
   num = 1 << num;
   e->locks.mask |= num;
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 * 
 */
void
evas_key_lock_off(Evas *e, const char *keyname)
{
   Evas_Modifier_Mask num;
   int n;
   
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return;
   MAGIC_CHECK_END();   
   n = evas_key_lock_number(&(e->locks), keyname);
   if (n < 0) return;
   num = (Evas_Modifier_Mask)n;
   num = 1 << num;
   e->locks.mask &= ~num;
}

/* errr need to add key grabbing/ungrabbing calls - missing modifier stuff. */

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 * 
 */
Evas_Modifier_Mask
evas_key_modifier_mask_get(Evas *e, const char *keyname)
{
   Evas_Modifier_Mask num;
   int n;
   
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return 0;
   MAGIC_CHECK_END();
   if (!keyname) return 0;
   n = evas_key_modifier_number(&(e->modifiers), keyname);
   if (n < 0) return 0;
   num = (Evas_Modifier_Mask)n;
   return 1 << num;
}
