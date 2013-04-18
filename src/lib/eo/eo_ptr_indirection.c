#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "eo_ptr_indirection.h"

/* Start of pointer indirection:
 *
 * This feature is responsible of hiding from the developer the real pointer of
 * the Eo object to supply a better memory management by preventing bad usage
 * of the pointers.
 *
 * Eo * is no more a pointer but an index to an entry into a ids table.
 * For a better memory usage, we don't allocate all the tables at the beginning,
 * but only when needed (i.e no more empty entries in allocated tables.
 * In addition, tables are composed of intermediate tables, this for memory
 * optimizations. Finding the different table, intermediate table and relative
 * entry is done by bits manipulation of the id:
 *
 * id = Table | Inter_table | Entry | Generation
 *
 * Generation helps finding abuse of ids. When an entry is assigned to an
 * object, a generation is inserted into the id. If the developer uses this id
 * although the object is freed and another one has replaced it into the same
 * entry of the table, the generation will be different and an error will
 * occur when accessing with the old id.
 *
 * Each table is composed of:
 * - pointers to the objects
 * - generations assigned to the objects
 * - a boolean table indicating if an entry is active
 * - an index 'start' indicating which entry is the next one to use.
 * - a queue that will help us to store the unused entries. It stores only the
 *   entries that have been used at least one time. The entries that have
 *   never been used are "pointed" by the start parameter.
 * When an entry is searched into a table, we first try to pop from the
 * queue. If a NULL value is returned, we have to use one of the entries that
 * have never been used. If a such entry doesn't exist, we pass to the next
 * table. Otherwise, we reserve this entry to the object pointer and create
 * the id with the table id, the intermediate table id, the entry and a
 * generation.
 * When an object is freed, the entry into the table is released by pushing
 * it into the queue.
 */

#if SIZEOF_UINTPTR_T == 4
/* 32 bits */
# define BITS_FOR_IDS_TABLE 8
# define BITS_FOR_IDS_INTER_TABLE 4
# define BITS_FOR_ID_IN_TABLE 10
# define BITS_FOR_GENERATION_COUNTER 10
#else
/* 64 bits */
# define BITS_FOR_IDS_TABLE 8
# define BITS_FOR_IDS_INTER_TABLE 20
# define BITS_FOR_ID_IN_TABLE 16
# define BITS_FOR_GENERATION_COUNTER 20
#endif

typedef uintptr_t Table_Index;

/* Shifts macros to manipulate the Eo id */
#define SHIFT_FOR_IDS_TABLE \
   (BITS_FOR_IDS_INTER_TABLE + BITS_FOR_ID_IN_TABLE + BITS_FOR_GENERATION_COUNTER)

#define SHIFT_FOR_IDS_INTER_TABLE \
   (BITS_FOR_ID_IN_TABLE + BITS_FOR_GENERATION_COUNTER)

#define SHIFT_FOR_ID_IN_TABLE (BITS_FOR_GENERATION_COUNTER)

/* Maximum ranges */
#define MAX_IDS_TABLES (1 << BITS_FOR_IDS_TABLE)
#define MAX_IDS_INTER_TABLES (1 << BITS_FOR_IDS_INTER_TABLE)
#define MAX_IDS_PER_TABLE (1 << BITS_FOR_ID_IN_TABLE)
#define MAX_GENERATIONS (1 << BITS_FOR_GENERATION_COUNTER)

/* Table */
typedef struct
{
   /* Pointers of objects stored in table */
   _Eo *ptrs[MAX_IDS_PER_TABLE];
   /* Generations */
   Table_Index generation[MAX_IDS_PER_TABLE];
   /* Active flags */
   char active[MAX_IDS_PER_TABLE >> 3];
   /* Queue to handle free entries */
   Eina_Trash *queue;
   /* Indicates where start the "never used" entries */
   Table_Index start;
} _Eo_Ids_Table;

