/* EINA - EFL data type library
 * Copyright (C) 2008 Cedric Bail
 * Copyright (C) 2011 Alexandre Becoulet
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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "eina_config.h"
#include "eina_private.h"
#include "eina_array.h"
#include "eina_trash.h"
#include "eina_lock.h"

/* undefs EINA_ARG_NONULL() so NULL checks are not compiled out! */
#include "eina_safety_checks.h"
#include "eina_rbtree.h"

/*============================================================================*
*                                  Local                                     *
*============================================================================*/

#define EINA_RBTREE_ITERATOR_PREFIX_MASK  0x1
#define EINA_RBTREE_ITERATOR_INFIX_MASK   0x2
#define EINA_RBTREE_ITERATOR_POSTFIX_MASK 0x4

typedef struct _Eina_Iterator_Rbtree Eina_Iterator_Rbtree;
typedef struct _Eina_Iterator_Rbtree_List Eina_Iterator_Rbtree_List;

struct _Eina_Iterator_Rbtree
{
   Eina_Iterator iterator;

   Eina_Array *stack;
   Eina_Trash *trash;

   unsigned char mask;
};

struct _Eina_Iterator_Rbtree_List
{
   Eina_Rbtree *tree;

   Eina_Rbtree_Direction dir : 1;
   Eina_Bool up : 1;
};

static Eina_Array iterator_trash;
static Eina_Spinlock iterator_trash_lock;

static Eina_Iterator_Rbtree_List *
_eina_rbtree_iterator_list_new(Eina_Iterator_Rbtree *it, const Eina_Rbtree *tree)
{
   Eina_Iterator_Rbtree_List *new;

   new = eina_trash_pop(&it->trash);
   if (!new)
     {
        new = malloc(sizeof (Eina_Iterator_Rbtree_List));
        if (!new) return NULL;
     }

   new->tree = (Eina_Rbtree *)tree;
   new->dir = EINA_RBTREE_RIGHT;
   new->up = EINA_FALSE;

   return new;
}

static Eina_Rbtree *
_eina_rbtree_iterator_get_content(Eina_Iterator_Rbtree *it)
{
   if (eina_array_count(it->stack) <= 0)
      return NULL;

   return eina_array_data_get(it->stack, 0);
}

static void
_eina_rbtree_iterator_forced_free(Eina_Iterator_Rbtree *it)
{
   Eina_Iterator_Rbtree_List *item;
   Eina_Array_Iterator et;
   unsigned int i;
   void *last;

   EINA_ARRAY_ITER_NEXT(it->stack, i, item, et)
     free(item);

   eina_array_free(it->stack);
   while ((last = eina_trash_pop(&it->trash)))
     free(last);
   free(it);
}

static void
_eina_rbtree_iterator_free(Eina_Iterator_Rbtree *it)
{
   if (eina_array_count(&iterator_trash) >= 7)
     {
        _eina_rbtree_iterator_forced_free(it);
        return;
     }
   if (eina_array_count(it->stack) > 0)
     {
        unsigned int i;
        Eina_Array_Iterator et;
        Eina_Iterator_Rbtree_List *item;

        EINA_ARRAY_ITER_NEXT(it->stack, i, item, et)
          free(item);
     }

   eina_array_clean(it->stack);

   eina_spinlock_take(&iterator_trash_lock);
   eina_array_push(&iterator_trash, it);
   eina_spinlock_release(&iterator_trash_lock);
}

