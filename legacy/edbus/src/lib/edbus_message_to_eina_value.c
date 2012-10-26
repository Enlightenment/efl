#include "edbus_private.h"
#include "edbus_private_types.h"

static void _message_iter_basic_array_to_eina_value(char type, Eina_Value *value, EDBus_Message_Iter *iter);
static void _eina_value_struct_free(Eina_Value *value, Eina_Array *array);

static const Eina_Value_Type *
_dbus_type_to_eina_value_type(char type)
{
   switch (type)
     {
      case 'i':
      case 'h':
         return EINA_VALUE_TYPE_INT;
      case 's':
      case 'o':
      case 'g':
         return EINA_VALUE_TYPE_STRING;
      case 'b':
      case 'y':
         return EINA_VALUE_TYPE_UCHAR;
      case 'n':
         return EINA_VALUE_TYPE_SHORT;
      case 'q':
         return EINA_VALUE_TYPE_USHORT;
      case 'u':
         return EINA_VALUE_TYPE_UINT;
      case 'x':
         return EINA_VALUE_TYPE_INT64;
      case 't':
         return EINA_VALUE_TYPE_UINT64;
      case 'd':
         return EINA_VALUE_TYPE_DOUBLE;
      case 'a':
         return EINA_VALUE_TYPE_ARRAY;
      case '(':
      case '{':
      case 'e':
      case 'r':
      case 'v':
         return EINA_VALUE_TYPE_STRUCT;
      default:
         ERR("Unknown type %c", type);
         return NULL;
     }
}

static unsigned int
_type_size(char type)
{
   switch (type)
     {
      case 'i':
      case 'h':
      case 'u':
         return(sizeof(int32_t));
      case 's':
      case 'o':
      case 'g':
         return(sizeof(char *));
      case 'b':
      case 'y':
         return(sizeof(unsigned char));
      case 'n':
      case 'q':
         return(sizeof(int16_t));
      case 'x':
      case 't':
         return(sizeof(int64_t));
      case 'd':
         return(sizeof(double));
      case 'a':
         return(sizeof(Eina_Value_Array));
      case '(':
      case '{':
      case 'e':
      case 'r':
      case 'v':
         return(sizeof(Eina_Value_Struct));
      default:
         ERR("Unknown type %c", type);
         return 0;
     }
}

static unsigned int
_type_offset(char type, unsigned base)
{
   unsigned size, padding;
   size = _type_size(type);
   if (!(base % size))
     return base;
   padding = abs(base - size);
   return base + padding;
}

static Eina_Value *
_message_iter_array_to_eina_value(EDBus_Message_Iter *iter)
{
   Eina_Value *array_value;
   char *sig;

   sig = edbus_message_iter_signature_get(iter);
   DBG("array of %s", sig);
   array_value = eina_value_array_new(_dbus_type_to_eina_value_type(sig[0]), 0);
   if (sig[0] == '(' || sig[0] == '{' || sig[0] == 'v')
     {
        EDBus_Message_Iter *entry;

        if (sig[0] == '{')
          sig[0] = 'e';
        else if (sig[0] == '(')
          sig[0] = 'r';

        while (edbus_message_iter_get_and_next(iter, sig[0], &entry))
          {
             Eina_Value *data = _message_iter_struct_to_eina_value(entry);
             Eina_Value_Struct st;
             eina_value_get(data, &st);
             eina_value_array_append(array_value, st);
          }
     }
   else if (sig[0] == 'a')
     {
        EDBus_Message_Iter *entry;
        while (edbus_message_iter_get_and_next(iter, sig[0], &entry))
          {
             Eina_Value *data = _message_iter_array_to_eina_value(entry);
             Eina_Value_Array inner_array;
             eina_value_get(data, &inner_array);
             eina_value_array_append(array_value, inner_array);
          }
     }
   else
     _message_iter_basic_array_to_eina_value(sig[0], array_value, iter);

   DBG("return array of %s", sig);
   free(sig);
   return array_value;
}

