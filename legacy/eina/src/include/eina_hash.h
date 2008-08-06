#ifndef EINA_HASH_H_
#define EINA_HASH_H_

#include "eina_types.h"

/**
 *
 */
typedef struct _Eina_Hash Eina_Hash;
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
			       Eina_Key_Hash key_hash_cb);
EAPI Eina_Hash * eina_hash_string_djb2_new(void);
EAPI Eina_Hash * eina_hash_string_superfast_new(void);

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
EAPI Eina_Bool   eina_hash_del_by_hash(Eina_Hash *hash,
				       const void *key, int key_length, int key_hash,
				       const void *data);
EAPI void      * eina_hash_find_by_hash(const Eina_Hash *hash,
					const void *key, int key_length, int key_hash);
EAPI void      * eina_hash_modify_by_hash(Eina_Hash *hash,
					  const void *key, int key_length, int key_hash,
					  const void *data);

/* FIXME: Should go with iterator support. */
typedef Eina_Bool (*Eina_Foreach)(const Eina_Hash *hash, const void *key, void *data, void *fdata);
EAPI void        eina_hash_foreach(const Eina_Hash *hash,
				   Eina_Foreach cb,
				   const void *fdata);

/* Paul Hsieh (http://www.azillionmonkeys.com/qed/hash.html) hash function
   used by WebCore (http://webkit.org/blog/8/hashtables-part-2/) */
EAPI int eina_hash_superfast(const char *key, int len);

/* Hash function first reported by dan bernstein many years ago in comp.lang.c */
static inline int eina_hash_djb2(const char *key, int len);

#include "eina_inline_hash.x"

#endif /*EINA_HASH_H_*/
