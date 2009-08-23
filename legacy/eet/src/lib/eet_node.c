/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

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

EET_NODE_NEW(EET_T_CHAR, char, c, char)
EET_NODE_NEW(EET_T_SHORT, short, s, short)
EET_NODE_NEW(EET_T_INT, int, i, int)
EET_NODE_NEW(EET_T_LONG_LONG, long_long, l, long long)
EET_NODE_NEW(EET_T_FLOAT, float, f, float)
EET_NODE_NEW(EET_T_DOUBLE, double, d, double)
EET_NODE_NEW(EET_T_UCHAR, unsigned_char, uc, unsigned char)
EET_NODE_NEW(EET_T_USHORT, unsigned_short, us, unsigned short)
EET_NODE_NEW(EET_T_UINT, unsigned_int, ui, unsigned int)
EET_NODE_NEW(EET_T_ULONG_LONG, unsigned_long_long, ul, unsigned long long)
EET_NODE_STR_NEW(EET_T_STRING, string, str, const char *)
EET_NODE_STR_NEW(EET_T_INLINED_STRING, inlined_string, str, const char *)

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
eet_node_var_array_new(const char *name, Eina_List *nodes)
{
   Eet_Node *n;

   n = _eet_node_new(name, EET_G_VAR_ARRAY);
   if (!n) return NULL;

   n->count = eina_list_count(nodes);

   _eet_node_append(n, nodes);

   return n;
}

Eet_Node *
eet_node_hash_new(const char *name, const char *key, Eet_Node *node)
{
   Eina_List *nodes;
   Eet_Node *n;

   if (!node) return NULL;

   n = _eet_node_new(name, EET_G_HASH);
   if (!n) return NULL;

   n->key = eina_stringshare_add(key);
   nodes = eina_list_append(NULL, node);

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

Eet_Node *
eet_node_struct_child_new(const char *parent, Eet_Node *child)
{
   Eet_Node *n;

   if (child->type != EET_G_UNKNOWN)
     return child;

   n = _eet_node_new(parent, EET_G_UNKNOWN);
   if (!n) return NULL;

   _eet_node_append(n, eina_list_prepend(NULL, child));

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


static const char *eet_node_dump_g_name[6] = {
  "struct",
  "array",
  "var_array",
  "list",
  "hash",
  "???"
};

static const char *eet_node_dump_t_name[14][2] = {
  { "???: ", "???" },
  { "char: ", "%hhi" },
  { "short: ", "%hi" },
  { "int: ", "%i" },
  { "long_long: ", "%lli" },
  { "float: ", "%1.25f" },
  { "double: ", "%1.25f" },
  { "uchar: ", "%hhu" },
  { "ushort: ", "%i" },
  { "uint: ", "%u" },
  { "ulong_long: ", "%llu" },
  { "null", "" }
};

static void
eet_node_dump_level(int level, void (*dumpfunc) (void *data, const char *str), void *dumpdata)
{
   int i;

   for (i = 0; i < level; i++) dumpfunc(dumpdata, "  ");
}

static char *
eet_node_string_escape(const char *str)
{
   char *s, *sp;
   const char *strp;
   int sz = 0;

   for (strp = str; *strp; strp++)
     {
	if (*strp == '\"') sz += 2;
	else if (*strp == '\\') sz += 2;
	else sz += 1;
     }
   s = malloc(sz + 1);
   if (!s) return NULL;
   for (strp = str, sp = s; *strp; strp++, sp++)
     {
	if (*strp == '\"')
	  {
	     *sp = '\\';
	     sp++;
	  }
	else if (*strp == '\\')
	  {
	     *sp = '\\';
	     sp++;
	  }
	*sp = *strp;
     }
   *sp = 0;
   return s;
}

static void
eet_node_dump_string_escape(void *dumpdata, void dumpfunc(void *data, const char *str), const char *str)
{
   char *s;

   s = eet_node_string_escape(str);
   if (!s) return ;

   dumpfunc(dumpdata, s);
   free(s);
}

static void
eet_node_dump_simple_type(Eet_Node *n, int level,
			  void (*dumpfunc) (void *data, const char *str), void *dumpdata)
{
   const char *type_name = NULL;
   char tbuf[256];

   eet_node_dump_level(level, dumpfunc, dumpdata);
   dumpfunc(dumpdata, "  value \"");
   eet_node_dump_string_escape(dumpdata, dumpfunc, n->name);
   dumpfunc(dumpdata, "\" ");

#define EET_T_TYPE(Eet_Type, Type)					\
   case Eet_Type:							\
     {									\
	dumpfunc(dumpdata, eet_node_dump_t_name[Eet_Type][0]);		\
	snprintf(tbuf, sizeof (tbuf), eet_node_dump_t_name[Eet_Type][1], n->data.Type); \
	dumpfunc(dumpdata, tbuf);					\
	break;								\
     }

   switch (n->type)
     {
	EET_T_TYPE(EET_T_CHAR, c);
	EET_T_TYPE(EET_T_SHORT, s);
	EET_T_TYPE(EET_T_INT, i);
	EET_T_TYPE(EET_T_LONG_LONG, l);
	EET_T_TYPE(EET_T_FLOAT, f);
	EET_T_TYPE(EET_T_DOUBLE, d);
	EET_T_TYPE(EET_T_UCHAR, uc);
	EET_T_TYPE(EET_T_USHORT, us);
	EET_T_TYPE(EET_T_UINT, ui);
	EET_T_TYPE(EET_T_ULONG_LONG, ul);
      case EET_T_INLINED_STRING:
	 type_name = "inlined: \"";
      case EET_T_STRING:
	 if (!type_name) type_name = "string: \"";

	 dumpfunc(dumpdata, type_name);
	 eet_node_dump_string_escape(dumpdata, dumpfunc, n->data.str);
	 dumpfunc(dumpdata, "\"");
	 break;
      case EET_T_NULL:
	 dumpfunc(dumpdata, "null");
	 break;
      default:
	 dumpfunc(dumpdata, "???: ???");
	 break;
     }

   dumpfunc(dumpdata, ";\n");
}

void
eet_node_dump(Eet_Node *n, int dumplevel, void (*dumpfunc) (void *data, const char *str), void *dumpdata)
{
   switch (n->type)
     {
      case EET_G_HASH:
      case EET_G_UNKNOWN:
      case EET_G_VAR_ARRAY:
      case EET_G_ARRAY:
      case EET_G_LIST:
	 break;
      case EET_T_STRING:
      case EET_T_INLINED_STRING:
      case EET_T_CHAR:
      case EET_T_SHORT:
      case EET_T_INT:
      case EET_T_LONG_LONG:
      case EET_T_FLOAT:
      case EET_T_DOUBLE:
      case EET_T_UCHAR:
      case EET_T_USHORT:
      case EET_T_UINT:
	 eet_node_dump_simple_type(n, dumplevel, dumpfunc, dumpdata);
	 break;
     }
}
