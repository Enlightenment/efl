#ifdef HAVE_CONFIG_H
# include <config.h>
#endif /* ifdef HAVE_CONFIG_H */

#include <string.h>
#include <stdio.h>

#ifdef HAVE_EVIL
# include <Evil.h>
#endif /* ifdef HAVE_EVIL */

#include <Eina.h>

#include "Eet.h"
#include "Eet_private.h"

static Eina_Mempool *_eet_node_mp = NULL;

Eet_Node *
eet_node_new(void)
{
   Eet_Node *result;

   result = eina_mempool_malloc(_eet_node_mp, sizeof (Eet_Node));
   if (!result)
      return NULL;

   memset(result, 0, sizeof (Eet_Node));
   return result;
} /* eet_node_new */

void
eet_node_free(Eet_Node *node)
{
   eina_mempool_free(_eet_node_mp, node);
} /* eet_node_free */

static Eet_Node *
_eet_node_new(const char *name,
              int         type)
{
   Eet_Node *n;

   n = eet_node_new();
   if (!n)
      return NULL;

   n->type = type;
   n->name = eina_stringshare_add(name);

   return n;
} /* _eet_node_new */

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
} /* _eet_node_append */

#define EET_NODE_NEW(Eet_type, Name, Value, Type)\
   EAPI Eet_Node *\
   eet_node_ ## Name ## _new(const char *name, Type Value)\
   {\
      Eet_Node *n;\
\
      n = _eet_node_new(name, Eet_type);\
      if (!n) { return NULL; }\
\
      n->data.value.Value = Value;\
\
      return n;\
   }

#define EET_NODE_STR_NEW(Eet_type, Name, Value, Type)\
   EAPI Eet_Node *\
   eet_node_ ## Name ## _new(const char *name, Type Value)\
   {\
      Eet_Node *n;\
\
      n = _eet_node_new(name, Eet_type);\
      if (!n) { return NULL; }\
\
      n->data.value.Value = eina_stringshare_add(Value);\
\
      return n;\
   }

EET_NODE_NEW(EET_T_CHAR,                     char, c,  char)
EET_NODE_NEW(EET_T_SHORT,                   short, s,  short)
EET_NODE_NEW(EET_T_INT,                       int, i,  int)
EET_NODE_NEW(EET_T_LONG_LONG,           long_long, l,  long long)
EET_NODE_NEW(EET_T_FLOAT,                   float, f,  float)
EET_NODE_NEW(EET_T_DOUBLE,                 double, d,  double)
EET_NODE_NEW(EET_T_UCHAR,           unsigned_char, uc, unsigned char)
EET_NODE_NEW(EET_T_USHORT,         unsigned_short, us, unsigned short)
EET_NODE_NEW(EET_T_UINT,             unsigned_int, ui, unsigned int)
EET_NODE_NEW(EET_T_ULONG_LONG, unsigned_long_long, ul, unsigned long long)
EET_NODE_STR_NEW(EET_T_STRING,                 string, str, const char *)
EET_NODE_STR_NEW(EET_T_INLINED_STRING, inlined_string, str, const char *)

Eet_Node *
eet_node_null_new(const char *name)
{
   Eet_Node *n;

   n = _eet_node_new(name, EET_T_NULL);
   if (!n)
      return NULL;

   n->data.value.str = NULL;

   return n;
}

Eet_Node *
eet_node_list_new(const char *name,
                  Eina_List  *nodes)
{
   Eet_Node *n;

   n = _eet_node_new(name, EET_G_LIST);
   if (!n)
      return NULL;

   _eet_node_append(n, nodes);

   return n;
} /* eet_node_list_new */

Eet_Node *
eet_node_array_new(const char *name,
                   int         count,
                   Eina_List  *nodes)
{
   Eet_Node *n;

   n = _eet_node_new(name, EET_G_ARRAY);
   if (!n)
      return NULL;

   n->count = count;

   _eet_node_append(n, nodes);

   return n;
} /* eet_node_array_new */

Eet_Node *
eet_node_var_array_new(const char *name,
                       Eina_List  *nodes)
{
   Eet_Node *n;

   n = _eet_node_new(name, EET_G_VAR_ARRAY);
   if (!n)
      return NULL;

   n->count = eina_list_count(nodes);

   _eet_node_append(n, nodes);

   return n;
} /* eet_node_var_array_new */

