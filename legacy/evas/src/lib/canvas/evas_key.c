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

/**
 * Returns a handle to the modifiers available in the system.  This is required to check
 * for modifiers with the evas_key_modifier_is_set_get function.
 *
 *
 * @see evas_key_modifier_add
 * @see evas_key_modifier_del
 * @see evas_key_modifier_on
 * @see evas_key_modifier_off
 * @see evas_key_modifier_is_set_get
 *
 * @param e The pointer to the Evas Canvas
 *
 * @return An Evas_Modifier handle to query the modifier subsystem with
 * 	evas_key_modifier_is_set_get, or NULL on error.
 */
EAPI const Evas_Modifier *
evas_key_modifier_get(const Evas *e)
{
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return NULL;
   MAGIC_CHECK_END();
   return &(e->modifiers);
}

/**
 * Returns a handle to the locks available in the system.  This is required to check for
 * locks with the evas_key_lock_is_set_get function.
 *
 * @see evas_key_lock_add
 * @see evas_key_lock_del
 * @see evas_key_lock_on
 * @see evas_key_lock_off
 *
 * @see evas_key_lock_is_set_get
 * @param e The pointer to the Evas Canvas
 *
 * @return An Evas_Lock handle to query the lock subsystem with
 * 	evas_key_lock_is_set_get, or NULL on error.
 */
EAPI const Evas_Lock *
evas_key_lock_get(const Evas *e)
{
   MAGIC_CHECK(e, Evas, MAGIC_EVAS);
   return NULL;
   MAGIC_CHECK_END();
   return &(e->locks);
}

/**
 * Checks the state of a given modifier.  If the modifier is set, such as shift being pressed
 * this function returns true.
 * 
 * @see evas_key_modifier_add
 * @see evas_key_modifier_del
 * @see evas_key_modifier_get
 * @see evas_key_modifier_on
 * @see evas_key_modifier_off
 * 
 * @param m The current modifier set as returned by evas_key_modifier_get.
 * @param keyname The name of the key to check its status.
 *
 * @return 1 if the @p keyname is on, 0 otherwise.
 */
EAPI Evas_Bool
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

/**
 * Checks the state of a given lock.  If the lock is set, such as caps lock, this function
 * returns true.
 *
 * @see evas_key_lock_get
 * @see evas_key_lock_add
 * @see evas_key_lock_del
 * @see evas_key_lock_on
 * @see evas_key_lock_off
 *
 * @param l The current lock set as returned by evas_key_lock_get.
 * @param keyname The name of the lock to add the the list.
 * 
 * @param 1 if the @p keyname kock is set, 0 otherwise.
 */
EAPI Evas_Bool
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

/**
 * Adds the @p keyname to the current list of modifiers.
 * 
 * Modifiers can be keys like shift, alt and ctrl, as well as user defined.  This allows
 * custom modifiers to be added to the evas system as run time.  It is then possible to set
 * and unset the modifier for other parts of the program to check and act on.
 *
 * @see evas_key_modifier_del
 * @see evas_key_modifier_get
 * @see evas_key_modifier_on
 * @see evas_key_modifier_off
 * @see evas_key_modifier_is_set_get
 * 
 * @param e The pointer to the Evas Canvas
 * @param keyname The name of the modifier to add to the list.
 */
EAPI void
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
 * Removes the @p keyname from the current list of modifiers.
 *
 * @see evas_key_modifier_add
 * @see evas_key_modifier_get
 * @see evas_key_modifier_on
 * @see evas_key_modifier_off
 * @see evas_key_modifier_is_set_get
 * 
 * @param e The pointer to the Evas Canvas
 * @param keyname The name of the key to remove from the modifiers list.
 */
EAPI void
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
 * Adds the @p keyname to the current list of locks.
 *
 * Locks can be keys like caps lock, num lock or scroll lock, as well as user defined.  This
 * allows custom locks to be added to the evas system at run time.  It is then possible to
 * set and unset the lock for other parts of the program to check and act on.
 * 
 * @see evas_key_lock_get
 * @see evas_key_lock_del
 * @see evas_key_lock_on
 * @see evas_key_lock_off
 * 
 * @param e The pointer to the Evas Canvas
 * @param keyname The name of the key to remove from the modifier list.
 */
EAPI void
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
 * Removes The @p keyname from the current list of locks.
 *
 * @see evas_key_lock_get
 * @see evas_key_lock_add
 * @see evas_key_lock_on
 * @see evas_key_lock_off
 * 
 * @param e The pointer to the Evas Canvas
 * @param keyname The name of the key to remove from the lock list.
 */
EAPI void
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
 * Enables or turns on the modifier with name @p keyname.
 * 
 * @see evas_key_modifier_add
 * @see evas_key_modifier_get
 * @see evas_key_modifier_off
 * @see evas_key_modifier_is_set_get
 * 
 * @param e The pointer to the Evas Canvas
 * @param keyname The name of the modifier to set.
 */
EAPI void
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
 * Disables or turns off the modifier with name @p keyname.
 * 
 * @see evas_key_modifier_add
 * @see evas_key_modifier_get
 * @see evas_key_modifier_on
 * @see evas_key_modifier_is_set_get
 * 
 * @param e The pointer to the Evas Canvas
 * @param keyname The name of the modifier to un-set.
 */
EAPI void
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
 * Enables or turns on the lock with name @p keyname.
 *
 * @see evas_key_lock_get
 * @see evas_key_lock_add
 * @see evas_key_lock_del
 * @see evas_key_lock_off
 * 
 * @param e The pointer to the Evas Canvas
 * @param keyname The name of the lock to set.
 */
EAPI void
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
 * Disables or turns off the lock with name @p keyname.
 *
 * @see evas_key_lock_get
 * @see evas_key_lock_add
 * @see evas_key_lock_del
 * @see evas_key_lock_on
 * 
 * @param e The pointer to the Evas Canvas
 * @param keyname The name of the lock to un-set.
 */
EAPI void
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
 * Creates a bit mask from the @p keyname for use with events to check for the presence
 * of the @p keyname modifier.
 *
 * @see evas_key_modifier_add
 * @see evas_key_modifier_get
 * @see evas_key_modifier_on
 * @see evas_key_modifier_off
 * @see evas_key_modifier_is_set_get
 * 
 * @param keyname The name of the modifier to create the mask for.
 *
 * @returns the bit mask or 0 if the @p keyname wasn't registered as a modifier.
 */
EAPI Evas_Modifier_Mask
evas_key_modifier_mask_get(const Evas *e, const char *keyname)
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
