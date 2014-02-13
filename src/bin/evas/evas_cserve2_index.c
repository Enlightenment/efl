/* Shared index for cserve2.
 * EXPERIMENTAL WORK.
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "evas_cserve2.h"
#include "evas_cs2_utils.h"

#include <stdint.h>

typedef struct _Data_Shm Data_Shm;
typedef struct _Block Block;
typedef struct _Shared_Index Shared_Index;

static int _instances = 0;

// Static memory pool used for storing strings
static Shared_Mempool *_string_mempool = NULL;

// Map const char* --> buffer id (valid in _string_mempool)
static Eina_Hash *_string_entries = NULL;

struct _Data_Shm
{
   Shm_Handle *shm;
   char *data;
};

struct _Shared_Array
{
   Data_Shm *ds;
   Shared_Array_Header *header;
};

struct _Shared_Index
{
   // Random buffer index
   Shared_Array *sa;
   int32_t lastid;
};

struct _Shared_Mempool
{
   Data_Shm *ds;
   Shared_Index *index;
   int empty_size;
   Block *empty_blocks;
};

// Used for empty blocks. RB tree ordered by length.
struct _Block
{
   EINA_RBTREE;
   int32_t length;
   int32_t offset;
   int32_t shmid;
};


// Data blocks will be aligned to blocks of DATA_BLOCKSIZE bytes to reduce
// fragmentation (after del). 16 is convenient for debugging with hd :)
#define DATA_BLOCKSIZE 8

// Recommended minimum size for arrays and mempools
#define ARRAY_MINSIZE (32 * 1024)

static inline int
_data_blocksize_roundup(int len)
{
   return ((len + DATA_BLOCKSIZE - 1) / DATA_BLOCKSIZE) * DATA_BLOCKSIZE;
}

static Eina_Rbtree_Direction
_block_rbtree_cmp(const Eina_Rbtree *l, const Eina_Rbtree *r,
                  void *data EINA_UNUSED)
{
   const Block *left  = (Block *) l;
   const Block *right = (Block *) r;

   if (!left)
     return EINA_RBTREE_RIGHT;

   if (!right)
     return EINA_RBTREE_LEFT;

   if (left->length <= right->length)
     return EINA_RBTREE_LEFT;
   else
     return EINA_RBTREE_RIGHT;
}

static inline int
_block_rbtree_empty_spot_find(const Block *node, const void *key,
                              int key_length EINA_UNUSED,
                              void *data EINA_UNUSED)
{
   int32_t length = (int32_t) (intptr_t) key;

   // Found best spot
   if (node->length == length)
     return 0;

   // We're good, can we find better?
   if (node->length > length)
     {
        Block *lson = (Block *) node->__rbtree.son[0];

        if (lson && lson->length >= length)
          return -1;

        // This is the best we can do...
        return 0;
     }

   // Keep calm and carry on
   return 1;
}

static inline int
_block_rbtree_block_find(const Block *node, const void *key,
                         int key_length EINA_UNUSED,
                         void *data EINA_UNUSED)
{
   const Index_Entry *ie = key;

   // Found best spot
   if ((node->length == ie->length)
       && (node->offset == ie->offset)
       && (node->shmid == ie->shmid))
     return 0;

   // We're good, can we find better?
   if (node->length > ie->length)
     {
        Block *lson = (Block *) node->__rbtree.son[0];

        if (lson && lson->length >= ie->length)
          return -1;

        // This is the best we can do...
        return 0;
     }

   // Keep calm and carry on
   return 1;
}


// Data shm

static Data_Shm *
_shared_data_shm_new(const char *infix, int size)
{
   Data_Shm *ds;
   size_t mapping_size;

   if (size <= 0)
     return NULL;

   ds = calloc(1, sizeof(Data_Shm));
   if (!ds) return NULL;

   mapping_size = cserve2_shm_size_normalize((size_t) size, 0);

   ds->shm = cserve2_shm_request(infix, mapping_size);
   if (!ds->shm)
     {
        ERR("Could not create shm of size %u", (unsigned) mapping_size);
        free(ds);
        return NULL;
     }

   ds->data = cserve2_shm_map(ds->shm);
   if (!ds->data)
     {
        ERR("Could not map shm of size %u", (unsigned) mapping_size);
        cserve2_shm_unref(ds->shm);
        free(ds);
        return NULL;
     }

   DBG("Created data shm of size %d: %d", size, cserve2_shm_id_get(ds->shm));
   return ds;
}

static void
_shared_data_shm_del(Data_Shm *ds)
{
   if (!ds) return;
   cserve2_shm_unref(ds->shm);
   free(ds);
}

static int
_shared_data_shm_resize(Data_Shm *ds, size_t newsize)
{
   Shm_Handle *shm;
   size_t mapping_size;

   if (newsize <= 0)
     return -1;

   mapping_size = cserve2_shm_size_normalize(newsize, ARRAY_MINSIZE);
   cserve2_shm_unmap(ds->shm);
   ds->data = NULL;

   shm = cserve2_shm_resize(ds->shm, mapping_size);
   if (!shm)
     {
        ERR("Could not resize shm %d to %u",
            cserve2_shm_id_get(ds->shm), (unsigned) newsize);
        return -1;
     }

   ds->shm = shm;
   ds->data = cserve2_shm_map(ds->shm);
   if (!ds->data)
     {
        ERR("Failed to remap shm %d after resizing to %u bytes",
            cserve2_shm_id_get(ds->shm), (unsigned) mapping_size);
        return -1;
     }

   return mapping_size;
}


// Arrays

Shared_Array *
cserve2_shared_array_new(int tag, int generation_id, int elemsize, int initcount)
{
   Shared_Array *sa;
   Data_Shm *ds;
   int mapping_size;

   if (elemsize <= 0 || initcount < 0)
     return NULL;

   sa = calloc(1, sizeof(Shared_Array));
   if (!sa) return NULL;

   if (!initcount) initcount = 1;
   mapping_size = cserve2_shm_size_normalize(elemsize * initcount
                                             + sizeof(Shared_Array_Header),
                                             ARRAY_MINSIZE);
   ds = _shared_data_shm_new("array", mapping_size);
   if (!ds)
     {
        free(sa);
        return NULL;
     }

   sa->ds = ds;
   sa->header = (Shared_Array_Header *) ds->data;
   sa->header->count = (mapping_size - sizeof(Shared_Array_Header)) / elemsize;
   sa->header->elemsize = elemsize;
   sa->header->generation_id = generation_id;
   sa->header->emptyidx = 0;
   sa->header->sortedidx = 0;
   sa->header->tag = tag;
   memset(&sa->header->_reserved1, 0, sizeof(int32_t) * 2);

   return sa;
}

const char *
cserve2_shared_array_name_get(Shared_Array *sa)
{
   if (!sa) return NULL;
   return cserve2_shm_name_get(sa->ds->shm);
}

void
cserve2_shared_array_del(Shared_Array *sa)
{
   if (!sa) return;
   _shared_data_shm_del(sa->ds);
   free(sa);
}

int
cserve2_shared_array_size_get(Shared_Array *sa)
{
   if (!sa) return -1;
   return sa->header->count;
}

int
cserve2_shared_array_count_get(Shared_Array *sa)
{
   if (!sa) return -1;
   return sa->header->emptyidx;
}

int
cserve2_shared_array_map_size_get(Shared_Array *sa)
{
   if (!sa || !sa->ds) return 0;
   return cserve2_shm_map_size_get(sa->ds->shm);
}

int
cserve2_shared_array_item_size_get(Shared_Array *sa)
{
   if (!sa) return -1;
   return sa->header->elemsize;
}

int
cserve2_shared_array_generation_id_get(Shared_Array *sa)
{
   if (!sa) return -1;
   return sa->header->generation_id;
}

int cserve2_shared_array_generation_id_set(Shared_Array *sa, int generation_id)
{
   if (!sa) return -1;
   if (sa->header->generation_id == generation_id)
     return 0;

   sa->header->generation_id = generation_id;
   return 1;
}

int
cserve2_shared_array_size_set(Shared_Array *sa, int newcount)
{
   int mapping_size;

   if (!sa) return -1;
   mapping_size = cserve2_shm_size_normalize(sa->header->elemsize * newcount
                                             + sizeof(Shared_Array_Header),
                                             ARRAY_MINSIZE);
   if (_shared_data_shm_resize(sa->ds, mapping_size) < 0)
     {
        sa->header = NULL;
        return -1;
     }
   sa->header = (Shared_Array_Header *) sa->ds->data;
   sa->header->count = (mapping_size - sizeof(Shared_Array_Header))
         / sa->header->elemsize;

   return sa->header->count;
}

int
cserve2_shared_array_item_new(Shared_Array *sa)
{
   if (!sa) return -1;
   if (sa->header->emptyidx >= sa->header->count)
     {
        int newcount = cserve2_shared_array_size_set(sa, sa->header->count + 1);
        if (newcount < 0) return -1;
     }

   return sa->header->emptyidx++;
}

const void *
cserve2_shared_array_item_data(Shared_Array *sa)
{
   return cserve2_shared_array_item_data_get(sa, 0);
}

void *
cserve2_shared_array_item_data_get(Shared_Array *sa, int elemid)
{
   char *ptr;

   if (!sa) return NULL;
   if (elemid < 0 || elemid >= sa->header->count)
     return NULL;

   ptr  = (char *) sa->header;
   ptr += sizeof(Shared_Array_Header);
   ptr += elemid * sa->header->elemsize;

   return ptr;
}

int
cserve2_shared_array_foreach(Shared_Array *sa, Eina_Each_Cb cb, void *data)
{
   char *ptr;
   int k;

   if (!sa || !cb) return -1;
   ptr = sa->ds->data + sizeof(Shared_Array_Header);

   for (k = 0; k < sa->header->emptyidx; k++)
     {
        if (!cb(sa, ptr, data))
          break;
        ptr += sa->header->elemsize;
     }

   return k;
}

Shared_Array *
cserve2_shared_array_repack(Shared_Array *sa, int generation_id,
                            Shared_Array_Repack_Skip_Cb skip,
                            Eina_Compare_Cb cmp,
                            void *user_data)
{
   Eina_List *l = NULL;
   Shared_Array *sa2;
   const char *srcdata;
   char *dstdata;
   int k, elemsize, newcount = 0;

   if (!sa || !skip || !cmp) return NULL;
   srcdata = sa->ds->data + sizeof(Shared_Array_Header);
   elemsize = sa->header->elemsize;

   // Build ordered list of pointers
   for (k = 0; k < sa->header->emptyidx; k++)
     {
        const char *data = srcdata + k * elemsize;
        if (skip(sa, data, user_data)) continue;
        l = eina_list_sorted_insert(l, cmp, data);
        newcount++;
     }

   // Create new array
   sa2 = cserve2_shared_array_new(sa->header->tag, generation_id, elemsize, newcount);
   if (!sa2)
     {
        ERR("Can not repack array: failed to create new array");
        return NULL;
     }

   // Write data
   dstdata = sa2->ds->data + sizeof(Shared_Array_Header);
   while (l)
     {
        const char *data = eina_list_data_get(l);
        l = eina_list_remove_list(l, l);
        memcpy(dstdata, data, elemsize);
        dstdata += elemsize;
     }

   // Finalize & return
   sa2->header->emptyidx = newcount;
   sa2->header->sortedidx = newcount;
   sa2->header->tag = sa->header->tag;
   return sa2;
}

int
cserve2_shared_array_item_find(Shared_Array *sa, void *data,
                               Eina_Compare_Cb cmp)
{
   int k;
   const char *ptr;

   if (!sa || !cmp) return -1;

   // Binary search
   if (sa->header->sortedidx > 0)
     {
        int low = 0;
        int high = sa->header->sortedidx;
        int prev = -1;
        int r;
        k = high / 2;
        while (prev != k)
          {
             ptr = cserve2_shared_array_item_data_get(sa, k);
             r = cmp(ptr, data);
             if (!r)
               return k;
             else if (r > 0)
               high = k;
             else
               low = k;
             prev = k;
             k = low + (high - low) / 2;
          }
     }

   // Linear search O(n)
   k = sa->header->sortedidx;
   ptr = sa->ds->data + sizeof(Shared_Array_Header) + k * sa->header->elemsize;
   for (; k < sa->header->emptyidx; k++)
     {
        if (!cmp(ptr, data))
          return k;
        ptr += sa->header->elemsize;
     }

   return -1;
}

void *
cserve2_shared_array_item_data_find(Shared_Array *sa, void *data,
                                    Eina_Compare_Cb cmp)
{
   int elemid;

   elemid = cserve2_shared_array_item_find(sa, data, cmp);
   if (elemid < 0)
     return NULL;

   return cserve2_shared_array_item_data_get(sa, elemid);
}


// Shared index (used by the random mempool)

static Index_Entry *
_shared_index_entry_new(Shared_Index *si)
{
   Index_Entry *ie;
   int id;

   if (!si) return NULL;

   id = cserve2_shared_array_item_new(si->sa);
   if (id < 0) return NULL;

   ie = cserve2_shared_array_item_data_get(si->sa, id);
   ie->id = si->lastid++;
   return ie;
}

static Index_Entry *
_shared_index_entry_get_by_id(Shared_Index *si, unsigned int id)
{
   Index_Entry *obj;
   const char *base;
   int low = 0, high, start_high, elemsize;
   int cur;

   if (!si || !si->sa || !id)
     return NULL;

   // FIXME: HACK (consider all arrays always sorted by id)
   high = si->sa->header->emptyidx; // Should be si->sa->header->sortedidx

   if (high > si->sa->header->count)
     high = si->sa->header->count;

   base = si->sa->ds->data + sizeof(Shared_Array_Header);
   elemsize = si->sa->header->elemsize;

   // Direct access, works for non-repacked arrays
   if ((int) id < high)
     {
        obj = (Index_Entry *) (base + (elemsize * id));
        if (obj->id == id)
          return obj;
        if (obj->id < id)
          low = id + 1;
        else
          high = id;
     }

   // Binary search
   start_high = high;
   while (high > low)
     {
        cur = low + ((high - low) / 2);
        obj = (Index_Entry *) (base + (elemsize * cur));
        if (obj->id == id)
          return obj;
        if (obj->id < id)
          low = cur + 1;
        else
          high = cur;
     }

   // Linear search
   for (cur = start_high; cur < si->sa->header->count; cur++)
     {
        obj = (Index_Entry *) (base + (elemsize * cur));
        if (!obj->id)
          return NULL;
        if (obj->id == id)
          return obj;
     }

   return NULL;
}

static Shared_Index *
_shared_index_new(int tag, int index_elemsize, int generation_id)
{
   Shared_Index *si;
   Index_Entry *ie;

   EINA_SAFETY_ON_FALSE_RETURN_VAL(index_elemsize >= 0, NULL);

   si = calloc(1, sizeof(Shared_Index));
   if (!si) return NULL;

   if (!index_elemsize)
     index_elemsize = sizeof(Index_Entry);

   si->sa = cserve2_shared_array_new(tag, generation_id, index_elemsize, 0);
   if (!si->sa)
     {
        free(si);
        return NULL;
     }

   si->lastid = 0;
   ie = _shared_index_entry_new(si);
   if (!ie)
     {
        cserve2_shared_array_del(si->sa);
        free(si);
        return NULL;
     }

   return si;
}

void
_shared_index_del(Shared_Index *si)
{
   if (!si) return;
   cserve2_shared_array_del(si->sa);
   free(si);
}


// Shared memory pool

Shared_Mempool *
cserve2_shared_mempool_new(int indextag, int index_elemsize,
                           int generation_id, int initsize)
{
   Shared_Mempool *sm;
   size_t mapping_size;

   EINA_SAFETY_ON_FALSE_RETURN_VAL(initsize >= 0, NULL);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(index_elemsize >= 0, NULL);

   sm = calloc(1, sizeof(Shared_Mempool));
   if (!sm) return NULL;

   if (!initsize) initsize = 1;
   mapping_size = cserve2_shm_size_normalize((size_t) initsize, ARRAY_MINSIZE);

   sm->ds = _shared_data_shm_new("mempool", mapping_size);
   if (!sm->ds)
     {
        free(sm);
        return NULL;
     }

   sm->index = _shared_index_new(indextag, index_elemsize, generation_id);
   if (!sm->index)
     {
        _shared_data_shm_del(sm->ds);
        free(sm);
        return NULL;
     }

   sm->empty_size = mapping_size;
   return sm;
}

Shared_Array *
cserve2_shared_mempool_index_get(Shared_Mempool *sm)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(sm, NULL);
   return sm->index->sa;
}

static void
_shared_mempool_block_del(Eina_Rbtree *node, void *data EINA_UNUSED)
{
   Block *blk = (Block *) node;
   free(blk);
}

void
cserve2_shared_mempool_del(Shared_Mempool *sm)
{
   if (!sm) return;
   eina_rbtree_delete(EINA_RBTREE_GET(sm->empty_blocks),
                      EINA_RBTREE_FREE_CB(_shared_mempool_block_del), sm);
   _shared_data_shm_del(sm->ds);
   _shared_index_del(sm->index);
   free(sm);
}

static Index_Entry *
_shared_mempool_buffer_new(Shared_Mempool *sm, int size)
{

   Index_Entry *ie;
   Block *blk;
   int mapping_size, new_mapping_size;
   int rsize = _data_blocksize_roundup(size);

   if (!sm) return NULL;
   if (size <= 0) return NULL;

   mapping_size = cserve2_shm_map_size_get(sm->ds->shm);

   ie = _shared_index_entry_new(sm->index);
   if (!ie) return NULL;

   // Append if possible
   if (rsize <= sm->empty_size)
     {
        ie->length = rsize;
        ie->offset = mapping_size - sm->empty_size;
        ie->shmid = cserve2_shm_id_get(sm->ds->shm);
        ie->refcount = 1;
        sm->empty_size -= rsize;
        return ie;
     }

   // Find empty spot
   blk = (Block *) eina_rbtree_inline_lookup(
            EINA_RBTREE_GET(sm->empty_blocks), (void *) (intptr_t) rsize, 0,
            EINA_RBTREE_CMP_KEY_CB(_block_rbtree_empty_spot_find), NULL);
   if (blk && blk->length >= rsize)
     {
        ie->length = blk->length;
        ie->offset = blk->offset;
        ie->shmid = cserve2_shm_id_get(sm->ds->shm);
        ie->refcount = 1;

        sm->empty_blocks = (Block *) eina_rbtree_inline_remove(
                 EINA_RBTREE_GET(sm->empty_blocks), EINA_RBTREE_GET(blk),
                 EINA_RBTREE_CMP_NODE_CB(_block_rbtree_cmp), NULL);
        if (blk->length == rsize)
          free(blk);
        else
          {
             blk->length -= rsize;
             blk->offset += rsize;
             sm->empty_blocks = (Block *) eina_rbtree_inline_insert(
                      EINA_RBTREE_GET(sm->empty_blocks),  EINA_RBTREE_GET(blk),
                      EINA_RBTREE_CMP_NODE_CB(_block_rbtree_cmp), NULL);
          }

        return ie;
     }

   // Grow mempool
   new_mapping_size = _shared_data_shm_resize(
            sm->ds, mapping_size + rsize - sm->empty_size);
   if (new_mapping_size < 0)
     {
        memset(ie, 0, sizeof(Index_Entry));
        return NULL;
     }
   ie->length = rsize;
   ie->offset = mapping_size - sm->empty_size;
   ie->shmid = cserve2_shm_id_get(sm->ds->shm);
   ie->refcount = 1;
   sm->empty_size += (new_mapping_size - mapping_size) - rsize;
   return ie;
}

int
cserve2_shared_mempool_buffer_new(Shared_Mempool *sm, int size)
{
   Index_Entry *ie;

   ie = _shared_mempool_buffer_new(sm, size);
   if (!ie) return -1;

   return ie->id;
}

int
cserve2_shared_mempool_buffer_ref(Shared_Mempool *sm, int bufferid)
{
   Index_Entry *ie;

   if (!sm) return -1;
   ie = _shared_index_entry_get_by_id(sm->index, bufferid);
   if (!ie) return -1;

   if (!ie->refcount)
     {
        Block *blk = (Block *)
              eina_rbtree_inline_lookup(EINA_RBTREE_GET(sm->empty_blocks),
                                        ie, sizeof(Index_Entry),
                                        EINA_RBTREE_CMP_KEY_CB(
                                           _block_rbtree_block_find),
                                        sm);
        if (blk && (blk->length == ie->length)
            && (blk->offset == ie->offset)
            && (blk->shmid == ie->shmid))
          {
             sm->empty_blocks = (Block *) eina_rbtree_inline_remove(
                      EINA_RBTREE_GET(sm->empty_blocks), EINA_RBTREE_GET(blk),
                      EINA_RBTREE_CMP_NODE_CB(_block_rbtree_cmp), NULL);
             free(blk);
          }
     }

   ie->refcount++;
   return ie->id;
}

static const void *
_shared_mempool_buffer_del(Shared_Mempool *sm, int bufferid)
{
   Index_Entry *ie;
   const char *data;

   if (!sm || !bufferid) return NULL;

   ie = _shared_index_entry_get_by_id(sm->index, bufferid);
   if (!ie || ie->refcount <= 0)
     {
        CRI("Tried to delete invalid buffer or with refcount 0");
        return NULL;
     }

   ie->refcount--;
   if (!ie->refcount)
     {
        Block *newblk = calloc(1, sizeof(Block));
        newblk->length = ie->length;
        newblk->offset = ie->offset;
        newblk->shmid = ie->shmid;
        sm->empty_blocks = (Block *) eina_rbtree_inline_insert(
                 EINA_RBTREE_GET(sm->empty_blocks), EINA_RBTREE_GET(newblk),
                 EINA_RBTREE_CMP_NODE_CB(_block_rbtree_cmp), NULL);
        data = sm->ds->data + ie->offset;
        return data;
     }

   return NULL;
}

void
cserve2_shared_mempool_buffer_del(Shared_Mempool *sm, int bufferid)
{
   _shared_mempool_buffer_del(sm, bufferid);
}

void *
cserve2_shared_mempool_buffer_get(Shared_Mempool *sm, int bufferid)
{
   Index_Entry *ie;
   char *data;

   if (!sm) return NULL;
   ie = _shared_index_entry_get_by_id(sm->index, bufferid);
   if (!ie || ie->refcount <= 0)
     {
        CRI("Tried to access invalid buffer or with refcount 0");
        return NULL;
     }

   data = sm->ds->data + ie->offset;
   return data;
}

int
cserve2_shared_mempool_buffer_offset_get(Shared_Mempool *sm, int bufferid)
{
   Index_Entry *ie;

   if (!sm) return -1;
   ie = _shared_index_entry_get_by_id(sm->index, bufferid);
   if (!ie || ie->refcount <= 0)
     {
        CRI("Tried to access invalid buffer or with refcount 0");
        return -1;
     }

   return ie->offset;
}

size_t
cserve2_shared_mempool_size_get(Shared_Mempool *sm)
{
   if (!sm) return 0;
   return cserve2_shm_map_size_get(sm->ds->shm)
         + cserve2_shared_array_map_size_get(sm->index->sa);
}

const char *
cserve2_shared_mempool_name_get(Shared_Mempool *sm)
{
   if (!sm) return NULL;
   return cserve2_shm_name_get(sm->ds->shm);
}

int
cserve2_shared_mempool_generation_id_get(Shared_Mempool *sm)
{
   if (!sm) return -1;
   return sm->index->sa->header->generation_id;
}

int
cserve2_shared_mempool_generation_id_set(Shared_Mempool *sm, int generation_id)
{
   if (!sm) return -1;
   if (sm->index->sa->header->generation_id == generation_id)
     return 0;

   sm->index->sa->header->generation_id = generation_id;
   return 1;
}


// Shared strings

static int _shared_strings_unref_items = 0;

const char *
cserve2_shared_strings_table_name_get()
{
   if (!_string_mempool)
     return NULL;

   return cserve2_shm_name_get(_string_mempool->ds->shm);
}

const char *
cserve2_shared_strings_index_name_get()
{
   if (!_string_mempool)
     return NULL;

   return cserve2_shared_array_name_get(_string_mempool->index->sa);
}

int
cserve2_shared_string_add(const char *str)
{
   Index_Entry *ie;
   char *data;
   int len, id;

   if (!str) return 0;

   // Find in known strings
   id = (int) (intptr_t) eina_hash_find(_string_entries, str);
   if (id > 0)
     {
        ie = _shared_index_entry_get_by_id(_string_mempool->index, id);
        if (!ie || ie->refcount <= 0)
          {
             ERR("String found in hash but not in mempool!");
             eina_hash_del(_string_entries, str, (void *) (intptr_t) id);
             goto new_entry;
          }
        ie->refcount++;
        return ie->id;
     }

   // Add new entry
new_entry:
   len = strlen(str) + 1;
   ie = _shared_mempool_buffer_new(_string_mempool, len);
   if (!ie)
     {
        ERR("Could not store new string in shm");
        return 0;
     }

   data = _string_mempool->ds->data + ie->offset;
   memcpy(data, str, len);
   eina_hash_add(_string_entries, data, (void *) (intptr_t) ie->id);
   return ie->id;
}

int
cserve2_shared_string_ref(int id)
{
   Index_Entry *ie;
   if (id <= 0) return 0;
   ie = _shared_index_entry_get_by_id(_string_mempool->index, id);
   if (!ie) return 0;
   if (!ie->refcount)
     _string_mempool--;
   return cserve2_shared_mempool_buffer_ref(_string_mempool, id);
}

void
cserve2_shared_string_del(int id)
{
   const char *data;

   if (id <= 0) return;
   if ((data = _shared_mempool_buffer_del(_string_mempool, id)) != NULL)
     {
        if (!eina_hash_del_by_key(_string_entries, data))
          {
             if (!eina_hash_del_by_data(_string_entries, (void *) (intptr_t) id))
               CRI("Invalid free");
          }
     }
   _shared_strings_unref_items++;
}

const char *
cserve2_shared_string_get(int id)
{
   if (id <= 0) return NULL;
   return cserve2_shared_mempool_buffer_get(_string_mempool, id);
}

int
cserve2_shared_strings_repack(Shared_Array_Repack_Skip_Cb skip,
                              Eina_Compare_Cb cmp)
{
   int count;

   if (!_string_mempool->index) return -1;
   count = _string_mempool->index->lastid;
   if (!count) return -1;
   if (_shared_strings_unref_items * 100 / count >= 25
       && _shared_strings_unref_items > 100)
     {
        Shared_Array *sa;
        int genid;

        genid = _string_mempool->index->sa->header->generation_id + 1;
        sa = cserve2_shared_array_repack(_string_mempool->index->sa, genid,
                                         skip, cmp, NULL);
        if (!sa) return -1;

        cserve2_shared_array_del(_string_mempool->index->sa);
        _string_mempool->index->sa = sa;
        _shared_strings_unref_items = 0;
        return 1;
     }

   return 0;
}



// Init/destroy

void
cserve2_shared_index_init(void)
{
   if (!_instances)
     {
        char faketag[5] = {0};
        int ifaketag = STRING_MEMPOOL_FAKETAG;

        DBG("Initializing shared index");
        _string_mempool = cserve2_shared_mempool_new(STRING_INDEX_ARRAY_TAG, 0, 0, 0);
        _string_entries = eina_hash_string_djb2_new(NULL);

        memcpy(faketag, &ifaketag, sizeof(int));
        cserve2_shared_string_add(faketag);
        _shared_strings_unref_items = 0;
     }
   _instances++;
}

void
cserve2_shared_index_shutdown(void)
{
   _instances--;
   if (!_instances)
     {
        DBG("Destroying shared index");
        cserve2_shared_mempool_del(_string_mempool);
        eina_hash_free(_string_entries);

        _string_mempool = NULL;
        _string_entries = NULL;
     }
}

