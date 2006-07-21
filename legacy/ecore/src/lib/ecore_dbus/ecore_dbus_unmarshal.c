/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#include "ecore_private.h"
#include "Ecore_Con.h"
#include "Ecore_DBus.h"
#include "ecore_dbus_private.h"

Ecore_DBus_Message_Field *
_ecore_dbus_message_unmarshal_int32(unsigned char *buf,
				    unsigned int *old_length)
{
   Ecore_DBus_Message_Field *f;

   _ecore_dbus_message_padding_skip(old_length, 4);
   f = _ecore_dbus_message_new_field(ECORE_DBUS_DATA_TYPE_INT32, *old_length);
   *old_length += 4;
   return f;
}

Ecore_DBus_Message_Field *
_ecore_dbus_message_unmarshal_uint32(unsigned char *buf,
				     unsigned int *old_length)
{
   Ecore_DBus_Message_Field *f;

   _ecore_dbus_message_padding_skip(old_length, 4);
   f = _ecore_dbus_message_new_field(ECORE_DBUS_DATA_TYPE_UINT32, *old_length);
   *old_length += 4;
   return f;
}

Ecore_DBus_Message_Field *
_ecore_dbus_message_unmarshal_string(unsigned char *buf,
				     unsigned int *old_length)
{
   Ecore_DBus_Message_Field *f;
   unsigned int              str_len;

   _ecore_dbus_message_padding_skip(old_length, 4);
   f = _ecore_dbus_message_new_field(ECORE_DBUS_DATA_TYPE_STRING, *old_length);
   str_len = _ecore_dbus_message_read_uint32(buf + *old_length);
   *old_length += 4;
   *old_length += str_len + 1;
   return f;
}

Ecore_DBus_Message_Field *
_ecore_dbus_message_unmarshal_object_path(unsigned char *buf, unsigned int *old_length)
{
   Ecore_DBus_Message_Field *f;
   unsigned int              str_len;

   _ecore_dbus_message_padding_skip(old_length, 4);
   f = _ecore_dbus_message_new_field(ECORE_DBUS_DATA_TYPE_OBJECT_PATH,
				     *old_length);
   str_len = _ecore_dbus_message_read_uint32(buf + *old_length);
   *old_length += 4;
   *old_length += str_len + 1;
   return f;
}

Ecore_DBus_Message_Field *
_ecore_dbus_message_unmarshal_signature(unsigned char *buf,
				     unsigned int *old_length)
{
   Ecore_DBus_Message_Field *f;
   unsigned int              str_len;

   f = _ecore_dbus_message_new_field(ECORE_DBUS_DATA_TYPE_SIGNATURE, *old_length);
   str_len = _ecore_dbus_message_read_byte(buf + *old_length);
   *old_length += 1;
   *old_length += str_len + 1;
   return f;
}

