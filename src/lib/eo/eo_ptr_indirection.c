#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "eo_ptr_indirection.h"

extern Eina_Thread _efl_object_main_thread;

//////////////////////////////////////////////////////////////////////////

Eo_Id_Data       *_eo_table_data_shared;
Eo_Id_Table *_eo_table_data_shared_data;


Eo_Id_Data _eo_main_id_data;
Eo_Id_Table _eo_main_id_table;

Eo_Id_Data *_eo_id_data;

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
                        const Eo_Id_Data *data,
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
                  (unsigned long)((obj_id >> SHIFT_ENTRY_ID)) & MASK_ENTRY_ID,
                  (int)(obj_id >> REF_TAG_SHIFT) & 0x1);
   _eo_log_obj_report(obj_id, EINA_LOG_LEVEL_ERR, func_name, file, line);
}

static long unsigned int _calls = 0;
static long unsigned int _hits = 0;

_Eo_Object *
_eo_obj_pointer_get(const Eo_Id obj_id, const char *restrict func_name, const char *restrict file, int line)
{
   if (EINA_UNLIKELY(! obj_id)) goto err_null;

   static const void *const jump[] = {
      &&do_domain_main,
      &&do_domain_shared,
      &&do_domain_thread,
      &&do_domain_other,
   };
   const unsigned int domain = (obj_id >> SHIFT_DOMAIN) & MASK_DOMAIN;
   goto *jump[domain];

do_domain_main: EINA_HOT
     {
//        __builtin_prefetch(&_eo_main_id_table, 0, 3);
        if (obj_id == _eo_main_id_table.cache.id)
          return _eo_main_id_table.cache.object;

        /* XXX This could definitely be done in one go with vectorization */
        const size_t entry_id = (obj_id >> SHIFT_ENTRY_ID) & MASK_ENTRY_ID;
        const unsigned int generation = obj_id & MASK_GENERATIONS;

        // get tag bit to check later down below - pipelining
        const Eo_Id tag_bit = (obj_id) & MASK_OBJ_TAG;
        if (EINA_UNLIKELY(!tag_bit ||
                 (entry_id >= _eo_main_id_table.count)))
          goto main_err;


        register const Eo_Id_Entry *const entry = &(_eo_main_id_table.entries[entry_id]);
//        __builtin_prefetch(entry, 0, 0);
        if (EINA_LIKELY(entry->data.generation == generation))
          {
             // Cache the result of that lookup
             _eo_main_id_table.cache.object = entry->data.ptr;
             _eo_main_id_table.cache.id = obj_id;
             return _eo_main_id_table.cache.object;
          }

        goto main_err;
     }


main_err: EINA_COLD
   _eo_obj_pointer_invalid(obj_id, &_eo_main_id_data, domain, func_name, file, line);
   return NULL;



do_domain_thread: EINA_COLD
do_domain_other: EINA_COLD
     {
        Eo_Id_Data *const data = _eo_id_data_get();
        Eo_Id_Table *const table = _eo_id_data_table_get(data, domain);
        if (EINA_UNLIKELY(!table)) goto err;


        if (obj_id == table->cache.id)
           return table->cache.object;

        const size_t entry_id = (obj_id >> SHIFT_ENTRY_ID) & MASK_ENTRY_ID;
        const unsigned int generation = obj_id & MASK_GENERATIONS;

        // get tag bit to check later down below - pipelining
        const Eo_Id tag_bit = (obj_id) & MASK_OBJ_TAG;
        if (EINA_UNLIKELY(!tag_bit ||
                          (entry_id >= table->count))) goto err;

        register const Eo_Id_Entry *const entry = &(table->entries[entry_id]);
        if (EINA_LIKELY(entry->data.generation == generation))
          {
             // Cache the result of that lookup
             table->cache.object = entry->data.ptr;
             table->cache.id = obj_id;
             return entry->data.ptr;
          }
        goto err;

err_null: EINA_COLD
   return NULL;
err: EINA_COLD
   _eo_obj_pointer_invalid(obj_id, data, domain, func_name, file, line);
   return NULL;
     }

do_domain_shared: EINA_COLD
     {
        Eo_Id_Data *const data = _eo_table_data_shared;
        Eo_Id_Table *const table = _eo_table_data_shared_data;

        eina_lock_take(&(table->obj_lock));
        if (EINA_LIKELY(obj_id == table->cache.id))
        // yes we return keeping the lock locked. thats why
        // you must call _eo_obj_pointer_done() wrapped
        // by EO_OBJ_DONE() to release
          return table->cache.object;

        const size_t entry_id = (obj_id >> SHIFT_ENTRY_ID) & MASK_ENTRY_ID;
        const unsigned int generation = obj_id & MASK_GENERATIONS;

        // get tag bit to check later down below - pipelining
        const Eo_Id tag_bit = (obj_id) & MASK_OBJ_TAG;
        if (EINA_UNLIKELY((!tag_bit ||
                           entry_id >= table->count))) goto err_shared;

        Eo_Id_Entry *const entry = &(table->entries[entry_id]);
        if (EINA_LIKELY(entry->data.generation == generation))
          {
             // Cache the result of that lookup
             table->cache.object = entry->data.ptr;
             table->cache.id = obj_id;
             // yes we return keeping the lock locked. thats why
             // you must call _eo_obj_pointer_done() wrapped
             // by EO_OBJ_DONE() to release
             return entry->data.ptr;
          }
        goto err_shared;

err_shared_null: EINA_COLD
   eina_lock_release(&(_eo_table_data_shared_data->obj_lock));
   eina_log_print(_eo_log_dom,
                  EINA_LOG_LEVEL_DBG,
                  file, func_name, line,
                  "obj_id is NULL. Possibly unintended access?");
   return NULL;
err_shared: EINA_COLD
   eina_lock_release(&(_eo_table_data_shared_data->obj_lock));
err_shared_err: EINA_COLD
   _eo_obj_pointer_invalid(obj_id, data, domain, func_name, file, line);
   return NULL;
     }
}

void eo_dump_stats(void)
{
 //  printf("Eo ptr resolution. %lu calls, %lu hits\n",
 //        _calls, _hits);
}
