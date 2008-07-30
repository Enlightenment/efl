/*
** Copyright Cedric BAIL, 2006
** contact: cedric.bail@free.fr
**
*/

#ifndef		MEMPOOL_STRUCT_H__
# define	MEMPOOL_STRUCT_H__

#include	"config.h"

#include	<stdint.h>

#ifdef HAVE_PTHREAD
# include	<pthread.h>
#endif

#include        "ememoa_memory_base.h"
//#include        "ememoa_mempool_error.h"

struct ememoa_memory_base_chunck_s
{
   uint16_t                                     start;
   uint16_t                                     end;
   uint16_t                                     length;

   uint16_t                                     next;
   uint16_t                                     prev;

   uint8_t                                      use;
};

struct ememoa_memory_base_s
{
#ifdef DEBUG
   unsigned int                                 magic;
#endif
   void                                         *base;

   struct ememoa_memory_base_chunck_s           *chunks;
   uint16_t                                     *pages;

   unsigned int                                 chunks_count;

   uint16_t                                     start;
   uint16_t                                     over;
   uint16_t                                     jump;
};

#endif		/* MEMPOOL_STRUCT_H__ */