/* Tables handling pointers indirection */
_Eo_Ids_Table **_eo_ids_tables[MAX_IDS_TABLES] = { NULL };

/* Next generation to use when assigning a new entry to a Eo pointer */
Table_Index _eo_generation_counter;

/* Internal macro for active flag manipulation */
#define _ENTRY_ACTIVE_DO_OP(table, id_in_table, op) \
       (table)->active[(id_in_table) >> 3] op (1 << ((id_in_table) % 8))

/* Macro that indicates if an entry is active */
#define IS_ENTRY_ACTIVE(table, id_in_table) \
       (_ENTRY_ACTIVE_DO_OP(table, id_in_table, &))
/* Macro that activates an entry */
#define ACTIVATE_ENTRY(table, id_in_table)  \
       _ENTRY_ACTIVE_DO_OP(table, id_in_table, |=)
/* Macro that de-activates an entry */
#define DEACTIVATE_ENTRY(table, id_in_table)  \
       _ENTRY_ACTIVE_DO_OP(table, id_in_table, &=~)

/* Macro used to compose an Eo id */
#define EO_COMPOSE_ID(TABLE, INTER_TABLE, ENTRY, GENERATION)                        \
   (Eo_Id)(((TABLE & (MAX_IDS_TABLES - 1)) << SHIFT_FOR_IDS_TABLE) |                \
         ((INTER_TABLE & (MAX_IDS_INTER_TABLES - 1)) << SHIFT_FOR_IDS_INTER_TABLE) |\
         ((ENTRY & (MAX_IDS_PER_TABLE - 1)) << SHIFT_FOR_ID_IN_TABLE) |             \
         (GENERATION & (MAX_GENERATIONS - 1) ))

/* Macro to extract from an Eo id the indexes of the tables */
#define EO_DECOMPOSE_ID(ID, TABLE, INTER_TABLE, ENTRY, GENERATION) \
   TABLE = (ID >> SHIFT_FOR_IDS_TABLE) & (MAX_IDS_TABLES - 1); \
   INTER_TABLE = (ID >> SHIFT_FOR_IDS_INTER_TABLE) & (MAX_IDS_INTER_TABLES - 1); \
   ENTRY = (ID >> SHIFT_FOR_ID_IN_TABLE) & (MAX_IDS_PER_TABLE - 1); \
   GENERATION = ID & (MAX_GENERATIONS - 1); \

/* Macro used for readability */
#define ID_TABLE _eo_ids_tables[table_id][int_table_id]

_Eo *
_eo_obj_pointer_get(const Eo_Id obj_id)
{
#ifdef HAVE_EO_ID
   Table_Index table_id, int_table_id, entry_id, generation;

   EO_DECOMPOSE_ID((Table_Index) obj_id, table_id, int_table_id, entry_id, generation);

   /* Checking the validity of the entry */
   if (_eo_ids_tables[table_id] && ID_TABLE && IS_ENTRY_ACTIVE(ID_TABLE, entry_id) &&
         ID_TABLE->generation[entry_id] == generation)
      return ID_TABLE->ptrs[entry_id];

   ERR("obj_id %p is not pointing to a valid object. Maybe it has already been freed.",
         (void *)obj_id);

   return NULL;
#else
   return (_Eo *)obj_id;
#endif
}

