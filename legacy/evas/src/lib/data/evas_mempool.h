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
