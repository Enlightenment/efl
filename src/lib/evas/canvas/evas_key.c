#include "evas_common_private.h"
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

EVAS_API const Evas_Modifier*
evas_key_modifier_get(const Evas *eo_e)
{
   EVAS_LEGACY_API(eo_e, e, NULL);
   return &(e->modifiers);
}

EVAS_API const Evas_Lock*
evas_key_lock_get(const Evas *eo_e)
{
   EVAS_LEGACY_API(eo_e, e, NULL);
   return &(e->locks);
}

static Eina_Bool
_key_is_set(int n, Eina_Hash *masks, const Evas_Device *seat)
{
   Evas_Modifier_Mask num, *seat_mask;

   if (n < 0) return 0;
   else if (n >= 64) return 0;
   num = (Evas_Modifier_Mask)n;
   num = 1ULL << num;
   seat_mask = eina_hash_find(masks, &seat);
   if (!seat_mask) return 0;
   if (*seat_mask & num) return 1;
   return 0;
}

EVAS_API Eina_Bool
evas_seat_key_modifier_is_set(const Evas_Modifier *m, const char *keyname,
                              const Evas_Device *seat)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(m, EINA_FALSE);
   if (!seat)
     seat = m->e->default_seat;
   EINA_SAFETY_ON_NULL_RETURN_VAL(seat, 0);
   if (!keyname) return 0;
   return _key_is_set(evas_key_modifier_number(m, keyname), m->masks, seat);
}

EVAS_API Eina_Bool
evas_key_modifier_is_set(const Evas_Modifier *m, const char *keyname)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(m, EINA_FALSE);
   return evas_seat_key_modifier_is_set(m, keyname, NULL);
}

EVAS_API Eina_Bool
evas_key_lock_is_set(const Evas_Lock *l, const char *keyname)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(l, EINA_FALSE);
   return evas_seat_key_lock_is_set(l, keyname, NULL);
}

EVAS_API Eina_Bool
evas_seat_key_lock_is_set(const Evas_Lock *l, const char *keyname,
                          const Evas_Device *seat)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(l, EINA_FALSE);
   if (!seat)
     seat = l->e->default_seat;
   EINA_SAFETY_ON_NULL_RETURN_VAL(seat, 0);
   if (!keyname) return 0;
   return _key_is_set(evas_key_lock_number(l, keyname), l->masks, seat);
}

EOLIAN void
_evas_canvas_key_modifier_add(Eo *eo_e, Evas_Public_Data *e, const char *keyname)
{
   if (!keyname) return;
   if (e->modifiers.mod.count >= 64) return;
   evas_key_modifier_del(eo_e, keyname);
   e->modifiers.mod.count++;
   e->modifiers.mod.list = realloc(e->modifiers.mod.list, e->modifiers.mod.count * sizeof(char *));
   e->modifiers.mod.list[e->modifiers.mod.count - 1] = strdup(keyname);
   eina_hash_free_buckets(e->modifiers.masks);
}

EOLIAN void
_evas_canvas_key_modifier_del(Eo *eo_e EINA_UNUSED, Evas_Public_Data *e, const char *keyname)
{
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
             eina_hash_free_buckets(e->modifiers.masks);
	     return;
	  }
     }
}

EOLIAN void
_evas_canvas_key_lock_add(Eo *eo_e, Evas_Public_Data *e, const char *keyname)
{
   if (!keyname) return;
   if (e->locks.lock.count >= 64) return;
   evas_key_lock_del(eo_e, keyname);
   e->locks.lock.count++;
   e->locks.lock.list = realloc(e->locks.lock.list, e->locks.lock.count * sizeof(char *));
   e->locks.lock.list[e->locks.lock.count - 1] = strdup(keyname);
   eina_hash_free_buckets(e->locks.masks);
}

EOLIAN void
_evas_canvas_key_lock_del(Eo *eo_e EINA_UNUSED, Evas_Public_Data *e, const char *keyname)
{
   int i;
   if (!keyname) return;
   for (i = 0; i < e->locks.lock.count; i++)
     {
	if (!strcmp(e->locks.lock.list[i], keyname))
	  {
	     int j;

	     free(e->locks.lock.list[i]);
	     e->locks.lock.count--;
	     for (j = i; j < e->locks.lock.count; j++)
	       e->locks.lock.list[j] = e->locks.lock.list[j + 1];
             eina_hash_free_buckets(e->locks.masks);
	     return;
	  }
     }
}

