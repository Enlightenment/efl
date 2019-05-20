/* EINA - EFL data type library
 * Copyright (C) 2002,2003,2004,2005,2006,2007,2008,2010
 *                         Carsten Haitzler,
 *                         Jorge Luis Zapata Muga,
 *                         Cedric Bail,
 *                         Gustavo Sverzut Barbieri
 *                         Tom Hacohen
 *                         Brett Nash
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
 *
 * This file incorporates work covered by the following copyright and
 * permission notice:
 *
 * Copyright (C) 2008 Peter Wehrfritz
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to
 *  deal in the Software without restriction, including without limitation the
 *  rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 *  sell copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in
 *  all copies of the Software and its Copyright notices. In addition publicly
 *  documented acknowledgment must be given that this software has been used if no
 *  source code of this software is made available publicly. This includes
 *  acknowledgments in either Copyright notices, Manuals, Publicity and Marketing
 *  documents or any documentation provided with any product containing this
 *  software. This License does not apply to any software that links to the
 *  libraries provided by this software (statically or dynamically), but only to
 *  the software provided.
 *
 *  Please see the OLD-COPYING.PLAIN for a plain-english explanation of this notice
 *  and it's intent.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 *  THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 *  IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 *  CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stddef.h>

#include "eina_config.h"
#include "eina_private.h"
#include "eina_hash.h"
#include "eina_rbtree.h"
#include "eina_lock.h"

/* undefs EINA_ARG_NONULL() so NULL checks are not compiled out! */
#include "eina_safety_checks.h"
#include "eina_share_common.h"

/*============================================================================*
*                                  Local                                     *
*============================================================================*/

/**
 * @cond LOCAL
 */

/** The global Eina stringshare log domain */
extern int _eina_share_stringshare_log_dom;

#ifdef DBG_STRINGSHARE
#undef DBG_STRINGSHARE
#endif
#define DBG_STRINGSHARE(...) EINA_LOG_DOM_DBG(_eina_share_stringshare_log_dom, __VA_ARGS__)

#define EINA_SHARE_COMMON_BUCKETS 256
#define EINA_SHARE_COMMON_MASK 0xFF
#define EINA_SHARE_COMMON_BUCKET_IDX(h) ((h >> 8) & EINA_SHARE_COMMON_MASK)
#define EINA_SHARE_COMMON_NODE_HASH(h) (h & EINA_SHARE_COMMON_MASK)

static const char EINA_MAGIC_SHARE_STR[] = "Eina Share";
static const char EINA_MAGIC_SHARE_HEAD_STR[] = "Eina Share Head";

static int _eina_share_common_count = 0;

#define EINA_MAGIC_CHECK_SHARE_COMMON_HEAD(d, unlock, ...)      \
   do {                                                          \
        if (!EINA_MAGIC_CHECK((d), EINA_MAGIC_SHARE_HEAD))  \
          {                                                           \
             EINA_MAGIC_FAIL((d), EINA_MAGIC_SHARE_HEAD);    \
             unlock;                                                 \
             return __VA_ARGS__;                                     \
          }                                                           \
     } while (0)

#define EINA_MAGIC_CHECK_SHARE_COMMON_NODE(d, _node_magic, unlock)              \
   do {                                                          \
        if (!EINA_MAGIC_CHECK((d), _node_magic))    \
          {                                                           \
             EINA_MAGIC_FAIL((d), _node_magic);        \
             unlock;                                                   \
          }                                                           \
     } while (0)

#ifdef EINA_STRINGSHARE_USAGE
typedef struct _Eina_Share_Common_Population Eina_Share_Common_Population;
struct _Eina_Share_Common_Population
{
   int count;
   int max;
};
#endif

typedef struct _Eina_Share_Common Eina_Share_Common;
typedef struct _Eina_Share_Common_Node Eina_Share_Common_Node;
typedef struct _Eina_Share_Common_Head Eina_Share_Common_Head;

