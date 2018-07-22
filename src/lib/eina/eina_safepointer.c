#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#ifdef HAVE_VALGRIND
# include <valgrind.h>
# include <memcheck.h>
#endif

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <inttypes.h>

#include "eina_config.h"
#include "eina_private.h"

#define _EINA_INTERNAL_SAFEPOINTER
#include "eina_safepointer.h"
#include "eina_mempool.h"
#include "eina_trash.h"
#include "eina_log.h"
#include "eina_lock.h"

typedef struct _Eina_Memory_Header Eina_Memory_Header;

#ifdef ERR
#undef ERR
#endif
#define ERR(...) EINA_LOG_DOM_ERR(_eina_sp_log_dom, __VA_ARGS__)

#ifdef DBG
#undef DBG
#endif
#define DBG(...) EINA_LOG_DOM_DBG(_eina_sp_log_dom, __VA_ARGS__)

/* Macro used to compose an Eo id */
#define SP_COMPOSE_PARTIAL_ID(MID_TABLE, TABLE)                         \
  ( \
    ((Eina_Sp_Id)(MID_TABLE & EINA_MASK_MID_TABLE_ID) << EINA_SHIFT_MID_TABLE_ID)   |  \
    ((Eina_Sp_Id)(TABLE & EINA_MASK_TABLE_ID) << EINA_SHIFT_TABLE_ID) \
    )

#define SP_COMPOSE_FINAL_ID(PARTIAL_ID, ENTRY, GENERATION)     \
    (PARTIAL_ID                                             |  \
     ((ENTRY & EINA_MASK_ENTRY_ID) << EINA_SHIFT_ENTRY_ID)            |  \
     ((GENERATION & EINA_MASK_GENERATIONS) << EINA_SHIFT_GENERATION))

struct _Eina_Memory_Header
{
   EINA_MAGIC;
   size_t size;
};

EAPI Eina_Memory_Table **_eina_sp_ids_tables[EINA_MAX_MID_TABLE_ID] = { NULL };
EAPI int _eina_sp_log_dom = -1;

/* Spare empty table */
static Eina_Memory_Table *empty_table = NULL;

// We are using a Spinlock even with the amount of syscall we do as it shouldn't
// take that long anyway.
static Eina_Spinlock sl;

#define MEM_PAGE_SIZE 4096
#define SAFEPOINTER_MAGIC 0x7DEADC03

static void *
_eina_safepointer_calloc(int number, size_t size)
{
#ifdef HAVE_MMAP
# ifdef HAVE_VALGRIND
   if (RUNNING_ON_VALGRIND) return calloc(number, size);
   else
# endif
     {
        Eina_Memory_Header *header;
        size_t newsize;

        size = size * number + sizeof (Eina_Memory_Header);
        newsize = ((size / MEM_PAGE_SIZE) +
                   (size % MEM_PAGE_SIZE ? 1 : 0))
          * MEM_PAGE_SIZE;

        header = mmap(NULL, newsize, PROT_READ | PROT_WRITE,
                      MAP_PRIVATE | MAP_ANON, -1, 0);
        if (header == MAP_FAILED)
          {
             ERR("mmap of Eina_Safepointer table region failed.");
             return NULL;
          }

        header->size = newsize;
        EINA_MAGIC_SET(header, SAFEPOINTER_MAGIC);

        return (void *)(header + 1);
     }
#else
   return calloc(number, size);
#endif
}

static void
_eina_safepointer_free(void *pointer)
{
#ifdef HAVE_MMAP
# ifdef HAVE_VALGRIND
   if (RUNNING_ON_VALGRIND) free((void *)((uintptr_t) pointer & ~0x3));
   else
# endif
     {
        Eina_Memory_Header *header;

        if (!pointer) return;

        header = (Eina_Memory_Header*)(pointer) - 1;
        if (!EINA_MAGIC_CHECK(header, SAFEPOINTER_MAGIC))
          EINA_MAGIC_FAIL(header, SAFEPOINTER_MAGIC);

        EINA_MAGIC_SET(header, 0);
        munmap(header, header->size);
     }
#else
   free((void *)((uintptr_t) pointer & ~0x3));
#endif
}

