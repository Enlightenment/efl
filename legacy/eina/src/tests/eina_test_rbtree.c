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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdlib.h>
#include <time.h>
#include <stdio.h>

#include "eina_array.h"
#include "eina_suite.h"
#include "eina_rbtree.h"

static inline Eina_Bool
_eina_rbtree_is_red(Eina_Rbtree *tree)
{
   return tree != NULL && tree->color == EINA_RBTREE_RED;
}

static int
_eina_rbtree_black_height(Eina_Rbtree *tree, Eina_Rbtree_Cmp_Node_Cb cmp)
{
   Eina_Rbtree *left;
   Eina_Rbtree *right;
   Eina_Rbtree_Direction dir;
   int left_height;
   int right_height;

   if (!tree) return 1;

   left = tree->son[EINA_RBTREE_LEFT];
   right = tree->son[EINA_RBTREE_RIGHT];

   /* Consecutive red links. */
   fail_if(_eina_rbtree_is_red(tree) && (_eina_rbtree_is_red(left) || _eina_rbtree_is_red(right)));

   left_height = _eina_rbtree_black_height(left, cmp);
   right_height = _eina_rbtree_black_height(right, cmp);

   /* Check binary search tree. */
   if (left)
     {
	dir = cmp(tree, left);
	fail_if(dir != EINA_RBTREE_LEFT);
     }

   if (right)
     {
	dir = cmp(tree, right);
	fail_if(dir != EINA_RBTREE_RIGHT);
     }

   /* Check black height */
   if (left_height != right_height)
     fprintf(stderr, "%i != %i\n", left_height, right_height);
   fail_if(left_height != right_height);

   return _eina_rbtree_is_red(tree) ? left_height : left_height + 1;
}

typedef struct _Eina_Rbtree_Int Eina_Rbtree_Int;
struct _Eina_Rbtree_Int
{
   Eina_Rbtree node;
   int value;
};

static Eina_Rbtree_Direction
eina_rbtree_int_cmp(const Eina_Rbtree_Int *left, const Eina_Rbtree_Int *right)
{
   if (!left) return EINA_RBTREE_RIGHT;
   if (!right) return EINA_RBTREE_LEFT;

   if (left->value < right->value) return EINA_RBTREE_LEFT;
   return EINA_RBTREE_RIGHT;
}

static int
eina_rbtree_int_key(const Eina_Rbtree_Int *node, const int *key, __UNUSED__ int length)
{
   if (!node) return 1;
   return node->value - *key;
}

static Eina_Rbtree_Int *
_eina_rbtree_int_new(int value)
{
   Eina_Rbtree_Int *it;

   it = malloc(sizeof (Eina_Rbtree_Int));
   fail_if(!it);

   it->value = value;

   return it;
}

START_TEST(eina_rbtree_insertion)
{
   Eina_Rbtree_Int *root = NULL;
   Eina_Rbtree_Int *item;
   int i;

   srand(time(NULL));

   for (i = 0; i < 500; ++i)
     {
	item = _eina_rbtree_int_new(rand());
	root = (Eina_Rbtree_Int*) eina_rbtree_inline_insert(&root->node, &item->node, EINA_RBTREE_CMP_NODE_CB(eina_rbtree_int_cmp));
     }

   _eina_rbtree_black_height(&root->node, EINA_RBTREE_CMP_NODE_CB(eina_rbtree_int_cmp));
}
END_TEST

