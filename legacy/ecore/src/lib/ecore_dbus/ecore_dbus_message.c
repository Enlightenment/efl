/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#include <stdarg.h>

#include "ecore_private.h"
#include "Ecore_Con.h"
#include "Ecore_DBus.h"
#include "ecore_dbus_private.h"

static void  _ecore_dbus_message_common_header(Ecore_DBus_Message *msg, int type, int flags);
static void  _ecore_dbus_message_field_free(void *data);

/* printing functions */
static void _ecore_dbus_message_field_print(Ecore_DBus_Message_Field *f);
static void _ecore_dbus_message_header_field_print(Ecore_DBus_Message_Field_Container *arr);
static void _ecore_dbus_message_print_raw(Ecore_DBus_Message *msg);

EAPI unsigned int
ecore_dbus_message_new_method_call(Ecore_DBus_Server *svr, char *destination,
				   char *path, char *interface, char *method,
				   Ecore_DBus_Method_Return_Cb method_cb,
				   Ecore_DBus_Error_Cb error_cb,
				   void *data,
				   char *fmt, ...)
{
   unsigned int                    body_start;
   char                            buf[1024];
   Ecore_DBus_Message_Field_Array *arr;

   if (!method) return 0;

   /* init message */
   Ecore_DBus_Message *msg = _ecore_dbus_message_new(svr);
   if (method_cb)
     {
	msg->cb.method_return = method_cb;
	msg->cb.error = error_cb;
	msg->cb.data = data;
     }

   /* common header */
   _ecore_dbus_message_common_header(msg, ECORE_DBUS_MESSAGE_TYPE_METHOD_CALL, 0);

   arr = _ecore_dbus_message_marshal_array_begin(msg, ECORE_DBUS_DATA_TYPE_STRUCT);
   /* custom header */
   if (path)
     {
	Ecore_DBus_Message_Field_Struct *s;

	s = _ecore_dbus_message_marshal_struct_begin(msg);
	_ecore_dbus_message_marshal_byte(msg, ECORE_DBUS_HEADER_FIELD_PATH);
	_ecore_dbus_message_marshal_variant(msg, ECORE_DBUS_DATA_TYPE_OBJECT_PATH, path);
	_ecore_dbus_message_marshal_struct_end(msg, s);
     }
   if (destination)
     {
	Ecore_DBus_Message_Field_Struct *s;

	s = _ecore_dbus_message_marshal_struct_begin(msg);
	_ecore_dbus_message_marshal_byte(msg, ECORE_DBUS_HEADER_FIELD_DESTINATION);
	_ecore_dbus_message_marshal_variant(msg, ECORE_DBUS_DATA_TYPE_STRING, destination);
	_ecore_dbus_message_marshal_struct_end(msg, s);
     }
   if (interface)
     {
	Ecore_DBus_Message_Field_Struct *s;

	s = _ecore_dbus_message_marshal_struct_begin(msg);
	_ecore_dbus_message_marshal_byte(msg, ECORE_DBUS_HEADER_FIELD_INTERFACE);
	_ecore_dbus_message_marshal_variant(msg, ECORE_DBUS_DATA_TYPE_STRING, interface);
	_ecore_dbus_message_marshal_struct_end(msg, s);
     }
   if (method)
     {
	Ecore_DBus_Message_Field_Struct *s;

	s = _ecore_dbus_message_marshal_struct_begin(msg);
	_ecore_dbus_message_marshal_byte(msg, ECORE_DBUS_HEADER_FIELD_MEMBER);
	_ecore_dbus_message_marshal_variant(msg, ECORE_DBUS_DATA_TYPE_STRING, method);
	_ecore_dbus_message_marshal_struct_end(msg, s);
     }
   if (fmt)
     {
	Ecore_DBus_Message_Field_Struct *s;

	s = _ecore_dbus_message_marshal_struct_begin(msg);
	_ecore_dbus_message_marshal_byte(msg, ECORE_DBUS_HEADER_FIELD_SIGNATURE);
	_ecore_dbus_message_marshal_variant(msg, ECORE_DBUS_DATA_TYPE_SIGNATURE, fmt);
	_ecore_dbus_message_marshal_struct_end(msg, s);
     }
   _ecore_dbus_message_marshal_array_end(msg, arr);
   msg->header = ecore_list_remove_first(msg->fields);
   _ecore_dbus_message_padding(msg, 8);

   /* message body */
   body_start = msg->length;
   if (fmt)
     {
	va_list ap;
	va_start(ap, fmt);
	while (*fmt)
	  {
	     Ecore_DBus_Data_Type type = *fmt;
	     switch (type)
	       {
		case ECORE_DBUS_DATA_TYPE_BYTE:
		   _ecore_dbus_message_marshal_byte(msg, va_arg(ap, int));
		   break;
		case ECORE_DBUS_DATA_TYPE_UINT32:
		    _ecore_dbus_message_marshal_uint32(msg, va_arg(ap, unsigned int));
		   break;
		case ECORE_DBUS_DATA_TYPE_STRING:
		    _ecore_dbus_message_marshal_string(msg, (char *)va_arg(ap, char *));
		   break;
		case ECORE_DBUS_DATA_TYPE_OBJECT_PATH:
		    _ecore_dbus_message_marshal_object_path(msg, (char *)va_arg(ap, char *));
		   break;
		case ECORE_DBUS_DATA_TYPE_SIGNATURE:
		    _ecore_dbus_message_marshal_signature(msg, (char *)va_arg(ap, char *));
		   break;
		case ECORE_DBUS_DATA_TYPE_INVALID:
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
		   printf("[ecore_dbus] unknown/unhandled data type %c\n", *fmt);
		   break;
	       }
	     fmt++;
	  }
	va_end(ap);
     }
   /* set body length */
   *(unsigned int *)(msg->buffer + 4) = msg->length - body_start;

   /* show message */
   //ecore_dbus_message_print(msg);
   /* send message */
   ecore_dbus_server_send(svr, (char *)msg->buffer, msg->length);
   if (interface)
     snprintf(buf, sizeof(buf), "%s.%s", interface, method);
   else
     strcpy(buf, method);
   ecore_hash_set(svr->messages, (void *)msg->serial, msg);

   return msg->serial;
}

