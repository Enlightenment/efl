/* EINA - EFL data type library
 * Copyright (C) 2002-2008 Carsten Haitzler, Gustavo Sverzut Barbieri,
 *                         Vincent Torri, Jorge Luis Zapata Muga, Cedric Bail
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

#ifndef EINA_HASH_H_
#define EINA_HASH_H_

#include "eina_types.h"
#include "eina_iterator.h"

/**
 * @addtogroup Eina_Data_Types_Group Data Types
 *
 * @{
 */

/**
 * @addtogroup Eina_Containers_Group Containers
 *
 * @{
 */

/**
 * @defgroup Eina_Hash_Group Hash Table
 *
 * @{
 */

typedef struct _Eina_Hash Eina_Hash;
typedef struct _Eina_Hash_Tuple Eina_Hash_Tuple;
struct _Eina_Hash_Tuple
{
   const void *key;
   void *data;

   unsigned int key_length;
};

typedef unsigned int (*Eina_Key_Length)(const void *key);
#define EINA_KEY_LENGTH(Function) ((Eina_Key_Length)Function)
typedef int (*Eina_Key_Cmp)(const void *key1, int key1_length,
			    const void *key2, int key2_length);
#define EINA_KEY_CMP(Function) ((Eina_Key_Cmp)Function)
typedef int (*Eina_Key_Hash)(const void *key, int key_length);
#define EINA_KEY_HASH(Function) ((Eina_Key_Hash)Function)

EAPI int eina_hash_init(void);
EAPI int eina_hash_shutdown(void);

EAPI Eina_Hash * eina_hash_new(Eina_Key_Length key_length_cb,
			       Eina_Key_Cmp key_cmp_cb,
			       Eina_Key_Hash key_hash_cb,
			       Eina_Free_Cb data_free_cb);
EAPI Eina_Hash * eina_hash_string_djb2_new(Eina_Free_Cb data_free_cb);
EAPI Eina_Hash * eina_hash_string_superfast_new(Eina_Free_Cb data_free_cb);

EAPI Eina_Bool   eina_hash_add(Eina_Hash *hash, const void *key, const void *data);
EAPI Eina_Bool   eina_hash_direct_add(Eina_Hash *hash, const void *key, const void *data);
EAPI Eina_Bool   eina_hash_del(Eina_Hash *hash, const void *key, const void *data);
EAPI void      * eina_hash_find(const Eina_Hash *hash, const void *key);
EAPI void      * eina_hash_modify(Eina_Hash *hash, const void *key, const void *data);
EAPI void        eina_hash_free(Eina_Hash *hash);
EAPI int         eina_hash_population(const Eina_Hash *hash);

EAPI Eina_Bool   eina_hash_add_by_hash(Eina_Hash *hash,
				       const void *key, int key_length, int key_hash,
				       const void *data);
EAPI Eina_Bool   eina_hash_direct_add_by_hash(Eina_Hash *hash,
					      const void *key, int key_length, int key_hash,
					      const void *data);

EAPI Eina_Bool   eina_hash_del_by_key_hash(Eina_Hash *hash, const void *key, int key_length, int key_hash);

EAPI Eina_Bool   eina_hash_del_by_key(Eina_Hash *hash, const void *key);
EAPI Eina_Bool   eina_hash_del_by_data(Eina_Hash *hash, const void *data);

EAPI Eina_Bool   eina_hash_del_by_hash(Eina_Hash *hash,
				       const void *key, int key_length, int key_hash,
				       const void *data);
EAPI void      * eina_hash_find_by_hash(const Eina_Hash *hash,
					const void *key, int key_length, int key_hash);
EAPI void      * eina_hash_modify_by_hash(Eina_Hash *hash,
					  const void *key, int key_length, int key_hash,
					  const void *data);

EAPI Eina_Iterator * eina_hash_iterator_key_new(const Eina_Hash *hash);
EAPI Eina_Iterator * eina_hash_iterator_data_new(const Eina_Hash *hash);
EAPI Eina_Iterator * eina_hash_iterator_tuple_new(const Eina_Hash *hash);

typedef Eina_Bool (*Eina_Hash_Foreach)(const Eina_Hash *hash, const void *key, void *data, void *fdata);
EAPI void        eina_hash_foreach(const Eina_Hash *hash,
				   Eina_Hash_Foreach cb,
				   const void *fdata);

/* Paul Hsieh (http://www.azillionmonkeys.com/qed/hash.html) hash function
   used by WebCore (http://webkit.org/blog/8/hashtables-part-2/) */
EAPI int eina_hash_superfast(const char *key, int len);

/* Hash function first reported by dan bernstein many years ago in comp.lang.c */
static inline int eina_hash_djb2(const char *key, int len);

#include "eina_inline_hash.x"

/**
 * @}
 */

/**
 * @}
 */

/**
 * @}
 */

#endif /*EINA_HASH_H_*/
