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
   Evas_Object_List  _list_data;
   char             *str;
   int               references;
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
   Evas_Stringshare_El *el;
   Evas_Object_List *l;

   hash_num = _evas_stringshare_hash_gen(str);
   for (l = share.buckets[hash_num]; l; l = l->next)
     {
	el = (Evas_Stringshare_El *)l;
	if (!strcmp(el->str, str))
	  {
	     if (l != share.buckets[hash_num])
	       {
		  share.buckets[hash_num] = evas_object_list_remove(share.buckets[hash_num], el);
		  share.buckets[hash_num] = evas_object_list_prepend(share.buckets[hash_num], el);
	       }
	     el->references++;
	     return el->str;
	  }
     }
   if (!(el = malloc(sizeof(struct _Evas_Stringshare_El) + strlen(str) + 1))) return NULL;
   el->str = ((unsigned char *)el) + sizeof(struct _Evas_Stringshare_El);
   strcpy(el->str, str);
   el->references = 1;
   share.buckets[hash_num] = evas_object_list_prepend(share.buckets[hash_num], el);
   return el->str;
}

void
evas_stringshare_del(const char *str)
{
   int hash_num;
   Evas_Stringshare_El *el;
   Evas_Object_List *l;

   hash_num = _evas_stringshare_hash_gen(str);
   for (l = share.buckets[hash_num]; l; l = l->next)
     {
	el = (Evas_Stringshare_El *)l;
	if (!strcmp(el->str, str))
	  {
	     el->references--;
	     if (el->references == 0)
	       {
		  share.buckets[hash_num] = evas_object_list_remove(share.buckets[hash_num], el);
		  free(el);
	       }
	     else
	       {
		  share.buckets[hash_num] = evas_object_list_remove(share.buckets[hash_num], el);
		  share.buckets[hash_num] = evas_object_list_prepend(share.buckets[hash_num], el);
	       }
	     return;
	  }
     }
}