EAPI void
ecore_dbus_message_print(Ecore_DBus_Message *msg)
{
   Ecore_DBus_Message_Field *f;
   static const char        *msg_type[] = {
	"INVALID", "METHOD_CALL", "METHOD_RETURN", "ERROR", "SIGNAL"
   };
   _ecore_dbus_message_print_raw(msg);

   printf("[ecore_dbus] per field message:\n");
   /* header common fields */
   printf("[ecore_dbus] header endianess	: %c\n", msg->byte_order);
   printf("[ecore_dbus] header type     	: %s\n", msg_type[msg->type]);
   printf("[ecore_dbus] header flags    	: %c\n", msg->flags);
   printf("[ecore_dbus] header protocol 	: %c\n", msg->protocol);
   printf("[ecore_dbus] body length	: %u\n",
	 *(unsigned int *)(msg->buffer + 4));
   printf("[ecore_dbus] header serial   	: %u\n",
	 *(unsigned int *)(msg->buffer + 8));

   /* header custom fields */
   printf("[ecore_dbus] header fields:\n");
   _ecore_dbus_message_header_field_print(msg->header);
   /* body fields */
   printf("[ecore_dbus] body fields:\n");
   ecore_list_goto_first(msg->fields);
   while ((f = ecore_list_next(msg->fields)))
     _ecore_dbus_message_field_print(f);
}

