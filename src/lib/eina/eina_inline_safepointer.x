/* EINA - EFL data type library
 * Copyright (C) 2015-2016 Carsten Haitzler, Cedric Bail
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library;
 * if not, see <http://www.gnu.org/licenses/>.
 */

#ifndef EINA_SAFEPOINTER_INLINE_H_
#define EINA_SAFEPOINTER_INLINE_H_

#include <eina_trash.h>
#include <eina_log.h>

typedef struct _Eina_Memory_Table Eina_Memory_Table;
typedef struct _Eina_Memory_Entry Eina_Memory_Entry;
typedef uintptr_t Eina_Sp_Id;

#if EINA_SIZEOF_UINTPTR_T == 4
/* 32 bits */
# define EINA_BITS_MID_TABLE_ID        5
# define EINA_BITS_TABLE_ID            5
# define EINA_BITS_ENTRY_ID           12
# define EINA_BITS_GENERATION_COUNTER  8
# define EINA_BITS_FREE_COUNTER        2
# define EINA_DROPPED_TABLES           0
# define EINA_DROPPED_ENTRIES          3
typedef int16_t Eina_Table_Index;
typedef uint16_t Eina_Generation_Counter;
#else
# ifndef EINA_FULL64BIT
/* 47 bits */
#  define EINA_BITS_MID_TABLE_ID       11
#  define EINA_BITS_TABLE_ID           11
#  define EINA_BITS_ENTRY_ID           12
#  define EINA_BITS_GENERATION_COUNTER 11
#  define EINA_BITS_FREE_COUNTER        2
#  define EINA_DROPPED_TABLES           2
#  define EINA_DROPPED_ENTRIES          2
typedef int16_t Eina_Table_Index;
typedef uint16_t Eina_Generation_Counter;
# else
/* 64 bits */
#  define EINA_BITS_MID_TABLE_ID       11
#  define EINA_BITS_TABLE_ID           11
#  define EINA_BITS_ENTRY_ID           12
#  define EINA_BITS_GENERATION_COUNTER 28
#  define EINA_BITS_FREE_COUNTER        2
#  define EINA_DROPPED_TABLES           2
#  define EINA_DROPPED_ENTRIES          2
typedef int16_t Eina_Table_Index;
typedef uint32_t Eina_Generation_Counter;
# endif
#endif

/* Shifts macros to manipulate the SP id */
#define EINA_SHIFT_GENERATION      (EINA_BITS_FREE_COUNTER)
#define EINA_SHIFT_ENTRY_ID        (EINA_SHIFT_GENERATION + \
				    EINA_BITS_GENERATION_COUNTER)
#define EINA_SHIFT_TABLE_ID        (EINA_SHIFT_ENTRY_ID + \
				    EINA_BITS_ENTRY_ID)
#define EINA_SHIFT_MID_TABLE_ID    (EINA_SHIFT_TABLE_ID + \
				    EINA_BITS_TABLE_ID)

/* Maximum ranges - a few tables and entries are dropped to minimize the amount
 * of wasted bytes, see _eina_safepointer_calloc */
#define EINA_MAX_MID_TABLE_ID      (1 << EINA_BITS_MID_TABLE_ID)
#define EINA_MAX_TABLE_ID          ((1 << EINA_BITS_TABLE_ID) - EINA_DROPPED_TABLES )
#define EINA_MAX_ENTRY_ID          ((1 << EINA_BITS_ENTRY_ID) - EINA_DROPPED_ENTRIES)
#define EINA_MAX_GENERATIONS       (1 << EINA_BITS_GENERATION_COUNTER)

/* Masks */
#define EINA_MASK_MID_TABLE_ID     (EINA_MAX_MID_TABLE_ID - 1)
#define EINA_MASK_TABLE_ID         ((1 << EINA_BITS_TABLE_ID) - 1)
#define EINA_MASK_ENTRY_ID         ((1 << EINA_BITS_ENTRY_ID) - 1)
#define EINA_MASK_GENERATIONS      (EINA_MAX_GENERATIONS - 1)