Eet_Node *
eet_node_hash_new(const char *name,
                  const char *key,
                  Eet_Node   *node)
{
   Eina_List *nodes;
   Eet_Node *n;

   if (!node)
      return NULL;

   n = _eet_node_new(name, EET_G_HASH);
   if (!n)
      return NULL;

   n->key = eina_stringshare_add(key);
   nodes = eina_list_append(NULL, node);

   _eet_node_append(n, nodes);

   return n;
} /* eet_node_hash_new */

Eet_Node *
eet_node_struct_new(const char *name,
                    Eina_List  *nodes)
{
   Eet_Node *n;

   n = _eet_node_new(name, EET_G_UNKNOWN);
   if (!n)
      return NULL;

   _eet_node_append(n, nodes);

   return n;
} /* eet_node_struct_new */

Eet_Node *
eet_node_struct_child_new(const char *parent,
                          Eet_Node   *child)
{
   Eet_Node *n;

   if (!child) return NULL;

   if (child->type != EET_G_UNKNOWN)
      return child;

   n = _eet_node_new(parent, EET_G_UNKNOWN);
   if (!n)
      return NULL;

   _eet_node_append(n, eina_list_prepend(NULL, child));

   return n;
} /* eet_node_struct_child_new */

Eet_Node *
eet_node_children_get(Eet_Node *node)
{
   if (!node) return NULL;
   return node->values;
}

Eet_Node *
eet_node_next_get(Eet_Node *node)
{
   if (!node) return NULL;
   return node->next;
}

Eet_Node *
eet_node_parent_get(Eet_Node *node)
{
   if (!node) return NULL;
   return node->parent;
}

void
eet_node_list_append(Eet_Node   *parent,
                     const char *name,
                     Eet_Node   *child)
{
   const char *tmp;
   Eet_Node *nn;

   if ((!parent) || (!child)) return;
   tmp = eina_stringshare_add(name);

   for (nn = parent->values; nn; nn = nn->next)
      if (nn->name == tmp && nn->type == EET_G_LIST)
        {
           Eet_Node *n;

           if (!nn->values)
              nn->values = child;
           else
             {
                for (n = nn->values; n->next; n = n->next)
                   ;
                n->next = child;
             }

           child->next = NULL;

           eina_stringshare_del(tmp);

           return;
        }

   /* No list found, so create it. */
   nn = eet_node_list_new(tmp, eina_list_append(NULL, child));

   /* And add it to the parent. */
   nn->next = parent->values;
   parent->values = nn;

   eina_stringshare_del(tmp);
} /* eet_node_list_append */

void
eet_node_struct_append(Eet_Node   *parent,
                       const char *name,
                       Eet_Node   *child)
{
   const char *tmp;
   Eet_Node *prev;
   Eet_Node *nn;

   if ((!parent) || (!child)) return;
   if (parent->type != EET_G_UNKNOWN)
     {
        ERR("[%s] is not a structure. Will not insert [%s] in it",
            parent->name,
            name);
        eet_node_del(child);
        return;
     }

   tmp = eina_stringshare_add(name);

   for (prev = NULL, nn = parent->values; nn; prev = nn, nn = nn->next)
      if (nn->name == tmp && nn->type == child->type)
        {
           if (prev)
              prev->next = nn->next;
           else
              parent->values = nn->next;

           nn->next = NULL;
           eet_node_del(nn);

           break;
        }

   if (prev)
     {
        prev->next = child;
        child->next = NULL;
     }
   else
     {
        child->next = NULL;
        parent->values = child;
     }

   eina_stringshare_del(tmp);
} /* eet_node_struct_append */

void
eet_node_hash_add(Eet_Node   *parent,
                  const char *name,
                  const char *key,
                  Eet_Node   *child)
{
   Eet_Node *nn;

   if ((!parent) || (!child)) return;

   /* No list found, so create it. */
   nn = eet_node_hash_new(name, key, child);

   /* And add it to the parent. */
   nn->next = parent->values;
   parent->values = nn;
} /* eet_node_hash_add */


int
eet_node_type_get(Eet_Node *node)
{
   if (!node) return EET_T_UNKNOW;
   return node->type;
}

Eet_Node_Data *
eet_node_value_get(Eet_Node *node)
{
   if (!node) return NULL;
   return &node->data;
}

const char *
eet_node_name_get(Eet_Node *node)
{
   if (!node) return NULL;
   return node->name;
}

void
eet_node_del(Eet_Node *n)
{
   Eet_Node *nn;
   Eet_Node *tmp;

   if (!n)
      return;

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
         eina_stringshare_del(n->data.value.str);
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
     } /* switch */

   eina_stringshare_del(n->name);
   eet_node_free(n);
} /* eet_node_del */

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
eet_node_dump_level(int level,
                    Eet_Dump_Callback dumpfunc,
                    void *dumpdata)
{
   int i;

   for (i = 0; i < level; i++) dumpfunc(dumpdata, "  ");
} /* eet_node_dump_level */

