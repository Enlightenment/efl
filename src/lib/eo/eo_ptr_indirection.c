#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "eo_ptr_indirection.h"
#include <assert.h>
#ifdef __linux__
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#endif

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
 * - an index 'start' indicating which entry is the next one to use.
 * - 2 indexes 'queue_head' and 'queue_tail' defining a queue (fifo),
 *   that will help us to store the entries to be reused. It stores only the
 *   entries that have been used at least one time. The entries that have
 *   never been used are "pointed" by the start parameter.
 * - entries composed of:
 *    - a pointer to the object
 *    - a flag indicating if the entry is active
 *    - a generation assigned to the object
 *    - an index 'next_in_queue' used to chain the entries in the queue
 * When an entry is searched into a table, we first use one of the entries that
 * has never been used. If there is none, we try to pop from the queue.
 * If a such entry doesn't exist, we pass to the next table.
 * When an entry is found, we reserve it to the object pointer and create
 * the id with the table id, the intermediate table id, the entry and a
 * generation.
 * When an object is freed, the entry into the table is released by appending
 * it to the queue.
 * Assigning all the entries of a table before trying to reuse them from
 * the fifo ensures that we are not going to soon recycle a released entry,
 * thus minimize the risks of an aggressive del() then use() on a single entry.
 */

#if SIZEOF_UINTPTR_T == 4
/* 32 bits */
# define BITS_FOR_IDS_TABLE           5
# define BITS_FOR_IDS_INTER_TABLE     5
# define BITS_FOR_ID_IN_TABLE        12
# define BITS_FOR_GENERATION_COUNTER 10
typedef int16_t Table_Index;
typedef uint16_t Generation_Counter;
#else
/* 64 bits */
# define BITS_FOR_IDS_TABLE          11
# define BITS_FOR_IDS_INTER_TABLE    11
# define BITS_FOR_ID_IN_TABLE        12
# define BITS_FOR_GENERATION_COUNTER 30
typedef int16_t Table_Index;
typedef uint32_t Generation_Counter;
#endif

/* Shifts macros to manipulate the Eo id */
#define SHIFT_FOR_IDS_TABLE \
   (BITS_FOR_IDS_INTER_TABLE + BITS_FOR_ID_IN_TABLE + BITS_FOR_GENERATION_COUNTER)

#define SHIFT_FOR_IDS_INTER_TABLE \
   (BITS_FOR_ID_IN_TABLE + BITS_FOR_GENERATION_COUNTER)

#define SHIFT_FOR_ID_IN_TABLE (BITS_FOR_GENERATION_COUNTER)

/* Maximum ranges */
#define MAX_IDS_TABLES       (1 << BITS_FOR_IDS_TABLE)
#define MAX_IDS_INTER_TABLES (1 << BITS_FOR_IDS_INTER_TABLE)
#define MAX_IDS_PER_TABLE    (1 << BITS_FOR_ID_IN_TABLE)
#define MAX_GENERATIONS      (1 << BITS_FOR_GENERATION_COUNTER)

#define MEM_HEADER_SIZE 16
#define MEM_PAGE_SIZE   4096
#define MEM_MAGIC       0x3f61ec8a

typedef struct _Mem_Header
{
   size_t size;
   size_t magic;
} Mem_Header;