Ecore_DBus_Message_Field *
_ecore_dbus_message_unmarshal_data(unsigned char *buf, unsigned int *old_length)
{
   Ecore_DBus_Message_Field *f = NULL;
   Ecore_DBus_Data_Type      type;
  
   type = _ecore_dbus_message_read_byte(buf + *old_length);
   *old_length += 1;

   switch (type)
     {
     case ECORE_DBUS_DATA_TYPE_INT32:
	f = _ecore_dbus_message_unmarshal_int32(buf, old_length);
	break;
     case ECORE_DBUS_DATA_TYPE_UINT32:
	f = _ecore_dbus_message_unmarshal_uint32(buf, old_length);
	break;
     case ECORE_DBUS_DATA_TYPE_STRING:
	f = _ecore_dbus_message_unmarshal_string(buf, old_length);
	break;
     case ECORE_DBUS_DATA_TYPE_OBJECT_PATH:
	f = _ecore_dbus_message_unmarshal_object_path(buf, old_length);
	break;
     case ECORE_DBUS_DATA_TYPE_SIGNATURE:
	f = _ecore_dbus_message_unmarshal_signature(buf, old_length);
	break;
     case ECORE_DBUS_DATA_TYPE_INVALID:
     case ECORE_DBUS_DATA_TYPE_BYTE:
     case ECORE_DBUS_DATA_TYPE_BOOLEAN:
     case ECORE_DBUS_DATA_TYPE_INT16:
     case ECORE_DBUS_DATA_TYPE_UINT16:
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
   return f;
}

Ecore_DBus_Message_Field *
_ecore_dbus_message_unmarshal_custom_header(unsigned char *buf,
					    unsigned int *old_length)
{
   Ecore_DBus_Message_Field *f;
   unsigned int              code, length;

   _ecore_dbus_message_padding_skip(old_length, 8);
   /* get header field name at cur_length */
   code = _ecore_dbus_message_read_byte(buf + *old_length);
   *old_length += 1;
   /* Read signature length */
   length = _ecore_dbus_message_read_byte(buf + *old_length);
   if (length != 1)
     fprintf(stderr, "[ecore_dbus]: Wrong signature length in header\n");
   *old_length += 1;

   /* unmarshal header field data */
   f = _ecore_dbus_message_unmarshal_data(buf, old_length);
   if (f) f->hfield = code;
   return f;
}

Ecore_DBus_Message *
_ecore_dbus_message_unmarshal(Ecore_DBus_Server *svr, unsigned char *message)
{
   Ecore_DBus_Message_Field *f, *sig;

   /* init */
   Ecore_DBus_Message *msg = _ecore_dbus_message_new(svr);

   printf("[ecore_dbus] unmarshaling\n");

   /* message header */

   /* common fields  */
   msg->byte_order = *(message + 0);
   msg->type = *(message + 1);
   msg->flags = *(message + 2);
   msg->protocol = *(message + 3);
   msg->blength = *(unsigned int *)(message + 4);
   msg->serial = *(unsigned int *)(message + 8);
   msg->hlength = *(unsigned int *)(message + 12) + 16;
   _ecore_dbus_message_padding_skip(&(msg->hlength), 8);
   if (msg->type == ECORE_DBUS_MESSAGE_TYPE_INVALID)
     {
	printf("[ecore_dbus] message type invalid\n");
	return NULL;
     }
   msg->hpos += 16;
   /* memcpy the header part */
   _ecore_dbus_message_increase_length(&msg->header, msg->hlength);
   _ecore_dbus_message_append_nbytes(msg->header, message, msg->hlength);
   /* custom fields */
   sig = NULL;
   while ((msg->hpos + 8 - (msg->hpos % 8)) < msg->hlength)
     {
	f = _ecore_dbus_message_unmarshal_custom_header(message, &msg->hpos);
	if (f)
	  {
	     msg->header_fields = _ecore_list2_append(msg->header_fields, f);
	     if (f->type == ECORE_DBUS_DATA_TYPE_SIGNATURE)
	       sig = f;
	  }
     }
   msg->hpos = msg->hlength;
   message += msg->hlength;

   /* message body */
   if (sig)
     {
	unsigned char        *s;
	Ecore_DBus_Data_Type  type;

	_ecore_dbus_message_increase_length(&msg->body, msg->blength);
	_ecore_dbus_message_append_nbytes(msg->body, message, msg->blength);

	s = msg->header + sig->offset + 2;
	while (*s)
	  {
	     f = NULL;
	     printf("body: %c\n", *s);
	     type = *s;
	     switch (type)
	       {
		case ECORE_DBUS_DATA_TYPE_INT32:
		   f = _ecore_dbus_message_unmarshal_int32(msg->body, &(msg->bpos));
		   break;
		case ECORE_DBUS_DATA_TYPE_UINT32:
		   f = _ecore_dbus_message_unmarshal_uint32(msg->body, &(msg->bpos));
		   break;
		case ECORE_DBUS_DATA_TYPE_STRING:
		   f = _ecore_dbus_message_unmarshal_string(msg->body, &(msg->bpos));
		   break;
		case ECORE_DBUS_DATA_TYPE_OBJECT_PATH:
		   f = _ecore_dbus_message_unmarshal_object_path(msg->body, &(msg->bpos));
		   break;
		case ECORE_DBUS_DATA_TYPE_SIGNATURE:
		   f = _ecore_dbus_message_unmarshal_signature(msg->body, &(msg->bpos));
		   break;
		case ECORE_DBUS_DATA_TYPE_INVALID:
		case ECORE_DBUS_DATA_TYPE_BYTE:
		case ECORE_DBUS_DATA_TYPE_BOOLEAN:
		case ECORE_DBUS_DATA_TYPE_INT16:
		case ECORE_DBUS_DATA_TYPE_UINT16:
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
	     if (f)
	       msg->body_fields = _ecore_list2_append(msg->body_fields, f);
	     s++;
	  }
     }
   return msg;
}
