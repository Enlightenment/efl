#include <assert.h>
#ifdef HAVE_MMAP
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#endif


#if SIZEOF_UINTPTR_T == 4
# error I am not 32 bits
/* 32 bits */
# define BITS_ENTRY_ID             21
# define BITS_GENERATION_COUNTER  7
# define BITS_DOMAIN              2
# define BITS_CLASS               1
# define REF_TAG_SHIFT           31
# define DROPPED_TABLES           0
# define DROPPED_ENTRIES          4
#else
# ifndef EO_FULL64BIT
/* 47 bits */
#  define BITS_ENTRY_ID             33
#  define BITS_GENERATION_COUNTER 10
#  define BITS_DOMAIN              2
#  define BITS_CLASS               1
#  define REF_TAG_SHIFT           46
#  define DROPPED_TABLES           2
#  define DROPPED_ENTRIES          3
# else
/* 64 bits */
#  define BITS_ENTRY_ID             33
#  define BITS_GENERATION_COUNTER 27
#  define BITS_DOMAIN              2
#  define BITS_CLASS               1
#  define REF_TAG_SHIFT           63
#  define DROPPED_TABLES           2
#  define DROPPED_ENTRIES          3
# endif
#endif

#define MEM_PAGE_SIZE         4096
#define EO_ALIGN_SIZE(size) eina_mempool_alignof(size)

typedef unsigned int Generation_Counter;

/* Shifts macros to manipulate the Eo id */
#define SHIFT_DOMAIN          (BITS_ENTRY_ID + BITS_GENERATION_COUNTER)
#define SHIFT_ENTRY_ID    (BITS_GENERATION_COUNTER)
/* Maximum ranges - a few tables and entries are dropped to minimize the amount
 * of wasted bytes, see _eo_id_mem_alloc */
#define MAX_DOMAIN            (1UL << BITS_DOMAIN)
#define MAX_ENTRY_ID          (1UL << BITS_ENTRY_ID)
#define MAX_GENERATIONS       (1UL << BITS_GENERATION_COUNTER)

/* Masks */
#define MASK_DOMAIN           (MAX_DOMAIN - 1UL)
#define MASK_ENTRY_ID         ((1UL << BITS_ENTRY_ID) - 1UL)
#define MASK_GENERATIONS      (MAX_GENERATIONS - 1UL)
#define MASK_OBJ_TAG          (1UL << (REF_TAG_SHIFT))


/* This only applies to classes. Used to artificially enlarge the class ids
 * to reduce the likelihood of a clash with normal integers. */
#define CLASS_TAG_SHIFT       (REF_TAG_SHIFT - 1UL)
#define MASK_CLASS_TAG        (((Eo_Id) 1UL) << (CLASS_TAG_SHIFT))

#define EO_ENTRIES_STEP 512
//#define EO_ENTRY_LAST SIZE_MAX
#define EO_ENTRY_NULL ((uintptr_t)0)


typedef union
{
   struct {
      _Eo_Object *ptr;
      unsigned int generation;
   } data;

   struct {
      uintptr_t null;
      size_t next;
   } meta;
} Eo_Id_Entry;

typedef struct
{
   struct {
      Eo_Id             id;
      _Eo_Object       *object;
      const Eo         *isa_id;
      const Efl_Class  *klass;
      Eina_Bool         isa;
   } cache;

   Eo_Id_Entry *entries;
   size_t count;
   size_t next;
   unsigned int generation;

   /* Optional */
   Eina_Lock obj_lock;
   Eina_Bool shared;
} Eo_Id_Table;

typedef struct
{
   Eo_Id_Table        *tables[4];
   int                 local_domain;
   unsigned int        stack_top;
   unsigned char       domain_stack[256 - (sizeof(void *) * 4) - (2 * sizeof(unsigned int))];
} Eo_Id_Data;



extern Eo_Id_Data       *_eo_table_data_shared;
extern Eo_Id_Table *_eo_table_data_shared_data;


extern Eo_Id_Data _eo_main_id_data;
extern Eo_Id_Table _eo_main_id_table;

extern Eo_Id_Data *_eo_id_data;

static inline void
_eo_id_entries_setup(Eo_Id_Entry *ptr,
                     size_t count,
                     size_t start_index)
{
   const size_t end = count + start_index;
   for (size_t i = start_index; i < end; i++)
     {
        ptr[i].meta.null = EO_ENTRY_NULL;
        ptr[i].meta.next = i + 1;
     }
//   ptr[end - 1].meta.null = EO_ENTRY_NULL;
//   ptr[end - 1].meta.next = EO_ENTRY_LAST;
}

static inline Eina_Bool
_eo_id_table_setup(Eo_Id_Table *table)
{
   /* Allocate a first bunch of entries */
   table->entries = malloc(sizeof(Eo_Id_Entry) * EO_ENTRIES_STEP);
   if (EINA_UNLIKELY(! table->entries)) return EINA_FALSE;
   table->count = EO_ENTRIES_STEP;
   table->next = 0;

   _eo_id_entries_setup(table->entries, table->count, 0);

   return EINA_TRUE;
}

