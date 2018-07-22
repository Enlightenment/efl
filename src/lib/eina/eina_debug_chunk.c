/* EINA - EFL data type library
 * Copyright (C) 2015 Carsten Haitzler
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

#include <string.h>

#include "eina_debug.h"

# ifdef HAVE_CONFIG_H
#  include "config.h"
# endif

#ifdef HAVE_VALGRIND
# include <valgrind.h>
# include <memcheck.h>
#endif

#ifdef HAVE_MMAP
# include <sys/mman.h>

// custom memory allocators to avoid malloc/free during backtrace handling
// just in case we're inside some signal handler due to mem corruption and
// are inside a malloc/free lock and thus would deadlock ourselves if we
// allocated memory, so implement scratch space just big enough for what we
// need and then some via either a static 8k+4k buffer pair or via a growable
// mmaped mem chunk pair
// implement using mmap so we can grow if needed - unlikelt though
static unsigned char *chunk1 = NULL;
static unsigned char *chunk2 = NULL;
static unsigned char *chunk3 = NULL;
static int chunk1_size = 0;
static int chunk1_num = 0;
static int chunk2_size = 0;
static int chunk2_num = 0;
static int chunk3_size = 0;
static int chunk3_num = 0;

// get a new chunk of "anonymous mmaped memory"
static void *
_eina_debug_chunk_need(int size)
{
   void *ptr;

#ifdef HAVE_VALGRIND
   if (RUNNING_ON_VALGRIND) ptr = malloc(size);
   else
#endif
     {
        ptr = mmap(NULL, size, PROT_READ | PROT_WRITE,
                   MAP_PRIVATE | MAP_ANON, -1, 0);
        if (ptr == MAP_FAILED) return NULL;
     }
   return ptr;
}

// release a chunk of this mmaped anon mem if we don't need it anymore
static void
_eina_debug_chunk_noneed(void *ptr, int size)
{
#ifdef HAVE_VALGRIND
   if (RUNNING_ON_VALGRIND) free(ptr);
   else
#endif
     munmap(ptr, size);
}

// push a new bit of mem on our growing stack of mem - given our workload,
// we never free anything here, only ever grow new things on this stack
void *
_eina_debug_chunk_push(int size)
{
   void *ptr;

   // no initial chunk1 block - allocate it
   if (!chunk1)
     {
        chunk1 = _eina_debug_chunk_need(8 * 1024);
        if (!chunk1) return NULL;
        chunk1_size = 8 * 1024;
     }
   // round size up to the nearest pointer size for alignment
   size = sizeof(void *) * ((size + sizeof(void *) - 1) / sizeof(void *));
   // if our chunk is too small - grow it
   if ((chunk1_num + size) > chunk1_size)
     {
        // get a new chunk twice as big
        void *newchunk = _eina_debug_chunk_need(chunk1_size * 2);
        if (!newchunk) return NULL;
        // copy content over
        memcpy(newchunk, chunk1, chunk1_num);
        // release old chunk
        _eina_debug_chunk_noneed(chunk1, chunk1_size);
        // switch to our new 2x as big chunk
        chunk1 = newchunk;
        chunk1_size = chunk1_size * 2;
     }
   // get the mem at the top of this stack and return it, then move along
   ptr = chunk1 + chunk1_num;
   chunk1_num += size;
   return ptr;
}

// grow a single existing chunk (we use this for the filename -> path lookup)
void *
_eina_debug_chunk_realloc(int size)
{
   // we have a null/empty second chunk - allocate one
   if (!chunk2)
     {
        chunk2 = _eina_debug_chunk_need(4 * 1024);
        if (!chunk2) return NULL;
        chunk2_size = 4 * 1024;
     }
   // if our chunk is too small - grow it
   if (size > chunk2_size)
     {
        // get a new chunk twice as big
        void *newchunk = _eina_debug_chunk_need(chunk2_size * 2);
        if (!newchunk) return NULL;
        // copy content over
        memcpy(newchunk, chunk2, chunk2_num);
        // release old chunk
        _eina_debug_chunk_noneed(chunk2, chunk2_size);
        // switch to our new 2x as big chunk
        chunk2 = newchunk;
        chunk2_size = chunk2_size * 2;
     }
   // record new size and return chunk ptr as we just re-use it
   chunk2_num = size;
   return chunk2;
}

// grow a single existing chunk (we use this for the filename -> path lookup)
void *
_eina_debug_chunk_tmp_push(int size)
{
   void *ptr;

   // no initial chunk1 block - allocate it
   if (!chunk3)
     {
        chunk3 = _eina_debug_chunk_need(32 * 1024);
        if (!chunk3) return NULL;
        chunk3_size = 32 * 1024;
     }
   // round size up to the nearest pointer size for alignment
   size = sizeof(void *) * ((size + sizeof(void *) - 1) / sizeof(void *));
   // if our chunk is too small - grow it
   if ((chunk3_num + size) > chunk3_size)
     {
        // get a new chunk twice as big
        void *newchunk = _eina_debug_chunk_need(chunk3_size * 2);
        if (!newchunk) return NULL;
        // copy content over
        memcpy(newchunk, chunk3, chunk3_num);
        // release old chunk
        _eina_debug_chunk_noneed(chunk3, chunk3_size);
        // switch to our new 2x as big chunk
        chunk3 = newchunk;
        chunk3_size = chunk3_size * 2;
     }
   // get the mem at the top of this stack and return it, then move along
   ptr = chunk3 + chunk3_num;
   chunk3_num += size;
   return ptr;
}

void
_eina_debug_chunk_tmp_reset(void)
{
   chunk3_num = 0;
}
# else
// implement with static buffers - once we exceed these we will fail. sorry
// maybe one day find another solution, but these buffers should be enough
// for now for thos eplatforms (like windows) where we can't do the mmap
// tricks above.
static unsigned char chunk1[8 * 1024];
static unsigned char chunk2[4 * 1024];
static unsigned char chunk3[128 * 1024];
static int chunk1_size = sizeof(chunk1);
static int chunk1_num = 0;
static int chunk2_size = sizeof(chunk2);
static int chunk2_num = 0;
static int chunk3_size = sizeof(chunk3);
static int chunk3_num = 0;

// push a new bit of mem on our growing stack of mem - given our workload,
// we never free anything here, only ever grow new things on this stack
void *
_eina_debug_chunk_push(int size)
{
   void *ptr;

   // round size up to the nearest pointer size for alignment
   size = sizeof(void *) * ((size + sizeof(void *) - 1) / sizeof(void *));
   // if we ran out of space - fail
   if ((chunk1_num + size) > chunk1_size) return NULL;
   // get the mem at the top of this stack and return it, then move along
   ptr = chunk1 + chunk1_num;
   chunk1_num += size;
   return ptr;
}

// grow a single existing chunk (we use this for the filename -> path lookup)
void *
_eina_debug_chunk_realloc(int size)
{
   // if we ran out of space - fail
   if (size > chunk2_size) return NULL;
   // record new size and return chunk ptr as we just re-use it
   chunk2_num = size;
   return chunk2;
}

// grow a single existing chunk (we use this for the filename -> path lookup)
void *
_eina_debug_chunk_tmp_push(int size)
{
   void *ptr;

   // round size up to the nearest pointer size for alignment
   size = sizeof(void *) * ((size + sizeof(void *) - 1) / sizeof(void *));
   // if we ran out of space - fail
   if ((chunk3_num + size) > chunk3_size) return NULL;
   // get the mem at the top of this stack and return it, then move along
   ptr = chunk3 + chunk1_num;
   chunk3_num += size;
   return ptr;
}

void
_eina_debug_chunk_tmp_reset(void)
{
   chunk3_num = 0;
}
# endif

// handy - duplicate a string on our growing stack - never expect to free it
char *
_eina_debug_chunk_strdup(const char *str)
{
   int len = strlen(str);
   char *s = _eina_debug_chunk_push(len + 1);
   if (!s) return NULL;
   strcpy(s, str);
   return s;
}
