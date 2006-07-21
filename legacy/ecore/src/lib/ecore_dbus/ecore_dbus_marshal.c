/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#include "ecore_private.h"
#include "Ecore_Con.h"
#include "Ecore_DBus.h"
#include "ecore_dbus_private.h"

static int _ecore_dbus_alignment_get(Ecore_DBus_Data_Type type);

Ecore_DBus_Message_Field *
_ecore_dbus_message_marshal_byte(unsigned char **buf, unsigned int *old_length,
				 unsigned char c)
{
   Ecore_DBus_Message_Field *f;

   /* increase the length + 1, old_length changed */
   _ecore_dbus_message_increase_length(buf, *old_length + 1);
   /* append the data type */
   _ecore_dbus_message_append_byte(*buf + *old_length, ECORE_DBUS_DATA_TYPE_BYTE);
   *old_length += 1;
   /* increase the length + 1, old_length changed */
   _ecore_dbus_message_increase_length(buf, *old_length + 1);
   /* append c at cur_length */
   _ecore_dbus_message_append_byte(*buf + *old_length, c);
   /* create field */
   f = _ecore_dbus_message_new_field(ECORE_DBUS_DATA_TYPE_BYTE, *old_length);
   *old_length += 1;
   return f;
}

Ecore_DBus_Message_Field *
_ecore_dbus_message_marshal_uint32(unsigned char **buf,
				   unsigned int *old_length,
				   unsigned int i)
{
   Ecore_DBus_Message_Field *f;

   /* increase the length + 1, old_length changed */
   _ecore_dbus_message_increase_length(buf, *old_length + 1);
   /* append the data type */
   _ecore_dbus_message_append_byte(*buf + *old_length, ECORE_DBUS_DATA_TYPE_UINT32);
   *old_length += 1;
   _ecore_dbus_message_padding(buf, old_length, 4);
   _ecore_dbus_message_increase_length(buf, *old_length + 4);
   _ecore_dbus_message_append_uint32(*buf + *old_length, i);
   f = _ecore_dbus_message_new_field(ECORE_DBUS_DATA_TYPE_UINT32, *old_length);
   *old_length += 4;
   return f;
}

Ecore_DBus_Message_Field *
_ecore_dbus_message_marshal_object_path(unsigned char **buf, unsigned int *old_length,
				 char *str)
{
   int                       str_len;
   Ecore_DBus_Message_Field *f;

   /* increase the length + 1, old_length changed */
   _ecore_dbus_message_increase_length(buf, *old_length + 1);
   /* append the data type */
   _ecore_dbus_message_append_byte(*buf + *old_length, ECORE_DBUS_DATA_TYPE_OBJECT_PATH);
   *old_length += 1;

   str_len = strlen(str);
   _ecore_dbus_message_padding(buf, old_length, 4);
   _ecore_dbus_message_increase_length(buf, *old_length + 4);
   f = _ecore_dbus_message_new_field(ECORE_DBUS_DATA_TYPE_OBJECT_PATH, *old_length);
   _ecore_dbus_message_append_uint32(*buf + *old_length, str_len);
   *old_length += 4;
   /* + 1 for \0 */
   _ecore_dbus_message_increase_length(buf, *old_length + str_len + 1);
   _ecore_dbus_message_append_nbytes(*buf + *old_length, (unsigned char *)str, str_len + 1);
   *old_length += str_len + 1;

   return f;
}

Ecore_DBus_Message_Field *
_ecore_dbus_message_marshal_string(unsigned char **buf,
				   unsigned int *old_length, char *str)
{
   int                       str_len;
   Ecore_DBus_Message_Field *f;

   /* increase the length + 1, new_length changed */
   _ecore_dbus_message_increase_length(buf, *old_length + 1);
   /* append the data type */
   _ecore_dbus_message_append_byte(*buf + *old_length, ECORE_DBUS_DATA_TYPE_STRING);
   *old_length += 1;

   str_len = strlen(str);
   _ecore_dbus_message_padding(buf, old_length, 4);
   _ecore_dbus_message_increase_length(buf, *old_length + 4);
   f = _ecore_dbus_message_new_field(ECORE_DBUS_DATA_TYPE_STRING, *old_length);
   _ecore_dbus_message_append_uint32(*buf + *old_length, str_len);
   *old_length += 4;
   /* + 1 for \0 */
   _ecore_dbus_message_increase_length(buf, *old_length + str_len + 1);
   _ecore_dbus_message_append_nbytes(*buf + *old_length, (unsigned char *)str, str_len + 1);
   *old_length += str_len + 1;
   return f;
}

Ecore_DBus_Message_Field *
_ecore_dbus_message_marshal_array(unsigned char **buf, unsigned int *old_length,
				  Ecore_DBus_Data_Type contained_type)
{
   Ecore_DBus_Message_Field *f;

   /* increase the length with 1 + contained length */
   _ecore_dbus_message_increase_length(buf, *old_length + 1);
   /* append the data type */
   _ecore_dbus_message_append_byte(*buf + *old_length, ECORE_DBUS_DATA_TYPE_ARRAY);
   *old_length += 1;

   /* for the array length value */
   _ecore_dbus_message_padding(buf, old_length, 4);
   _ecore_dbus_message_increase_length(buf, *old_length + 4);
   f = _ecore_dbus_message_new_field(ECORE_DBUS_DATA_TYPE_ARRAY, *old_length);
   *old_length += 4;

   /* pad for contained type */
   _ecore_dbus_message_padding(buf, old_length, _ecore_dbus_alignment_get(contained_type));

   return f;
}

