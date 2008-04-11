/*  Small compiler  - maintenance of various lists
 *
 *  Name list (aliases)
 *  Include path list
 *
 *  Copyright (c) ITB CompuPhase, 2001-2003
 *
 *  This software is provided "as-is", without any express or implied warranty.
 *  In no event will the authors be held liable for any damages arising from
 *  the use of this software.
 *
 *  Permission is granted to anyone to use this software for any purpose,
 *  including commercial applications, and to alter it and redistribute it
 *  freely, subject to the following restrictions:
 *
 *  1.  The origin of this software must not be misrepresented; you must not
 *      claim that you wrote the original software. If you use this software in
 *      a product, an acknowledgment in the product documentation would be
 *      appreciated but is not required.
 *  2.  Altered source versions must be plainly marked as such, and must not be
 *      misrepresented as being the original software.
 *  3.  This notice may not be removed or altered from any source distribution.
 *
 *  Version: $Id$
 */

/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "embryo_cc_sc.h"

static stringpair  *
insert_stringpair(stringpair * root, char *first, char *second, int matchlength)
{
   stringpair         *cur, *pred;

   assert(root != NULL);
   assert(first != NULL);
   assert(second != NULL);
   /* create a new node, and check whether all is okay */
   if ((cur = (stringpair *) malloc(sizeof(stringpair))) == NULL)
      return NULL;
   cur->first = strdup(first);
   cur->second = strdup(second);
   cur->matchlength = matchlength;
   if (cur->first == NULL || cur->second == NULL)
     {
	if (cur->first != NULL)
	   free(cur->first);
	if (cur->second != NULL)
	   free(cur->second);
	free(cur);
	return NULL;
     }				/* if */
   /* link the node to the tree, find the position */
   for (pred = root; pred->next != NULL && strcmp(pred->next->first, first) < 0;
	pred = pred->next)
      /* nothing */ ;
   cur->next = pred->next;
   pred->next = cur;
   return cur;
}

static void
delete_stringpairtable(stringpair * root)
{
   stringpair         *cur, *next;

   assert(root != NULL);
   cur = root->next;
   while (cur != NULL)
     {
	next = cur->next;
	assert(cur->first != NULL);
	assert(cur->second != NULL);
	free(cur->first);
	free(cur->second);
	free(cur);
	cur = next;
     }				/* while */
   memset(root, 0, sizeof(stringpair));
}

static stringpair  *
find_stringpair(stringpair * cur, char *first, int matchlength)
{
   int                 result = 0;

   assert(matchlength > 0);	/* the function cannot handle zero-length comparison */
   assert(first != NULL);
   while (cur != NULL && result <= 0)
     {
	result = (int)*cur->first - (int)*first;
	if (result == 0 && matchlength == cur->matchlength)
	  {
	     result = strncmp(cur->first, first, matchlength);
	     if (result == 0)
		return cur;
	  }			/* if */
	cur = cur->next;
     }				/* while */
   return NULL;
}

static int
delete_stringpair(stringpair * root, stringpair * item)
{
   stringpair         *cur;

   assert(root != NULL);
   cur = root;
   while (cur->next != NULL)
     {
	if (cur->next == item)
	  {
	     cur->next = item->next;	/* unlink from list */
	     assert(item->first != NULL);
	     assert(item->second != NULL);
	     free(item->first);
	     free(item->second);
	     free(item);
	     return TRUE;
	  }			/* if */
	cur = cur->next;
     }				/* while */
   return FALSE;
}

/* ----- alias table --------------------------------------------- */
static stringpair   alias_tab = { NULL, NULL, NULL, 0 };    /* alias table */

stringpair *
insert_alias(char *name, char *alias)
{
   stringpair         *cur;

   assert(name != NULL);
   assert(strlen(name) <= sNAMEMAX);
   assert(alias != NULL);
   assert(strlen(alias) <= sEXPMAX);
   if ((cur = insert_stringpair(&alias_tab, name, alias, strlen(name))) == NULL)
      error(103);		/* insufficient memory (fatal error) */
   return cur;
}

int
lookup_alias(char *target, char *name)
{
   stringpair         *cur =
      find_stringpair(alias_tab.next, name, strlen(name));
   if (cur != NULL)
     {
	assert(strlen(cur->second) <= sEXPMAX);
	strcpy(target, cur->second);
     }				/* if */
   return cur != NULL;
}

void
delete_aliastable(void)
{
   delete_stringpairtable(&alias_tab);
}

/* ----- include paths list -------------------------------------- */
static stringlist   includepaths = { NULL, NULL };	/* directory list for include files */

stringlist *
insert_path(char *path)
{
   stringlist         *cur;

   assert(path != NULL);
   if ((cur = (stringlist *) malloc(sizeof(stringlist))) == NULL)
      error(103);		/* insufficient memory (fatal error) */
   if ((cur->line = strdup(path)) == NULL)
      error(103);		/* insufficient memory (fatal error) */
   cur->next = includepaths.next;
   includepaths.next = cur;
   return cur;
}

char       *
get_path(int index)
{
   stringlist         *cur = includepaths.next;

   while (cur != NULL && index-- > 0)
      cur = cur->next;
   if (cur != NULL)
     {
	assert(cur->line != NULL);
	return cur->line;
     }				/* if */
   return NULL;
}

void
delete_pathtable(void)
{
   stringlist         *cur = includepaths.next, *next;

   while (cur != NULL)
     {
	next = cur->next;
	assert(cur->line != NULL);
	free(cur->line);
	free(cur);
	cur = next;
     }				/* while */
   memset(&includepaths, 0, sizeof(stringlist));
}

/* ----- text substitution patterns ------------------------------ */

static stringpair   substpair = { NULL, NULL, NULL, 0 };    /* list of substitution pairs */
static stringpair  *substindex['z' - 'A' + 1];	/* quick index to first character */

static void
adjustindex(char c)
{
   stringpair         *cur;

   assert((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == '_');
   assert('A' < '_' && '_' < 'z');

   for (cur = substpair.next; cur != NULL && cur->first[0] != c;
	cur = cur->next)
      /* nothing */ ;
   substindex[(int)c - 'A'] = cur;
}

stringpair *
insert_subst(char *pattern, char *substitution, int prefixlen)
{
   stringpair         *cur;

   assert(pattern != NULL);
   assert(substitution != NULL);
   if ((cur =
	insert_stringpair(&substpair, pattern, substitution,
			  prefixlen)) == NULL)
      error(103);		/* insufficient memory (fatal error) */
   adjustindex(*pattern);
   return cur;
}

stringpair *
find_subst(char *name, int length)
{
   stringpair         *item;

   assert(name != NULL);
   assert(length > 0);
   assert((*name >= 'A' && *name <= 'Z') || (*name >= 'a' && *name <= 'z')
	  || *name == '_');
   item = substindex[(int)*name - 'A'];
   if (item != NULL)
      item = find_stringpair(item, name, length);
   return item;
}

int
delete_subst(char *name, int length)
{
   stringpair         *item;

   assert(name != NULL);
   assert(length > 0);
   assert((*name >= 'A' && *name <= 'Z') || (*name >= 'a' && *name <= 'z')
	  || *name == '_');
   item = substindex[(int)*name - 'A'];
   if (item != NULL)
      item = find_stringpair(item, name, length);
   if (item == NULL)
      return FALSE;
   delete_stringpair(&substpair, item);
   adjustindex(*name);
   return TRUE;
}

void
delete_substtable(void)
{
   int                 i;

   delete_stringpairtable(&substpair);
   for (i = 0; i < sizeof substindex / sizeof substindex[0]; i++)
      substindex[i] = NULL;
}