static char *
eet_node_string_escape(const char *str)
{
   char *s, *sp;
   const char *strp;
   int sz = 0;

   for (strp = str; *strp; strp++)
     {
        if (*strp == '\"')
           sz += 2;
        else if (*strp == '\\')
           sz += 2;
        else if (*strp == '\n')
           sz += 2;
        else
           sz += 1;
     }
   s = malloc(sz + 1);
   if (!s)
      return NULL;

   for (strp = str, sp = s; *strp; strp++, sp++)
     {
        if (*strp == '\"'
            || *strp == '\\'
            || *strp == '\n')
          {
             *sp = '\\';
             sp++;
          }

        if (*strp == '\n')
           *sp = 'n';
        else
           *sp = *strp;
     }
   *sp = 0;
   return s;
} /* eet_node_string_escape */

static void
eet_node_dump_string_escape(void *dumpdata,
                            Eet_Dump_Callback dumpfunc,
                            const char *str)
{
   char *s;

   s = eet_node_string_escape(str);
   if (!s)
      return;

   dumpfunc(dumpdata, s);
   free(s);
} /* eet_node_dump_string_escape */

static void
eet_node_dump_simple_type(Eet_Node *n, int level,
                          Eet_Dump_Callback dumpfunc,
                          void *dumpdata)
{
   const char *type_name = NULL;
   char tbuf[256];

   eet_node_dump_level(level, dumpfunc, dumpdata);
   dumpfunc(dumpdata, "value \"");
   eet_node_dump_string_escape(dumpdata, dumpfunc, n->name);
   dumpfunc(dumpdata, "\" ");

#ifdef EET_T_TYPE
# undef EET_T_TYPE
#endif /* ifdef EET_T_TYPE */

#define EET_T_TYPE(Eet_Type, Type)\
 case Eet_Type:\
 {\
    dumpfunc(dumpdata, eet_node_dump_t_name[Eet_Type][0]);\
    snprintf(tbuf,\
             sizeof (tbuf),\
             eet_node_dump_t_name[Eet_Type][1],\
             n->data.value.Type);\
    dumpfunc(dumpdata, tbuf);\
    break;\
 }

   switch (n->type)
     {
        EET_T_TYPE(EET_T_CHAR,        c);
        EET_T_TYPE(EET_T_SHORT,       s);
        EET_T_TYPE(EET_T_INT,         i);
        EET_T_TYPE(EET_T_LONG_LONG,   l);
        EET_T_TYPE(EET_T_FLOAT,       f);
        EET_T_TYPE(EET_T_DOUBLE,      d);
        EET_T_TYPE(EET_T_UCHAR,      uc);
        EET_T_TYPE(EET_T_USHORT,     us);
        EET_T_TYPE(EET_T_UINT,       ui);
        EET_T_TYPE(EET_T_ULONG_LONG, ul);

      case EET_T_INLINED_STRING:
         type_name = "inlined: \"";

      case EET_T_STRING:
         if (!type_name)
            type_name = "string: \"";

         dumpfunc(dumpdata, type_name);
         eet_node_dump_string_escape(dumpdata, dumpfunc, n->data.value.str);
         dumpfunc(dumpdata, "\"");
         break;

      case EET_T_NULL:
         dumpfunc(dumpdata, "null");
         break;

      default:
         dumpfunc(dumpdata, "???: ???");
         break;
     } /* switch */

   dumpfunc(dumpdata, ";\n");
} /* eet_node_dump_simple_type */

static void
eet_node_dump_group_start(int level,
                          Eet_Dump_Callback dumpfunc,
                          void *dumpdata,
                          int group_type,
                          const char *name)
{
   int chnk_type;

   chnk_type = (group_type >= EET_G_UNKNOWN && group_type <= EET_G_HASH) ?
      group_type : EET_G_LAST;

   eet_node_dump_level(level, dumpfunc, dumpdata);
   dumpfunc(dumpdata, "group \"");
   eet_node_dump_string_escape(dumpdata, dumpfunc, name);
   dumpfunc(dumpdata,                "\" ");

   dumpfunc(dumpdata, eet_node_dump_g_name[chnk_type - EET_G_UNKNOWN]);
   dumpfunc(dumpdata,               " {\n");
} /* eet_node_dump_group_start */