struct _Eina_Share
{
   Eina_Share_Common *share;
   Eina_Magic node_magic;
#ifdef EINA_STRINGSHARE_USAGE
   Eina_Share_Common_Population population;
   Eina_Share_Common_Population population_group[4];
   int max_node_population;
#endif
};

struct _Eina_Share_Common
{
   Eina_Share_Common_Head *buckets[EINA_SHARE_COMMON_BUCKETS];

   EINA_MAGIC
};

struct _Eina_Share_Common_Node
{
   Eina_Share_Common_Node *next;

   EINA_MAGIC

   unsigned int length;
   unsigned int references;
   char str[];
};

struct _Eina_Share_Common_Head
{
   EINA_RBTREE;
   EINA_MAGIC

   int hash;

#ifdef EINA_STRINGSHARE_USAGE
   int population;
#endif

   Eina_Share_Common_Node *head;
   Eina_Share_Common_Node builtin_node;
};

Eina_Bool _share_common_threads_activated = EINA_FALSE;

static Eina_Spinlock _mutex_big;

#ifdef EINA_STRINGSHARE_USAGE

static void
_eina_share_common_population_init(Eina_Share *share)
{
   unsigned int i;

   for (i = 0;
        i < sizeof (share->population_group) /
        sizeof (share->population_group[0]);
        ++i)
     {
        share->population_group[i].count = 0;
        share->population_group[i].max = 0;
     }
}

static void
_eina_share_common_population_shutdown(Eina_Share *share)
{
   unsigned int i;

   share->max_node_population = 0;
   share->population.count = 0;
   share->population.max = 0;

   for (i = 0;
        i < sizeof (share->population_group) /
        sizeof (share->population_group[0]);
        ++i)
     {
        share->population_group[i].count = 0;
        share->population_group[i].max = 0;
     }
}

static void
_eina_share_common_population_stats(Eina_Share *share)
{
   unsigned int i;

      DBG_STRINGSHARE("eina share_common statistic:");
      DBG_STRINGSHARE(" * maximum shared strings : %i",
                      share->population.max);
      DBG_STRINGSHARE(" * maximum shared strings per node : %i",
                      share->max_node_population);

   for (i = 0;
        i < sizeof (share->population_group) /
        sizeof (share->population_group[0]);
        ++i)
      DBG_STRINGSHARE("DDD: %i strings of length %u, max strings: %i",
                      share->population_group[i].count, i,
                      share->population_group[i].max);
}

static void
eina_share_common_population_nolock_add(Eina_Share *share, int slen)
{
   share->population.count++;
   if (share->population.count > share->population.max)
      share->population.max = share->population.count;

   if (slen < 4)
     {
        share->population_group[slen].count++;
        if (share->population_group[slen].count >
            share->population_group[slen].max)
           share->population_group[slen].max =
              share->population_group[slen].count;
     }
}

void
eina_share_common_population_add(Eina_Share *share, int slen)
{
   eina_spinlock_take(&_mutex_big);
   eina_share_common_population_nolock_add(share, slen);
   eina_spinlock_release(&_mutex_big);
}

static void
eina_share_common_population_nolock_del(Eina_Share *share, int slen)
{
   share->population.count--;
   if (slen < 4)
      share->population_group[slen].count--;
}

void
eina_share_common_population_del(Eina_Share *share, int slen)
{
   eina_spinlock_take(&_mutex_big);
   eina_share_common_population_nolock_del(share, slen);
   eina_spinlock_release(&_mutex_big);
}

static void
_eina_share_common_population_head_init(EINA_UNUSED Eina_Share *share,
                                        Eina_Share_Common_Head *head)
{
   head->population = 1;
}

static void
_eina_share_common_population_head_add(Eina_Share *share,
                                       Eina_Share_Common_Head *head)
{
   head->population++;
   if (head->population > share->max_node_population)
      share->max_node_population = head->population;
}

