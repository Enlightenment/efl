#include <assert.h>
#ifdef HAVE_MMAP
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#endif

#ifdef HAVE_VALGRIND
# include <valgrind.h>
# include <memcheck.h>
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

// enable this to test and use all 64bits of a pointer, otherwise limit to
// 47 bits because of luajit. it wants to check if any bits in the upper 17 are
// set for a sanity check for lightuserdata ... basically it does this:
// #define checklightudptr(L, p) (((uint64_t)(p) >> 47) ? (lj_err_msg(L, LJ_ERR_BADLU), NULL) : (p))
//#define EO_FULL64BIT 1

#if SIZEOF_UINTPTR_T == 4
/* 32 bits */
# define BITS_MID_TABLE_ID        5
# define BITS_TABLE_ID            5
# define BITS_ENTRY_ID           11
# define BITS_GENERATION_COUNTER  7
# define BITS_DOMAIN              2
# define BITS_CLASS               1
# define REF_TAG_SHIFT           31
# define DROPPED_TABLES           0
# define DROPPED_ENTRIES          4
typedef int16_t Table_Index;
typedef uint16_t Generation_Counter;
#else
# ifndef EO_FULL64BIT
/* 47 bits */
#  define BITS_MID_TABLE_ID       11
#  define BITS_TABLE_ID           11
#  define BITS_ENTRY_ID           11
#  define BITS_GENERATION_COUNTER 10
#  define BITS_DOMAIN              2
#  define BITS_CLASS               1
#  define REF_TAG_SHIFT           46
#  define DROPPED_TABLES           2
#  define DROPPED_ENTRIES          3
typedef int16_t Table_Index;
typedef uint16_t Generation_Counter;
# else
/* 64 bits */
#  define BITS_MID_TABLE_ID       11
#  define BITS_TABLE_ID           11
#  define BITS_ENTRY_ID           11
#  define BITS_GENERATION_COUNTER 27
#  define BITS_DOMAIN              2
#  define BITS_CLASS               1
#  define REF_TAG_SHIFT           63
#  define DROPPED_TABLES           2
#  define DROPPED_ENTRIES          3
typedef int16_t Table_Index;
typedef uint32_t Generation_Counter;
# endif
#endif

/* Shifts macros to manipulate the Eo id */
#define SHIFT_DOMAIN          (BITS_MID_TABLE_ID + BITS_TABLE_ID + \
                               BITS_ENTRY_ID + BITS_GENERATION_COUNTER)
#define SHIFT_MID_TABLE_ID    (BITS_TABLE_ID + \
                               BITS_ENTRY_ID + BITS_GENERATION_COUNTER)
#define SHIFT_TABLE_ID        (BITS_ENTRY_ID + BITS_GENERATION_COUNTER)
#define SHIFT_ENTRY_ID        (BITS_GENERATION_COUNTER)

/* Maximum ranges - a few tables and entries are dropped to minimize the amount
 * of wasted bytes, see _eo_id_mem_alloc */
#define MAX_DOMAIN            (1 << BITS_DOMAIN)
#define MAX_MID_TABLE_ID      (1 << BITS_MID_TABLE_ID)
#define MAX_TABLE_ID          ((1 << BITS_TABLE_ID) - DROPPED_TABLES )
#define MAX_ENTRY_ID          ((1 << BITS_ENTRY_ID) - DROPPED_ENTRIES)
#define MAX_GENERATIONS       (1 << BITS_GENERATION_COUNTER)

/* Masks */
#define MASK_DOMAIN           (MAX_DOMAIN - 1)
#define MASK_MID_TABLE_ID     (MAX_MID_TABLE_ID - 1)
#define MASK_TABLE_ID         ((1 << BITS_TABLE_ID) - 1)
#define MASK_ENTRY_ID         ((1 << BITS_ENTRY_ID) - 1)
#define MASK_GENERATIONS      (MAX_GENERATIONS - 1)
#define MASK_OBJ_TAG          (((Eo_Id) 1) << (REF_TAG_SHIFT))

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
# ifdef HAVE_VALGRIND
   if (RUNNING_ON_VALGRIND) return malloc(size);
   else
# endif
     {
        void *ptr;
        Mem_Header *hdr;
        size_t newsize;
        newsize = MEM_PAGE_SIZE * ((size + MEM_HEADER_SIZE + MEM_PAGE_SIZE - 1) / 
                                   MEM_PAGE_SIZE);
        ptr = mmap(NULL, newsize, PROT_READ | PROT_WRITE,
                   MAP_PRIVATE | MAP_ANON, -1, 0);
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
     }
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
# ifdef HAVE_VALGRIND
   if (RUNNING_ON_VALGRIND) free(ptr);
   else
