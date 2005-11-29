#include "evas_common.h"
#include "evas_private.h"

typedef struct _Evas_Stringshare             Evas_Stringshare;
typedef struct _Evas_Stringshare_El          Evas_Stringshare_El;

struct _Evas_Stringshare
{
   Evas_Object_List *buckets[256];
};

struct _Evas_Stringshare_El
{
   int                  references;
   Evas_Stringshare_El *next, *prev;
};

static inline int _evas_stringshare_hash_gen(const char *str);

static inline int
_evas_stringshare_hash_gen(const char *str)
{
   unsigned int hash_num = 0, i;
   const unsigned char *ptr;

   if (!str) return 0;

   for (i = 0, ptr = (unsigned char *)str; *ptr; ptr++, i++)
     hash_num ^= ((int)(*ptr) | ((int)(*ptr) << 8)) >> (i % 8);

   hash_num &= 0xff;
   return (int)hash_num;
}

static Evas_Stringshare share = 
{
     {
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL
     }
};

const char *
evas_stringshare_add(const char *str)
{
   int hash_num;
   char *el_str;
   Evas_Stringshare_El *el;

   hash_num = _evas_stringshare_hash_gen(str);
   for (el = share.buckets[hash_num]; el; el = el->next)
     {
	el_str = ((char *)el) + sizeof(Evas_Stringshare_El);
	if (!strcmp(el_str, str))
	  {
	     if (el->prev)
	       {
		  el->prev->next = el->next;
		  if (el->next) el->next->prev = el->prev;
		  el->prev = NULL;
		  el->next = share.buckets[hash_num];
		  share.buckets[hash_num] = el;
	       }
	     el->references++;
	     return el_str;
	  }
     }
   if (!(el = malloc(sizeof(Evas_Stringshare_El) + strlen(str) + 1))) return NULL;
   el_str = ((char *)el) + sizeof(Evas_Stringshare_El);
   strcpy(el_str, str);
   el->references = 1;
   el->prev = NULL;
   el->next = share.buckets[hash_num];
   share.buckets[hash_num] = el;
   return el_str;
}

void
evas_stringshare_del(const char *str)
{
   int hash_num;
   char *el_str;
   Evas_Stringshare_El *el;

   hash_num = _evas_stringshare_hash_gen(str);
   for (el = share.buckets[hash_num]; el; el = el->next)
     {
	el_str = ((char *)el) + sizeof(Evas_Stringshare_El);
	if (!strcmp(el_str, str))
	  {
	     el->references--;
	     if (el->references == 0)
	       {
		  if (el->next) el->next->prev = el->prev;
		  if (el->prev) el->prev->next = el->next;
		  else share.buckets[hash_num] = el->next;
		  share.buckets[hash_num] = evas_object_list_remove(share.buckets[hash_num], el);
		  free(el);
	       }
	     else
	       {
		  if (el->prev)
		    {
		       el->prev->next = el->next;
		       if (el->next) el->next->prev = el->prev;
		       el->prev = NULL;
		       el->next = share.buckets[hash_num];
		       share.buckets[hash_num] = el;
		    }
	       }
	     return;
	  }
     }
}
