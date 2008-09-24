/* EINA - EFL data type library
 * Copyright (C) 2008 Cedric Bail
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

#ifndef EINA_RBTREE_H__
#define EINA_RBTREE_H__

#include "eina_types.h"
#include "eina_error.h"
#include "eina_iterator.h"

typedef enum {
  EINA_RBTREE_RED,
  EINA_RBTREE_BLACK
} Eina_Rbtree_Color;

typedef enum {
  EINA_RBTREE_LEFT = 0,
  EINA_RBTREE_RIGHT = 1
} Eina_Rbtree_Direction;

typedef struct _Eina_Rbtree Eina_Rbtree;
struct _Eina_Rbtree
{
   Eina_Rbtree       *son[2];

   Eina_Rbtree_Color  color : 1;
};

#define EINA_RBTREE Eina_Rbtree __rbtree
#define EINA_RBTREE_GET(Rbtree) &(Rbtree->__rbtree)

typedef Eina_Rbtree_Direction (*Eina_Rbtree_Cmp_Node_Cb)(const Eina_Rbtree *left, const Eina_Rbtree *right, void *data);
#define EINA_RBTREE_CMP_NODE_CB(Function) ((Eina_Rbtree_Cmp_Node_Cb) Function)

typedef int (*Eina_Rbtree_Cmp_Key_Cb)(const Eina_Rbtree *node, const void *key, int length, void *data);
#define EINA_RBTREE_CMP_KEY_CB(Function) ((Eina_Rbtree_Cmp_Key_Cb) Function)

typedef void (*Eina_Rbtree_Free_Cb)(Eina_Rbtree *node);
#define EINA_RBTREE_FREE_CB(Function) ((Eina_Rbtree_Free_Cb) Function)

EAPI Eina_Rbtree *eina_rbtree_inline_insert(Eina_Rbtree *root, Eina_Rbtree *node, Eina_Rbtree_Cmp_Node_Cb cmp, const void *data);
EAPI Eina_Rbtree *eina_rbtree_inline_remove(Eina_Rbtree *root, Eina_Rbtree *node, Eina_Rbtree_Cmp_Node_Cb cmp, const void *data);
EAPI void eina_rbtree_delete(Eina_Rbtree *root, Eina_Rbtree_Free_Cb func);

static inline Eina_Rbtree *eina_rbtree_inline_lookup(Eina_Rbtree *root, const void *key, int length, Eina_Rbtree_Cmp_Key_Cb cmp, const void *data);

EAPI Eina_Iterator *eina_rbtree_iterator_prefix(const Eina_Rbtree *root);
EAPI Eina_Iterator *eina_rbtree_iterator_infix(const Eina_Rbtree *root);
EAPI Eina_Iterator *eina_rbtree_iterator_postfix(const Eina_Rbtree *root);

#include "eina_inline_rbtree.x"

#endif