#ifdef EINA_DEBUG_MALLOC
static void
_eina_safepointer_protect(void *pointer, Eina_Bool may_not_write)
{
#ifdef HAVE_MMAP
# ifdef HAVE_VALGRIND
   if (RUNNING_ON_VALGRIND) { (void) pointer; }
   else
# endif
     {
        Eina_Memory_Header *header;

        if (!pointer) return;

        header = (Eina_Memory_Header*)(pointer) - 1;
        if (!EINA_MAGIC_CHECK(header, SAFEPOINTER_MAGIC))
          EINA_MAGIC_FAIL(header, SAFEPOINTER_MAGIC);

        mprotect(header, header->size, PROT_READ | ( may_not_write ? 0 : PROT_WRITE));
     }
#else
   (void) pointer;
#endif
}

#define   PROTECT(Ptr) _eina_safepointer_protect(Ptr, EINA_TRUE)
#define UNPROTECT(Ptr) _eina_safepointer_protect(Ptr, EINA_FALSE)

#else

#define   PROTECT(Ptr)
#define UNPROTECT(Ptr)

#endif

static Eina_Memory_Table *
_eina_safepointer_table_new(Eina_Table_Index mid_table_id,
                            Eina_Table_Index table_id)
{
   Eina_Memory_Table *table;

   if (empty_table)
     {
        /* Recycle the available empty table */
        table = empty_table;
        empty_table = NULL;
        UNPROTECT(table);
     }
   else
     {
        table = _eina_safepointer_calloc(1, sizeof (Eina_Memory_Table));
        if (!table)
          {
             ERR("Failed to allocate leaf table at [%i][%i]", mid_table_id, table_id);
             return NULL;
          }
     }

   table->partial_id = SP_COMPOSE_PARTIAL_ID(mid_table_id,
                                             table_id);
   PROTECT(table);
   UNPROTECT(_eina_sp_ids_tables[mid_table_id]);
   _eina_sp_ids_tables[mid_table_id][table_id] = table;
   PROTECT(_eina_sp_ids_tables[mid_table_id]);

   return table;
}

static Eina_Memory_Table *
_eina_safepointer_table_find(void)
{
   Eina_Table_Index mid_table_id;

   for (mid_table_id = 0; mid_table_id < EINA_MAX_MID_TABLE_ID; mid_table_id++)
     {
        Eina_Table_Index table_id;

        if (!_eina_sp_ids_tables[mid_table_id])
          {
             _eina_sp_ids_tables[mid_table_id] = _eina_safepointer_calloc(EINA_MAX_TABLE_ID, sizeof (Eina_Memory_Table*));
          }
        if (!_eina_sp_ids_tables[mid_table_id])
          {
             ERR("Failed to allocate mid table at [%i]", mid_table_id);
             return NULL;
          }

        for (table_id = 0; table_id < EINA_MAX_TABLE_ID; table_id++)
          {
             Eina_Memory_Table *table;

             table = _eina_sp_ids_tables[mid_table_id][table_id];

             if (!table)
               table = _eina_safepointer_table_new(mid_table_id, table_id);

             if (!table) return NULL;

             if (table->trash ||
                 table->start < EINA_MAX_ENTRY_ID)
               return table;
          }
     }

   return NULL;
}

static Eina_Memory_Entry *
_eina_safepointer_entry_find(Eina_Memory_Table *table)
{
   Eina_Memory_Entry *entry = NULL;

   if (table->trash)
     {
        entry = eina_trash_pop(&table->trash);
     }
   else if (table->start < EINA_MAX_ENTRY_ID)
     {
        entry = &(table->entries[table->start]);
        table->start++;
     }
   else
     {
        ERR("Impossible to find an entry in %" PRIxPTR ".", table->partial_id);
     }

   return entry;
}