# endif
     {
        Mem_Header *hdr;
        if (!ptr) return;
        hdr = (Mem_Header *)(((unsigned char *)ptr) - MEM_HEADER_SIZE);
        if (hdr->magic != MEM_MAGIC)
          {
             ERR("unmap of eo table region has bad magic!");
             return;
          }
        munmap(hdr, hdr->size);
     }
#else
   free(ptr);
#endif
}

#ifdef EINA_DEBUG_MALLOC
static void
_eo_id_mem_protect(void *ptr, Eina_Bool may_not_write)
{
# ifdef HAVE_MMAP
#  ifdef HAVE_VALGRIND
   if (RUNNING_ON_VALGRIND) { return; }
   else
#  endif
     {
        Mem_Header *hdr;
        if (!ptr) return;
        hdr = (Mem_Header *)(((unsigned char *)ptr) - MEM_HEADER_SIZE);
        if (hdr->magic != MEM_MAGIC)
          {
             ERR("mprotect of eo table region has bad magic!");
             return;
          }
        mprotect(hdr, hdr->size, PROT_READ | ( may_not_write ? 0 : PROT_WRITE) );
     }
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

//////////////////////////////////////////////////////////////////////////

typedef struct _Eo_Id_Data       Eo_Id_Data;
typedef struct _Eo_Id_Table_Data Eo_Id_Table_Data;

struct _Eo_Id_Table_Data
{
   /* Cached eoid lookups */
   struct
     {
        Eo_Id             id;
        _Eo_Object       *object;
        const Eo         *isa_id;
        const Efl_Class  *klass;
        Eina_Bool         isa;
     }
   cache;
   /* Tables handling pointers indirection */
   _Eo_Ids_Table     **eo_ids_tables[MAX_MID_TABLE_ID];
   /* Current table used for following allocations */
   _Eo_Ids_Table      *current_table;
   /* Spare empty table */
   _Eo_Ids_Table      *empty_table;
   /* Optional lock around all objects in eoid table - only used if shared */
   Eina_Lock           obj_lock;
   /* Next generation to use when assigning a new entry to a Eo pointer */
   Generation_Counter  generation;
   /* are we shared so we need lock/unlock? */
   Eina_Bool           shared;
};

struct _Eo_Id_Data
{
   Eo_Id_Table_Data   *tables[4];
   unsigned char       local_domain;
   unsigned char       stack_top;
   unsigned char       domain_stack[255 - (sizeof(void *) * 4) - 2];
};

extern Eina_TLS          _eo_table_data;
extern Eo_Id_Data       *_eo_table_data_shared;
extern Eo_Id_Table_Data *_eo_table_data_shared_data;

static inline Eo_Id_Table_Data *
_eo_table_data_table_new(Efl_Id_Domain domain)
{
   Eo_Id_Table_Data *tdata;

   tdata = calloc(1, sizeof(Eo_Id_Table_Data));
   if (!tdata) return NULL;
   if (domain == EFL_ID_DOMAIN_SHARED)
     {
        if (!eina_lock_recursive_new(&(tdata->obj_lock)))
          {
             free(tdata);
             return NULL;
          }
        tdata->shared = EINA_TRUE;
     }
   tdata->generation = rand() % MAX_GENERATIONS;
   return tdata;
}

static inline Eo_Id_Data *
_eo_table_data_new(Efl_Id_Domain domain)
{
   Eo_Id_Data *data;

   data = calloc(1, sizeof(Eo_Id_Data));
   if (!data) return NULL;
   data->local_domain = domain;
   data->domain_stack[data->stack_top] = data->local_domain;
   data->tables[data->local_domain] =
     _eo_table_data_table_new(data->local_domain);
   if (domain != EFL_ID_DOMAIN_SHARED)
     data->tables[EFL_ID_DOMAIN_SHARED] = _eo_table_data_shared_data;
   return data;
}

static void
_eo_table_data_table_free(Eo_Id_Table_Data *tdata)
{
   if (tdata->shared) eina_lock_free(&(tdata->obj_lock));
   free(tdata);
}

static inline Eo_Id_Data *
_eo_table_data_get(void)
{
   Eo_Id_Data *data = eina_tls_get(_eo_table_data);
   if (EINA_LIKELY(data != NULL)) return data;

   data = _eo_table_data_new(EFL_ID_DOMAIN_THREAD);
   if (!data) return NULL;

   eina_tls_set(_eo_table_data, data);
   return data;
}

static inline Eo_Id_Table_Data *
_eo_table_data_current_table_get(Eo_Id_Data *data)
{
   return data->tables[data->domain_stack[data->stack_top]];
}

static inline Eo_Id_Table_Data *
_eo_table_data_table_get(Eo_Id_Data *data, Efl_Id_Domain domain)
{
   return data->tables[domain];
}

static inline Eina_Bool
_eo_id_domain_compatible(const Eo *o1, const Eo *o2)
{
   Efl_Id_Domain domain1 = ((Eo_Id)o1 >> SHIFT_DOMAIN) & MASK_DOMAIN;
   Efl_Id_Domain domain2 = ((Eo_Id)o2 >> SHIFT_DOMAIN) & MASK_DOMAIN;
   if (domain1 == domain2) return EINA_TRUE;
   ERR("Object %p and %p are not compatible. Domain %i and %i do not match",
       o1, o2, domain1, domain2);
   return EINA_FALSE;
}

static inline void
_eo_obj_pointer_done(const Eo_Id obj_id)
{
   Efl_Id_Domain domain = (obj_id >> SHIFT_DOMAIN) & MASK_DOMAIN;
   if (EINA_LIKELY(domain != EFL_ID_DOMAIN_SHARED)) return;
   eina_lock_release(&(_eo_table_data_shared_data->obj_lock));
}

//////////////////////////////////////////////////////////////////////////


/* Macro used to compose an Eo id */
#define EO_COMPOSE_PARTIAL_ID(MID_TABLE, TABLE)                      \
   (((Eo_Id) 0x1 << REF_TAG_SHIFT)                                 | \
    ((Eo_Id)(MID_TABLE & MASK_MID_TABLE_ID) << SHIFT_MID_TABLE_ID) | \
    ((Eo_Id)(TABLE & MASK_TABLE_ID) << SHIFT_TABLE_ID))

#define EO_COMPOSE_FINAL_ID(PARTIAL_ID, ENTRY, DOMAIN, GENERATION)  \
    (PARTIAL_ID                                                   | \
     (((Eo_Id)DOMAIN & MASK_DOMAIN) << SHIFT_DOMAIN)              | \
     ((ENTRY & MASK_ENTRY_ID) << SHIFT_ENTRY_ID)                  | \
     (GENERATION & MASK_GENERATIONS))

/* Macro to extract from an Eo id the indexes of the tables */
#define EO_DECOMPOSE_ID(ID, MID_TABLE, TABLE, ENTRY, GENERATION) \
   MID_TABLE = (ID >> SHIFT_MID_TABLE_ID) & MASK_MID_TABLE_ID;   \
   TABLE = (ID >> SHIFT_TABLE_ID) & MASK_TABLE_ID;               \
   ENTRY = (ID >> SHIFT_ENTRY_ID) & MASK_ENTRY_ID;               \
   GENERATION = ID & MASK_GENERATIONS;

/* Macro used for readability */
#define TABLE_FROM_IDS tdata->eo_ids_tables[mid_table_id][table_id]

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
_search_tables(Eo_Id_Table_Data *tdata)
{
   _Eo_Ids_Table *table;
   _Eo_Id_Entry *entry;

   if (!tdata) return NULL;
   for (Table_Index mid_table_id = 0; mid_table_id < MAX_MID_TABLE_ID; mid_table_id++)
     {
        if (!tdata->eo_ids_tables[mid_table_id])
          {
             /* Allocate a new intermediate table */
             tdata->eo_ids_tables[mid_table_id] = _eo_id_mem_calloc(MAX_TABLE_ID, sizeof(_Eo_Ids_Table*));
          }

        for (Table_Index table_id = 0; table_id < MAX_TABLE_ID; table_id++)
          {
             table = TABLE_FROM_IDS;

             if (!table)
               {
                  if (tdata->empty_table)
                    {
                       /* Recycle the available empty table */
                       table = tdata->empty_table;
                       tdata->empty_table = NULL;
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
                  UNPROTECT(tdata->eo_ids_tables[mid_table_id]);
                  TABLE_FROM_IDS = table;
                  PROTECT(tdata->eo_ids_tables[mid_table_id]);
               }
             else
               entry = _get_available_entry(table);

             if (entry)
               {
                  /* Store table info into current table */
                  tdata->current_table = table;
                  return entry;
               }
          }
     }

   ERR("no more available entries to store eo objects");
   tdata->current_table = NULL;
   return NULL;
}

/* Gives a fake id that serves as a marker if eo id is off. */
static inline Eo_Id
_eo_id_allocate(const _Eo_Object *obj, const Eo *parent_id)
{
   _Eo_Id_Entry *entry = NULL;
   Eo_Id_Data *data;
   Eo_Id_Table_Data *tdata;
   Eo_Id id;

   data = _eo_table_data_get();
   if (parent_id)
     {
        Efl_Id_Domain domain = ((Eo_Id)parent_id >> SHIFT_DOMAIN) & MASK_DOMAIN;
        tdata = _eo_table_data_table_get(data, domain);
     }
   else tdata = _eo_table_data_current_table_get(data);
   if (!tdata) return 0;

   if (EINA_LIKELY(!tdata->shared))
     {
        if (tdata->current_table)
          entry = _get_available_entry(tdata->current_table);

        if (!entry) entry = _search_tables(tdata);

        if (!tdata->current_table || !entry)
          {
             return 0;
          }

        UNPROTECT(tdata->current_table);
        /* [1;max-1] thus we never generate an Eo_Id equal to 0 */
        tdata->generation++;
        if (tdata->generation >= MAX_GENERATIONS) tdata->generation = 1;
        /* Fill the entry and return it's Eo Id */
        entry->ptr = (_Eo_Object *)obj;
        entry->active = 1;
        entry->generation = tdata->generation;
        PROTECT(tdata->current_table);
        id = EO_COMPOSE_FINAL_ID(tdata->current_table->partial_id,
                                 (entry - tdata->current_table->entries),
                                 data->domain_stack[data->stack_top],
                                 entry->generation);
     }
   else
     {
        eina_lock_take(&(_eo_table_data_shared_data->obj_lock));
        if (tdata->current_table)
          entry = _get_available_entry(tdata->current_table);

        if (!entry) entry = _search_tables(tdata);

        if (!tdata->current_table || !entry)
          {
             id = 0;
             goto shared_err;
          }

        UNPROTECT(tdata->current_table);
        /* [1;max-1] thus we never generate an Eo_Id equal to 0 */
        tdata->generation++;
        if (tdata->generation == MAX_GENERATIONS) tdata->generation = 1;
        /* Fill the entry and return it's Eo Id */
        entry->ptr = (_Eo_Object *)obj;
        entry->active = 1;
        entry->generation = tdata->generation;
        PROTECT(tdata->current_table);
        id = EO_COMPOSE_FINAL_ID(tdata->current_table->partial_id,
                                 (entry - tdata->current_table->entries),
                                 EFL_ID_DOMAIN_SHARED,
                                 entry->generation);
shared_err:
        eina_lock_release(&(_eo_table_data_shared_data->obj_lock));
     }
   return id;
}

static inline void
_eo_id_release(const Eo_Id obj_id)
{
   _Eo_Ids_Table *table;
   _Eo_Id_Entry *entry;
   Generation_Counter generation;
   Table_Index mid_table_id, table_id, entry_id;
   Efl_Id_Domain domain;
   Eo_Id_Data *data;
   Eo_Id_Table_Data *tdata;

   domain = (obj_id >> SHIFT_DOMAIN) & MASK_DOMAIN;
   data = _eo_table_data_get();
   tdata = _eo_table_data_table_get(data, domain);
   if (!tdata) return;

   EO_DECOMPOSE_ID(obj_id, mid_table_id, table_id, entry_id, generation);

   if (EINA_LIKELY(domain != EFL_ID_DOMAIN_SHARED))
     {
        // Check the validity of the entry
        if (tdata->eo_ids_tables[mid_table_id] && (table = TABLE_FROM_IDS))
          {
             entry = &(table->entries[entry_id]);
             if (entry && entry->active && (entry->generation == generation))
               {
                  UNPROTECT(table);
                  table->free_entries++;
                  // Disable the entry
                  entry->active = 0;
                  entry->next_in_fifo = -1;
                  // Push the entry into the fifo
                  if (table->fifo_tail == -1)
                    table->fifo_head = table->fifo_tail = entry_id;
                  else
                    {
                       table->entries[table->fifo_tail].next_in_fifo = entry_id;
                       table->fifo_tail = entry_id;
                    }
                  PROTECT(table);
                  if (table->free_entries == MAX_ENTRY_ID)
                    {
                       UNPROTECT(tdata->eo_ids_tables[mid_table_id]);
                       TABLE_FROM_IDS = NULL;
                       PROTECT(tdata->eo_ids_tables[mid_table_id]);
                       // Recycle or free the empty table
                       if (!tdata->empty_table) tdata->empty_table = table;
                       else _eo_id_mem_free(table);
                       if (tdata->current_table == table)
                         tdata->current_table = NULL;
                    }
                  // In case an object is destroyed, wipe out the cache
                  if (tdata->cache.id == obj_id)
                    {
                       tdata->cache.id = 0;
                       tdata->cache.object = NULL;
                    }
                  if ((Eo_Id)tdata->cache.isa_id == obj_id)
                    {
                       tdata->cache.isa_id = NULL;
                       tdata->cache.klass = NULL;;
                       tdata->cache.isa = EINA_FALSE;
                    }
                  return;
               }
          }
     }
   else
     {
        eina_lock_take(&(_eo_table_data_shared_data->obj_lock));
        // Check the validity of the entry
        if (tdata->eo_ids_tables[mid_table_id] && (table = TABLE_FROM_IDS))
          {
             entry = &(table->entries[entry_id]);
             if (entry && entry->active && (entry->generation == generation))
               {
                  UNPROTECT(table);
                  table->free_entries++;
                  // Disable the entry
                  entry->active = 0;
                  entry->next_in_fifo = -1;
                  // Push the entry into the fifo
                  if (table->fifo_tail == -1)
                    table->fifo_head = table->fifo_tail = entry_id;
                  else
                    {
                       table->entries[table->fifo_tail].next_in_fifo = entry_id;
                       table->fifo_tail = entry_id;
                    }
                  PROTECT(table);
                  if (table->free_entries == MAX_ENTRY_ID)
                    {
                       UNPROTECT(tdata->eo_ids_tables[mid_table_id]);
                       TABLE_FROM_IDS = NULL;
                       PROTECT(tdata->eo_ids_tables[mid_table_id]);
                       // Recycle or free the empty table
                       if (!tdata->empty_table) tdata->empty_table = table;
                       else _eo_id_mem_free(table);
                       if (tdata->current_table == table)
                         tdata->current_table = NULL;
                    }
                  // In case an object is destroyed, wipe out the cache
                  if (tdata->cache.id == obj_id)
                    {
                       tdata->cache.id = 0;
                       tdata->cache.object = NULL;
                    }
                  if ((Eo_Id)tdata->cache.isa_id == obj_id)
                    {
                       tdata->cache.isa_id = NULL;
                       tdata->cache.klass = NULL;;
                       tdata->cache.isa = EINA_FALSE;
                    }
                  eina_lock_release(&(_eo_table_data_shared_data->obj_lock));
                  return;
               }
          }
        eina_lock_release(&(_eo_table_data_shared_data->obj_lock));
     }
   ERR("obj_id %p is not pointing to a valid object. Maybe it has already been freed.", (void *)obj_id);
}

static inline void
_eo_free_ids_tables(Eo_Id_Data *data)
{
   Eo_Id_Table_Data *tdata;

   if (!data) return;
   tdata = data->tables[data->local_domain];
   for (Table_Index mid_table_id = 0; mid_table_id < MAX_MID_TABLE_ID; mid_table_id++)
     {
        if (tdata->eo_ids_tables[mid_table_id])
          {
             for (Table_Index table_id = 0; table_id < MAX_TABLE_ID; table_id++)
               {
                  if (TABLE_FROM_IDS)
                    {
                       _eo_id_mem_free(TABLE_FROM_IDS);
                    }
               }
             _eo_id_mem_free(tdata->eo_ids_tables[mid_table_id]);
          }
        tdata->eo_ids_tables[mid_table_id] = NULL;
     }
   if (tdata->empty_table) _eo_id_mem_free(tdata->empty_table);
   tdata->empty_table = tdata->current_table = NULL;
   _eo_table_data_table_free(tdata);
   data->tables[data->local_domain] = NULL;
   free(data);
}

#ifdef EFL_DEBUG
static inline void
_eo_print(Eo_Id_Table_Data *tdata)
{
   _Eo_Id_Entry *entry;
   unsigned long obj_number = 0;

   for (Table_Index mid_table_id = 0; mid_table_id < MAX_MID_TABLE_ID; mid_table_id++)
     {
        if (tdata->eo_ids_tables[mid_table_id])
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