static Eina_Bool
_eina_rbtree_iterator_next(Eina_Iterator_Rbtree *it, void **data)
{
   Eina_Iterator_Rbtree_List *last;
   Eina_Iterator_Rbtree_List *new;
   Eina_Rbtree *tree;

   if (eina_array_count(it->stack) <= 0)
      return EINA_FALSE;

   last = eina_array_data_get(it->stack, eina_array_count(it->stack) - 1);
   tree = last->tree;

   if (!last->tree || last->up == EINA_TRUE)
     {
        last = eina_array_pop(it->stack);
        while (last->dir == EINA_RBTREE_LEFT
               || !last->tree)
          {
             if (tree)
                if ((it->mask & EINA_RBTREE_ITERATOR_POSTFIX_MASK) ==
                    EINA_RBTREE_ITERATOR_POSTFIX_MASK)
                  {
                     eina_trash_push(&it->trash, last);

                     if (eina_array_count(it->stack) > 0)
                       {
                          last = eina_array_data_get(it->stack,
                                                     eina_array_count(
                                                        it->
                                                        stack)
                                                     - 1);
                          last->up = EINA_TRUE;
                       }

                     goto onfix;
                  }

             eina_trash_push(&it->trash, last);

             last = eina_array_pop(it->stack);
             if (!last)
                return EINA_FALSE;

             tree = last->tree;
          }

        last->dir = EINA_RBTREE_LEFT;
        last->up = EINA_FALSE;

        eina_array_push(it->stack, last);

        if ((it->mask & EINA_RBTREE_ITERATOR_INFIX_MASK) ==
            EINA_RBTREE_ITERATOR_INFIX_MASK)
           goto onfix;
     }

   new = _eina_rbtree_iterator_list_new(it, last->tree->son[last->dir]);
   if (!new)
     return EINA_FALSE;

   eina_array_push(it->stack, new);

   if (last->dir == EINA_RBTREE_RIGHT)
      if ((it->mask & EINA_RBTREE_ITERATOR_PREFIX_MASK) ==
          EINA_RBTREE_ITERATOR_PREFIX_MASK)
         goto onfix;

   return _eina_rbtree_iterator_next(it, data);

onfix:
   *data = tree;
   return EINA_TRUE;
}

static Eina_Iterator *
_eina_rbtree_iterator_build(const Eina_Rbtree *root, unsigned char mask)
{
   Eina_Iterator_Rbtree_List *first;
   Eina_Iterator_Rbtree *it;

   eina_spinlock_take(&iterator_trash_lock);
   it = eina_array_pop(&iterator_trash);
   eina_spinlock_release(&iterator_trash_lock);

   if (!it)
     {
        it = calloc(1, sizeof (Eina_Iterator_Rbtree));
        if (!it) return NULL;

        eina_trash_init(&it->trash);

        it->stack = eina_array_new(8);
        if (!it->stack)
          {
             free(it);
             return NULL;
          }
     }

   first = _eina_rbtree_iterator_list_new(it, root);
   if (!first) goto on_error;

   eina_array_push(it->stack, first);

   it->mask = mask;

   it->iterator.version = EINA_ITERATOR_VERSION;
   it->iterator.next = FUNC_ITERATOR_NEXT(_eina_rbtree_iterator_next);
   it->iterator.get_container = FUNC_ITERATOR_GET_CONTAINER(
         _eina_rbtree_iterator_get_content);
   it->iterator.free = FUNC_ITERATOR_FREE(_eina_rbtree_iterator_free);

   EINA_MAGIC_SET(&it->iterator, EINA_MAGIC_ITERATOR);

   return &it->iterator;

on_error:
   _eina_rbtree_iterator_free(it);
   return NULL;
}

static void
_eina_rbtree_node_init(Eina_Rbtree *node)
{
   if (!node)
      return;

   node->son[0] = NULL;
   node->son[1] = NULL;

   node->color = EINA_RBTREE_RED;
}

static inline Eina_Rbtree *
_eina_rbtree_inline_single_rotation(Eina_Rbtree *node,
                                    Eina_Rbtree_Direction dir)
{
   Eina_Rbtree *save = node->son[dir ^ 1];

   node->son[dir ^ 1] = save->son[dir];
   save->son[dir] = node;

   node->color = EINA_RBTREE_RED;
   save->color = EINA_RBTREE_BLACK;

   return save;
}

static inline Eina_Rbtree *
_eina_rbtree_inline_double_rotation(Eina_Rbtree *node,
                                    Eina_Rbtree_Direction dir)
{
   node->son[dir ^ 1] = _eina_rbtree_inline_single_rotation(node->son[dir ^ 1], dir ^ 1);
   return _eina_rbtree_inline_single_rotation(node, dir);
}

