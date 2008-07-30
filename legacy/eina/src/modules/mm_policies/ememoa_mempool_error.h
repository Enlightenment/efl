/*
** Copyright Cedric BAIL, 2006
** contact: cedric.bail@free.fr
**
*/

#ifndef		EMEMOA_MEMPOOL_ERROR_H__
# define	EMEMOA_MEMPOOL_ERROR_H__

typedef enum _ememoa_mempool_error_e
  {
    EMEMOA_NO_ERROR,
    EMEMOA_ERROR_INIT_ALREADY_DONE,
    EMEMOA_ERROR_REALLOC_AVAILABLE_OBJECTS_FAILED,
    EMEMOA_ERROR_REALLOC_OBJECTS_USE_FAILED,
    EMEMOA_ERROR_REALLOC_JUMP_OBJECT_FAILED,
    EMEMOA_ERROR_REALLOC_OBJECTS_POOL_FAILED,
    EMEMOA_ERROR_MALLOC_NEW_POOL,
    EMEMOA_ERROR_PUSH_ADDRESS_NOT_FOUND,
    EMEMOA_NO_EMPTY_POOL,
    EMEMOA_DOUBLE_PUSH,
    EMEMOA_NO_MORE_MEMORY,
    EMEMOA_INVALID_MEMPOOL
  } ememoa_mempool_error_t;

const char*
ememoa_mempool_error2string (ememoa_mempool_error_t error_code);

#endif		/* EMEMOA_MEMPOOL_ERROR_H__ */
