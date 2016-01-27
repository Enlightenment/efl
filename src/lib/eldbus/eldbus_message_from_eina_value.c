#include "eldbus_private.h"
#include "eldbus_private_types.h"

#include <dbus/dbus-protocol.h>

static Eina_Bool
_compatible_type(int dbus_type, const Eina_Value_Type *value_type)
{
   switch (dbus_type)
     {
      case 'i':
      case 'h':
         return value_type == EINA_VALUE_TYPE_INT;
      case 's':
      case 'o':
      case 'g':
         return value_type == EINA_VALUE_TYPE_STRING;
      case 'b':
      case 'y':
         return value_type == EINA_VALUE_TYPE_UCHAR;
      case 'n':
         return value_type == EINA_VALUE_TYPE_SHORT;
      case 'q':
         return value_type == EINA_VALUE_TYPE_USHORT;
      case 'u':
         return value_type == EINA_VALUE_TYPE_UINT;
      case 'x':
         return value_type == EINA_VALUE_TYPE_INT64;
      case 't':
         return value_type == EINA_VALUE_TYPE_UINT64;
      case 'd':
         return value_type == EINA_VALUE_TYPE_DOUBLE;
      case 'a':
         return value_type == EINA_VALUE_TYPE_ARRAY;
      case '(':
      case '{':
      case 'e':
      case 'r':
         return value_type == EINA_VALUE_TYPE_STRUCT;
      default:
         ERR("Unknown type %c", dbus_type);
         return EINA_FALSE;
     }
}