/*============================================================================*
*                                 Global                                     *
*============================================================================*/

/*============================================================================*
*                                   API                                      *
*============================================================================*/

EINA_API Eina_Rbtree *
eina_rbtree_inline_insert(Eina_Rbtree *root,
                          Eina_Rbtree *node,
                          Eina_Rbtree_Cmp_Node_Cb cmp,
                          const void *data)
{
   Eina_Rbtree **r = &root;
   Eina_Rbtree *q = root;
   uintptr_t stack[48];
   unsigned int s = 0;

   EINA_SAFETY_ON_NULL_RETURN_VAL(node, root);
   EINA_SAFETY_ON_NULL_RETURN_VAL( cmp, root);

   /* Find insertion leaf */
   while (q != NULL)
      {
	 Eina_Rbtree_Direction dir = cmp(q, node, (void *)data);

	 /* Keep path in stack */
	 stack[s++] = (uintptr_t)r | dir;

	 r = q->son + dir;
	 q = *r;
      }

   /* Insert */
   *r = node;
   _eina_rbtree_node_init(node);

   /* Rebalance */
   while (s > 0)
      {
	 Eina_Rbtree *a, *b;
	 uintptr_t top = stack[--s]; 	 /* Pop link pointer and direction */
	 Eina_Rbtree_Direction dir = top & 1;

	 r = (Eina_Rbtree **)(top & ~(uintptr_t)1);
	 q = *r;

	 a = q->son[dir];
	 /* Rebalance done ? */
	 if (a == NULL || a->color == EINA_RBTREE_BLACK)
	    break;

	 b = q->son[dir ^ 1];
	 if (b != NULL && b->color == EINA_RBTREE_RED)
	    {
	       q->color = EINA_RBTREE_RED;
	       b->color = a->color = EINA_RBTREE_BLACK;
	    }
	 else
	    {
	       Eina_Rbtree *c = a->son[dir];
	       Eina_Rbtree *d = a->son[dir ^ 1];

	       if (c != NULL && c->color == EINA_RBTREE_RED)
	 	  *r = _eina_rbtree_inline_single_rotation(*r, dir ^ 1);
	       else if (d != NULL && d->color == EINA_RBTREE_RED)
	 	  *r = _eina_rbtree_inline_double_rotation(*r, dir ^ 1);
	    }
      }

   root->color = EINA_RBTREE_BLACK;
   return root;
}