static void
_eina_share_common_population_head_del(EINA_UNUSED Eina_Share *share,
                                       Eina_Share_Common_Head *head)
{
   head->population--;
}

#else /* EINA_STRINGSHARE_USAGE undefined */

static void _eina_share_common_population_init(EINA_UNUSED Eina_Share *share) {
}
static void _eina_share_common_population_shutdown(EINA_UNUSED Eina_Share *share)
{
}
static void _eina_share_common_population_stats(EINA_UNUSED Eina_Share *share) {
}
static void eina_share_common_population_nolock_add(EINA_UNUSED Eina_Share *share,
                                                    EINA_UNUSED int slen) {
}
void eina_share_common_population_add(EINA_UNUSED Eina_Share *share,
                                      EINA_UNUSED int slen) {
}
static void eina_share_common_population_nolock_del(EINA_UNUSED Eina_Share *share,
                                                    EINA_UNUSED int slen) {
}
void eina_share_common_population_del(EINA_UNUSED Eina_Share *share,
                                      EINA_UNUSED int slen) {
}
static void _eina_share_common_population_head_init(
   EINA_UNUSED Eina_Share *share,
   EINA_UNUSED Eina_Share_Common_Head *head) {
}
static void _eina_share_common_population_head_add(
   EINA_UNUSED Eina_Share *share,
   EINA_UNUSED
   Eina_Share_Common_Head *head) {
}
static void _eina_share_common_population_head_del(
   EINA_UNUSED Eina_Share *share,
   EINA_UNUSED
   Eina_Share_Common_Head *head) {
}
#endif

static int
_eina_share_common_cmp(const Eina_Share_Common_Head *ed,
                       const int *hash,
                       EINA_UNUSED int length,
                       EINA_UNUSED void *data)
{
   EINA_MAGIC_CHECK_SHARE_COMMON_HEAD(ed, , 0);

   return EINA_SHARE_COMMON_NODE_HASH(ed->hash) - *hash;
}

static Eina_Rbtree_Direction
_eina_share_common_node(const Eina_Share_Common_Head *left,
                        const Eina_Share_Common_Head *right,
                        EINA_UNUSED void *data)
{
   EINA_MAGIC_CHECK_SHARE_COMMON_HEAD(left,  , 0);
   EINA_MAGIC_CHECK_SHARE_COMMON_HEAD(right, , 0);

   if (EINA_SHARE_COMMON_NODE_HASH(left->hash) - EINA_SHARE_COMMON_NODE_HASH(right->hash) < 0)
      return EINA_RBTREE_LEFT;

   return EINA_RBTREE_RIGHT;
}

static void
_eina_share_common_head_free(Eina_Share_Common_Head *ed, EINA_UNUSED void *data)
{
   EINA_MAGIC_CHECK_SHARE_COMMON_HEAD(ed, );

   while (ed->head)
     {
        Eina_Share_Common_Node *el = ed->head;

        ed->head = ed->head->next;
        if (el != &ed->builtin_node)
           MAGIC_FREE(el);
     }
           MAGIC_FREE(ed);
}

static void
_eina_share_common_node_init(Eina_Share_Common_Node *node,
                             const char *str,
                             int slen,
                             unsigned int null_size,
                             Eina_Magic node_magic)
{
   EINA_MAGIC_SET(node, node_magic);
   node->references = 1;
   node->length = slen;
   memcpy(node->str, str, slen);
   memset(node->str + slen, 0, null_size); /* Nullify the null */

   (void) node_magic; /* When magic are disable, node_magic is unused, this remove a warning. */
}

static Eina_Share_Common_Head *
_eina_share_common_head_alloc(int slen)
{
   Eina_Share_Common_Head *head;
   const size_t head_size = offsetof(Eina_Share_Common_Head, builtin_node.str);

   head = malloc(head_size + slen);
   return head;
}

