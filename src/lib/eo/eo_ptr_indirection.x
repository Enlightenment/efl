#include <assert.h>
#ifdef HAVE_MMAP
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
 * Eo * is no more a pointer but indexes to an entry into an ids table.
 * For a better memory usage:
 * - a tree structure is used, composed of a top level table pointing at
 *   mid tables pointing at tables composed of entries.
 * - tables are allocated when needed (i.e no more empty entries in allocated tables.
 * - empty tables are freed, except one kept as spare table.
 *
 * An Eo id is contructed by bits manipulation of table indexes and a generation.
 *
 * id = Mid Table | Table | Entry | Generation
 *
 * Generation helps finding abuse of ids. When an entry is assigned to an
 * object, a generation is inserted into the id. If the developer uses this id
 * although the object is freed and another one has replaced it into the same
 * entry of the table, the generation will be different and an error will
 * occur when accessing with the old id.
 *
 * Each Table is composed of:
 * - an index 'start' indicating which free entry is the next one to use.
 * - 2 indexes 'fifo_head' and 'fifo_tail' defining a fifo,
 *   that will help us to store the entries to be reused. It stores only the
 *   entries that have been used at least one time. The entries that have
 *   never been used are "pointed" by the start parameter.
 * - entries composed of:
 *    - a pointer to the object
 *    - an index 'next_in_fifo' used to chain the free entries in the fifo
 *    - a flag indicating if the entry is active
 *    - a generation assigned to the object
 *
 * When an entry is searched into a table, we first use one of the entries that
 * has never been used. If there is none, we try to pop from the fifo.
 * If a such entry doesn't exist, we pass to the next table.
 * When an entry is found, we reserve it to the object pointer
 * then contruct and return the related Eo id.
 *
 * Assigning all the entries of a table before trying to reuse them from
 * the fifo ensures that we are not going to soon recycle a released entry,
 * thus minimize the risks of an aggressive del() then use() on a single entry.
 *
 * The indexes and a reference to the last table which served an entry is kept
 * and is reused prior to the others untill it is full.
 * When an object is freed, the entry into the table is released by appending
 * it to the fifo.
 */

/* most significant bit is kept to tag Eo_Id with 1 */
#if SIZEOF_UINTPTR_T == 4
/* 32 bits */
# define BITS_MID_TABLE_ID        5
# define BITS_TABLE_ID            5
# define BITS_ENTRY_ID           12
# define BITS_GENERATION_COUNTER  9
# define REF_TAG_SHIFT           31
# define DROPPED_TABLES           0
# define DROPPED_ENTRIES          4
typedef int16_t Table_Index;
typedef uint16_t Generation_Counter;
#else
/* 64 bits */
# define BITS_MID_TABLE_ID       11
# define BITS_TABLE_ID           11
# define BITS_ENTRY_ID           12
# define BITS_GENERATION_COUNTER 29
# define REF_TAG_SHIFT           63
# define DROPPED_TABLES           2
# define DROPPED_ENTRIES          3
typedef int16_t Table_Index;
typedef uint32_t Generation_Counter;
#endif

/* Shifts macros to manipulate the Eo id */
#define SHIFT_MID_TABLE_ID    (BITS_TABLE_ID + \
                               BITS_ENTRY_ID + BITS_GENERATION_COUNTER)
#define SHIFT_TABLE_ID        (BITS_ENTRY_ID + BITS_GENERATION_COUNTER)
#define SHIFT_ENTRY_ID        (BITS_GENERATION_COUNTER)

/* Maximum ranges - a few tables and entries are dropped to minimize the amount
 * of wasted bytes, see _eo_id_mem_alloc */
#define MAX_MID_TABLE_ID      (1 << BITS_MID_TABLE_ID)
#define MAX_TABLE_ID          ((1 << BITS_TABLE_ID) - DROPPED_TABLES )
#define MAX_ENTRY_ID          ((1 << BITS_ENTRY_ID) - DROPPED_ENTRIES)
#define MAX_GENERATIONS       (1 << BITS_GENERATION_COUNTER)

/* Masks */
#define MASK_MID_TABLE_ID     (MAX_MID_TABLE_ID - 1)
#define MASK_TABLE_ID         ((1 << BITS_TABLE_ID) - 1)
#define MASK_ENTRY_ID         ((1 << BITS_ENTRY_ID) - 1)
#define MASK_GENERATIONS      (MAX_GENERATIONS - 1)