static void *
_eo_id_mem_alloc(size_t size)
{
#ifdef __linux__
   void *ptr;
   Mem_Header *hdr;
   size_t newsize;
   newsize = MEM_PAGE_SIZE * ((size + MEM_HEADER_SIZE + MEM_PAGE_SIZE - 1) / 
                              MEM_PAGE_SIZE);
   ptr = mmap(NULL, newsize, PROT_READ | PROT_WRITE,
              MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
   if (ptr == MAP_FAILED)
     {
        ERR("mmap of eo id table region failed!");
        return NULL;
     }
   hdr = ptr;
   hdr->size = newsize;
   hdr->magic = MEM_MAGIC;
   return (void *)(((unsigned char *)ptr) + MEM_HEADER_SIZE);
#else
   return malloc(size);
#endif
}

static void *
_eo_id_mem_calloc(size_t num, size_t size)
{
   void *ptr = _eo_id_mem_alloc(num * size);
   if (!ptr) return NULL;
   memset(ptr, 0, num * size);
   return ptr;
}

static void
_eo_id_mem_free(void *ptr)
{
#ifdef __linux__
   Mem_Header *hdr;
   if (!ptr) return;
   hdr = (Mem_Header *)(((unsigned char *)ptr) - MEM_HEADER_SIZE);
   if (hdr->magic != MEM_MAGIC)
     {
        ERR("unmap of eo table region has bad magic!");
        return;
     }
   munmap(hdr, hdr->size);
#else
   free(ptr);
#endif
}

/* Entry */
typedef struct
{
   /* Pointer to the object */
   _Eo *ptr;
   /* Active flag */
   unsigned int active     : 1;
   /* Generation */
   unsigned int generation : BITS_FOR_GENERATION_COUNTER;
   /* Indicates where to find the next entry to recycle */
   Table_Index next_in_queue;

} _Eo_Id_Entry;

/* Table */
typedef struct
{
   /* Entries of the table holding real pointers and generations */
   _Eo_Id_Entry entries[MAX_IDS_PER_TABLE];
   /* Indicates where start the "never used" entries */
   Table_Index start;
   /* Indicates where to find the next entry to recycle */
   Table_Index queue_head;
   /* Indicates where to add an entry to recycle */
   Table_Index queue_tail;
} _Eo_Ids_Table;

/* Tables handling pointers indirection */
_Eo_Ids_Table **_eo_ids_tables[MAX_IDS_TABLES] = { NULL };

/* Next generation to use when assigning a new entry to a Eo pointer */
Generation_Counter _eo_generation_counter;

/* Macro used to compose an Eo id */
#define EO_COMPOSE_ID(TABLE, INTER_TABLE, ENTRY, GENERATION)                              \
   (((Eo_Id)(TABLE & (MAX_IDS_TABLES - 1)) << SHIFT_FOR_IDS_TABLE)                     |  \
    ((Eo_Id)(INTER_TABLE & (MAX_IDS_INTER_TABLES - 1)) << SHIFT_FOR_IDS_INTER_TABLE)   |  \
    ((ENTRY & (MAX_IDS_PER_TABLE - 1)) << SHIFT_FOR_ID_IN_TABLE)                       |  \
    (GENERATION & (MAX_GENERATIONS - 1) ))

/* Macro to extract from an Eo id the indexes of the tables */
#define EO_DECOMPOSE_ID(ID, TABLE, INTER_TABLE, ENTRY, GENERATION)               \
   TABLE = (ID >> SHIFT_FOR_IDS_TABLE) & (MAX_IDS_TABLES - 1);                   \
   INTER_TABLE = (ID >> SHIFT_FOR_IDS_INTER_TABLE) & (MAX_IDS_INTER_TABLES - 1); \
   ENTRY = (ID >> SHIFT_FOR_ID_IN_TABLE) & (MAX_IDS_PER_TABLE - 1);              \
   GENERATION = ID & (MAX_GENERATIONS - 1);                                      \

/* Macro used for readability */
#define ID_TABLE _eo_ids_tables[table_id][int_table_id]

_Eo *
_eo_obj_pointer_get(const Eo_Id obj_id)
{
#ifdef HAVE_EO_ID
   _Eo_Id_Entry *entry;
   Generation_Counter generation;
   Table_Index table_id, int_table_id, entry_id;

   EO_DECOMPOSE_ID(obj_id, table_id, int_table_id, entry_id, generation);

   /* Checking the validity of the entry */
   if (_eo_ids_tables[table_id] && ID_TABLE)
     {
        entry = &(ID_TABLE->entries[entry_id]);
        if (entry && entry->active && (entry->generation == generation))
          return entry->ptr;
     }

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
   _Eo_Ids_Table *table;
   _Eo_Id_Entry *entry = NULL;
   for (Table_Index table_id = 1; table_id < MAX_IDS_TABLES; table_id++)
     {
        if (!_eo_ids_tables[table_id])
          {
             /* We allocate a new intermediate table */
             _eo_ids_tables[table_id] = _eo_id_mem_calloc(MAX_IDS_INTER_TABLES, sizeof(_Eo_Ids_Table*));
          }
        for (Table_Index int_table_id = 0; int_table_id < MAX_IDS_INTER_TABLES; int_table_id++)
          {
             table = ID_TABLE;
             if (!table)
               {
                  /* We allocate a new table */
                  table = _eo_id_mem_calloc(1, sizeof(_Eo_Ids_Table));
                  table->start = 1;
                  table->queue_head = table->queue_tail = -1;
                  ID_TABLE = table;
                  /* We select directly the first entry of the new table */
                  entry = &(table->entries[0]);
               }
             else if (table->start < MAX_IDS_PER_TABLE)
               {
                  /* We use an empty entries in the table */
                  entry = &(table->entries[table->start]);
                  table->start++;
               }
             else if (table->queue_head != -1)
               {
                  /* We pop an unused entry from the queue */
                  entry = &(table->entries[table->queue_head]);
                  if (entry->next_in_queue == -1)
                    table->queue_head = table->queue_tail = -1;
                  else
                     table->queue_head = entry->next_in_queue;
               }
             else
               {
                  /* Table is full of actives entries */
                  continue;
               }

             assert(entry);
             /* An entry was found - need to find the entry id and fill it */
             entry->ptr = (_Eo *)obj;
             entry->active = 1;
             entry->generation = _eo_generation_counter;
             _eo_generation_counter++;
             _eo_generation_counter %= MAX_GENERATIONS;
             return EO_COMPOSE_ID(table_id, int_table_id,
                                  (entry - table->entries),
                                  entry->generation);
          }
     }
   return 0;
#else
   return (Eo_Id)obj;
#endif
}

void
_eo_id_release(const Eo_Id obj_id)
{
#ifdef HAVE_EO_ID
   _Eo_Ids_Table *table;
   _Eo_Id_Entry *entry;
   Generation_Counter generation;
   Table_Index table_id, int_table_id, entry_id;
   EO_DECOMPOSE_ID(obj_id, table_id, int_table_id, entry_id, generation);

   /* Checking the validity of the entry */
   if (_eo_ids_tables[table_id] && (table = ID_TABLE))
     {
        entry = &(table->entries[entry_id]);
        if (entry && entry->active && (entry->generation == generation))
          {
             /* Disable the entry */
             entry->active = 0;
             entry->next_in_queue = -1;
             /* Push the entry into the queue */
             if (table->queue_tail == -1)
               {
                  table->queue_head = table->queue_tail = entry_id;
               }
             else
               {
                  table->entries[table->queue_tail].next_in_queue = entry_id;
                  table->queue_tail = entry_id;
               }
             return;
          }
     }

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
                       _eo_id_mem_free(ID_TABLE);
                    }
               }
             _eo_id_mem_free(_eo_ids_tables[table_id]);
          }
        _eo_ids_tables[table_id] = NULL;
     }
}

#ifdef EFL_DEBUG
void
_eo_print()
{
   _Eo_Id_Entry *entry;
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
                            entry = &(ID_TABLE->entries[entry_id]);
                            if (entry->active)
                              {
                                 printf("%ld: %p -> (%p, %p, %p, %p)\n", obj_number++,
                                       entry->ptr,
                                       (void *)table_id, (void *)int_table_id, (void *)entry_id,
                                       (void *)entry->generation);
                              }
                         }
                    }
               }
          }
     }
}
#endif
