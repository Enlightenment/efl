#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "eo_ptr_indirection.h"

extern Eina_Thread _efl_object_main_thread;

//////////////////////////////////////////////////////////////////////////

Eina_TLS          _eo_table_data;
Eo_Id_Data       *_eo_table_data_shared = NULL;
Eo_Id_Table_Data *_eo_table_data_shared_data = NULL;

//////////////////////////////////////////////////////////////////////////

void
_eo_pointer_error(const Eo *obj_id, const char *func_name, const char *file, int line, const char *fmt, ...)
{
   /* NOTE: this function exists to allow easy breakpoint on pointer errors */
   va_list args;
   va_start(args, fmt);
   eina_log_vprint(_eo_log_dom, EINA_LOG_LEVEL_ERR, file, func_name, line, fmt, args);
   va_end(args);
   _eo_log_obj_report((Eo_Id)obj_id, EINA_LOG_LEVEL_ERR, func_name, file, line);
}

static void
_eo_obj_pointer_invalid(const Eo_Id obj_id,
                        Eo_Id_Data *data,
                        unsigned char domain,
                        const char *func_name,
                        const char *file,
                        int line)
{
   Eina_Thread thread = eina_thread_self();
   const char *tself = "main";
   const char *type = "object";
   const char *reason = "This ID has probably been deleted";
   char tbuf[128];
   if (obj_id & MASK_CLASS_TAG) type = "class";
   if (thread != _efl_object_main_thread)
     {
        snprintf(tbuf, sizeof(tbuf), "%p", (void *)thread);
        tself = tbuf;
     }

   if (!data->tables[(int)data->local_domain])
     reason = "This ID does not seem to belong to this thread";
   else if ((Efl_Id_Domain)domain == EFL_ID_DOMAIN_SHARED)
     reason = "This shared ID has probably been deleted";

   eina_log_print(_eo_log_dom, EINA_LOG_LEVEL_ERR,
                  file, func_name, line,
                  "Eo ID %p is not a valid %s. "
                  "Current thread: %s. "
                  "%s or this was never a valid %s ID. "
                  "(domain=%i, current_domain=%i, local_domain=%i, "
                  "available_domains=[%s %s %s %s], "
                  "generation=%lx, id=%lx, ref=%i)",
                  (void *)obj_id,
                  type,
                  tself,
                  reason,
                  type,
                  (int)domain,
                  (int)data->domain_stack[data->stack_top],
                  (int)data->local_domain,
                  (data->tables[0]) ? "0" : " ",
                  (data->tables[1]) ? "1" : " ",
                  (data->tables[2]) ? "2" : " ",
                  (data->tables[3]) ? "3" : " ",
                  (unsigned long)(obj_id & MASK_GENERATIONS),
                  (unsigned long)(obj_id >> SHIFT_ENTRY_ID) & (MAX_ENTRY_ID | MAX_TABLE_ID | MAX_MID_TABLE_ID),
                  (int)(obj_id >> REF_TAG_SHIFT) & 0x1);
   _eo_log_obj_report(obj_id, EINA_LOG_LEVEL_ERR, func_name, file, line);
}

_Eo_Object *
_eo_obj_pointer_get(const Eo_Id obj_id, const char *func_name, const char *file, int line)
{
   _Eo_Id_Entry *entry;
   Generation_Counter generation;
   Table_Index mid_table_id, table_id, entry_id;
   Eo_Id tag_bit;
   Eo_Id_Data *data;
   Eo_Id_Table_Data *tdata;
   unsigned char domain;

   // NULL objects will just be sensibly ignored. not worth complaining
   // every single time.

   data = _eo_table_data_get();
   EINA_PREFETCH(&(data->tables[0]));
   domain = (obj_id >> SHIFT_DOMAIN) & MASK_DOMAIN;
   tdata = _eo_table_data_table_get(data, domain);
   EINA_PREFETCH(&(tdata->cache.id));
   if (EINA_UNLIKELY(!tdata)) goto err;


   if (EINA_LIKELY(domain != EFL_ID_DOMAIN_SHARED))
     {
        if (obj_id == tdata->cache.id)
          return tdata->cache.object;

        mid_table_id = (obj_id >> SHIFT_MID_TABLE_ID) & MASK_MID_TABLE_ID;
        EINA_PREFETCH(&(tdata->eo_ids_tables[mid_table_id]));
        table_id = (obj_id >> SHIFT_TABLE_ID) & MASK_TABLE_ID;
        entry_id = (obj_id >> SHIFT_ENTRY_ID) & MASK_ENTRY_ID;
        generation = obj_id & MASK_GENERATIONS;

        // get tag bit to check later down below - pipelining
        tag_bit = (obj_id) & MASK_OBJ_TAG;
        if (!obj_id) goto err_null;
        else if (!tag_bit) goto err;

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
                       return entry->ptr;
                    }
               }
          }
        goto err;
     }
   else
     {
        eina_lock_take(&(_eo_table_data_shared_data->obj_lock));
        if (obj_id == tdata->cache.id)
        // yes we return keeping the lock locked. thats why
        // you must call _eo_obj_pointer_done() wrapped
        // by EO_OBJ_DONE() to release
          return tdata->cache.object;

        mid_table_id = (obj_id >> SHIFT_MID_TABLE_ID) & MASK_MID_TABLE_ID;
        EINA_PREFETCH(&(tdata->eo_ids_tables[mid_table_id]));
        table_id = (obj_id >> SHIFT_TABLE_ID) & MASK_TABLE_ID;
        entry_id = (obj_id >> SHIFT_ENTRY_ID) & MASK_ENTRY_ID;
        generation = obj_id & MASK_GENERATIONS;

        // get tag bit to check later down below - pipelining
        tag_bit = (obj_id) & MASK_OBJ_TAG;
        if (!obj_id) goto err_shared_null;
        else if (!tag_bit) goto err_shared;

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
                       // yes we return keeping the lock locked. thats why
                       // you must call _eo_obj_pointer_done() wrapped
                       // by EO_OBJ_DONE() to release
                       return entry->ptr;
                    }
               }
          }
        goto err_shared;
     }
err_shared_null:
   eina_lock_release(&(_eo_table_data_shared_data->obj_lock));
err_null:
   eina_log_print(_eo_log_dom,
                  EINA_LOG_LEVEL_DBG,
                  file, func_name, line,
                  "obj_id is NULL. Possibly unintended access?");
   return NULL;
err_shared:
   eina_lock_release(&(_eo_table_data_shared_data->obj_lock));
err:
   _eo_obj_pointer_invalid(obj_id, data, domain, func_name, file, line);
   return NULL;
}