/* Macro to extract from an Eo id the indexes of the tables */
#define EINA_SP_DECOMPOSE_ID(ID, MID_TABLE, TABLE, ENTRY, GENERATION)   \
  MID_TABLE = (ID >> EINA_SHIFT_MID_TABLE_ID) & EINA_MASK_MID_TABLE_ID; \
  TABLE = (ID >> EINA_SHIFT_TABLE_ID) & EINA_MASK_TABLE_ID;             \
  ENTRY = (ID >> EINA_SHIFT_ENTRY_ID) & EINA_MASK_ENTRY_ID;             \
  GENERATION = (ID >> EINA_SHIFT_GENERATION) & EINA_MASK_GENERATIONS;

struct _Eina_Memory_Entry
{
   /* Pointer to the object or
      Eina_Trash entry if not active */
   void *ptr;

   unsigned int active : 1;
   /* Valid generation for this entry */
   unsigned int generation : EINA_BITS_GENERATION_COUNTER;
};

struct _Eina_Memory_Table
{
   /* Pointer to the first recycled entry */
   Eina_Trash *trash;

   /* Packed mid table and table indexes */
   Eina_Sp_Id partial_id;

   /* Indicates where start the "never used" entries */
   Eina_Table_Index start;

   /* Entries of the table holding real pointers and generations */
   Eina_Memory_Entry entries[EINA_MAX_ENTRY_ID];
};

EINA_API extern Eina_Memory_Table **_eina_sp_ids_tables[EINA_MAX_MID_TABLE_ID];
EINA_API extern int _eina_sp_log_dom;

#ifdef _EINA_SP_ERR
#undef _EINA_SP_ERR
#endif
#define _EINA_SP_ERR(...) EINA_LOG_DOM_ERR(_eina_sp_log_dom, __VA_ARGS__)

static inline Eina_Memory_Entry *
_eina_safepointer_entry_get(const Eina_Safepointer *safe,
                            Eina_Memory_Table **rtable)
{
   Eina_Table_Index mid_table_id, table_id, entry_id;
   Eina_Generation_Counter generation;
   Eina_Sp_Id id = (Eina_Sp_Id) safe;

   EINA_SP_DECOMPOSE_ID(id, mid_table_id, table_id, entry_id, generation);

   if (_eina_sp_ids_tables[mid_table_id] &&
       _eina_sp_ids_tables[mid_table_id][table_id] &&
       entry_id < EINA_MAX_ENTRY_ID)
     {
        Eina_Memory_Table *table;
        Eina_Memory_Entry *entry;

        table = _eina_sp_ids_tables[mid_table_id][table_id];
        entry = &(table->entries[entry_id]);

        if (entry->active &&
            entry->generation == generation)
          {
             if (rtable) *rtable = table;
             return entry;
          }
     }

   _EINA_SP_ERR("Pointer %p is not a pointer to a valid object.", (void *) safe);

   return NULL;
}

static inline void *
eina_safepointer_get(const Eina_Safepointer *safe)
{
   Eina_Memory_Entry *entry;

   if (!safe) return NULL;

   entry = _eina_safepointer_entry_get(safe, NULL);
   if (!entry) return NULL;

   return entry->ptr;
}

#undef _EINA_SP_ERR

#ifndef _EINA_INTERNAL_SAFEPOINTER

#undef EINA_BITS_MID_TABLE_ID
#undef EINA_BITS_TABLE_ID
#undef EINA_BITS_ENTRY_ID
#undef EINA_BITS_GENERATION_COUNTER
#undef EINA_DROPPED_TABLES
#undef EINA_DROPPED_ENTRIES
#undef EINA_SHIFT_MID_TABLE_ID
#undef EINA_SHIFT_TABLE_ID
#undef EINA_SHIFT_ENTRY_ID
#undef EINA_MAX_MID_TABLE_ID
#undef EINA_MAX_TABLE_ID
#undef EINA_MAX_ENTRY_ID
#undef EINA_MAX_GENERATIONS
#undef EINA_MASK_MID_TABLE_ID
#undef EINA_MASK_TABLE_ID
#undef EINA_MASK_ENTRY_ID
#undef EINA_MASK_GENERATIONS
#undef EINA_SP_DECOMPOSE_ID

#endif

#endif