static void
_message_iter_basic_array_to_eina_value(char type, Eina_Value *value, EDBus_Message_Iter *iter)
{
   switch (type)
    {
       case 'i':
       case 'h'://fd
         {
            int32_t i;
            while (edbus_message_iter_get_and_next(iter, type, &i))
              eina_value_array_append(value, i);
            break;
         }
       case 's':
       case 'o'://object path
       case 'g'://signature
         {
            const char *txt;
            while (edbus_message_iter_get_and_next(iter, type, &txt))
              eina_value_array_append(value, txt);
            break;
         }
       case 'b'://boolean
       case 'y'://byte
         {
            unsigned char byte;
            while (edbus_message_iter_get_and_next(iter, type, &byte))
              eina_value_array_append(value, byte);
            break;
         }
       case 'n'://int16
         {
            int16_t i;
            while (edbus_message_iter_get_and_next(iter, type, &i))
              eina_value_array_append(value, i);
            break;
         }
       case 'q'://uint16
         {
            uint16_t i;
            while (edbus_message_iter_get_and_next(iter, type, &i))
              eina_value_array_append(value, i);
            break;
         }
       case 'u'://uint32
         {
            uint32_t i;
            while (edbus_message_iter_get_and_next(iter, type, &i))
              eina_value_array_append(value, i);
            break;
         }
       case 'x'://int64
         {
            int64_t i;
            while (edbus_message_iter_get_and_next(iter, type, &i))
              eina_value_array_append(value, i);
            break;
         }
       case 't'://uint64
         {
            uint64_t i;
            while (edbus_message_iter_get_and_next(iter, type, &i))
              eina_value_array_append(value, i);
            break;
         }
       case 'd'://double
         {
            double d;
            while (edbus_message_iter_get_and_next(iter, type, &d))
              eina_value_array_append(value, d);
            break;
         }
    }
}

#define ARG "arg%d"

Eina_Value *
_message_iter_struct_to_eina_value(EDBus_Message_Iter *iter)
{
   int type;
   Eina_Value *value_st;
   Eina_Array *st_members = eina_array_new(1);
   unsigned int offset = 0, z;
   static char name[7];//arg000 + \0
   Eina_Value_Struct_Member *members;
   Eina_Value_Struct_Desc *st_desc;
   Eina_Array *st_values = eina_array_new(1);

   DBG("begin struct");
   st_desc = calloc(1, sizeof(Eina_Value_Struct_Desc));
   st_desc->version = EINA_VALUE_STRUCT_DESC_VERSION;
   st_desc->ops = NULL;

   //create member list
   z = 0;
   while ((type = dbus_message_iter_get_arg_type(&iter->dbus_iterator)) != DBUS_TYPE_INVALID)
     {
        Eina_Value_Struct_Member *m;
        Eina_Value *v;

        m = calloc(1, sizeof(Eina_Value_Struct_Member));
        sprintf(name, ARG, z);
        m->name = strdup(name);
        offset = _type_offset(type, offset);
        m->offset = offset;
        offset += _type_size(type);
        m->type = _dbus_type_to_eina_value_type(type);
        eina_array_push(st_members, m);

        DBG("type = %c", type);
        switch (type)
          {
           case 'i'://int
           case 'h'://fd
             {
                int32_t i;
                v = eina_value_new(EINA_VALUE_TYPE_INT);
                edbus_message_iter_basic_get(iter, &i);
                eina_value_set(v, i);
                break;
             }
           case 's':
           case 'o'://object path
           case 'g'://signature
             {
                const char *txt;
                v = eina_value_new(EINA_VALUE_TYPE_STRING);
                edbus_message_iter_basic_get(iter, &txt);
                eina_value_set(v, txt);
                break;
             }
           case 'b'://boolean
           case 'y'://byte
             {
                unsigned char byte;
                v = eina_value_new(EINA_VALUE_TYPE_UCHAR);
                edbus_message_iter_basic_get(iter, &byte);
                eina_value_set(v, byte);
                break;
             }
           case 'n'://int16
             {
                int16_t i;
                v = eina_value_new(EINA_VALUE_TYPE_SHORT);
                edbus_message_iter_basic_get(iter, &i);
                eina_value_set(v, i);
                break;
             }
           case 'q'://uint16
             {
                uint16_t i;
                v = eina_value_new(EINA_VALUE_TYPE_USHORT);
                edbus_message_iter_basic_get(iter, &i);
                eina_value_set(v, i);
                break;
             }
           case 'u'://uint32
             {
                uint32_t i;
                v = eina_value_new(EINA_VALUE_TYPE_UINT);
                edbus_message_iter_basic_get(iter, &i);
                eina_value_set(v, i);
                break;
             }
           case 'x'://int64
             {
                int64_t i;
                v = eina_value_new(EINA_VALUE_TYPE_INT64);
                edbus_message_iter_basic_get(iter, &i);
                eina_value_set(v, i);
                break;
             }
           case 't'://uint64
             {
                uint64_t i;
                v = eina_value_new(EINA_VALUE_TYPE_UINT64);
                edbus_message_iter_basic_get(iter, &i);
                eina_value_set(v, i);
                break;
             }
           case 'd'://double
             {
                double d;
                v = eina_value_new(EINA_VALUE_TYPE_DOUBLE);
                edbus_message_iter_basic_get(iter, &d);
                eina_value_set(v, d);
                break;
             }
           case 'a'://array
             {
                EDBus_Message_Iter *dbus_array;
                dbus_array = edbus_message_iter_sub_iter_get(iter);
                v = _message_iter_array_to_eina_value(dbus_array);
                break;
             }
           case '('://struct
           case 'r'://struct
           case 'v'://variant
             {
                EDBus_Message_Iter *dbus_st;
                dbus_st = edbus_message_iter_sub_iter_get(iter);
                v = _message_iter_struct_to_eina_value(dbus_st);
                break;
             }
           default:
             ERR("Unexpected type %c", type);
             v = NULL;
          }
        eina_array_push(st_values, v);
        edbus_message_iter_next(iter);
        z++;
     }

   members = malloc(eina_array_count(st_members) * sizeof(Eina_Value_Struct_Member));
   for (z = 0; z < eina_array_count(st_members); z++)
     {
        Eina_Value_Struct_Member *m = eina_array_data_get(st_members, z);
        members[z].name = m->name;
        members[z].offset = m->offset;
        members[z].type = m->type;
        free(m);
     }

   //setup
   st_desc->members = members;
   st_desc->member_count = eina_array_count(st_members);
   st_desc->size = offset;
   value_st = eina_value_struct_new(st_desc);
   eina_array_free(st_members);

   //filling with data
   for (z = 0; z < eina_array_count(st_values); z++)
     {
        Eina_Value *v = eina_array_data_get(st_values, z);
        sprintf(name, ARG, z);
        eina_value_struct_value_set(value_st, name, v);
        eina_value_free(v);
     }
   eina_array_free(st_values);
   DBG("end struct");
   return value_st;
}