static inline Eo_Id_Table *
_eo_id_table_new(Efl_Id_Domain domain)
{
   Eo_Id_Table *const table = calloc(1, sizeof(*table));
   if (EINA_UNLIKELY(! table)) return NULL;

   /* Initialize the array of entries */
   if (EINA_UNLIKELY(! _eo_id_table_setup(table)))
     goto free_table;

   table->generation = rand() % MAX_GENERATIONS;

   if (domain == EFL_ID_DOMAIN_SHARED)
     {
        if (!eina_lock_recursive_new(&(table->obj_lock)))
          goto free_entries;
        table->shared = EINA_TRUE;
     }
   return table;

free_entries:
   free(table->entries);
free_table:
   free(table);
   return NULL;
}

static inline void
_eo_id_table_clear(Eo_Id_Table *table)
{
   free(table->entries);
   table->entries = NULL;
   table->next = 0;
   table->count = 0;

   if (table->shared) eina_lock_free(&(table->obj_lock));
}

static inline void
_eo_id_table_free(Eo_Id_Table *table)
{
   _eo_id_table_clear(table);
   free(table);
}


static inline Eo_Id_Data *
_eo_main_id_data_new(void)
{
   Eo_Id_Data *const d = &_eo_main_id_data;
   d->local_domain = EFL_ID_DOMAIN_MAIN;
   d->domain_stack[d->stack_top] = EFL_ID_DOMAIN_MAIN;
   d->tables[EFL_ID_DOMAIN_MAIN] = &_eo_main_id_table;
   d->tables[EFL_ID_DOMAIN_SHARED] = _eo_table_data_shared_data;

   /* Init table */
   _eo_id_table_setup(&_eo_main_id_table);

   return d;
}

static inline Eo_Id_Data *
_eo_id_data_new(Efl_Id_Domain domain)
{
   if (domain == EFL_ID_DOMAIN_MAIN)
     return _eo_main_id_data_new();

   Eo_Id_Data *const data = calloc(1, sizeof(Eo_Id_Data));
   if (EINA_UNLIKELY(! data)) return NULL;

   data->local_domain = domain;
   data->domain_stack[data->stack_top] = data->local_domain;
   data->tables[data->local_domain] =
     _eo_id_table_new(data->local_domain);
   if (domain != EFL_ID_DOMAIN_SHARED)
     data->tables[EFL_ID_DOMAIN_SHARED] = _eo_table_data_shared_data;
   return data;
}

static inline Eo_Id_Data *
_eo_id_data_get(void)
{
   return _eo_id_data;
}

static inline Eo_Id_Table *
_eo_current_table_get(Eo_Id_Data *data)
{
   return data->tables[data->domain_stack[data->stack_top]];
}

static inline Eo_Id_Table *
_eo_id_data_table_get(Eo_Id_Data *data, Efl_Id_Domain domain)
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
#define EO_COMPOSE_ID(ENTRY, DOMAIN, GENERATION)  \
   (((Eo_Id) 0x1 << REF_TAG_SHIFT)                                 | \
     (((Eo_Id)DOMAIN & MASK_DOMAIN) << SHIFT_DOMAIN)              | \
     (((Eo_Id)ENTRY & MASK_ENTRY_ID) << SHIFT_ENTRY_ID)                  | \
     ((Eo_Id)GENERATION & MASK_GENERATIONS))

/* Macro to extract from an Eo id the indexes of the tables */
#define EO_DECOMPOSE_ID(ID, ENTRY, GENERATION) \
   ENTRY = (ID >> SHIFT_ENTRY_ID) & MASK_ENTRY_ID;               \
   GENERATION = ID & MASK_GENERATIONS;


static inline Eo_Id_Entry *
_eo_id_table_next_entry_get(Eo_Id_Table *table,
                            size_t *idx_out)
{
   /* The table is saturated with Eo IDs. Give it more room. */
   if (EINA_UNLIKELY(table->next == table->count))
     {
        const size_t new_count = table->count + EO_ENTRIES_STEP;
        // XXX Unlikely to overflow.
        Eo_Id_Entry *const tmp = realloc(table->entries, new_count * sizeof(Eo_Id_Entry));
        if (EINA_UNLIKELY(! tmp))
          {
             CRI("Failed to reallocate %zu elements", new_count);
             return NULL;
          }

        _eo_id_entries_setup(tmp, EO_ENTRIES_STEP, table->count);

        table->entries = tmp;
        table->count = new_count;
     }
   else if (EINA_UNLIKELY(table->next > table->count))
     {
        CRI("Invalid");
        return NULL;
     }

   *idx_out = table->next;
   Eo_Id_Entry *const entry = &(table->entries[table->next]);
   if (EINA_UNLIKELY(entry->meta.null != EO_ENTRY_NULL))
     {
        CRI("Boom. Eo is broken. Cell %zu is not free, but was expected to be...", *idx_out);
        return NULL;
     }
   table->next = entry->meta.next;

   return entry;
}

