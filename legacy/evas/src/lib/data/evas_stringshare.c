#include "evas_common.h"
#include "evas_private.h"

typedef struct _Evas_Stringshare             Evas_Stringshare;
typedef struct _Evas_Stringshare_El          Evas_Stringshare_El;

struct _Evas_Stringshare
{
   Evas_Stringshare_El *buckets[256];
};

struct _Evas_Stringshare_El
{
   Evas_Stringshare_El *next;
   int                  references;
};

static inline int
_evas_stringshare_hash_gen(const char *str, int *len)
{
   unsigned int hash_num = 5381, i;
   const unsigned char *ptr;

   for (i = 0, ptr = (unsigned char *)str; *ptr; ptr++, i++)
     hash_num = (hash_num * 33) ^ *ptr;

   hash_num &= 0xff;
   *len = i;
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

EAPI const char *
evas_stringshare_add(const char *str)
{
   int hash_num, slen;
   char *el_str;
   Evas_Stringshare_El *el, *pel = NULL;

   hash_num = _evas_stringshare_hash_gen(str, &slen);
   for (el = share.buckets[hash_num]; el; pel = el, el = el->next)
     {
	el_str = ((char *)el) + sizeof(Evas_Stringshare_El);
	if (!strcmp(el_str, str))
	  {
	     if (pel)
	       {
		  pel->next = el->next;
		  el->next = share.buckets[hash_num];
		  share.buckets[hash_num] = el;
	       }
	     el->references++;
//	     printf("SSHARE + \"%s\", ref %i\n", el_str, el->references);
	     return el_str;
	  }
     }
   if (!(el = malloc(sizeof(Evas_Stringshare_El) + slen + 1))) return NULL;
   el_str = ((char *)el) + sizeof(Evas_Stringshare_El);
   strcpy(el_str, str);
   el->references = 1;
   el->next = share.buckets[hash_num];
   share.buckets[hash_num] = el;
   return el_str;
}

EAPI void
evas_stringshare_del(const char *str)
{
   int hash_num, slen;
   char *el_str;
   Evas_Stringshare_El *el, *pel = NULL;

   hash_num = _evas_stringshare_hash_gen(str, &slen);
   for (el = share.buckets[hash_num]; el; pel = el, el = el->next)
     {
	el_str = ((char *)el) + sizeof(Evas_Stringshare_El);
	if (el_str == str)
	  {
	     el->references--;
	     if (el->references == 0)
	       {
		  if (pel) pel->next = el->next;
		  else share.buckets[hash_num] = el->next;
		  free(el);
	       }
	     else
	       {
		  if (pel)
		    {
		       pel->next = el->next;
		       el->next = share.buckets[hash_num];
		       share.buckets[hash_num] = el;
		    }
	       }
	     return;
	  }
     }
   printf("EEEK trying to del non-shared stringshare \"%s\"\n", str);
}