EAPI void *
ecore_dbus_message_header_field_get(Ecore_DBus_Message *m,
				    Ecore_DBus_Message_Header_Field field)
{
   Ecore_DBus_Message_Field_Container *s;

   ecore_list_goto_first(ECORE_DBUS_MESSAGE_FIELD_CONTAINER(m->header)->values);
   while ((s = ecore_list_next(ECORE_DBUS_MESSAGE_FIELD_CONTAINER(m->header)->values)))
     {
	Ecore_DBus_Message_Field_Byte *b;
	b = ecore_list_first(s->values);
	if (b->value == field)
	  {
	     Ecore_DBus_Message_Field_Variant *v;

	     v = ecore_list_last(s->values);
	     switch (v->contained_type)
	       {
		case ECORE_DBUS_DATA_TYPE_UINT32:
		   return &ECORE_DBUS_MESSAGE_FIELD_UINT32(v->value)->value;
		case ECORE_DBUS_DATA_TYPE_STRING:
		   return ECORE_DBUS_MESSAGE_FIELD_STRING(v->value)->value;
		case ECORE_DBUS_DATA_TYPE_OBJECT_PATH:
		   return ECORE_DBUS_MESSAGE_FIELD_OBJECT_PATH(v->value)->value;
		case ECORE_DBUS_DATA_TYPE_SIGNATURE:
		   return ECORE_DBUS_MESSAGE_FIELD_SIGNATURE(v->value)->value;
		default:
		   return NULL;
	       }
	  }
     }
   return NULL;
}

EAPI void *
ecore_dbus_message_body_field_get(Ecore_DBus_Message *m, unsigned int pos)
{
   Ecore_DBus_Message_Field *f;
   unsigned int              i = 0;

   ecore_list_goto_first(m->fields);
   while ((f = ecore_list_next(m->fields)))
     {
	if (i == pos)
	  return _ecore_dbus_message_field_value_get(f);
	i++;
     }
   return NULL;
}

/* message functions */

Ecore_DBus_Message *
_ecore_dbus_message_new(Ecore_DBus_Server *svr)
{
   Ecore_DBus_Message *msg;
  
   msg = calloc(1, sizeof(Ecore_DBus_Message));
   if (!msg) return NULL;

   msg->server = svr;

   msg->length = 0;
   msg->size = 128;
   msg->buffer = calloc(msg->size, sizeof(unsigned char));

   msg->all = ecore_list_new();
   ecore_list_set_free_cb(msg->all, _ecore_dbus_message_field_free);
   msg->fields = ecore_list_new();
   msg->recurse = ecore_list_new();

   return msg;
}