Ecore_DBus_Message_Field *
_ecore_dbus_message_marshal_struct_begin(unsigned char **buf, unsigned int *old_length)
{
   Ecore_DBus_Message_Field *f;

   /* increase the length with 1 */
   _ecore_dbus_message_increase_length(buf, *old_length + 1);
   /* append the data type */
   _ecore_dbus_message_append_byte(*buf + *old_length, ECORE_DBUS_DATA_TYPE_STRUCT_BEGIN);
   *old_length += 1;

   /* padding */
   _ecore_dbus_message_padding(buf, old_length, 8);
   f = _ecore_dbus_message_new_field(ECORE_DBUS_DATA_TYPE_STRUCT, *old_length);

   return f;
}

Ecore_DBus_Message_Field *
_ecore_dbus_message_marshal_struct_end(unsigned char **buf, unsigned int *old_length)
{
   Ecore_DBus_Message_Field *f;

   /* increase the length with 1 */
   _ecore_dbus_message_increase_length(buf, *old_length + 1);
   /* append the data type */
   _ecore_dbus_message_append_byte(*buf + *old_length, ECORE_DBUS_DATA_TYPE_STRUCT_END);
   *old_length += 1;

   f = _ecore_dbus_message_new_field(ECORE_DBUS_DATA_TYPE_STRUCT, *old_length);

   return f;
}

Ecore_DBus_Message_Field *
_ecore_dbus_message_marshal_data(unsigned char **buf, unsigned int *old_length,
				 Ecore_DBus_Data_Type type, void *data)
{
   Ecore_DBus_Message_Field *f = NULL;

   _ecore_dbus_message_increase_length(buf, *old_length + 1);
   /* append the signature length */
   _ecore_dbus_message_append_byte(*buf + *old_length, 1);
   *old_length += 1;

   switch (type)
     {
     case ECORE_DBUS_DATA_TYPE_UINT32:
	f = _ecore_dbus_message_marshal_uint32(buf, old_length,
					       *(unsigned int *)data);
	break;
     case ECORE_DBUS_DATA_TYPE_STRING:
	f = _ecore_dbus_message_marshal_string(buf, old_length,
					       (char *)data);
	break;
     case ECORE_DBUS_DATA_TYPE_OBJECT_PATH:
	f = _ecore_dbus_message_marshal_object_path(buf, old_length,
						    (char *)data);
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
     case ECORE_DBUS_DATA_TYPE_SIGNATURE:
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
   return f;
}

Ecore_DBus_Message_Field *
_ecore_dbus_message_marshal_custom_header(unsigned char **buf,
					  unsigned int *old_length,
					  unsigned int code,
					  Ecore_DBus_Data_Type type, void *data)
{
   Ecore_DBus_Message_Field *f;

   _ecore_dbus_message_padding(buf, old_length, 8);
   /* increase the length + 1, old_length changed */
   _ecore_dbus_message_increase_length(buf, *old_length + 1);
   /* append header field name at cur_length */
   _ecore_dbus_message_append_byte(*buf + *old_length, code);
   *old_length += 1;

   /* marshal header field data */
   f = _ecore_dbus_message_marshal_data(buf, old_length, type, data);
   f->hfield = code;
   return f;
}

static int
_ecore_dbus_alignment_get(Ecore_DBus_Data_Type type)
{
   switch (type)
     {
        case ECORE_DBUS_DATA_TYPE_BYTE:
        case ECORE_DBUS_DATA_TYPE_SIGNATURE:
        case ECORE_DBUS_DATA_TYPE_VARIANT:
	   return 1;
        case ECORE_DBUS_DATA_TYPE_INT16:
        case ECORE_DBUS_DATA_TYPE_UINT16:
	   return 2;
        case ECORE_DBUS_DATA_TYPE_BOOLEAN:
        case ECORE_DBUS_DATA_TYPE_INT32:
        case ECORE_DBUS_DATA_TYPE_UINT32:
        case ECORE_DBUS_DATA_TYPE_STRING:
        case ECORE_DBUS_DATA_TYPE_OBJECT_PATH:
        case ECORE_DBUS_DATA_TYPE_ARRAY:
	   return 4;
        case ECORE_DBUS_DATA_TYPE_INT64:
        case ECORE_DBUS_DATA_TYPE_UINT64:
        case ECORE_DBUS_DATA_TYPE_DOUBLE:
	   return 8;
        case ECORE_DBUS_DATA_TYPE_STRUCT:
        case ECORE_DBUS_DATA_TYPE_DICT_ENTRY:
	   return 8;
	default:
	   fprintf(stderr, "Ecore_DBus: Alignment requested for invalid data type!\n");
	   return 0;
     }
}
