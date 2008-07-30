#ifndef EINA_HASH_H_
#define EINA_HASH_H_

/**
 *
 */
typedef struct _Eina_Hash Eina_Hash;

EAPI Eina_Hash *eina_hash_add (Eina_Hash *hash, const char *key, const void *data);
EAPI Eina_Hash *eina_hash_direct_add (Eina_Hash *hash, const char *key, const void *data);
EAPI Eina_Hash *eina_hash_del (Eina_Hash *hash, const char *key, const void *data);
EAPI void *eina_hash_find(const Eina_Hash *hash, const char *key);
EAPI void *eina_hash_modify(Eina_Hash *hash, const char *key, const void *data);
EAPI int eina_hash_size(const Eina_Hash *hash);
EAPI void eina_hash_free(Eina_Hash *hash);
EAPI void eina_hash_foreach(const Eina_Hash *hash, Eina_Bool (*func) (const Eina_Hash *hash, const char *key, void *data, void *fdata),
		const void *fdata);

#endif /*EINA_HASH_H_*/