START_TEST(eina_rbtree_lookup)
{
   Eina_Rbtree_Int *root = NULL;
   Eina_Rbtree_Int *item;
   int list[] = { 50, 100, 10, 43, 23 };
   unsigned int i;

   for (i = 0; i < sizeof (list) / sizeof (int); ++i)
     {
	item = _eina_rbtree_int_new(list[i]);
	root = (Eina_Rbtree_Int*) eina_rbtree_inline_insert(&root->node, &item->node, EINA_RBTREE_CMP_NODE_CB(eina_rbtree_int_cmp));
     }

   item = (Eina_Rbtree_Int*) eina_rbtree_inline_lookup(&root->node, &list[0], sizeof(int), EINA_RBTREE_CMP_KEY_CB(eina_rbtree_int_key));
   fail_if(!item);

   i = 42;
   item = (Eina_Rbtree_Int*) eina_rbtree_inline_lookup(&root->node, &i, sizeof(int), EINA_RBTREE_CMP_KEY_CB(eina_rbtree_int_key));
   fail_if(item);
}
END_TEST

START_TEST(eina_rbtree_remove)
{
   Eina_Rbtree_Int *root = NULL;
   Eina_Rbtree_Int *item;
   Eina_Array *ea;
   Eina_Array_Iterator it;
   unsigned int i;

   eina_array_init();

   ea =  eina_array_new(11);
   fail_if(!ea);

   srand(time(NULL));

   for (i = 0; i < 100; ++i)
     {
	item = _eina_rbtree_int_new(rand());
	eina_array_push(ea, item);
	root = (Eina_Rbtree_Int*) eina_rbtree_inline_insert(&root->node, &item->node, EINA_RBTREE_CMP_NODE_CB(eina_rbtree_int_cmp));
     }

   _eina_rbtree_black_height(&root->node, EINA_RBTREE_CMP_NODE_CB(eina_rbtree_int_cmp));

   EINA_ARRAY_ITER_NEXT(ea, i, item, it)
     {
	root = (Eina_Rbtree_Int*) eina_rbtree_inline_remove(&root->node, &item->node, EINA_RBTREE_CMP_NODE_CB(eina_rbtree_int_cmp));
	_eina_rbtree_black_height(&root->node, EINA_RBTREE_CMP_NODE_CB(eina_rbtree_int_cmp));
     }

   fail_if(root != NULL);

   eina_array_shutdown();
}
END_TEST

static void
eina_rbtree_display_int(Eina_Rbtree_Int *root, int offset)
{
   int i;

   for (i = 0; i < offset; ++i)
     fprintf(stderr, "\t");

   if (!root)
     {
	fprintf(stderr, "NULL\n");
	return ;
     }

   fprintf(stderr, "v: %i ", root->value);

   if (root->node.color == EINA_RBTREE_RED)
     fprintf(stderr, "r");
   else
     fprintf(stderr, "b");
   fprintf(stderr, "\n");

   for (i = 0; i < offset; ++i)
     fprintf(stderr, "\t");
   eina_rbtree_display_int((Eina_Rbtree_Int*) root->node.son[EINA_RBTREE_RIGHT], offset + 1);

   for (i = 0; i < offset; ++i)
     fprintf(stderr, "\t");
   eina_rbtree_display_int((Eina_Rbtree_Int*) root->node.son[EINA_RBTREE_LEFT], offset + 1);
}

START_TEST(eina_rbtree_simple_remove)
{
   Eina_Rbtree *root = NULL;
   Eina_Rbtree *lookup;
   int i;

   root = eina_rbtree_inline_insert(root, (Eina_Rbtree*) _eina_rbtree_int_new(10), EINA_RBTREE_CMP_NODE_CB(eina_rbtree_int_cmp));
   root = eina_rbtree_inline_insert(root, (Eina_Rbtree*) _eina_rbtree_int_new(42), EINA_RBTREE_CMP_NODE_CB(eina_rbtree_int_cmp));
   root = eina_rbtree_inline_insert(root, (Eina_Rbtree*) _eina_rbtree_int_new(69), EINA_RBTREE_CMP_NODE_CB(eina_rbtree_int_cmp));
   root = eina_rbtree_inline_insert(root, (Eina_Rbtree*) _eina_rbtree_int_new(1337), EINA_RBTREE_CMP_NODE_CB(eina_rbtree_int_cmp));
   _eina_rbtree_black_height(root, EINA_RBTREE_CMP_NODE_CB(eina_rbtree_int_cmp));

   fail_if(root == NULL);

   fprintf(stderr, "******\n");
   eina_rbtree_display_int((Eina_Rbtree_Int*) root, 0);
   fprintf(stderr, "******\n");

   i = 69;
   lookup = eina_rbtree_inline_lookup(root, &i, sizeof (int), EINA_RBTREE_CMP_KEY_CB(eina_rbtree_int_key));
   _eina_rbtree_black_height(root, EINA_RBTREE_CMP_NODE_CB(eina_rbtree_int_cmp));
   fail_if(lookup == NULL);

   root = eina_rbtree_inline_remove(root, lookup, EINA_RBTREE_CMP_NODE_CB(eina_rbtree_int_cmp));
   _eina_rbtree_black_height(root, EINA_RBTREE_CMP_NODE_CB(eina_rbtree_int_cmp));
}
END_TEST