static const char *
_eina_share_common_add_head(Eina_Share *share,
                            Eina_Share_Common_Head **p_bucket,
                            int hash,
                            const char *str,
                            unsigned int slen,
                            unsigned int null_size)
{
   Eina_Rbtree **p_tree = (Eina_Rbtree **)p_bucket;
   Eina_Share_Common_Head *head;

   head = _eina_share_common_head_alloc(slen + null_size);
   if (!head)
      return NULL;

   EINA_MAGIC_SET(head, EINA_MAGIC_SHARE_HEAD);
   head->hash = hash;
   head->head = &head->builtin_node;
   _eina_share_common_node_init(head->head,
                                str,
                                slen,
                                null_size,
                                share->node_magic);
   head->head->next = NULL;

   _eina_share_common_population_head_init(share, head);

   *p_tree = eina_rbtree_inline_insert
         (*p_tree, EINA_RBTREE_GET(head),
         EINA_RBTREE_CMP_NODE_CB(_eina_share_common_node), NULL);

   return head->head->str;
}

static void
_eina_share_common_del_head(Eina_Share_Common_Head **p_bucket,
                            Eina_Share_Common_Head *head)
{
   Eina_Rbtree **p_tree = (Eina_Rbtree **)p_bucket;

   *p_tree = eina_rbtree_inline_remove
         (*p_tree, EINA_RBTREE_GET(head),
         EINA_RBTREE_CMP_NODE_CB(_eina_share_common_node), NULL);

         MAGIC_FREE(head);
}


static inline Eina_Bool
_eina_share_common_node_eq(const Eina_Share_Common_Node *node,
                           const char *str,
                           unsigned int slen)
{
   return ((node->length == slen) &&
           (memcmp(node->str, str, slen) == 0));
}

static Eina_Share_Common_Node *
_eina_share_common_head_find(Eina_Share_Common_Head *head,
                             const char *str,
                             unsigned int slen)
{
   Eina_Share_Common_Node *node, *prev;

   node = head->head;
   if (_eina_share_common_node_eq(node, str, slen))
      return node;

   prev = node;
   node = node->next;
   for (; node; prev = node, node = node->next)
      if (_eina_share_common_node_eq(node, str, slen))
        {
           /* promote node except builtin_node, make hot items be at the beginning */
           if (node->next)
             {
                prev->next = node->next;
                node->next = head->head;
                head->head = node;
             }
           return node;
        }

   return NULL;
}

static Eina_Bool
_eina_share_common_head_remove_node(Eina_Share_Common_Head *head,
                                    const Eina_Share_Common_Node *node)
{
   Eina_Share_Common_Node *cur, *prev;

   if (head->head == node)
     {
        head->head = node->next;
        return 1;
     }

   prev = head->head;
   cur = head->head->next;
   for (; cur; prev = cur, cur = cur->next)
      if (cur == node)
        {
           prev->next = cur->next;
           return 1;
        }

   return 0;
}

static Eina_Share_Common_Head *
_eina_share_common_find_hash(Eina_Share_Common_Head *bucket, int hash)
{
   return (Eina_Share_Common_Head *)eina_rbtree_inline_lookup
             (EINA_RBTREE_GET(bucket), &hash, 0,
             EINA_RBTREE_CMP_KEY_CB(_eina_share_common_cmp), NULL);
}

static Eina_Share_Common_Head *
_eina_share_common_head_from_node(Eina_Share_Common_Node *node)
{
   Eina_Share_Common_Head *head;
   const size_t offset = offsetof(Eina_Share_Common_Head, builtin_node);

   while (node->next)
     node = node->next;
   head = (Eina_Share_Common_Head *)((char*)node - offset);
   EINA_MAGIC_CHECK_SHARE_COMMON_HEAD(head, , 0);

   return head;
}

static Eina_Share_Common_Node *
_eina_share_common_node_alloc(unsigned int slen, unsigned int null_size)
{
   Eina_Share_Common_Node *node;
   const size_t node_size = offsetof(Eina_Share_Common_Node, str);

   node = malloc(node_size + slen + null_size);
   return node;
}