void *
_ecore_dbus_message_field_new(Ecore_DBus_Message *msg, Ecore_DBus_Data_Type type)
{
   Ecore_DBus_Message_Field *f;

   switch (type)
     {
      case ECORE_DBUS_DATA_TYPE_INVALID:
	 f = NULL;
	 break;
      case ECORE_DBUS_DATA_TYPE_BYTE:
	 f = calloc(1, sizeof(Ecore_DBus_Message_Field_Byte));
	 break;
      case ECORE_DBUS_DATA_TYPE_BOOLEAN:
	 f = calloc(1, sizeof(Ecore_DBus_Message_Field_Boolean));
	 break;
      case ECORE_DBUS_DATA_TYPE_INT16:
	 f = calloc(1, sizeof(Ecore_DBus_Message_Field_Int16));
	 break;
      case ECORE_DBUS_DATA_TYPE_UINT16:
	 f = calloc(1, sizeof(Ecore_DBus_Message_Field_UInt16));
	 break;
      case ECORE_DBUS_DATA_TYPE_INT32:
	 f = calloc(1, sizeof(Ecore_DBus_Message_Field_Int32));
	 break;
      case ECORE_DBUS_DATA_TYPE_UINT32:
	 f = calloc(1, sizeof(Ecore_DBus_Message_Field_UInt32));
	 break;
      case ECORE_DBUS_DATA_TYPE_INT64:
	 f = calloc(1, sizeof(Ecore_DBus_Message_Field_Int64));
	 break;
      case ECORE_DBUS_DATA_TYPE_UINT64:
	 f = calloc(1, sizeof(Ecore_DBus_Message_Field_UInt64));
	 break;
      case ECORE_DBUS_DATA_TYPE_DOUBLE:
	 f = calloc(1, sizeof(Ecore_DBus_Message_Field_Double));
	 break;
      case ECORE_DBUS_DATA_TYPE_STRING:
	 f = calloc(1, sizeof(Ecore_DBus_Message_Field_String));
	 break;
      case ECORE_DBUS_DATA_TYPE_OBJECT_PATH:
	 f = calloc(1, sizeof(Ecore_DBus_Message_Field_Object_Path));
	 break;
      case ECORE_DBUS_DATA_TYPE_SIGNATURE:
	 f = calloc(1, sizeof(Ecore_DBus_Message_Field_Signature));
	 break;
      case ECORE_DBUS_DATA_TYPE_ARRAY:
	 f = calloc(1, sizeof(Ecore_DBus_Message_Field_Array));
	 ECORE_DBUS_MESSAGE_FIELD_CONTAINER(f)->values = ecore_list_new();
	 break;
      case ECORE_DBUS_DATA_TYPE_VARIANT:
	 f = calloc(1, sizeof(Ecore_DBus_Message_Field_Variant));
	 ECORE_DBUS_MESSAGE_FIELD_CONTAINER(f)->values = ecore_list_new();
	 break;
      case ECORE_DBUS_DATA_TYPE_STRUCT:
	 f = calloc(1, sizeof(Ecore_DBus_Message_Field_Struct));
	 ECORE_DBUS_MESSAGE_FIELD_CONTAINER(f)->values = ecore_list_new();
	 break;
      case ECORE_DBUS_DATA_TYPE_STRUCT_BEGIN:
	 f = NULL;
	 break;
      case ECORE_DBUS_DATA_TYPE_STRUCT_END:
	 f = NULL;
	 break;
      case ECORE_DBUS_DATA_TYPE_DICT_ENTRY:
	 f = calloc(1, sizeof(Ecore_DBus_Message_Field_Dict_Entry));
	 break;
      case ECORE_DBUS_DATA_TYPE_DICT_ENTRY_BEGIN:
	 f = NULL;
	 break;
      case ECORE_DBUS_DATA_TYPE_DICT_ENTRY_END:
	 f = NULL;
	 break;
     }
   if (!f) return NULL;

   f->type = type;
   if (!ecore_list_is_empty(msg->recurse))
     {
	Ecore_DBus_Message_Field_Container *c;

	c = ecore_list_first(msg->recurse);
	ecore_list_append(c->values, f);
     }
   else
     {
	ecore_list_append(msg->fields, f);
     }
   ecore_list_append(msg->all, f);

   f->offset = msg->length;
   f->buffer = msg->buffer + f->offset;
   return f;
}

void
_ecore_dbus_message_free(Ecore_DBus_Message *msg)
{
   ecore_list_destroy(msg->fields);
   ecore_list_destroy(msg->recurse);
   ecore_list_destroy(msg->all);
   free(msg->buffer);
   free(msg);
}

/* private functions */

/* header functions */

static void
_ecore_dbus_message_common_header(Ecore_DBus_Message *msg, int type, int flags)
{
   /* endiannes (1) */
   msg->buffer[0] = msg->byte_order = 'l';
   /* type (1) */
   msg->buffer[1] = msg->type = (char)type;
   /* flags (1) 0x1 = no reply expected, 0x2 auto activation */
   msg->buffer[2] = msg->flags = 0x0;
   /* protocol (1) */
   msg->buffer[3] = msg->protocol = ECORE_DBUS_MAJOR_PROTOCOL_VERSION;
   /* autoincrement the client_serial (0 is invalid) */
   msg->server->cnt_msg++;

   *(unsigned int *)(msg->buffer + 8) = msg->serial = msg->server->cnt_msg;
   msg->length = 12;
}