static Eina_Bool
_array_append(const char *type, const Eina_Value *value_array, Eldbus_Message_Iter *iter)
{
   Eldbus_Message_Iter *array;
   Eina_Bool ok = eldbus_message_iter_arguments_append(iter, type, &array);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(ok, EINA_FALSE);
   DBG("array of type %c", type[1]);
   switch (type[1])
     {
      case '{':
      case '(':
        {
           unsigned i = strlen(type+2);//remove 'a()' of len a(sv)
           char *entry_sig = malloc(sizeof(char) * i);
           memcpy(entry_sig, type+2, i);
           entry_sig[i-1] = 0;

           for (i = 0; i < eina_value_array_count(value_array); i++)
             {
                Eina_Value st;
                Eldbus_Message_Iter *entry;
                eina_value_array_value_get(value_array, i, &st);
                eldbus_message_iter_arguments_append(array, type+1, &entry);
                _message_iter_from_eina_value_struct(entry_sig, entry, &st);
                eldbus_message_iter_container_close(array, entry);
                eina_value_flush(&st);
             }
           free(entry_sig);
           break;
        }
      case 'a':
        {
           unsigned i;
           for (i = 0; i < eina_value_array_count(value_array); i++)
             {
                Eina_Value inner_array;
                Eldbus_Message_Iter *sub_array;
                eina_value_array_value_get(value_array, i, &inner_array);
                eldbus_message_iter_arguments_append(array, type+1, &sub_array);
                _array_append(type+1, &inner_array, sub_array);
                eldbus_message_iter_container_close(array, sub_array);
                eina_value_flush(&inner_array);
             }
           break;
        }
      case 'v':
        {
           ERR("Variant not supported.");
           return EINA_FALSE;
        }
      case 'i':
      case 'h'://fd
        {
           int32_t z;
           unsigned i;
           for (i = 0; i < eina_value_array_count(value_array); i++)
             {
                eina_value_array_get(value_array, i, &z);
                eldbus_message_iter_basic_append(array, type[1], z);
             }
           break;
        }
      case 's':
      case 'o'://object path
      case 'g'://signature
        {
           const char *txt;
           unsigned i;
           for (i = 0; i < eina_value_array_count(value_array); i++)
             {
                eina_value_array_get(value_array, i, &txt);
                eldbus_message_iter_basic_append(array, type[1], txt);
             }
           break;
        }
      case 'y'://byte
        {
           unsigned char z;
           unsigned i;
           for (i = 0; i < eina_value_array_count(value_array); i++)
             {
                eina_value_array_get(value_array, i, &z);
                eldbus_message_iter_basic_append(array, type[1], z);
             }
           break;
        }
      case 'b'://boolean
        {
           unsigned char z;
           unsigned i;
           for (i = 0; i < eina_value_array_count(value_array); i++)
             {
                eina_value_array_get(value_array, i, &z);
                eldbus_message_iter_basic_append(array, type[1], (uint32_t)z);
             }
           break;
        }
      case 'n'://int16
        {
           int16_t z;
           unsigned i;
           for (i = 0; i < eina_value_array_count(value_array); i++)
             {
                eina_value_array_get(value_array, i, &z);
                eldbus_message_iter_basic_append(array, type[1], z);
             }
           break;
        }
      case 'q'://uint16
        {
           uint16_t z;
           unsigned i;
           for (i = 0; i < eina_value_array_count(value_array); i++)
             {
                eina_value_array_get(value_array, i, &z);
                eldbus_message_iter_basic_append(array, type[1], z);
             }
           break;
        }
      case 'u'://uint32
        {
           uint32_t z;
           unsigned i;
           for (i = 0; i < eina_value_array_count(value_array); i++)
             {
                eina_value_array_get(value_array, i, &z);
                eldbus_message_iter_basic_append(array, type[1], z);
             }
           break;
        }
      case 'x'://int64
        {
           int64_t z;
           unsigned i;
           for (i = 0; i < eina_value_array_count(value_array); i++)
             {
                eina_value_array_get(value_array, i, &z);
                eldbus_message_iter_basic_append(array, type[1], z);
             }
           break;
        }
      case 't'://uint64
        {
           uint64_t z;
           unsigned i;
           for (i = 0; i < eina_value_array_count(value_array); i++)
             {
                eina_value_array_get(value_array, i, &z);
                eldbus_message_iter_basic_append(array, type[1], z);
             }
           break;
        }
      case 'd'://double
        {
           double z;
           unsigned i;
           for (i = 0; i < eina_value_array_count(value_array); i++)
             {
                eina_value_array_get(value_array, i, &z);
                eldbus_message_iter_basic_append(array, type[1], z);
             }
           break;
        }
      default:
        {
           ERR("Unknown type %c", type[1]);
           return EINA_FALSE;
        }
     }
   eldbus_message_iter_container_close(iter, array);
   return EINA_TRUE;
}

static Eina_Bool
_basic_append_value_struct(char type, const Eina_Value *value, const Eina_Value_Struct_Desc *desc, unsigned idx, Eldbus_Message_Iter *iter)
{
   EINA_SAFETY_ON_FALSE_RETURN_VAL(
            _compatible_type(type, desc->members[idx].type), EINA_FALSE);
   switch (type)
     {
      case 'i'://int
      case 'h'://fd
        {
           int32_t i;
           eina_value_struct_get(value, desc->members[idx].name, &i);
           eldbus_message_iter_basic_append(iter, type, i);
           break;
        }
      case 's':
      case 'o'://object path
      case 'g'://signature
        {
           const char *txt;
           eina_value_struct_get(value, desc->members[idx].name, &txt);
           eldbus_message_iter_basic_append(iter, type, txt);
           break;
        }
      case 'y'://byte
        {
           unsigned char byte;
           eina_value_struct_get(value, desc->members[idx].name, &byte);
           eldbus_message_iter_basic_append(iter, type, byte);
           break;
        }
      case 'b'://boolean
        {
           unsigned char boolean;
           eina_value_struct_get(value, desc->members[idx].name, &boolean);
           eldbus_message_iter_basic_append(iter, type, (uint32_t)boolean);
           break;
        }
      case 'n'://int16
        {
           int16_t i;
           eina_value_struct_get(value, desc->members[idx].name, &i);
           eldbus_message_iter_basic_append(iter, type, i);
           break;
        }
      case 'q'://uint16
        {
           uint16_t i;
           eina_value_struct_get(value, desc->members[idx].name, &i);
           eldbus_message_iter_basic_append(iter, type, i);
           break;
        }
      case 'u'://uint32
        {
           uint32_t i;
           eina_value_struct_get(value, desc->members[idx].name, &i);
           eldbus_message_iter_basic_append(iter, type, i);
           break;
        }
      case 'x'://int64
        {
           int64_t i;
           eina_value_struct_get(value, desc->members[idx].name, &i);
           eldbus_message_iter_basic_append(iter, type, i);
           break;
        }
      case 't'://uint64
        {
           uint64_t i;
           eina_value_struct_get(value, desc->members[idx].name, &i);
           eldbus_message_iter_basic_append(iter, type, i);
           break;
        }
      case 'd'://double
        {
           double d;
           eina_value_struct_get(value, desc->members[idx].name, &d);
           eldbus_message_iter_basic_append(iter, type, d);
           break;
        }
      default:
        ERR("Unexpected type %c", type);
        return EINA_FALSE;
     }
   return EINA_TRUE;
}

