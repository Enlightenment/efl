#ifndef EINA_MEMPOOL_H_
#define EINA_MEMPOOL_H_

#include "eina_types.h"

/**
 * @defgroup Memory_Pool_Group Memory Pool
 * @{
 */
typedef struct _Eina_Mempool Eina_Mempool;

EAPI int eina_mempool_init(void);
EAPI int eina_mempool_shutdown(void);
EAPI void eina_mempool_delete(Eina_Mempool *mp);
EAPI Eina_Mempool * eina_mempool_new_from_buffer(const char *name, void *buffer,
		unsigned int size, const char *options, ...);
EAPI Eina_Mempool * eina_mempool_new(const char *name, unsigned int size, const char 
		*options, ...);
EAPI void * eina_mempool_realloc(Eina_Mempool *mp, void *element, unsigned int size);
EAPI void * eina_mempool_alloc(Eina_Mempool *mp, unsigned int size);
EAPI void eina_mempool_free(Eina_Mempool *mp, void *element);

/** @} */

#endif /* EINA_MEMPOOL_H_ */