static Eina_Share_Common_Node *
_eina_share_common_node_from_str(const char *str, Eina_Magic node_magic)
{
   Eina_Share_Common_Node *node;
   const size_t offset = offsetof(Eina_Share_Common_Node, str);

   node = (Eina_Share_Common_Node *)(str - offset);
   EINA_MAGIC_CHECK_SHARE_COMMON_NODE(node, node_magic, node = NULL);
   return node;

   (void) node_magic; /* When magic are disable, node_magic is unused, this remove a warning. */
}

static Eina_Bool
eina_iterator_array_check(const Eina_Rbtree *rbtree EINA_UNUSED,
                          Eina_Share_Common_Head *head,
                          struct dumpinfo *fdata)
{
   Eina_Share_Common_Node *node;

   fdata->used += sizeof(Eina_Share_Common_Head);
   for (node = head->head; node; node = node->next)
     {
        EINA_LOG_DBG("DDD: %5i %5i ", node->length, node->references);
        EINA_LOG_DBG("'%.*s'", node->length, ((char *)node) + sizeof(Eina_Share_Common_Node));
        fdata->used += sizeof(Eina_Share_Common_Node);
        fdata->used += node->length;
        fdata->saved += (node->references - 1) * node->length;
        fdata->dups += node->references - 1;
        fdata->unique++;
     }

   return EINA_TRUE;
}

/**
 * @endcond
 */


/*============================================================================*
*                                 Global                                     *
*============================================================================*/

/**
 * @internal
 * @brief Initialize the share_common module.
 *
 * @return #EINA_TRUE on success, #EINA_FALSE on failure.
 *
 * This function sets up the share_common module of Eina. It is called by
 * eina_init().
 *
 * @see eina_init()
 */
Eina_Bool
eina_share_common_init(Eina_Share **_share,
                       Eina_Magic node_magic,
                       const char *node_magic_STR)
{
   Eina_Share *share;

   share = *_share = calloc(1, sizeof(Eina_Share));
   if (!share) goto on_error;

   share->share = calloc(1, sizeof(Eina_Share_Common));
   if (!share->share) goto on_error;

   share->node_magic = node_magic;
#define EMS(n) eina_magic_string_static_set(n, n ## _STR)
   EMS(EINA_MAGIC_SHARE);
   EMS(EINA_MAGIC_SHARE_HEAD);
   EMS(node_magic);
#undef EMS
   EINA_MAGIC_SET(share->share, EINA_MAGIC_SHARE);

   _eina_share_common_population_init(share);

   /* below is the common part among other all eina_share_common user */
   if (_eina_share_common_count++ != 0)
     return EINA_TRUE;

   eina_spinlock_new(&_mutex_big);
   return EINA_TRUE;

 on_error:
   _eina_share_common_count--;
   return EINA_FALSE;
}

/**
 * @internal
 * @brief Shut down the share_common module.
 *
 * @return #EINA_TRUE on success, #EINA_FALSE on failure.
 *
 * This function shuts down the share_common module set up by
 * eina_share_common_init(). It is called by eina_shutdown().
 *
 * @see eina_shutdown()
 */
Eina_Bool
eina_share_common_shutdown(Eina_Share **_share)
{
   unsigned int i;
   Eina_Share *share = *_share;

   eina_spinlock_take(&_mutex_big);

   _eina_share_common_population_stats(share);

   /* remove any string still in the table */
   for (i = 0; i < EINA_SHARE_COMMON_BUCKETS; i++)
     {
        eina_rbtree_delete(EINA_RBTREE_GET(
                              share->share->buckets[i]),
                           EINA_RBTREE_FREE_CB(
                              _eina_share_common_head_free), NULL);
        share->share->buckets[i] = NULL;
     }
   MAGIC_FREE(share->share);

   _eina_share_common_population_shutdown(share);

   eina_spinlock_release(&_mutex_big);

   free(*_share);
   *_share = NULL;

   /* below is the common part among other all eina_share_common user */
   if (--_eina_share_common_count != 0)
     return EINA_TRUE;

   eina_spinlock_free(&_mutex_big);

   return EINA_TRUE;
}