EAPI const Eina_Safepointer *
eina_safepointer_register(const void *target)
{
   Eina_Memory_Table *table;
   Eina_Memory_Entry *entry = NULL;
   Eina_Sp_Id id = 0;
   unsigned int gen;

   // We silently handle NULL
   if (!target) return NULL;

   eina_spinlock_take(&sl);

   table = _eina_safepointer_table_find();
   if (!table) goto no_table;

   UNPROTECT(table);
   entry = _eina_safepointer_entry_find(table);
   if (!entry) goto on_error;

   entry->ptr = (void*) target;
   entry->active = 1;
   gen = entry->generation + 1;
   entry->generation = (gen == EINA_MAX_GENERATIONS) ? 1 : gen;

   id = SP_COMPOSE_FINAL_ID(table->partial_id,
                            (entry - table->entries),
                            entry->generation);

 on_error:
   PROTECT(table);
 no_table:
   eina_spinlock_release(&sl);

   return (void*) id;
}

EAPI void
eina_safepointer_unregister(const Eina_Safepointer *safe)
{
   Eina_Memory_Table *table;
   Eina_Memory_Entry *entry;
   Eina_Table_Index entry_id;

   // We silently handle NULL
   if (!safe) return ;

   entry = _eina_safepointer_entry_get(safe, &table);
   if (!entry) return ;

   eina_spinlock_take(&sl);

   // In case of a race condition during a double free attempt
   // The entry could have been unactivated since we did found it
   // So check again.
   if (!entry->active) goto on_error;

   UNPROTECT(table);
   entry->active = 0;
   eina_trash_push(&table->trash, entry);
   PROTECT(table);

   entry_id = entry - table->entries;
   if (entry_id == EINA_MAX_ENTRY_ID - 1)
     {
        Eina_Table_Index i;

        for (i = entry_id; i >= 0; i--)
          {
             if (table->entries[i].active)
               break ;
          }

        // No more active entry
        // Could be speed up by tracking the
        // number of allocated entries, but
        // with all the syscall around, not sure
        // it is worth it.
        if (i == -1)
          {
             Eina_Table_Index mid_table_id, table_id;

             mid_table_id = (table->partial_id >> EINA_SHIFT_MID_TABLE_ID) & EINA_MASK_MID_TABLE_ID;
             table_id = (table->partial_id >> EINA_SHIFT_TABLE_ID) & EINA_MASK_TABLE_ID;
             UNPROTECT(_eina_sp_ids_tables[mid_table_id]);
             _eina_sp_ids_tables[mid_table_id][table_id] = NULL;
             PROTECT(_eina_sp_ids_tables[mid_table_id]);
             if (!empty_table)
               empty_table = table;
             else
               _eina_safepointer_free(table);
          }
     }

 on_error:
   eina_spinlock_release(&sl);
}

Eina_Bool
eina_safepointer_init(void)
{
   eina_magic_string_set(SAFEPOINTER_MAGIC, "Safepointer");
   _eina_sp_log_dom = eina_log_domain_register("eina_safepointer",
                                               EINA_LOG_COLOR_DEFAULT);
   if (_eina_sp_log_dom < 0)
     {
        EINA_LOG_ERR("Could not register log domain: eina_safepointer.");
        return EINA_FALSE;
     }

   eina_spinlock_new(&sl);

   DBG("entry[Size, Align] = { %zu, %u }",
       sizeof (Eina_Memory_Entry), eina_mempool_alignof(sizeof (Eina_Memory_Entry)));
   DBG("table[Size, Align] = { %zu, %u }",
       sizeof (Eina_Memory_Table), eina_mempool_alignof(sizeof (Eina_Memory_Table)));

   return EINA_TRUE;
}

Eina_Bool
eina_safepointer_shutdown(void)
{
   eina_spinlock_free(&sl);

   return EINA_TRUE;
}
