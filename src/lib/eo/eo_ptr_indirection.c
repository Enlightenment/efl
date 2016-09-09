#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "eo_ptr_indirection.h"

//////////////////////////////////////////////////////////////////////////

Eina_TLS    _eo_table_data;
Eo_Id_Data *_eo_table_data_shared = NULL;

//////////////////////////////////////////////////////////////////////////

void
_eo_pointer_error(const char *msg)
{
   ERR("%s", msg);
}

_Eo_Object *
_eo_obj_pointer_get(const Eo_Id obj_id)
{
#ifdef HAVE_EO_ID
   _Eo_Id_Entry *entry;
   _Eo_Object *ptr;
   Generation_Counter generation;
   Table_Index mid_table_id, table_id, entry_id;
   Eo_Id tag_bit;
   Eo_Id_Data *data;
   Eo_Id_Table_Data *tdata;
   unsigned char domain;

   // NULL objects will just be sensibly ignored. not worth complaining
   // every single time.

   domain = (obj_id >> SHIFT_DOMAIN) & MASK_DOMAIN;
   data = _eo_table_data_get();
   tdata = _eo_table_data_table_get(data, domain);
   if (!tdata) goto err_invalid;


   if (EINA_LIKELY(domain != EFL_ID_DOMAIN_SHARED))
     {
        if (obj_id == tdata->cache.id)
          {
             ptr = tdata->cache.object;
             return ptr;
          }

        // get tag bit to check later down below - pipelining
        tag_bit = (obj_id) & MASK_OBJ_TAG;
        if (!obj_id) goto err_null;
        else if (!tag_bit) goto err_invalid;

        EO_DECOMPOSE_ID(obj_id, mid_table_id, table_id, entry_id, generation);

        // Check the validity of the entry
        if (tdata->eo_ids_tables[mid_table_id])
          {
             _Eo_Ids_Table *tab = TABLE_FROM_IDS;

             if (tab)
               {
                  entry = &(tab->entries[entry_id]);
                  if (entry->active && (entry->generation == generation))
                    {
                       // Cache the result of that lookup
                       tdata->cache.object = entry->ptr;
                       tdata->cache.id = obj_id;
                       ptr = entry->ptr;
                       return ptr;
                    }
               }
          }
        goto err;
     }
   else
     {
        eina_spinlock_take(&(tdata->lock));
        if (obj_id == tdata->cache.id)
          {
             ptr = tdata->cache.object;
             goto shared_ok;
          }

        // get tag bit to check later down below - pipelining
        tag_bit = (obj_id) & MASK_OBJ_TAG;
        if (!obj_id) goto err_null;
        else if (!tag_bit) goto err_invalid;

        EO_DECOMPOSE_ID(obj_id, mid_table_id, table_id, entry_id, generation);

        // Check the validity of the entry
        if (tdata->eo_ids_tables[mid_table_id])
          {
             _Eo_Ids_Table *tab = TABLE_FROM_IDS;

             if (tab)
               {
                  entry = &(tab->entries[entry_id]);
                  if (entry->active && (entry->generation == generation))
                    {
                       // Cache the result of that lookup
                       tdata->cache.object = entry->ptr;
                       tdata->cache.id = obj_id;
                       ptr = entry->ptr;
                       goto shared_ok;
                    }
               }
          }
        eina_spinlock_release(&(tdata->lock));
        goto err;
shared_ok:
        eina_spinlock_release(&(tdata->lock));
        return ptr;
     }
err_null:
   DBG("obj_id is NULL. Possibly unintended access?");
   return NULL;
err_invalid:
err:
   ERR("obj_id %p is not a valid object. Maybe it has been freed or does not belong to your thread?",
       (void *)obj_id);
   return NULL;
#else
   return (_Eo_Object *) obj_id;
#endif
}