#ifdef EFL_HAVE_THREADS

/**
 * @internal
 * @brief Activate the share_common mutexes.
 *
 * This function activate the mutexes in the eina share_common module. It is called by
 * eina_threads_init().
 *
 * @see eina_threads_init()
 */
void
eina_share_common_threads_init(void)
{
   _share_common_threads_activated = EINA_TRUE;
}

/**
 * @internal
 * @brief Shut down the share_common mutexes.
 *
 * This function shuts down the mutexes in the share_common module.
 * It is called by eina_threads_shutdown().
 *
 * @see eina_threads_shutdown()
 */
void
eina_share_common_threads_shutdown(void)
{
   _share_common_threads_activated = EINA_FALSE;
}

#endif

/*============================================================================*
*                                   API                                      *
*============================================================================*/

/**
 * @cond LOCAL
 */

const char *
eina_share_common_add_length(Eina_Share *share,
                             const char *str,
                             unsigned int slen,
                             unsigned int null_size)
{
   Eina_Share_Common_Head **p_bucket, *ed;
   Eina_Share_Common_Node *el;
   int hash;

   if (!str)
      return NULL;

   eina_share_common_population_add(share, slen);

   if (slen == 0)
      return NULL;

   hash = eina_hash_superfast(str, slen);

   eina_spinlock_take(&_mutex_big);
   p_bucket = share->share->buckets + EINA_SHARE_COMMON_BUCKET_IDX(hash);

   ed = _eina_share_common_find_hash(*p_bucket, EINA_SHARE_COMMON_NODE_HASH(hash));
   if (!ed)
     {
        const char *s = _eina_share_common_add_head(share,
                                                    p_bucket,
                                                    hash,
                                                    str,
                                                    slen,
                                                    null_size);
        eina_spinlock_release(&_mutex_big);
        return s;
     }

   EINA_MAGIC_CHECK_SHARE_COMMON_HEAD(ed, eina_spinlock_release(&_mutex_big), NULL);

   el = _eina_share_common_head_find(ed, str, slen);
   if (el)
     {
        EINA_MAGIC_CHECK_SHARE_COMMON_NODE
          (el, share->node_magic,
           eina_spinlock_release(&_mutex_big); return NULL);
        el->references++;
        eina_spinlock_release(&_mutex_big);
        return el->str;
     }

   el = _eina_share_common_node_alloc(slen, null_size);
   if (!el)
     {
                                           eina_spinlock_release(&_mutex_big);
        return NULL;
     }

   _eina_share_common_node_init(el, str, slen, null_size, share->node_magic);
   el->next = ed->head;
   ed->head = el;
   _eina_share_common_population_head_add(share, ed);

   eina_spinlock_release(&_mutex_big);

   return el->str;
}

const char *
eina_share_common_ref(Eina_Share *share, const char *str)
{
   Eina_Share_Common_Node *node;

   if (!str)
      return NULL;

   eina_spinlock_take(&_mutex_big);
   node = _eina_share_common_node_from_str(str, share->node_magic);
   if (!node)
     {
        eina_spinlock_release(&_mutex_big);
        return str;
     }
   node->references++;

   eina_share_common_population_nolock_add(share, node->length);

   eina_spinlock_release(&_mutex_big);

   return str;
}


