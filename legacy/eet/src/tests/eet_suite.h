#ifndef _EET_SUITE_H
# define _EET_SUITE_H

#include "Eet.h"

typedef struct _Eet_List Eet_List;
typedef struct _Eet_Hash Eet_Hash;

Eet_List* eet_list_prepend(Eet_List *list, const void *data);
Eet_List* eet_list_next(Eet_List *list);
void* eet_list_data(Eet_List *list);
void eet_list_free(Eet_List *list);

void eet_hash_foreach(const Eet_Hash *hash, int (*func) (const Eet_Hash *hash, const char *key, void *data, void *fdata), const void *fdata);
Eet_Hash* eet_hash_add(Eet_Hash *hash, const char *key, const void *data);
void eet_hash_free(Eet_Hash *hash);

void eet_test_setup_eddc(Eet_Data_Descriptor_Class *eddc);


#endif /* _EET_SUITE_H */