static inline void
_eo_id_table_entry_release(Eo_Id_Table *table,
                           size_t index)
{
   Eo_Id_Entry *const entry = &(table->entries[index]);
   entry->meta.null = EO_ENTRY_NULL; /* This will annihilate the contents of ptr */
   entry->meta.next = table->next;
   table->next = index;
}

static inline Eo_Id
_eo_id_allocate(const _Eo_Object *obj, const Eo *parent_id)
{
   Eo_Id_Table *table;
   Eo_Id id = 0;


   Eo_Id_Data *const data = _eo_id_data_get();
   if (parent_id)
     {
        const Efl_Id_Domain domain = ((Eo_Id)parent_id >> SHIFT_DOMAIN) & MASK_DOMAIN;
        table = _eo_id_data_table_get(data, domain);
     }
   else table = _eo_current_table_get(data);
   if (EINA_UNLIKELY(!table)) return 0;


   const Eo_Id domain = (table->shared)
      ? EFL_ID_DOMAIN_SHARED
      : data->domain_stack[data->stack_top];

   if (EINA_UNLIKELY(! table->shared))
     eina_lock_take(&(_eo_table_data_shared_data->obj_lock));


   size_t idx;
   Eo_Id_Entry *const entry = _eo_id_table_next_entry_get(table, &idx);
   if (EINA_UNLIKELY(! entry)) goto fail;

   /* [1;max-1] thus we never generate an Eo_Id equal to 0 */
   table->generation++;
   if (table->generation >= MAX_GENERATIONS) table->generation = 1;
   /* Fill the entry and return it's Eo Id */
   entry->data.ptr = (_Eo_Object *)obj;
   entry->data.generation = table->generation;
   id = EO_COMPOSE_ID(idx, domain, entry->data.generation);


fail:
   if (EINA_UNLIKELY(! table->shared))
     eina_lock_release(&(_eo_table_data_shared_data->obj_lock));

   return id;
}

static inline void
_eo_id_release(Eo_Id id)
{
   const Efl_Id_Domain domain = (id >> SHIFT_DOMAIN) & MASK_DOMAIN;
   Eo_Id_Data *const data = _eo_id_data_get();
   Eo_Id_Table *const table = _eo_id_data_table_get(data, domain);
   if (EINA_UNLIKELY(! table)) return;

   size_t entry_id;
   unsigned int generation;
   EO_DECOMPOSE_ID(id, entry_id, generation);

   if (EINA_UNLIKELY(domain == EFL_ID_DOMAIN_SHARED))
     eina_lock_take(&(_eo_table_data_shared_data->obj_lock));

   /* If the entry index is out of bounds, that sucks... We never "ungrow"
    * so this is definitely pure garbage. */
   if (EINA_UNLIKELY(entry_id >= table->count))
     {
        ERR("Invalid entry id %zu >= %zu", entry_id, table->count);
     goto fail;
     }

   /* We are now sure that the object MAY have been allocated. At least we
    * won't segfault trying to access it. */
   const Eo_Id_Entry *const entry = &(table->entries[entry_id]);

   /* If the entry has its 'null' field set to 0, it is highly probable that
    * the object has already been freed. */
   if (EINA_UNLIKELY(entry->meta.null == EO_ENTRY_NULL))
     {
        ERR("meta is already null");
     goto fail;
     }

   /* If the generation count mismatch, the pointer is garbage */
   if (EINA_UNLIKELY(entry->data.generation != generation))
     {
        ERR("ID: %p. invalid generation count: %u vs %u", (void*)id,entry->data.generation, generation);
     goto fail;
     }

   /* At this point we are almost sure that the object IS valid. We can release
    * the Eo Id. */
   _eo_id_table_entry_release(table, entry_id);

   /* If we destroyed the ID that was in the table cache wipe out the cache */
   if (table->cache.id == id)
     {
        table->cache.id = 0;
        table->cache.object = NULL;
     }
   if ((Eo_Id)table->cache.isa_id == id)
     {
        table->cache.isa_id = NULL;
        table->cache.klass = NULL;
        table->cache.isa = EINA_FALSE;
     }

   if (EINA_UNLIKELY(domain == EFL_ID_DOMAIN_SHARED))
     eina_lock_release(&(_eo_table_data_shared_data->obj_lock));
   return;

fail:
   if (EINA_UNLIKELY(domain == EFL_ID_DOMAIN_SHARED))
     eina_lock_release(&(_eo_table_data_shared_data->obj_lock));
   ERR("obj_id %p is not pointing to a valid object. Maybe it has already been freed.", (void *)id);
}


#ifdef EFL_DEBUG
static inline void
_eo_print(const Eo_Id_Table *table)
{
   unsigned int object = 0;
   for (size_t i = 0; i < table->count; i++)
     {
        const Eo_Id_Entry *const entry = &(table->entries[i]);
        if (entry->meta.null != EO_ENTRY_NULL)
          {
             printf("%u[%zu]: %p -> (%u)\n",
                    object, i, entry->data.ptr, entry->data.generation);
             object++;
          }
     }
}
#endif
