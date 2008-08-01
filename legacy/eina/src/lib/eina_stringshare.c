/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "eina_stringshare.h"
#include "eina_error.h"

typedef struct _Eina_Stringshare             Eina_Stringshare;
typedef struct _Eina_Stringshare_Node        Eina_Stringshare_Node;

struct _Eina_Stringshare
{
   Eina_Stringshare_Node *buckets[1024];
};

struct _Eina_Stringshare_Node
{
   Eina_Stringshare_Node *next;
   int                  references;
};

static Eina_Stringshare *share = NULL;
static int eina_stringshare_init_count = 0;

static inline int
_eina_stringshare_hash_gen(const char *str, int *len)
{
   unsigned int hash_num = 5381;
   const unsigned char *ptr;

   for (ptr = (const unsigned char *)str; *ptr; ptr++)
     {
	hash_num = (hash_num * 33) ^ *ptr;
     }

   hash_num &= 0x3ff;
   *len = ptr - (const unsigned char *)str;
   return (int)hash_num;
}

/**
 * @defgroup Eina_Stringshare_Group String Instance Functions
 *
 * These functions allow you to store one copy of a string, and use it
 * throughout your program.
 *
 * This is a method to reduce the number of duplicated strings kept in
 * memory. It's pretty common for the same strings to be dynamically
 * allocated repeatedly between applications and libraries, especially in
 * circumstances where you could have multiple copies of a structure that
 * allocates the string. So rather than duplicating and freeing these
 * strings, you request a read-only pointer to an existing string and
 * only incur the overhead of a hash lookup.
 *
 * It sounds like micro-optimizing, but profiling has shown this can have
 * a significant impact as you scale the number of copies up. It improves
 * string creation/destruction speed, reduces memory use and decreases
 * memory fragmentation, so a win all-around.
 */

/**
 * Initialize the eina stringshare internal structure.
 * @return  Zero on failure, non-zero on successful initialization.
 */
EAPI int
eina_stringshare_init()
{
   /*
    * No strings have been loaded at this point, so create the hash
    * table for storing string info for later.
    */
   if (!eina_stringshare_init_count)
     {
	share = calloc(1, sizeof(Eina_Stringshare));
	if (!share)
	  return 0;
	eina_error_init();
     }
   eina_stringshare_init_count++;

   return 1;
}

/**
 * Retrieves an instance of a string for use in a program.
 * @param   str The string to retrieve an instance of.
 * @return  A pointer to an instance of the string on success.
 *          @c NULL on failure.
 * @ingroup Eina_Stringshare_Group
 */
EAPI const char *
eina_stringshare_add(const char *str)
{
   int hash_num, slen;
   char *el_str;
   Eina_Stringshare_Node *el, *pel = NULL;

   if (!str) return NULL;
   hash_num = _eina_stringshare_hash_gen(str, &slen);
   for (el = share->buckets[hash_num]; el; pel = el, el = el->next)
     {
	el_str = ((char *)el) + sizeof(Eina_Stringshare_Node);
	if (!strcmp(el_str, str))
	  {
	     if (pel)
	       {
		  pel->next = el->next;
		  el->next = share->buckets[hash_num];
		  share->buckets[hash_num] = el;
	       }
	     el->references++;
	     return el_str;
	  }
     }
   if (!(el = malloc(sizeof(Eina_Stringshare_Node) + slen + 1))) return NULL;
   el_str = ((char *)el) + sizeof(Eina_Stringshare_Node);
   strcpy(el_str, str);
   el->references = 1;
   el->next = share->buckets[hash_num];
   share->buckets[hash_num] = el;
   return el_str;
}

/**
 * Notes that the given string has lost an instance.
 *
 * It will free the string if no other instances are left.
 *
 * @param   string The given string.
 * @ingroup Eina_Stringshare_Group
 */
EAPI void
eina_stringshare_del(const char *str)
{
   int hash_num, slen;
   char *el_str;
   Eina_Stringshare_Node *el, *pel = NULL;

   if (!str) return;
   hash_num = _eina_stringshare_hash_gen(str, &slen);
   for (el = share->buckets[hash_num]; el; pel = el, el = el->next)
     {
	el_str = ((char *)el) + sizeof(Eina_Stringshare_Node);
	if (el_str == str)
	  {
	     el->references--;
	     if (el->references == 0)
	       {
		  if (pel) pel->next = el->next;
		  else share->buckets[hash_num] = el->next;
		  free(el);
	       }
	     else
	       {
		  if (pel)
		    {
		       pel->next = el->next;
		       el->next = share->buckets[hash_num];
		       share->buckets[hash_num] = el;
		    }
	       }
	     return;
	  }
     }
   EINA_ERROR_PWARN("EEEK trying to del non-shared stringshare \"%s\"\n", str);
   if (getenv("EINA_ERROR_ABORT")) abort();
}

/**
 * Shutdown the eina string internal structures
 */
EAPI void
eina_stringshare_shutdown()
{
   --eina_stringshare_init_count;
   if (!eina_stringshare_init_count)
     {
	int i;
	/* remove any string still in the table */
	for (i = 0; i < 1024; i++)
	  {
	     Eina_Stringshare_Node *el = share->buckets[i];
	     while (el)
	       {
		  Eina_Stringshare_Node *cur = el;
		  el = el->next;
		  cur->next = NULL;
		  free(cur);
	       }
	     share->buckets[i] = NULL;
	  }
	free(share);
	share = NULL;

	eina_error_shutdown();
     }
}