START_TEST(eina_rbtree_simple_remove2)
{
   Eina_Rbtree *root = NULL;
   Eina_Rbtree *lookup;
   int i;

   root = eina_rbtree_inline_insert(root, (Eina_Rbtree*) _eina_rbtree_int_new(10), EINA_RBTREE_CMP_NODE_CB(eina_rbtree_int_cmp));
   root = eina_rbtree_inline_insert(root, (Eina_Rbtree*) _eina_rbtree_int_new(42), EINA_RBTREE_CMP_NODE_CB(eina_rbtree_int_cmp));
   root = eina_rbtree_inline_insert(root, (Eina_Rbtree*) _eina_rbtree_int_new(69), EINA_RBTREE_CMP_NODE_CB(eina_rbtree_int_cmp));
   root = eina_rbtree_inline_insert(root, (Eina_Rbtree*) _eina_rbtree_int_new(1337), EINA_RBTREE_CMP_NODE_CB(eina_rbtree_int_cmp));
   root = eina_rbtree_inline_insert(root, (Eina_Rbtree*) _eina_rbtree_int_new(77), EINA_RBTREE_CMP_NODE_CB(eina_rbtree_int_cmp));
   root = eina_rbtree_inline_insert(root, (Eina_Rbtree*) _eina_rbtree_int_new(75), EINA_RBTREE_CMP_NODE_CB(eina_rbtree_int_cmp));
   root = eina_rbtree_inline_insert(root, (Eina_Rbtree*) _eina_rbtree_int_new(81), EINA_RBTREE_CMP_NODE_CB(eina_rbtree_int_cmp));
   _eina_rbtree_black_height(root, EINA_RBTREE_CMP_NODE_CB(eina_rbtree_int_cmp));

   fail_if(root == NULL);

   fprintf(stderr, "******\n");
   eina_rbtree_display_int((Eina_Rbtree_Int*) root, 0);
   fprintf(stderr, "******\n");

   i = 69;
   lookup = eina_rbtree_inline_lookup(root, &i, sizeof (int), EINA_RBTREE_CMP_KEY_CB(eina_rbtree_int_key));
   _eina_rbtree_black_height(root, EINA_RBTREE_CMP_NODE_CB(eina_rbtree_int_cmp));
   fail_if(lookup == NULL);

   root = eina_rbtree_inline_remove(root, lookup, EINA_RBTREE_CMP_NODE_CB(eina_rbtree_int_cmp));
   _eina_rbtree_black_height(root, EINA_RBTREE_CMP_NODE_CB(eina_rbtree_int_cmp));
}
END_TEST

void
eina_test_rbtree(TCase *tc)
{
   tcase_add_test(tc, eina_rbtree_insertion);
   tcase_add_test(tc, eina_rbtree_lookup);
   tcase_add_test(tc, eina_rbtree_remove);
   tcase_add_test(tc, eina_rbtree_simple_remove);
   tcase_add_test(tc, eina_rbtree_simple_remove2);
}

