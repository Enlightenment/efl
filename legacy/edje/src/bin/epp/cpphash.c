/* Part of CPP library.  (Macro hash table support.)
 * Copyright (C) 1986, 87, 89, 92, 93, 94, 1995 Free Software Foundation, Inc.
 * Written by Per Bothner, 1994.
 * Based on CCCP program by by Paul Rubin, June 1986
 * Adapted to ANSI C, Richard Stallman, Jan 1987
 * Copyright (C) 2003-2011 Kim Woelders
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any
 * later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 * 
 * In other words, you are welcome to use, share and improve this program.
 * You are forbidden to forbid anyone else to use, share and improve
 * what you give them.   Help stamp out software-hoarding!  */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <string.h>
#include <stdlib.h>

#include "cpplib.h"
#include "cpphash.h"

static HASHNODE    *hashtab[HASHSIZE];

#define IS_IDCHAR(ch) is_idchar[(unsigned char)(ch)]

/*
 * return hash function on name.  must be compatible with the one
 * computed a step at a time, elsewhere
 */
int
hashf(const char *name, int len, int hashsize)
{
   int                 r = 0;

   while (len--)
      r = HASHSTEP(r, *name++);

   return MAKE_POS(r) % hashsize;
}

/*
 * find the most recent hash node for name name (ending with first
 * non-identifier char) installed by install
 *
 * If LEN is >= 0, it is the length of the name.
 * Otherwise, compute the length by scanning the entire name.
 *
 * If HASH is >= 0, it is the precomputed hash code.
 * Otherwise, compute the hash code.
 */
HASHNODE           *
cpp_lookup(const char *name, int len, int hash)
{
   const char         *bp;
   HASHNODE           *bucket;

   if (len < 0)
     {
	for (bp = name; IS_IDCHAR(*bp); bp++)
	   ;
	len = bp - name;
     }
   if (hash < 0)
      hash = hashf(name, len, HASHSIZE);

   bucket = hashtab[hash];
   while (bucket)
     {
	if (bucket->length == len
	    && strncmp((const char *)bucket->name, name, len) == 0)
	   return bucket;
	bucket = bucket->next;
     }
   return (HASHNODE *) 0;
}

/*
 * Delete a hash node.  Some weirdness to free junk from macros.
 * More such weirdness will have to be added if you define more hash
 * types that need it.
 */

/* Note that the DEFINITION of a macro is removed from the hash table
 * but its storage is not freed.  This would be a storage leak
 * except that it is not reasonable to keep undefining and redefining
 * large numbers of macros many times.
 * In any case, this is necessary, because a macro can be #undef'd
 * in the middle of reading the arguments to a call to it.
 * If #undef freed the DEFINITION, that would crash.  */

void
delete_macro(HASHNODE * hp)
{

   if (hp->prev)
      hp->prev->next = hp->next;
   if (hp->next)
      hp->next->prev = hp->prev;

   /* make sure that the bucket chain header that
    * the deleted guy was on points to the right thing afterwards. */
   if (hp == *hp->bucket_hdr)
      *hp->bucket_hdr = hp->next;

   if (hp->type == T_MACRO)
     {
	DEFINITION         *d = hp->value.defn;
	struct reflist     *ap, *nextap;

	for (ap = d->pattern; ap; ap = nextap)
	  {
	     nextap = ap->next;
	     free(ap);
	  }
	if (d->nargs >= 0)
	   free(d->args.argnames);
	free(d);
     }
   free(hp);
}
/*
 * install a name in the main hash table, even if it is already there.
 *   name stops with first non alphanumeric, except leading '#'.
 * caller must check against redefinition if that is desired.
 * delete_macro () removes things installed by install () in fifo order.
 * this is important because of the `defined' special symbol used
 * in #if, and also if pushdef/popdef directives are ever implemented.
 *
 * If LEN is >= 0, it is the length of the name.
 * Otherwise, compute the length by scanning the entire name.
 *
 * If HASH is >= 0, it is the precomputed hash code.
 * Otherwise, compute the hash code.
 */
HASHNODE           *
install(const char *name, int len, enum node_type type, int ivalue, char *value,
	int hash)
{
   HASHNODE           *hp;
   int                 i, bucket;
   const char         *p;

   if (len < 0)
     {
	p = name;
	while (IS_IDCHAR(*p))
	   p++;
	len = p - name;
     }
   if (hash < 0)
      hash = hashf(name, len, HASHSIZE);

   i = sizeof(HASHNODE) + len + 1;
   hp = (HASHNODE *) xmalloc(i);
   bucket = hash;
   hp->bucket_hdr = &hashtab[bucket];
   hp->next = hashtab[bucket];
   hashtab[bucket] = hp;
   hp->prev = NULL;
   if (hp->next)
      hp->next->prev = hp;
   hp->type = type;
   hp->length = len;
   if (hp->type == T_CONST)
      hp->value.ival = ivalue;
   else
      hp->value.cpval = value;
   hp->name = ((char *)hp) + sizeof(HASHNODE);
   memcpy(hp->name, name, len);
   hp->name[len] = 0;
   return hp;
}

void
cpp_hash_cleanup(cpp_reader * pfile __UNUSED__)
{
   int                 i;

   for (i = HASHSIZE; --i >= 0;)
     {
	while (hashtab[i])
	   delete_macro(hashtab[i]);
     }
}
