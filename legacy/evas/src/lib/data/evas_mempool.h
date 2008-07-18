/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#ifndef _EVAS_MEMPOOL_H
#define _EVAS_MEMPOOL_H


typedef struct _Evas_Mempool Evas_Mempool;

struct _Evas_Mempool
{
   int           item_size;
   int           pool_size;
   int           usage;
   void         *first, *last;
};


void *evas_mempool_malloc(Evas_Mempool *pool, int size);
void  evas_mempool_free(Evas_Mempool *pool, void *ptr);
void *evas_mempool_calloc(Evas_Mempool *pool, int size);


#endif /* _EVAS_MEMPOOL_H */
