#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "eo_ptr_indirection.h"

/* Tables handling pointers indirection */
_Eo_Ids_Table **_eo_ids_tables[MAX_MID_TABLE_ID] = { NULL };

/* Current table used for following allocations */
_Eo_Ids_Table *_current_table = NULL;

/* Spare empty table */
_Eo_Ids_Table *_empty_table = NULL;

/* Next generation to use when assigning a new entry to a Eo pointer */
Generation_Counter _eo_generation_counter = 0;