Eo_Id
_eo_id_allocate(const _Eo *obj)
{
#ifdef HAVE_EO_ID
   Eo_Id ret = 0;
   for (Table_Index table_id = 1; table_id < MAX_IDS_TABLES; table_id++)
     {
        if (!_eo_ids_tables[table_id])
          {
             /* We allocate a new table */
             _eo_ids_tables[table_id] = calloc(MAX_IDS_INTER_TABLES, sizeof(_Eo_Ids_Table*));
          }
        for (Table_Index int_table_id = 0; int_table_id < MAX_IDS_INTER_TABLES; int_table_id++)
          {
             _Eo **ptr;
             if (!ID_TABLE)
               {
                  /* We allocate a new intermediate table */
                  ID_TABLE = calloc(1, sizeof(_Eo_Ids_Table));
                  eina_trash_init(&(ID_TABLE->queue));
                  /* We select directly the first entry of the new table */
                  ptr = &(ID_TABLE->ptrs[0]);
                  ID_TABLE->start = 1;
               }
             else
               {
                  /* We try to pop from the queue an unused entry */
                  ptr = (_Eo **)eina_trash_pop(&(ID_TABLE->queue));
               }

             if (!ptr && ID_TABLE->start < MAX_IDS_PER_TABLE)
               {
                  /* No more unused entries in the trash but still empty entries in the table */
                  ptr = &(ID_TABLE->ptrs[ID_TABLE->start]);
                  ID_TABLE->start++;
               }

             if (ptr)
               {
                  /* An entry was found - need to find the entry id and fill it */
                  Table_Index id = ptr - ID_TABLE->ptrs;
                  ID_TABLE->generation[id] = _eo_generation_counter;
                  ACTIVATE_ENTRY(ID_TABLE, id);
                  *ptr = (_Eo *)obj;
                  ret = EO_COMPOSE_ID(table_id, int_table_id, id, _eo_generation_counter);
                  _eo_generation_counter++;
                  _eo_generation_counter %= MAX_GENERATIONS;
                  return ret;
               }
          }
     }
   return ret;
#else
   return (Eo_Id)obj;
#endif
}

void
_eo_id_release(const Eo_Id obj_id)
{
#ifdef HAVE_EO_ID
   Table_Index table_id, int_table_id, entry_id, generation;
   EO_DECOMPOSE_ID((Table_Index) obj_id, table_id, int_table_id, entry_id, generation);

   /* Checking the validity of the entry */
   if (!_eo_ids_tables[table_id]) goto error;
   if (!ID_TABLE) goto error;
   if (ID_TABLE->generation[entry_id] != generation) goto error;

   /* Disable the entry */
   DEACTIVATE_ENTRY(ID_TABLE, entry_id);
   /* Push the entry into the queue */
   eina_trash_push(&(ID_TABLE->queue), &(ID_TABLE->ptrs[entry_id]));

   return;

error:
   ERR("obj_id %p is not pointing to a valid object. Maybe it has already been freed.", (void *)obj_id);
#else
   (void) obj_id;
#endif
}

void
_eo_free_ids_tables()
{
   for (Table_Index table_id = 0; table_id < MAX_IDS_TABLES; table_id++)
     {
        if (_eo_ids_tables[table_id])
          {
             for (Table_Index int_table_id = 0; int_table_id < MAX_IDS_INTER_TABLES; int_table_id++)
               {
                  if (ID_TABLE)
                    {
                       free(ID_TABLE);
                    }
               }
             free(_eo_ids_tables[table_id]);
          }
        _eo_ids_tables[table_id] = NULL;
     }
}

#ifdef EFL_DEBUG
void
_eo_print()
{
   unsigned long obj_number = 0;
   for (Table_Index table_id = 0; table_id < MAX_IDS_TABLES; table_id++)
     {
        if (_eo_ids_tables[table_id])
          {
             for (Table_Index int_table_id = 0; int_table_id < MAX_IDS_INTER_TABLES; int_table_id++)
               {
                  if (ID_TABLE)
                    {
                       for (Table_Index entry_id = 0; entry_id < MAX_IDS_PER_TABLE; entry_id++)
                         {
                            if (IS_ENTRY_ACTIVE(ID_TABLE, entry_id))
                              {
                                 printf("%ld: %p -> (%p, %p, %p, %p)\n", obj_number++,
                                       ID_TABLE->ptrs[entry_id],
                                       (void *)table_id, (void *)int_table_id, (void *)entry_id,
                                       (void *)ID_TABLE->generation[entry_id]);
                              }
                         }
                    }
               }
          }
     }
}
#endif
