/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#include "ecore_private.h"
#include "Ecore_Con.h"
#include "Ecore_DBus.h"
#include "ecore_dbus_private.h"

Ecore_DBus_Message_Field_Byte *
_ecore_dbus_message_marshal_byte(Ecore_DBus_Message *msg, unsigned char c)
{
   Ecore_DBus_Message_Field_Byte *f;

   f = _ecore_dbus_message_field_new(msg, ECORE_DBUS_DATA_TYPE_BYTE);
   f->value = c;
   _ecore_dbus_message_append_byte(msg, c);
   return f;
}

Ecore_DBus_Message_Field_UInt32 *
_ecore_dbus_message_marshal_uint32(Ecore_DBus_Message *msg, unsigned int i)
{
   Ecore_DBus_Message_Field_UInt32 *f;
   unsigned char                   *c;

   _ecore_dbus_message_padding(msg, 4);
   f = _ecore_dbus_message_field_new(msg, ECORE_DBUS_DATA_TYPE_UINT32);
   f->value = i;
   c = (unsigned char *)&i;
   _ecore_dbus_message_append_bytes(msg, c, 4);
   return f;
}

Ecore_DBus_Message_Field_String *
_ecore_dbus_message_marshal_string(Ecore_DBus_Message *msg, char *str)
{
   Ecore_DBus_Message_Field_String *f;
   unsigned int                     str_len;

   _ecore_dbus_message_padding(msg, 4);
   f = _ecore_dbus_message_field_new(msg, ECORE_DBUS_DATA_TYPE_STRING);
   f->value = str;

   str_len = strlen(str);
   _ecore_dbus_message_append_uint32(msg, str_len);

   /* + 1 for \0 */
   _ecore_dbus_message_append_bytes(msg, (unsigned char *)str, str_len + 1);
   return f;
}

Ecore_DBus_Message_Field_Object_Path *
_ecore_dbus_message_marshal_object_path(Ecore_DBus_Message *msg, char *str)
{
   Ecore_DBus_Message_Field_Object_Path *f;
   unsigned int                          str_len;

   _ecore_dbus_message_padding(msg, 4);
   f = _ecore_dbus_message_field_new(msg, ECORE_DBUS_DATA_TYPE_OBJECT_PATH);
   f->value = str;

   str_len = strlen(str);
   _ecore_dbus_message_append_uint32(msg, str_len);

   /* + 1 for \0 */
   _ecore_dbus_message_append_bytes(msg, (unsigned char *)str, str_len + 1);
   return f;
}

Ecore_DBus_Message_Field_Signature *
_ecore_dbus_message_marshal_signature(Ecore_DBus_Message *msg, char *str)
{
   Ecore_DBus_Message_Field_Signature *f;
   unsigned int                        str_len;

   f = _ecore_dbus_message_field_new(msg, ECORE_DBUS_DATA_TYPE_SIGNATURE);
   f->value = str;

   str_len = strlen(str);
   _ecore_dbus_message_append_byte(msg, str_len);

   /* + 1 for \0 */
   _ecore_dbus_message_append_bytes(msg, (unsigned char *)str, str_len + 1);
   return f;
}

Ecore_DBus_Message_Field_Array *
_ecore_dbus_message_marshal_array_begin(Ecore_DBus_Message *msg,
					Ecore_DBus_Data_Type contained_type)
{
   Ecore_DBus_Message_Field_Array *arr;

   _ecore_dbus_message_padding(msg, 4);

   arr = _ecore_dbus_message_field_new(msg, ECORE_DBUS_DATA_TYPE_ARRAY);
   /* for the array length value */
   _ecore_dbus_message_append_uint32(msg, 0);
   arr->contained_type = contained_type;
   ecore_list_prepend(msg->recurse, arr);

   /* pad for contained type */
   _ecore_dbus_message_padding(msg, _ecore_dbus_alignment_get(contained_type));
   arr->start = msg->length;

   return arr;
}

void
_ecore_dbus_message_marshal_array_end(Ecore_DBus_Message *msg, Ecore_DBus_Message_Field_Array *arr)
{
   ecore_list_remove_first(msg->recurse);
   arr->end = msg->length;
   *(unsigned int *)ECORE_DBUS_MESSAGE_FIELD(arr)->buffer = arr->end - arr->start;
}

Ecore_DBus_Message_Field_Struct *
_ecore_dbus_message_marshal_struct_begin(Ecore_DBus_Message *msg)
{
   Ecore_DBus_Message_Field_Struct *s;

   _ecore_dbus_message_padding(msg, 8);
   s = _ecore_dbus_message_field_new(msg, ECORE_DBUS_DATA_TYPE_STRUCT);
   ecore_list_prepend(msg->recurse, s);

   return s;
}

void
_ecore_dbus_message_marshal_struct_end(Ecore_DBus_Message *msg, Ecore_DBus_Message_Field_Struct *s)
{
   ecore_list_remove_first(msg->recurse);
}

Ecore_DBus_Message_Field_Variant *
_ecore_dbus_message_marshal_variant(Ecore_DBus_Message *msg, Ecore_DBus_Data_Type type, void *data)
{
   Ecore_DBus_Message_Field_Variant *f = NULL;

   f = _ecore_dbus_message_field_new(msg, ECORE_DBUS_DATA_TYPE_VARIANT);
   ecore_list_prepend(msg->recurse, f);
   f->contained_type = type;

   /* signature length */
   _ecore_dbus_message_append_byte(msg, 1);
   /* signature */
   _ecore_dbus_message_append_byte(msg, type);

   switch (type)
     {
     case ECORE_DBUS_DATA_TYPE_UINT32:
	f->value = _ecore_dbus_message_marshal_uint32(msg, *(unsigned int *)data);
	break;
     case ECORE_DBUS_DATA_TYPE_STRING:
	f->value = _ecore_dbus_message_marshal_string(msg, (char *)data);
	break;
     case ECORE_DBUS_DATA_TYPE_OBJECT_PATH:
	f->value = _ecore_dbus_message_marshal_object_path(msg, (char *)data);
	break;
     case ECORE_DBUS_DATA_TYPE_SIGNATURE:
	f->value = _ecore_dbus_message_marshal_signature(msg, (char *)data);
	break;
     case ECORE_DBUS_DATA_TYPE_INVALID:
     case ECORE_DBUS_DATA_TYPE_BYTE:
     case ECORE_DBUS_DATA_TYPE_BOOLEAN:
     case ECORE_DBUS_DATA_TYPE_INT16:
     case ECORE_DBUS_DATA_TYPE_UINT16:
     case ECORE_DBUS_DATA_TYPE_INT32:
     case ECORE_DBUS_DATA_TYPE_INT64:
     case ECORE_DBUS_DATA_TYPE_UINT64:
     case ECORE_DBUS_DATA_TYPE_DOUBLE:
     case ECORE_DBUS_DATA_TYPE_ARRAY:
     case ECORE_DBUS_DATA_TYPE_VARIANT:
     case ECORE_DBUS_DATA_TYPE_STRUCT:
     case ECORE_DBUS_DATA_TYPE_STRUCT_BEGIN:
     case ECORE_DBUS_DATA_TYPE_STRUCT_END:
     case ECORE_DBUS_DATA_TYPE_DICT_ENTRY:
     case ECORE_DBUS_DATA_TYPE_DICT_ENTRY_BEGIN:
     case ECORE_DBUS_DATA_TYPE_DICT_ENTRY_END:
#if 0
     default:
#endif
	printf("[ecore_dbus] unknown/unhandled data type %c\n", type);
	break;
     }
   ecore_list_remove_first(msg->recurse);
   return f;
}