EAPI Eina_Value *
edbus_message_to_eina_value(const EDBus_Message *msg)
{
   EDBus_Message_Iter *iter;
   EINA_SAFETY_ON_FALSE_RETURN_VAL(msg, NULL);
   iter = edbus_message_iter_get(msg);
   EINA_SAFETY_ON_NULL_RETURN_VAL(iter, NULL);
   return _message_iter_struct_to_eina_value(iter);
}

static void
_eina_value_array_free(Eina_Value *value, Eina_Array *array)
{
   Eina_Value_Array value_array;
   unsigned i;

   eina_value_pget(value, &value_array);
   if (value_array.subtype == EINA_VALUE_TYPE_STRUCT)
     {
        for (i = 0; i < eina_value_array_count(value); i++)
          {
             Eina_Value st;
             eina_value_array_value_get(value, i, &st);
             _eina_value_struct_free(&st, array);
             eina_value_flush(&st);
          }
     }
   else if (value_array.subtype == EINA_VALUE_TYPE_ARRAY)
     {
        for (i = 0; i < eina_value_array_count(value); i++)
          {
             Eina_Value inner_array;
             eina_value_array_value_get(value, i, &inner_array);
             _eina_value_array_free(&inner_array, array);
             eina_value_flush(&inner_array);
          }
     }
}
static void
_eina_value_struct_free(Eina_Value *value, Eina_Array *array)
{
   Eina_Value_Struct st;
   unsigned i;
   static char name[7];

   DBG("value %p", value);
   EINA_SAFETY_ON_FALSE_RETURN(eina_value_pget(value, &st));

   for (i = 0; i < st.desc->member_count; i++)
     {
        DBG("arg%d of %p", i, value);
        if (st.desc->members[i].type == EINA_VALUE_TYPE_STRUCT)
          {
             Eina_Value sub;
             sprintf(name, ARG, i);
             eina_value_struct_value_get(value, name, &sub);
             _eina_value_struct_free(&sub, array);
             eina_value_flush(&sub);
          }
        else if (st.desc->members[i].type == EINA_VALUE_TYPE_ARRAY)
          {
             Eina_Value sub;
             sprintf(name, ARG, i);
             eina_value_struct_value_get(value, name, &sub);
             _eina_value_array_free(&sub, array);
             eina_value_flush(&sub);
          }
     }
   eina_array_push(array, st.desc);
   DBG("end value %p", value);
}

EAPI void
edbus_message_to_eina_value_free(Eina_Value *value)
{
   Eina_Array *descriptions;
   Eina_Value_Struct_Desc *st_desc;

   EINA_SAFETY_ON_NULL_RETURN(value);
   EINA_SAFETY_ON_FALSE_RETURN(eina_value_type_get(value) == EINA_VALUE_TYPE_STRUCT);

   descriptions = eina_array_new(1);
   _eina_value_struct_free(value, descriptions);
   eina_value_free(value);

   while ((st_desc = eina_array_pop(descriptions)))
     {
        unsigned i;
        for (i = 0; i < st_desc->member_count; i++)
          {
             char *name = (char *)st_desc->members[i].name;
             free(name);
          }
        free((Eina_Value_Struct_Member *)st_desc->members);
        free(st_desc);
     }
   eina_array_free(descriptions);
}