static void
_ecore_dbus_message_field_free(void *data)
{
   Ecore_DBus_Message_Field *f;

   f = data;
   switch (f->type)
     {
      case ECORE_DBUS_DATA_TYPE_BYTE:
      case ECORE_DBUS_DATA_TYPE_BOOLEAN:
      case ECORE_DBUS_DATA_TYPE_INT16:
      case ECORE_DBUS_DATA_TYPE_UINT16:
      case ECORE_DBUS_DATA_TYPE_INT32:
      case ECORE_DBUS_DATA_TYPE_UINT32:
      case ECORE_DBUS_DATA_TYPE_INT64:
      case ECORE_DBUS_DATA_TYPE_UINT64:
      case ECORE_DBUS_DATA_TYPE_DOUBLE:
      case ECORE_DBUS_DATA_TYPE_STRING:
      case ECORE_DBUS_DATA_TYPE_OBJECT_PATH:
      case ECORE_DBUS_DATA_TYPE_SIGNATURE:
	 free(f);
	 break;
      case ECORE_DBUS_DATA_TYPE_ARRAY:
      case ECORE_DBUS_DATA_TYPE_VARIANT:
      case ECORE_DBUS_DATA_TYPE_STRUCT:
      case ECORE_DBUS_DATA_TYPE_DICT_ENTRY:
	   {
	      Ecore_DBus_Message_Field_Container *c;

	      c = (Ecore_DBus_Message_Field_Container *)f;
	      ecore_list_destroy(c->values);
	   }
	 free(f);
	 break;
      case ECORE_DBUS_DATA_TYPE_INVALID:
      case ECORE_DBUS_DATA_TYPE_STRUCT_BEGIN:
      case ECORE_DBUS_DATA_TYPE_STRUCT_END:
      case ECORE_DBUS_DATA_TYPE_DICT_ENTRY_BEGIN:
      case ECORE_DBUS_DATA_TYPE_DICT_ENTRY_END:
	 break;
     }
}

/* printing functions */