Eina_Bool
eina_share_common_del(Eina_Share *share, const char *str)
{
   unsigned int slen;
   Eina_Share_Common_Head *ed;
   Eina_Share_Common_Head **p_bucket;
   Eina_Share_Common_Node *node;

   if (!str)
      return EINA_TRUE;

   eina_spinlock_take(&_mutex_big);

   node = _eina_share_common_node_from_str(str, share->node_magic);
   if (!node)
      goto on_error;

   slen = node->length;
   eina_share_common_population_nolock_del(share, slen);
   if (node->references > 1)
     {
        node->references--;
        eina_spinlock_release(&_mutex_big);
        return EINA_TRUE;
     }

   node->references = 0;

   ed = _eina_share_common_head_from_node(node);
   if (!ed)
      goto on_error;

   EINA_MAGIC_CHECK_SHARE_COMMON_HEAD(ed, eina_spinlock_release(&_mutex_big), EINA_FALSE);

   if (node != &ed->builtin_node)
     {
        if (!_eina_share_common_head_remove_node(ed, node))
          goto on_error;
        MAGIC_FREE(node);
     }

   if (!ed->head || ed->head->references == 0)
     {
        p_bucket = share->share->buckets + EINA_SHARE_COMMON_BUCKET_IDX(ed->hash);
        _eina_share_common_del_head(p_bucket, ed);
     }
   else
      _eina_share_common_population_head_del(share, ed);

   eina_spinlock_release(&_mutex_big);

   return EINA_TRUE;

on_error:
   eina_spinlock_release(&_mutex_big);
   /* possible segfault happened before here, but... */
   return EINA_FALSE;
}

int
eina_share_common_length(EINA_UNUSED Eina_Share *share, const char *str)
{
   const Eina_Share_Common_Node *node;

   if (!str)
      return -1;

   node = _eina_share_common_node_from_str(str, share->node_magic);
   if (!node) return 0;
   return node->length;
}

void
eina_share_common_dump(Eina_Share *share, void (*additional_dump)(
                          struct dumpinfo *), int used)
{
   Eina_Iterator *it;
   unsigned int i;
   struct dumpinfo di;

   if (!share)
      return;

   di.used = used;
   di.saved = 0;
   di.dups = 0;
   di.unique = 0;

   eina_spinlock_take(&_mutex_big);
   for (i = 0; i < EINA_SHARE_COMMON_BUCKETS; i++)
     {
        if (!share->share->buckets[i])
          {
             continue;
          }

        it = eina_rbtree_iterator_prefix(
              (Eina_Rbtree *)share->share->buckets[i]);
        eina_iterator_foreach(it, EINA_EACH_CB(eina_iterator_array_check), &di);
        eina_iterator_free(it);
     }
   if (additional_dump)
      additional_dump(&di);

#ifdef EINA_STRINGSHARE_USAGE
   /* One character strings are not counted in the hash. */
   di.saved += share->population_group[0].count * sizeof(char);
   di.saved += share->population_group[1].count * sizeof(char) * 2;
#endif
   EINA_LOG_DBG("DDD:-------------------");
   EINA_LOG_DBG("DDD: usage (bytes) = %i, saved = %i (%3.0f%%)",
                di.used, di.saved, di.used ? (di.saved * 100.0 / di.used) : 0.0);
   EINA_LOG_DBG("DDD: unique: %d, duplicates: %d (%3.0f%%)",
                di.unique, di.dups, di.unique ? (di.dups * 100.0 / di.unique) : 0.0);

#ifdef EINA_STRINGSHARE_USAGE
   DBG_STRINGSHARE("DDD: Allocated strings: %i", share->population.count);
   DBG_STRINGSHARE("DDD: Max allocated strings: %i", share->population.max);
   DBG_STRINGSHARE("DDD: Max shared strings per node : %i", share->max_node_population);

   for (i = 0;
        i < sizeof (share->population_group) /
        sizeof (share->population_group[0]);
        ++i)
      DBG_STRINGSHARE("DDD: %i strings of length %u, max strings: %i",
                      share->population_group[i].count, i,
                      share->population_group[i].max);
#endif

   eina_spinlock_release(&_mutex_big);
}

/**
 * @endcond
 */