/* This only applies to classes. Used to artificially enlarge the class ids
 * to reduce the likelihood of a clash with normal integers. */
#define CLASS_TAG_SHIFT       (REF_TAG_SHIFT - 1)
#define MASK_CLASS_TAG        (((Eo_Id) 1) << (CLASS_TAG_SHIFT))

#define MEM_HEADER_SIZE       16
#define MEM_PAGE_SIZE         4096
#define MEM_MAGIC             0x3f61ec8a

typedef struct _Mem_Header
{
   size_t size;
   size_t magic;
} Mem_Header;

static void *
_eo_id_mem_alloc(size_t size)
{
#ifdef HAVE_MMAP
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
   /* DBG("asked:%lu allocated:%lu wasted:%lu bytes", size, newsize, (newsize - size)); */
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
#ifdef HAVE_MMAP
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

#ifdef EINA_DEBUG_MALLOC
static void
_eo_id_mem_protect(void *ptr, Eina_Bool may_not_write)
{
# ifdef HAVE_MMAP
   Mem_Header *hdr;
   if (!ptr) return;
   hdr = (Mem_Header *)(((unsigned char *)ptr) - MEM_HEADER_SIZE);
   if (hdr->magic != MEM_MAGIC)
     {
        ERR("mprotect of eo table region has bad magic!");
        return;
     }
   mprotect(hdr, hdr->size, PROT_READ | ( may_not_write ? 0 : PROT_WRITE) );
# endif
}
# define   PROTECT(_ptr_)   _eo_id_mem_protect((_ptr_), EINA_TRUE)
# define UNPROTECT(_ptr_)   _eo_id_mem_protect((_ptr_), EINA_FALSE)
#else
# define   PROTECT(_ptr_)
# define UNPROTECT(_ptr_)
#endif

#define EO_ALIGN_SIZE(size) eina_mempool_alignof(size)

/* Entry */
typedef struct
{
   /* Pointer to the object */
   _Eo_Object *ptr;
   /* Indicates where to find the next entry to recycle */
   Table_Index next_in_fifo;
   /* Active flag */
   unsigned int active     : 1;
   /* Generation */
   unsigned int generation : BITS_GENERATION_COUNTER;

} _Eo_Id_Entry;

/* Table */
typedef struct
{
   /* Indicates where start the "never used" entries */
   Table_Index start;
   /* Indicates where to find the next entry to recycle */
   Table_Index fifo_head;
   /* Indicates where to add an entry to recycle */
   Table_Index fifo_tail;
   /* Packed mid table and table indexes */
   Eo_Id partial_id;
   /* Counter of free entries */
   unsigned int free_entries;
   /* Entries of the table holding real pointers and generations */
   _Eo_Id_Entry entries[MAX_ENTRY_ID];
} _Eo_Ids_Table;

/* Tables handling pointers indirection */
extern _Eo_Ids_Table **_eo_ids_tables[MAX_MID_TABLE_ID];

/* Current table used for following allocations */
extern _Eo_Ids_Table *_current_table;

/* Spare empty table */
extern _Eo_Ids_Table *_empty_table;

/* Next generation to use when assigning a new entry to a Eo pointer */
extern Generation_Counter _eo_generation_counter;

/* Macro used to compose an Eo id */
#define EO_COMPOSE_PARTIAL_ID(MID_TABLE, TABLE)                         \
   (((Eo_Id) 0x1 << REF_TAG_SHIFT)                                   |  \
    ((Eo_Id)(MID_TABLE & MASK_MID_TABLE_ID) << SHIFT_MID_TABLE_ID)   |  \
    ((Eo_Id)(TABLE & MASK_TABLE_ID) << SHIFT_TABLE_ID))

#define EO_COMPOSE_FINAL_ID(PARTIAL_ID, ENTRY, GENERATION)     \
    (PARTIAL_ID                                             |  \
     ((ENTRY & MASK_ENTRY_ID) << SHIFT_ENTRY_ID)            |  \
     (GENERATION & MASK_GENERATIONS ))

/* Macro to extract from an Eo id the indexes of the tables */
#define EO_DECOMPOSE_ID(ID, MID_TABLE, TABLE, ENTRY, GENERATION)  \
   MID_TABLE = (ID >> SHIFT_MID_TABLE_ID) & MASK_MID_TABLE_ID;    \
   TABLE = (ID >> SHIFT_TABLE_ID) & MASK_TABLE_ID;                \
   ENTRY = (ID >> SHIFT_ENTRY_ID) & MASK_ENTRY_ID;                \
   GENERATION = ID & MASK_GENERATIONS;

/* Macro used for readability */
#define TABLE_FROM_IDS _eo_ids_tables[mid_table_id][table_id]

static inline _Eo_Object *
_eo_obj_pointer_get(const Eo_Id obj_id)
{
#ifdef HAVE_EO_ID
   _Eo_Id_Entry *entry;
   Generation_Counter generation;
   Table_Index mid_table_id, table_id, entry_id;

   // NULL objects will just be sensibly ignored. not worth complaining
   // every single time.
   if (!obj_id)
     {
        DBG("obj_id is NULL. Possibly unintended access?");
        return NULL;
     }
   EO_DECOMPOSE_ID(obj_id, mid_table_id, table_id, entry_id, generation);

   /* Check the validity of the entry */
   if (_eo_ids_tables[mid_table_id] && TABLE_FROM_IDS)
     {
        entry = &(TABLE_FROM_IDS->entries[entry_id]);
        if (entry && entry->active && (entry->generation == generation))
          return entry->ptr;
     }

   ERR("obj_id %p is not pointing to a valid object. Maybe it has already been freed.",
         (void *)obj_id);

   return NULL;
#else
   return (_Eo_Object *) obj_id;
#endif
}

static inline _Eo_Id_Entry *
_get_available_entry(_Eo_Ids_Table *table)
{
   _Eo_Id_Entry *entry = NULL;

   if (table->start != MAX_ENTRY_ID)
     {
        /* Serve never used entries first */
        entry = &(table->entries[table->start]);
        UNPROTECT(table);
        table->start++;
        table->free_entries--;
     }
   else if (table->fifo_head != -1)
     {
        /* Pop a free entry from the fifo */
        entry = &(table->entries[table->fifo_head]);
        UNPROTECT(table);
        if (entry->next_in_fifo == -1)
          table->fifo_head = table->fifo_tail = -1;
        else
          table->fifo_head = entry->next_in_fifo;
        table->free_entries--;
     }

   return entry;
}

static inline _Eo_Id_Entry *
_search_tables(void)
{
   _Eo_Ids_Table *table;
   _Eo_Id_Entry *entry;

   for (Table_Index mid_table_id = 0; mid_table_id < MAX_MID_TABLE_ID; mid_table_id++)
     {
        if (!_eo_ids_tables[mid_table_id])
          {
             /* Allocate a new intermediate table */
             _eo_ids_tables[mid_table_id] = _eo_id_mem_calloc(MAX_TABLE_ID, sizeof(_Eo_Ids_Table*));
          }

        for (Table_Index table_id = 0; table_id < MAX_TABLE_ID; table_id++)
          {
             table = TABLE_FROM_IDS;

             if (!table)
               {
                  if (_empty_table)
                    {
                       /* Recycle the available empty table */
                       table = _empty_table;
                       _empty_table = NULL;
                       UNPROTECT(table);
                    }
                  else
                    {
                       /* Allocate a new table */
                       table = _eo_id_mem_calloc(1, sizeof(_Eo_Ids_Table));
                    }
                  /* Initialize the table and reserve the first entry */
                  table->start = 1;
                  table->free_entries = MAX_ENTRY_ID - 1;
                  table->fifo_head = table->fifo_tail = -1;
                  table->partial_id = EO_COMPOSE_PARTIAL_ID(mid_table_id, table_id);
                  entry = &(table->entries[0]);
                  UNPROTECT(_eo_ids_tables[mid_table_id]);
                  TABLE_FROM_IDS = table;
                  PROTECT(_eo_ids_tables[mid_table_id]);
               }
             else
               entry = _get_available_entry(table);

             if (entry)
               {
                  /* Store table info into current table */
                  _current_table = table;
                  return entry;
               }
          }
     }

   ERR("no more available entries to store eo objects");
   _current_table = NULL;
   return NULL;
}

/* Gives a fake id that serves as a marker if eo id is off. */
static inline Eo_Id
_eo_id_allocate(const _Eo_Object *obj)
{
#ifdef HAVE_EO_ID
   _Eo_Id_Entry *entry = NULL;

   if (_current_table)
     entry = _get_available_entry(_current_table);

   if (!entry)
     {
        entry = _search_tables();
     }

   if (!_current_table || !entry)
      return 0;

   /* [1;max-1] thus we never generate an Eo_Id equal to 0 */
   _eo_generation_counter++;
   if (_eo_generation_counter == MAX_GENERATIONS)
     _eo_generation_counter = 1;
   /* Fill the entry and return it's Eo Id */
   entry->ptr = (_Eo_Object *)obj;
   entry->active = 1;
   entry->generation = _eo_generation_counter;
   PROTECT(_current_table);
   return EO_COMPOSE_FINAL_ID(_current_table->partial_id,
                              (entry - _current_table->entries),
                              entry->generation);
#else
   Eo_Id ret = 0x1;
   (void) obj;
   return ret << REF_TAG_SHIFT;
#endif
}

static inline void
_eo_id_release(const Eo_Id obj_id)
{
#ifdef HAVE_EO_ID
   _Eo_Ids_Table *table;
   _Eo_Id_Entry *entry;
   Generation_Counter generation;
   Table_Index mid_table_id, table_id, entry_id;
   EO_DECOMPOSE_ID(obj_id, mid_table_id, table_id, entry_id, generation);

   /* Check the validity of the entry */
   if (_eo_ids_tables[mid_table_id] && (table = TABLE_FROM_IDS))
     {
        entry = &(table->entries[entry_id]);
        if (entry && entry->active && (entry->generation == generation))
          {
             UNPROTECT(table);
             table->free_entries++;
             /* Disable the entry */
             entry->active = 0;
             entry->next_in_fifo = -1;
             /* Push the entry into the fifo */
             if (table->fifo_tail == -1)
               {
                  table->fifo_head = table->fifo_tail = entry_id;
               }
             else
               {
                  table->entries[table->fifo_tail].next_in_fifo = entry_id;
                  table->fifo_tail = entry_id;
               }
             PROTECT(table);
             if (table->free_entries == MAX_ENTRY_ID)
               {
                  UNPROTECT(_eo_ids_tables[mid_table_id]);
                  TABLE_FROM_IDS = NULL;
                  PROTECT(_eo_ids_tables[mid_table_id]);
                  /* Recycle or free the empty table */
                  if (!_empty_table)
                    _empty_table = table;
                  else
                    _eo_id_mem_free(table);
                  if (_current_table == table)
                    _current_table = NULL;
               }
             return;
          }
     }

   ERR("obj_id %p is not pointing to a valid object. Maybe it has already been freed.", (void *)obj_id);
#else
   EINA_MAGIC_SET((Eo_Base *) obj_id, EO_FREED_EINA_MAGIC);
#endif
}

static inline void
_eo_free_ids_tables(void)
{
   for (Table_Index mid_table_id = 0; mid_table_id < MAX_MID_TABLE_ID; mid_table_id++)
     {
        if (_eo_ids_tables[mid_table_id])
          {
             for (Table_Index table_id = 0; table_id < MAX_TABLE_ID; table_id++)
               {
                  if (TABLE_FROM_IDS)
                    {
                       _eo_id_mem_free(TABLE_FROM_IDS);
                    }
               }
             _eo_id_mem_free(_eo_ids_tables[mid_table_id]);
          }
        _eo_ids_tables[mid_table_id] = NULL;
     }
   if (_empty_table) _eo_id_mem_free(_empty_table);
   _empty_table = _current_table = NULL;
}

#ifdef EFL_DEBUG
static inline void
_eo_print(void)
{
   _Eo_Id_Entry *entry;
   unsigned long obj_number = 0;
   for (Table_Index mid_table_id = 0; mid_table_id < MAX_MID_TABLE_ID; mid_table_id++)
     {
        if (_eo_ids_tables[mid_table_id])
          {
             for (Table_Index table_id = 0; table_id < MAX_TABLE_ID; table_id++)
               {
                  if (TABLE_FROM_IDS)
                    {
                       for (Table_Index entry_id = 0; entry_id < MAX_ENTRY_ID; entry_id++)
                         {
                            entry = &(TABLE_FROM_IDS->entries[entry_id]);
                            if (entry->active)
                              {
                                 printf("%ld: %p -> (%p, %p, %p, %p)\n", obj_number++,
                                       entry->ptr,
                                       (void *)mid_table_id, (void *)table_id, (void *)entry_id,
                                       (void *)entry->generation);
                              }
                         }
                    }
               }
          }
     }
}
#endif