static void
_ecore_dbus_message_field_print(Ecore_DBus_Message_Field *f)
{
   switch (f->type)
     {
      case ECORE_DBUS_DATA_TYPE_BYTE:
	 printf
	    ("[ecore_dbus] field BYTE: value offset = %d value = %c %d\n",
	     f->offset, (char)*(f->buffer), (char)*(f->buffer));
	 break;
      case ECORE_DBUS_DATA_TYPE_BOOLEAN:
	 printf
	    ("[ecore_dbus] field BOOLEAN: value offset = %d value = %u\n",
	     f->offset, (unsigned int)*(f->buffer));
	 break;
      case ECORE_DBUS_DATA_TYPE_INT32:
	 printf
	    ("[ecore_dbus] field INT32: value offset = %d value = %d\n",
	     f->offset, (int)*(f->buffer));
	 break;
      case ECORE_DBUS_DATA_TYPE_UINT32:
	 printf
	    ("[ecore_dbus] field UINT32: value offset = %d value = %u\n",
	     f->offset, (unsigned int)*(f->buffer));
	 break;
      case ECORE_DBUS_DATA_TYPE_STRING:
	 printf
	    ("[ecore_dbus] field STRING: value offset = %d length = %d value = %s\n",
	     f->offset, (unsigned int)*(f->buffer), (f->buffer + 4));
	 break;
      case ECORE_DBUS_DATA_TYPE_OBJECT_PATH:
	 printf
	    ("[ecore_dbus] field PATH: value offset = %d length = %d value = %s\n",
	     f->offset, (unsigned int)*(f->buffer), (f->buffer + 4));
	 break;
      case ECORE_DBUS_DATA_TYPE_SIGNATURE:
	 printf
	    ("[ecore_dbus] field SIGNATURE: value offset = %d length = %d value = %s\n",
	     f->offset, (unsigned int)*(f->buffer), (f->buffer + 1));
	 break;
      case ECORE_DBUS_DATA_TYPE_ARRAY:
	   {
	      Ecore_DBus_Message_Field_Container *c;
	      Ecore_DBus_Message_Field           *s;

	      c = (Ecore_DBus_Message_Field_Container *)f;
	      printf
		 ("[ecore_dbus] field ARRAY: value offset = %d length = %u\n",
		  f->offset, (unsigned int)*(f->buffer));
	      printf("[ecore_dbus] * ARRAY elements begin *\n");
	      ecore_list_goto_first(c->values);
	      while ((s = ecore_list_next(c->values)))
		_ecore_dbus_message_field_print(s);
	      printf("[ecore_dbus] * ARRAY elements end *\n");
	   }
	 break;
      case ECORE_DBUS_DATA_TYPE_STRUCT:
	   {
	      Ecore_DBus_Message_Field_Container *c;
	      Ecore_DBus_Message_Field           *s;

	      c = (Ecore_DBus_Message_Field_Container *)f;
	      printf
		 ("[ecore_dbus] field STRUCT: value offset = %d\n",
		  f->offset);
	      printf("[ecore_dbus] * STRUCT elements begin *\n");
	      ecore_list_goto_first(c->values);
	      while ((s = ecore_list_next(c->values)))
		_ecore_dbus_message_field_print(s);
	      printf("[ecore_dbus] * STRUCT elements end *\n");
	   }
	 break;
      case ECORE_DBUS_DATA_TYPE_VARIANT:
	   {
	      Ecore_DBus_Message_Field_Variant *v;

	      v = (Ecore_DBus_Message_Field_Variant *)f;
	      printf
		 ("[ecore_dbus] field VARIANT: value offset = %d length: %d signature = %c\n",
		  f->offset, (char)*(f->buffer), (char)*(f->buffer + 1));
	      printf("[ecore_dbus] * VARIANT element begin *\n");
	      _ecore_dbus_message_field_print(v->value);
	      printf("[ecore_dbus] * VARIANT element end *\n");
	   }
	 break;
      case ECORE_DBUS_DATA_TYPE_INVALID:
      case ECORE_DBUS_DATA_TYPE_INT16:
      case ECORE_DBUS_DATA_TYPE_UINT16:
      case ECORE_DBUS_DATA_TYPE_INT64:
      case ECORE_DBUS_DATA_TYPE_UINT64:
      case ECORE_DBUS_DATA_TYPE_DOUBLE:
      case ECORE_DBUS_DATA_TYPE_STRUCT_BEGIN:
      case ECORE_DBUS_DATA_TYPE_STRUCT_END:
      case ECORE_DBUS_DATA_TYPE_DICT_ENTRY:
      case ECORE_DBUS_DATA_TYPE_DICT_ENTRY_BEGIN:
      case ECORE_DBUS_DATA_TYPE_DICT_ENTRY_END:
	 /* TODO */
	 printf("[ecore_dbus] field not implemented: %c\n", f->type);
	 break;
#if 0
      default:
	 printf("[ecore_dbus] field !UNKNOWN!: %c\n", f->type);
	 break;
#endif
     }
}

static void
_ecore_dbus_message_header_field_print(Ecore_DBus_Message_Field_Container *arr)
{
   Ecore_DBus_Message_Field_Container *s;
   static const char                  *header_fields[] = {
	"INVALID", "PATH", "INTERFACE", "MEMBER", "ERROR_NAME",
	"REPLY_SERIAL", "DESTINATION", "SENDER", "SIGNATURE"
   };

   ecore_list_goto_first(arr->values);
   while ((s = ecore_list_next(arr->values)))
     {
	Ecore_DBus_Message_Field_Byte *b;
	b = ecore_list_first(s->values);
	printf("[ecore_dbus] header field %s ", header_fields[b->value]);
	_ecore_dbus_message_field_print(ecore_list_last(s->values));
     }
}

static void
_ecore_dbus_message_print_raw(Ecore_DBus_Message *msg)
{
   int                 i;

   printf("[ecore_dbus] raw message:\n");
   for (i = 0; i < msg->length; i++)
     {
	if ((i % 8) == 0)
	  printf("\n");
	if ((msg->buffer[i] > 31) && (msg->buffer[i] < 128))
	  printf(" '%c' ", msg->buffer[i]);
	else
	  printf("0x%02x ", msg->buffer[i]);
     }
   printf("\n");
   printf("[ecore_dbus] end raw message\n");
}