EINA_API Eina_Rbtree *
eina_rbtree_inline_remove(Eina_Rbtree *root,
                          Eina_Rbtree *node,
                          Eina_Rbtree_Cmp_Node_Cb cmp,
                          const void *data)
{
   Eina_Rbtree *l0, *l1, *r, **rt = &root;
   Eina_Rbtree_Direction dir;
   uintptr_t stack[48];
   unsigned int s = 0;

   EINA_SAFETY_ON_NULL_RETURN_VAL(node, root);
   EINA_SAFETY_ON_NULL_RETURN_VAL( cmp, root);

   /* Item search loop */
   for (r = *rt; r != NULL; r = *rt)
      {
	 if (r == node)
	    goto found;

	 dir = cmp(r, node, (void*)data);
	 stack[s++] = (uintptr_t)rt | dir;
	 rt = r->son + dir;
      }
   return root;

 found:
   /* remove entry */
   l0 = node->son[0];
   l1 = node->son[1];

   if (l0 != NULL && l1 != NULL)      /* two links case */
      {
	 Eina_Rbtree *q, **t, **p;
	 uintptr_t ss;

	 stack[s++] = (uintptr_t)rt | 1;
	 ss = s;	 /* keep predecessor right link stack index */

	 /* find predecessor */
	 p = node->son + 1;
	 q = *p;

	 while (1)
	    {
	       t = q->son;
	       q = *t;
	       if (q == NULL)
		  break;
	       stack[s++] = (uintptr_t)p | 0;
	       p = t;
	    }

	 /* detach predecessor */
	 q = *p;
	 *p = q->son[1];

	 int c = q->color;

	 /* replace entry by predecessor */
	 memcpy(q, node, sizeof(Eina_Rbtree));
	 *rt = q;

	 if (c == EINA_RBTREE_RED)
	    goto end;

	 /* fix stack for replaced entry */
	 if (s > ss)
	    stack[ss] = (uintptr_t)(q->son + 1) | 0;
      }
   else 	 /* single link case */
      {
	 if (l0 == NULL)
	    l0 = l1;

	 *rt = l0;

	 if (node->color == EINA_RBTREE_RED)
	    goto end; /* removed red */

	 if (l0 != NULL && l0->color == EINA_RBTREE_RED)
	    {
	       /* red child replace removed black */
	       l0->color = EINA_RBTREE_BLACK;
	       goto end;
	    }
      }

   /* rebalance */
   while (s > 0)
      {
	 Eina_Rbtree *q;
	 uintptr_t st = stack[--s];

	 rt = (Eina_Rbtree**)(st & ~(uintptr_t)1);
	 dir = st & 1;
	 r = *rt;
	 q = r->son[dir ^ 1];

	 if (q != NULL && q->color == EINA_RBTREE_RED)
	    {
	       *rt = _eina_rbtree_inline_single_rotation(*rt, dir);
	       q = r->son[dir ^ 1];
	       rt = (*rt)->son + dir;
	    }

	 if (q != NULL)
	    {
	       int r_color = r->color;
	       Eina_Rbtree *nd = q->son[dir ^ 1];

	       if (nd != NULL && nd->color == EINA_RBTREE_RED)
		  {
		     *rt = _eina_rbtree_inline_single_rotation(*rt, dir);
		  }
	       else
		  {
		     Eina_Rbtree *d = q->son[dir];

		     if (d != NULL && d->color == EINA_RBTREE_RED)
			{
			   *rt = _eina_rbtree_inline_double_rotation(*rt, dir);
			}
		     else
			{
			   r->color = EINA_RBTREE_BLACK;
			   q->color = EINA_RBTREE_RED;
			   if (r_color == EINA_RBTREE_RED)
			      break;
			   continue;
			}
		  }

	       r = *rt;
	       r->color = r_color;
	       r->son[1]->color = r->son[0]->color = EINA_RBTREE_BLACK;

	       break;
	    }
      }

 end:
   if (root != NULL)
      root->color = EINA_RBTREE_BLACK;
   return root;
}

EINA_API Eina_Iterator *
eina_rbtree_iterator_prefix(const Eina_Rbtree *root)
{
   return _eina_rbtree_iterator_build(root, EINA_RBTREE_ITERATOR_PREFIX_MASK);
}

EINA_API Eina_Iterator *
eina_rbtree_iterator_infix(const Eina_Rbtree *root)
{
   return _eina_rbtree_iterator_build(root, EINA_RBTREE_ITERATOR_INFIX_MASK);
}

EINA_API Eina_Iterator *
eina_rbtree_iterator_postfix(const Eina_Rbtree *root)
{
   return _eina_rbtree_iterator_build(root, EINA_RBTREE_ITERATOR_POSTFIX_MASK);
}

EINA_API void
eina_rbtree_delete(Eina_Rbtree *root, Eina_Rbtree_Free_Cb func, void *data)
{
   if (!root)
      return;

   EINA_SAFETY_ON_NULL_RETURN(func);

   eina_rbtree_delete(root->son[0], func, data);
   eina_rbtree_delete(root->son[1], func, data);
   func(root, data);
}

Eina_Bool
eina_rbtree_init(void)
{
   eina_array_step_set(&iterator_trash, sizeof(iterator_trash), 8);
   return eina_spinlock_new(&iterator_trash_lock);
}

Eina_Bool
eina_rbtree_shutdown(void)
{
   Eina_Iterator_Rbtree *it;

   while ((it = eina_array_pop(&iterator_trash)))
     _eina_rbtree_iterator_forced_free(it);
   eina_array_flush(&iterator_trash);
   eina_spinlock_free(&iterator_trash_lock);
   return EINA_TRUE;
}
