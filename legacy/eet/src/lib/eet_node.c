#include <stdio.h>
#include <Eina.h>

#include "Eet.h"
#include "Eet_private.h"

static Eet_Node *
_eet_node_new(const char *name, int type)
{
   Eet_Node *n;

   n = calloc(1, sizeof (Eet_Node));
   if (!n) return NULL;

   n->type = type;
   n->name = eina_stringshare_add(name);

   return n;
}

static void
_eet_node_append(Eet_Node *n, Eina_List *nodes)
{
   Eet_Node *value;
   Eina_List *l;

   EINA_LIST_REVERSE_FOREACH(nodes, l, value)
     {
	value->next = n->values;
	n->values = value;
     }
}

#define EET_NODE_NEW(Eet_type, Name, Value, Type)	\
  EAPI Eet_Node *					\
  eet_node_##Name##_new(const char *name, Type Value)	\
  {							\
     Eet_Node *n;					\
  							\
     n = _eet_node_new(name, Eet_type);			\
     if (!n) return NULL;				\
     							\
     n->data.Value = Value;				\
							\
     return n;						\
  }

#define EET_NODE_STR_NEW(Eet_type, Name, Value, Type)	\
  EAPI Eet_Node *					\
  eet_node_##Name##_new(const char *name, Type Value)	\
  {							\
     Eet_Node *n;					\
  							\
     n = _eet_node_new(name, Eet_type);			\
     if (!n) return NULL;				\
     							\
     n->data.Value = eina_stringshare_add(Value);	\
							\
     return n;						\
  }

EET_NODE_NEW(EET_T_CHAR, char, c, char);
EET_NODE_NEW(EET_T_SHORT, short, s, short);
EET_NODE_NEW(EET_T_INT, int, i, int);
EET_NODE_NEW(EET_T_LONG_LONG, long_long, l, long long);
EET_NODE_NEW(EET_T_FLOAT, float, f, float);
EET_NODE_NEW(EET_T_DOUBLE, double, d, double);
EET_NODE_NEW(EET_T_UCHAR, unsigned_char, uc, unsigned char);
EET_NODE_NEW(EET_T_USHORT, unsigned_short, us, unsigned short);
EET_NODE_NEW(EET_T_UINT, unsigned_int, ui, unsigned int);
EET_NODE_STR_NEW(EET_T_STRING, string, str, const char *);
EET_NODE_STR_NEW(EET_T_INLINED_STRING, inlined_string, str, const char *);

Eet_Node *
eet_node_null_new(const char *name)
{
   Eet_Node *n;

   n = _eet_node_new(name, EET_T_NULL);
   if (!n) return NULL;

   n->data.str = NULL;

   return n;
}

Eet_Node *
eet_node_list_new(const char *name, Eina_List *nodes)
{
   Eet_Node *n;

   n = _eet_node_new(name, EET_G_LIST);
   if (!n) return NULL;

   _eet_node_append(n, nodes);

   return n;
}

Eet_Node *
eet_node_array_new(const char *name, int count, Eina_List *nodes)
{
   Eet_Node *n;

   n = _eet_node_new(name, EET_G_ARRAY);
   if (!n) return NULL;

   n->count = count;

   _eet_node_append(n, nodes);

   return n;
}

Eet_Node *
eet_node_var_array_new(const char *name, int count, Eina_List *nodes)
{
   Eet_Node *n;

   n = _eet_node_new(name, EET_G_VAR_ARRAY);
   if (!n) return NULL;

   n->count = count;

   _eet_node_append(n, nodes);

   return n;
}

Eet_Node *
eet_node_hash_new(const char *name, const char *key, Eina_List *nodes)
{
   Eet_Node *n;

   n = _eet_node_new(name, EET_G_HASH);
   if (!n) return NULL;

   n->key = eina_stringshare_add(key);

   _eet_node_append(n, nodes);

   return n;
}

Eet_Node *
eet_node_struct_new(const char *name, Eina_List *nodes)
{
   Eet_Node *n;

   n = _eet_node_new(name, EET_G_UNKNOWN);
   if (!n) return NULL;

   _eet_node_append(n, nodes);

   return n;
}

void
eet_node_del(Eet_Node *n)
{
   Eet_Node *nn;
   Eet_Node *tmp;

   switch (n->type)
     {
      case EET_G_HASH:
	 eina_stringshare_del(n->key);
      case EET_G_UNKNOWN:
      case EET_G_VAR_ARRAY:
      case EET_G_ARRAY:
      case EET_G_LIST:
	 for (nn = n->values; nn; )
	   {
	      tmp = nn;
	      nn = nn->next;
	      eet_node_del(tmp);
	   }
	 break;
      case EET_T_STRING:
      case EET_T_INLINED_STRING:
	 eina_stringshare_del(n->data.str);
	 break;
      case EET_T_CHAR:
      case EET_T_SHORT:
      case EET_T_INT:
      case EET_T_LONG_LONG:
      case EET_T_FLOAT:
      case EET_T_DOUBLE:
      case EET_T_UCHAR:
      case EET_T_USHORT:
      case EET_T_UINT:
	 break;
     }

   eina_stringshare_del(n->name);
   free(n);
}