static void
_mask_set(int n, Eina_Hash *masks, Efl_Input_Device *seat, Eina_Bool add)
{
   Evas_Modifier_Mask *current_mask;
   Evas_Modifier_Mask num;

   if (n < 0 || n > 63) return;
   num = 1ULL << n;

   current_mask = eina_hash_find(masks, &seat);
   if (add)
     {
        if (!current_mask)
          {
             current_mask = calloc(1, sizeof(Evas_Modifier_Mask));
             EINA_SAFETY_ON_NULL_RETURN(current_mask);
             eina_hash_add(masks, &seat, current_mask);
          }
        *current_mask |= num;
     }
   else
     {
        if (!current_mask) return;
        *current_mask &= ~num;
        if (!(*current_mask))
          eina_hash_del_by_key(masks, &seat);
     }
}

EOLIAN void
_evas_canvas_seat_key_modifier_on(Eo *eo_e EINA_UNUSED, Evas_Public_Data *e,
                                  const char *keyname, Efl_Input_Device *seat)
{
   if (!seat)
     seat = e->default_seat;
   EINA_SAFETY_ON_NULL_RETURN(seat);
   if (efl_input_device_type_get(seat) != EFL_INPUT_DEVICE_TYPE_SEAT) return;
   _mask_set(evas_key_modifier_number(&(e->modifiers), keyname),
             e->modifiers.masks, seat, EINA_TRUE);
}

EOLIAN void
_evas_canvas_seat_key_modifier_off(Eo *eo_e EINA_UNUSED, Evas_Public_Data *e,
                                   const char *keyname, Efl_Input_Device *seat)
{
   if (!seat)
     seat = e->default_seat;
   EINA_SAFETY_ON_NULL_RETURN(seat);
   _mask_set(evas_key_modifier_number(&(e->modifiers), keyname),
             e->modifiers.masks, seat, EINA_FALSE);
}

EOLIAN void
_evas_canvas_key_modifier_on(Eo *eo_e, Evas_Public_Data *e, const char *keyname)
{
   _evas_canvas_seat_key_modifier_on(eo_e, e, keyname, NULL);
}

EOLIAN void
_evas_canvas_key_modifier_off(Eo *eo_e, Evas_Public_Data *e,
                              const char *keyname)
{
   _evas_canvas_seat_key_modifier_off(eo_e, e, keyname, NULL);
}

EOLIAN void
_evas_canvas_seat_key_lock_on(Eo *eo_e EINA_UNUSED, Evas_Public_Data *e,
                              const char *keyname, Efl_Input_Device *seat)
{
   if (!seat)
     seat = e->default_seat;
   EINA_SAFETY_ON_NULL_RETURN(seat);
   if (efl_input_device_type_get(seat) != EFL_INPUT_DEVICE_TYPE_SEAT) return;
   _mask_set(evas_key_lock_number(&(e->locks), keyname), e->locks.masks,
             seat, EINA_TRUE);
}

EOLIAN void
_evas_canvas_seat_key_lock_off(Eo *eo_e EINA_UNUSED, Evas_Public_Data *e,
                               const char *keyname, Efl_Input_Device *seat)
{
   if (!seat)
     seat = e->default_seat;
   EINA_SAFETY_ON_NULL_RETURN(seat);
   _mask_set(evas_key_lock_number(&(e->locks), keyname), e->locks.masks,
             seat, EINA_FALSE);
}

EOLIAN void
_evas_canvas_key_lock_on(Eo *eo_e, Evas_Public_Data *e, const char *keyname)
{
   _evas_canvas_seat_key_lock_on(eo_e, e, keyname, NULL);
}

EOLIAN void
_evas_canvas_key_lock_off(Eo *eo_e, Evas_Public_Data *e, const char *keyname)
{
   _evas_canvas_seat_key_lock_off(eo_e, e, keyname, NULL);
}

/* errr need to add key grabbing/ungrabbing calls - missing modifier stuff. */

EVAS_API Evas_Modifier_Mask
evas_key_modifier_mask_get(const Evas *eo_e, const char *keyname)
{
   int n;

   if (!keyname) return 0;
   EVAS_LEGACY_API(eo_e, e, 0);
   n = evas_key_modifier_number(&(e->modifiers), keyname);
   if (n < 0 || n > 63) return 0;
   return 1ULL << n;
}