Eina_Bool
_message_iter_from_eina_value_struct(const char *signature, Eldbus_Message_Iter *iter, const Eina_Value *value)
{
   unsigned i;
   DBusSignatureIter signature_iter;
   Eina_Bool r = EINA_TRUE;
   char *type;
   Eina_Value_Struct st;

   EINA_SAFETY_ON_FALSE_RETURN_VAL(
            eina_value_type_get(value) == EINA_VALUE_TYPE_STRUCT, EINA_FALSE);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(
      eina_value_pget(value, &st), EINA_FALSE);

   dbus_signature_iter_init(&signature_iter, signature);
   i = 0;
   while ((type = dbus_signature_iter_get_signature(&signature_iter)))
     {
        DBG("type: %s", type);
        if (type[0] != 'v' && !type[1])
          r = _basic_append_value_struct(type[0], value, st.desc, i, iter);
        else if (type[0] == 'a')
          {
             Eina_Value value_array;

             EINA_SAFETY_ON_FALSE_RETURN_VAL(
                      _compatible_type(type[0], st.desc->members[i].type),
                      EINA_FALSE);
             eina_value_struct_value_get(value, st.desc->members[i].name,
                                         &value_array);
             r = _array_append(type, &value_array, iter);
             eina_value_flush(&value_array);
          }
        else if (type[0] == '(')
          {
             Eina_Value inner_st;
             Eldbus_Message_Iter *sub_iter;
             char *sub_sig;
             unsigned len = strlen(type+1) -1;
             sub_sig = alloca(sizeof(char) * len);
             memcpy(sub_sig, type+1, len);
             sub_sig[len-1] = 0;
             EINA_SAFETY_ON_FALSE_RETURN_VAL(
                         _compatible_type(type[0], st.desc->members[i].type),
                         EINA_FALSE);
             eina_value_struct_value_get(value, st.desc->members[i].name, &inner_st);
             eldbus_message_iter_arguments_append(iter, type, &sub_iter);
             r = _message_iter_from_eina_value_struct(sub_sig, sub_iter,
                                                      &inner_st);
             eldbus_message_iter_container_close(iter, sub_iter);
          }
        else if (type[0] == 'v')
          {
             ERR("Variant not supported");
             r = EINA_FALSE;
          }
        else
          {
             ERR("Unknown type %c", type[0]);
             r = EINA_FALSE;
          }
        i++;
        dbus_free(type);
        if (!r || !dbus_signature_iter_next(&signature_iter)) break;
     }
   return r;
}