static void
eet_node_dump_group_end(int level,
                        Eet_Dump_Callback dumpfunc,
                        void *dumpdata)
{
   eet_node_dump_level(level, dumpfunc, dumpdata);
   dumpfunc(dumpdata, "}\n");
} /* eet_node_dump_group_end */

void
eet_node_dump(Eet_Node *n,
              int dumplevel,
              Eet_Dump_Callback dumpfunc,
              void *dumpdata)
{
   Eet_Node *it;

   if (!n)
      return;

   switch (n->type)
     {
      case EET_G_VAR_ARRAY:
      case EET_G_ARRAY:
      case EET_G_UNKNOWN:
      case EET_G_HASH:
      case EET_G_LIST:
         eet_node_dump_group_start(dumplevel,
                                   dumpfunc,
                                   dumpdata,
                                   n->type,
                                   n->name);

         if (n->type == EET_G_VAR_ARRAY
             || n->type == EET_G_ARRAY)
           {
              char tbuf[256];

              eet_node_dump_level(dumplevel, dumpfunc, dumpdata);
              dumpfunc(dumpdata, "    count ");
              eina_convert_itoa(n->count, tbuf);
              dumpfunc(dumpdata,  tbuf);
              dumpfunc(dumpdata, ";\n");
           }
         else if (n->type == EET_G_HASH)
           {
              eet_node_dump_level(dumplevel, dumpfunc, dumpdata);
              dumpfunc(dumpdata, "    key \"");
              eet_node_dump_string_escape(dumpdata, dumpfunc, n->key);
              dumpfunc(dumpdata, "\";\n");
           }

         for (it = n->values; it; it = it->next)
            eet_node_dump(it, dumplevel + 2, dumpfunc, dumpdata);

         eet_node_dump_group_end(dumplevel, dumpfunc, dumpdata);
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
      case EET_T_ULONG_LONG:
         eet_node_dump_simple_type(n, dumplevel, dumpfunc, dumpdata);
         break;
     } /* switch */
} /* eet_node_dump */

void *
eet_node_walk(void          *parent,
              const char    *name,
              Eet_Node      *root,
              Eet_Node_Walk *cb,
              void          *user_data)
{
   Eet_Node *it;
   void *me = NULL;
   int i;

   if (!root)
     {
        if (parent)
           cb->struct_add(parent, name, NULL, user_data);

        return NULL;
     }

   switch (root->type)
     {
      case EET_G_UNKNOWN:
         me = cb->struct_alloc(root->name, user_data);

         for (it = root->values; it; it = it->next)
            eet_node_walk(me, it->name, it, cb, user_data);

         break;

      case EET_G_VAR_ARRAY:
      case EET_G_ARRAY:
         me = cb->array(root->type == EET_G_VAR_ARRAY ? EINA_TRUE : EINA_FALSE,
                        root->name, root->count, user_data);

         for (i = 0, it = root->values; it; it = it->next)
            cb->insert(me, i++, eet_node_walk(NULL,
                                              NULL,
                                              it,
                                              cb,
                                              user_data), user_data);

         break;

      case EET_G_LIST:
         me = cb->list(root->name, user_data);

         for (it = root->values; it; it = it->next)
            cb->append(me, eet_node_walk(NULL,
                                         NULL,
                                         it,
                                         cb,
                                         user_data), user_data);

         break;

      case EET_G_HASH:
         if (!parent)
            return NULL;

         return cb->hash(parent, root->name, root->key,
                         eet_node_walk(NULL,
                                       NULL,
                                       root->values,
                                       cb,
                                       user_data), user_data);

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
      case EET_T_ULONG_LONG:
         me = cb->simple(root->type, &root->data, user_data);
         break;
     } /* switch */

   if (parent)
      cb->struct_add(parent, name, me, user_data);

   return me;
} /* eet_node_walk */

int
eet_node_init(void)
{
   const char *choice;
   const char *tmp;

#ifdef EINA_DEFAULT_MEMPOOL
   choice = "pass_through";
#else
   choice = "chained_mempool";
#endif
   tmp = getenv("EET_MEMPOOL");
   if (tmp && tmp[0])
      choice = tmp;

   _eet_node_mp =
      eina_mempool_add(choice, "eet-node-alloc", NULL, sizeof(Eet_Node), 1024);

   return _eet_node_mp ? 1 : 0;
} /* eet_node_init */

void
eet_node_shutdown(void)
{
   eina_mempool_del(_eet_node_mp);
   _eet_node_mp = NULL;
} /* eet_node_shutdown */