EAPI Eina_Bool
eldbus_message_from_eina_value(const char *signature, Eldbus_Message *msg, const Eina_Value *value)
{
   Eldbus_Message_Iter *iter;
   EINA_SAFETY_ON_NULL_RETURN_VAL(signature, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(msg, EINA_FALSE);
   EINA_SAFETY_ON_NULL_RETURN_VAL(value, EINA_FALSE);

   iter = eldbus_message_iter_get(msg);
   EINA_SAFETY_ON_NULL_RETURN_VAL(iter, EINA_FALSE);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(iter->writable, EINA_FALSE);

   return _message_iter_from_eina_value_struct(signature, iter, value);
}

static Eina_Bool
_basic_append_value(char type, const Eina_Value *value, Eldbus_Message_Iter *iter)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(value, EINA_FALSE);
   const Eina_Value_Type *value_type = eina_value_type_get(value);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(_compatible_type(type, value_type), EINA_FALSE);
   switch (type)
     {
      case 'i'://int
      case 'h'://fd
        {
           int32_t i;
           eina_value_get(value, &i);
           eldbus_message_iter_basic_append(iter, type, i);
           break;
        }
      case 's':
      case 'o'://object path
      case 'g'://signature
        {
           const char *txt;
           eina_value_get(value, &txt);
           eldbus_message_iter_basic_append(iter, type, txt);
           break;
        }
      case 'y'://byte
        {
           unsigned char byte;
           eina_value_get(value, &byte);
           eldbus_message_iter_basic_append(iter, type, byte);
           break;
        }
      case 'b'://boolean
        {
           unsigned char boolean;
           eina_value_get(value, &boolean);
           eldbus_message_iter_basic_append(iter, type, (uint32_t)boolean);
           break;
        }
      case 'n'://int16
        {
           int16_t i;
           eina_value_get(value, &i);
           eldbus_message_iter_basic_append(iter, type, i);
           break;
        }
      case 'q'://uint16
        {
           uint16_t i;
           eina_value_get(value, &i);
           eldbus_message_iter_basic_append(iter, type, i);
           break;
        }
      case 'u'://uint32
        {
           uint32_t i;
           eina_value_get(value, &i);
           eldbus_message_iter_basic_append(iter, type, i);
           break;
        }
      case 'x'://int64
        {
           int64_t i;
           eina_value_get(value, &i);
           eldbus_message_iter_basic_append(iter, type, i);
           break;
        }
      case 't'://uint64
        {
           uint64_t i;
           eina_value_get(value, &i);
           eldbus_message_iter_basic_append(iter, type, i);
           break;
        }
      case 'd'://double
        {
           double d;
           eina_value_get(value, &d);
           eldbus_message_iter_basic_append(iter, type, d);
           break;
        }
      default:
        ERR("Unexpected type %c", type);
        return EINA_FALSE;
     }
   return EINA_TRUE;
}

Eina_Bool
_message_iter_from_eina_value(const char *signature, Eldbus_Message_Iter *iter, const Eina_Value *value)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(value, EINA_FALSE);

   const Eina_Value_Type *value_type = eina_value_type_get(value);
   if (EINA_VALUE_TYPE_STRUCT == value_type || EINA_VALUE_TYPE_ARRAY == value_type)
     return _message_iter_from_eina_value_struct(signature, iter, value);

   Eina_Bool result = EINA_TRUE;
   DBusSignatureIter signature_iter;
   dbus_signature_iter_init(&signature_iter, signature);
   char *type;
   while ((type = dbus_signature_iter_get_signature(&signature_iter)))
     {
        DBG("type: %s", type);
        if (DBUS_TYPE_VARIANT != type[0] && DBUS_TYPE_INVALID == type[1])
          result = _basic_append_value(type[0], value, iter);
        else if (DBUS_TYPE_ARRAY == type[0] ||
                 DBUS_STRUCT_BEGIN_CHAR == type[0] ||
                 DBUS_TYPE_VARIANT == type[0])
          {
             ERR("Not a basic type");
             result = EINA_FALSE;
          }
        else
          {
             ERR("Unknown type %c", type[0]);
             result = EINA_FALSE;
          }
        dbus_free(type);
        if (!result || !dbus_signature_iter_next(&signature_iter)) break;
     }
   return result;
}
