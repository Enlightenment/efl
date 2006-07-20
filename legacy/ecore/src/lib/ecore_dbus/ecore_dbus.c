/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include "Ecore.h"
#include "ecore_private.h"
#include "Ecore_DBus.h"
#include "Ecore_Con.h"
#include "ecore_dbus_private.h"

/********************************************************************************/
/* About									*/
/********************************************************************************/
/* Author: Jorge Luis Zapata							*/
/* Version: 0.2.1								*/
/********************************************************************************/
/* Todo										*/
/********************************************************************************/
/* free allocated data								*/
/* make other authentication mechanisms besides external			*/
/********************************************************************************/
/* Changelog									*/
/********************************************************************************/
/* 0.0 	usable interface							*/
/* 0.1 	change dbus spec version (0.11):					*/
/* 	different header format							*/
/* 	new type signature							*/
/*	header length = 8 byte multiple 					*/
/*	paddings value must be null						*/
/*	body need not to end in a 8 byte boundary				*/
/*	new data type: variant,signature,dict					*/
/*	ecore_oldlist cant join two lists so is difficult to handle compound	*/
/*	data types (variant,struct,dict,array) in a stack way			*/
/* 0.2 	change again the spec version (0.8)					*/
/*	i realized that the first version was correct, when i read the spec	*/
/*	for ecore_dbus 0.1 i was reading a previous version :(			*/
/*	put again the data type byte in each marshaled data			*/
/*										*/
/* 29-03-05									*/
/* 0.2.1 some segfault fixes, new tests						*/

/* global variables  */

EAPI int                 ECORE_DBUS_EVENT_SERVER_ADD = 0;
EAPI int                 ECORE_DBUS_EVENT_SERVER_DEL = 0;
EAPI int                 ECORE_DBUS_EVENT_SERVER_DATA = 0;

/* private function declaration */

/* length functions */
static void          _ecore_dbus_message_append_length(unsigned char **buf, unsigned int old_length, unsigned int size);
static void          _ecore_dbus_message_increase_length(unsigned char **buf, unsigned int new_length);
/* padding functions */
static void          _ecore_dbus_message_4byte_padding(unsigned char **buf, unsigned int *old_length);
static void          _ecore_dbus_message_8byte_padding(unsigned char **buf, unsigned int *old_length);
static void          _ecore_dbus_message_skip_4byte_padding(unsigned int *old_length);
/*static void          _ecore_dbus_message_skip_8byte_padding(unsigned int *old_length);*/
/* appending/reading functions */
static void          _ecore_dbus_message_append_nbytes(unsigned char *buf, unsigned char *data, unsigned int data_length);
static void          _ecore_dbus_message_append_byte(unsigned char *buf, unsigned char c);
static void          _ecore_dbus_message_append_uint32(unsigned char *buf, unsigned long int i);
static unsigned char _ecore_dbus_message_read_byte(unsigned char *buf);
static unsigned long _ecore_dbus_message_read_uint32(unsigned char *buf);
/* message functions */
static Ecore_DBus_Message       *_ecore_dbus_message_new(Ecore_DBus_Server *svr);
static Ecore_DBus_Message_Field *_ecore_dbus_message_new_field(Ecore_DBus_Data_Type type, unsigned int offset);
static void                      _ecore_dbus_message_free(void *data, void *ev);
/* marshal functions */
static Ecore_DBus_Message_Field *_ecore_dbus_message_marshal_byte(unsigned char **buf, unsigned int *old_length, unsigned char c);
/*
static Ecore_DBus_Message_Field *_ecore_dbus_message_marshal_boolean(unsigned char **buf, unsigned int *old_length, uint32_t i);
static Ecore_DBus_Message_Field *_ecore_dbus_message_marshal_int16(unsigned char **buf, unsigned int *old_length, int16_t i);
static Ecore_DBus_Message_Field *_ecore_dbus_message_marshal_uint16(unsigned char **buf, unsigned int *old_length, uint16_t i);
static Ecore_DBus_Message_Field *_ecore_dbus_message_marshal_int32(unsigned char **buf, unsigned int *old_length, int32_t i);
*/
static Ecore_DBus_Message_Field *_ecore_dbus_message_marshal_uint32(unsigned char **buf, unsigned int *old_length, uint32_t i);
/*
static Ecore_DBus_Message_Field *_ecore_dbus_message_marshal_int64(unsigned char **buf, unsigned int *old_length, int64_t i);
static Ecore_DBus_Message_Field *_ecore_dbus_message_marshal_uint64(unsigned char **buf, unsigned int *old_length, uint64_t i);
static Ecore_DBus_Message_Field *_ecore_dbus_message_marshal_double(unsigned char **buf, unsigned int *old_length, double i);
*/
static Ecore_DBus_Message_Field *_ecore_dbus_message_marshal_string(unsigned char **buf, unsigned int *old_length, char *str);
static Ecore_DBus_Message_Field *_ecore_dbus_message_marshal_object_path(unsigned char **buf, unsigned int *old_length, char *str);
/*
static Ecore_DBus_Message_Field *_ecore_dbus_message_marshal_signature(unsigned char **buf, unsigned int *old_length, char *str);
*/
/* 
 * FIXME: These aren't correct !
static Ecore_DBus_Message_Field *_ecore_dbus_message_marshal_array(unsigned char **msg, unsigned int *old_length);
static Ecore_DBus_Message_Field *_ecore_dbus_message_marshal_variant(unsigned char **msg, unsigned int *old_length);
static Ecore_DBus_Message_Field *_ecore_dbus_message_marshal_struct(unsigned char **msg, unsigned int *old_length);
static Ecore_DBus_Message_Field *_ecore_dbus_message_marshal_dict_entry(unsigned char **msg, unsigned int *old_length);
*/
static Ecore_DBus_Message_Field *_ecore_dbus_message_marshal_data(unsigned char **buf, unsigned int *old_length, Ecore_DBus_Data_Type type, void *data);
static Ecore_DBus_Message_Field *_ecore_dbus_message_marshal_custom_header(unsigned char **buf, unsigned int *old_length, unsigned int code, Ecore_DBus_Data_Type type, void *data);
/* unmarshal functions */
/*
static Ecore_DBus_Message_Field *_ecore_dbus_message_unmarshal_byte(unsigned char *buf, unsigned int *old_length);
static Ecore_DBus_Message_Field *_ecore_dbus_message_unmarshal_boolean(unsigned char *buf, unsigned int *old_length);
static Ecore_DBus_Message_Field *_ecore_dbus_message_unmarshal_int16(unsigned char *buf, unsigned int *old_length);
static Ecore_DBus_Message_Field *_ecore_dbus_message_unmarshal_uint16(unsigned char *buf, unsigned int *old_length);
*/
static Ecore_DBus_Message_Field *_ecore_dbus_message_unmarshal_int32(unsigned char *buf, unsigned int *old_length);
static Ecore_DBus_Message_Field *_ecore_dbus_message_unmarshal_uint32(unsigned char *buf, unsigned int *old_length);
/*
static Ecore_DBus_Message_Field *_ecore_dbus_message_unmarshal_int64(unsigned char *buf, unsigned int *old_length);
static Ecore_DBus_Message_Field *_ecore_dbus_message_unmarshal_uint64(unsigned char *buf, unsigned int *old_length);
static Ecore_DBus_Message_Field *_ecore_dbus_message_unmarshal_double(unsigned char *buf, unsigned int *old_length);
*/
static Ecore_DBus_Message_Field *_ecore_dbus_message_unmarshal_string(unsigned char *buf, unsigned int *old_length);
static Ecore_DBus_Message_Field *_ecore_dbus_message_unmarshal_object_path(unsigned char *buf, unsigned int *old_length);
/*
static Ecore_DBus_Message_Field *_ecore_dbus_message_unmarshal_signature(unsigned char *buf, unsigned int *old_length);
*/
/*
 * FIXME: These aren't correct!
static Ecore_DBus_Message_Field *_ecore_dbus_message_unmarshal_array(unsigned char *buf, unsigned int *old_length);
static Ecore_DBus_Message_Field *_ecore_dbus_message_unmarshal_variant(unsigned char *buf, unsigned int *old_length);
static Ecore_DBus_Message_Field *_ecore_dbus_message_unmarshal_struct(unsigned char *buf, unsigned int *old_length);
static Ecore_DBus_Message_Field *_ecore_dbus_message_unmarshal_dict_entry(unsigned char *buf, unsigned int *old_length);
static Ecore_DBus_Message_Field *_ecore_dbus_message_unmarshal_data(unsigned char *buf, unsigned int *old_length);
*/
static Ecore_DBus_Message_Field *_ecore_dbus_message_unmarshal_custom_header(unsigned char *buf, unsigned int *old_length);
static Ecore_DBus_Message       *_ecore_dbus_message_unmarshal(Ecore_DBus_Server *svr, unsigned char *message);
/* header functions */
static void                      _ecore_dbus_message_common_header(Ecore_DBus_Message *msg, int type, int flags);
static void                     *_ecore_dbus_get_field(unsigned char *buf, Ecore_DBus_Message_Field *f);
/* helper functions */
static char        *_ecore_dbus_getuid(void);
static char        *_ecore_dbus_hex_encode(char *src_str);
/* auth functions */
unsigned char      *_ecore_dbus_auth_external(void *data);
/* con functions */
static int          _ecore_dbus_event_server_add(void *data, int ev_type, void *ev);
static int          _ecore_dbus_event_server_del(void *data, int ev_type, void *ev);
static int          _ecore_dbus_event_server_data(void *data, int ev_type, void *ev);
/* printing functions */
static Ecore_DBus_Message_Field *_ecore_dbus_message_print_field(Ecore_DBus_Message_Field *f, unsigned char *buf);
static Ecore_DBus_Message_Field *_ecore_dbus_message_print_header_field(Ecore_DBus_Message_Field *f, unsigned char *buf);
/*static void                      _ecore_dbus_message_print_fields(Ecore_DBus_Message_Field *f);*/
/*static void                      _ecore_dbus_message_print_raw(unsigned char *msg, unsigned int msg_len);*/

/* local variables  */

static const Ecore_DBus_Auth auths[] = {
   {"EXTERNAL", 1, {_ecore_dbus_auth_external, NULL, NULL, NULL, NULL}},
   {"MAGIC_COOKIE", 0, {NULL, NULL, NULL, NULL, NULL}},
   {"DBUS_COOKIE_SHA1", 0, {NULL, NULL, NULL, NULL, NULL}},
   {"KERBEROS_V4", 0, {NULL, NULL, NULL, NULL, NULL}},
   {"SKEY", 0, {NULL, NULL, NULL, NULL, NULL}},
};

static int                  init_count = 0;
static Ecore_List2         *servers = NULL;
static Ecore_Event_Handler *handler[3];

/* public functions */
EAPI int
ecore_dbus_init(void)
{
   int i = 0;

   if (++init_count != 1) return init_count;

   ecore_con_init();

   ECORE_DBUS_EVENT_SERVER_ADD = ecore_event_type_new();
   ECORE_DBUS_EVENT_SERVER_DEL = ecore_event_type_new();
   ECORE_DBUS_EVENT_SERVER_DATA = ecore_event_type_new();

   handler[i++] = ecore_event_handler_add(ECORE_CON_EVENT_SERVER_ADD,
					  _ecore_dbus_event_server_add, NULL);
   handler[i++] = ecore_event_handler_add(ECORE_CON_EVENT_SERVER_DEL,
					  _ecore_dbus_event_server_del, NULL);
   handler[i++] = ecore_event_handler_add(ECORE_CON_EVENT_SERVER_DATA,
					  _ecore_dbus_event_server_data, NULL);

   return init_count;
}

EAPI int
ecore_dbus_shutdown(void)
{
   int i = 0;

   if (--init_count != 0) return init_count;

   for (i = 0; i < 3; i++)
     ecore_event_handler_del(handler[i]);

   ecore_con_shutdown();

   return init_count;
}

EAPI Ecore_DBus_Server *
ecore_dbus_server_connect(Ecore_DBus_Type compl_type, char *name, int port,
			  const void *data)
{
   Ecore_DBus_Server  *svr;
   Ecore_DBus_Type     type;
   Ecore_Con_Type      extra = 0;

   svr = calloc(1, sizeof(Ecore_DBus_Server));
   if (!svr) return NULL;
   type = compl_type;
   switch (type)
     {
#if 0
     case ECORE_DBUS_BUS_SESSION:
	svr->server =
	   ecore_con_server_connect(ECORE_CON_LOCAL_USER | extra, name, port, svr);
	break;
#endif
     case ECORE_DBUS_BUS_SYSTEM:
	svr->server =
	   ecore_con_server_connect(ECORE_CON_LOCAL_SYSTEM | extra, name, port, svr);
	break;
     default:
	free(svr);
	return NULL;
     }
   if (!svr->server)
     {
	printf("Couldn't connect to server\n");
	free(svr);
	return NULL;
     }
   svr->authenticated = 0;
   svr->cnt_msg = 0;
   svr->auth_type = -1;
   svr->auth_type_transaction = 0;
   servers = _ecore_list2_append(servers, svr);

   return svr;
}

EAPI void
ecore_dbus_server_del(Ecore_DBus_Server *svr)
{
   if (svr->server) ecore_con_server_del(svr->server);
   servers = _ecore_list2_remove(servers, svr);
   free(svr);
}

EAPI int
ecore_dbus_server_send(Ecore_DBus_Server *svr, char *command, int length)
{
   int                 ret;

   ret = ecore_con_server_send(svr->server, command, length);
   printf("[ecore_dbus] ecore_dbus_server: %p ecore_con_server: %p sent %d of %d bytes\n",
	  svr, svr->server, ret, length);
   return ret;
}

EAPI unsigned int
ecore_dbus_message_new_method_call(Ecore_DBus_Server *svr, char *service,
				   char *path, char *interface, char *method,
				   char *fmt, ...)
{
   unsigned long       serial;
   va_list             ap;

   Ecore_DBus_Message_Field *f;

   /* init message */
   Ecore_DBus_Message *msg = _ecore_dbus_message_new(svr);

   /* common header */
   _ecore_dbus_message_common_header(msg, ECORE_DBUS_MESSAGE_TYPE_METHOD_CALL, 0);
   /* custom header */
   if (path)
     {
	f = _ecore_dbus_message_marshal_custom_header(&msg->header, &msg->hpos,
						      1,
						      ECORE_DBUS_DATA_TYPE_OBJECT_PATH,
						      path);
	msg->header_fields = _ecore_list2_append(msg->header_fields, f);
     }

   if (interface)
     {
	f = _ecore_dbus_message_marshal_custom_header(&msg->header, &msg->hpos,
						      2,
						      ECORE_DBUS_DATA_TYPE_STRING,
						      interface);
	msg->header_fields = _ecore_list2_append(msg->header_fields, f);
     }
   if (method)
     {
	f = _ecore_dbus_message_marshal_custom_header(&msg->header, &msg->hpos,
						      3,
						      ECORE_DBUS_DATA_TYPE_STRING,
						      method);
	msg->header_fields = _ecore_list2_append(msg->header_fields, f);
     }
   if (service)
     {
	f = _ecore_dbus_message_marshal_custom_header(&msg->header, &msg->hpos,
						      6,
						      ECORE_DBUS_DATA_TYPE_STRING,
						      service);
	msg->header_fields = _ecore_list2_append(msg->header_fields, f);
     }
   if (fmt)
     {
	f = _ecore_dbus_message_marshal_custom_header(&msg->header, &msg->hpos,
						      8,
						      ECORE_DBUS_DATA_TYPE_STRING,
						      fmt);
	msg->header_fields = _ecore_list2_append(msg->header_fields, f);
     }
   _ecore_dbus_message_8byte_padding(&msg->header, &msg->hpos);
   /* header length */
   *(msg->header + 4) = msg->hlength = msg->hpos;
   /* message body */
   if (fmt)
     {
	va_start(ap, fmt);
	while (*fmt)
	  {
	     switch (*fmt)
	       {
		case ECORE_DBUS_DATA_TYPE_BYTE:
		   f = _ecore_dbus_message_marshal_byte(&msg->body, &msg->bpos,
							va_arg(ap, int));
		   msg->body_fields = _ecore_list2_append(msg->body_fields, f);
		   break;
		case ECORE_DBUS_DATA_TYPE_UINT32:
		   f = _ecore_dbus_message_marshal_uint32(&msg->body, &msg->bpos,
							  va_arg(ap, unsigned long));
		   msg->body_fields = _ecore_list2_append(msg->body_fields, f);
		   break;
		case ECORE_DBUS_DATA_TYPE_STRING:
		   f = _ecore_dbus_message_marshal_string(&msg->body, &msg->bpos,
							  (char *)va_arg(ap, char *));
		   msg->body_fields = _ecore_list2_append(msg->body_fields, f);
		   break;
		case ECORE_DBUS_DATA_TYPE_OBJECT_PATH:
		   f = _ecore_dbus_message_marshal_object_path(&msg->body, &msg->bpos,
							       (char *)va_arg(ap, char *));
		   msg->body_fields = _ecore_list2_append(msg->body_fields, f);
		   break;
		case ECORE_DBUS_DATA_TYPE_INT32:
		case ECORE_DBUS_DATA_TYPE_BOOLEAN:
		case ECORE_DBUS_DATA_TYPE_INT64:
		case ECORE_DBUS_DATA_TYPE_UINT64:
		case ECORE_DBUS_DATA_TYPE_DOUBLE:
		case ECORE_DBUS_DATA_TYPE_ARRAY:
		case ECORE_DBUS_DATA_TYPE_DICT_ENTRY:
		case ECORE_DBUS_DATA_TYPE_INVALID:
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
   *(unsigned int *)(msg->header + 8) = msg->blength = msg->bpos;
   /* show message */
   /*ecore_dbus_message_print(msg);
    * _ecore_dbus_message_print_raw(msg->header, msg->hlength);
    * _ecore_dbus_message_print_raw(msg->body, msg->blength); */
   /* send message */
   ecore_dbus_server_send(svr, (char *)msg->header, msg->hlength);
   if (msg->body)
      ecore_dbus_server_send(svr, (char *)msg->body, msg->blength);

   serial = msg->serial;
   _ecore_dbus_message_free(NULL, msg);
   return serial;
}

EAPI void
ecore_dbus_message_print(Ecore_DBus_Message *msg)
{
   Ecore_DBus_Message_Field *f;
   static const char  *msg_type[] = {
	"INVALID", "METHOD_CALL", "METHOD_RETURN", "ERROR", "SIGNAL"
   };

   printf("[ecore_dbus] per field message:\n");
   printf("[ecore_dbus] header fields:\n");
   /* header common fields */
   printf("[ecore_dbus] header endianess	: %c\n", msg->byte_order);
   printf("[ecore_dbus] header type     	: %s\n", msg_type[msg->type]);
   printf("[ecore_dbus] header flags    	: %c\n", msg->flags + 48);
   printf("[ecore_dbus] header protocol 	: %c\n", msg->protocol + 48);
   printf("[ecore_dbus] header hlength  	: %u\n",
	 *(unsigned int *)(msg->header + 4));
   printf("[ecore_dbus] header blength  	: %lu\n", msg->blength);
   printf("[ecore_dbus] header serial   	: %lu\n", msg->serial);

   /* header custom fields */
   f = msg->header_fields;
   while (f)
     {
	f = _ecore_dbus_message_print_header_field(f, msg->header);
     }
   /* body fields */
   printf("[ecore_dbus] body fields:\n");
   f = msg->body_fields;
   while (f)
     {
	f = _ecore_dbus_message_print_field(f, msg->body);
     }
   printf("[ecore_dbus] end per field message\n");
}

EAPI void *
ecore_dbus_get_header_field(Ecore_DBus_Message *m,
			    Ecore_DBus_Message_Field *mf,
			    Ecore_DBus_Message_Header_Field hft)
{
   Ecore_List2      *l;

   for (l = (Ecore_List2 *)mf; l; l = l->next)
     if (((Ecore_DBus_Message_Field *)l)->hfield == hft)
       return _ecore_dbus_get_field(m->header, (Ecore_DBus_Message_Field *)l);
   return NULL;
}

EAPI void *
ecore_dbus_get_body_field(Ecore_DBus_Message *m, Ecore_DBus_Message_Field *mf,
			  unsigned int pos)
{
   Ecore_List2      *l;
   unsigned int      i = 0;

   for (l = (Ecore_List2 *) mf; l; l = l->next)
     {
	if (i == pos)
	  return _ecore_dbus_get_field(m->body, (Ecore_DBus_Message_Field *)l);
	i++;
     }
   return NULL;
}

/* private functions */

/* length functions */

static void
_ecore_dbus_message_append_length(unsigned char **buf,
				  unsigned int old_length, unsigned int size)
{
   *buf = realloc(*buf, old_length + size);
   memset(*buf + old_length, 0, size);
}

static void
_ecore_dbus_message_increase_length(unsigned char **buf,
				    unsigned int new_length)
{
   *buf = realloc(*buf, new_length);
}

/* padding functions */

static void
_ecore_dbus_message_4byte_padding(unsigned char **buf, unsigned int *old_length)
{
   unsigned int        padding;

   padding = *old_length % 4;
   if (padding != 0)
     {
	padding = 4 - padding;
	_ecore_dbus_message_append_length(buf, *old_length, padding);
     }
   *old_length += padding;
}

static void
_ecore_dbus_message_8byte_padding(unsigned char **buf, unsigned int *old_length)
{
   unsigned int        padding;

   padding = *old_length % 8;
   if (padding != 0)
     {
	padding = 8 - padding;
	_ecore_dbus_message_append_length(buf, *old_length, padding);
     }
   *old_length += padding;
}

static void
_ecore_dbus_message_skip_4byte_padding(unsigned int *old_length)
{
   unsigned int        padding;

   padding = *old_length % 4;
   if (padding != 0)
      *old_length += 4 - padding;
   return;
}


/*static void
_ecore_dbus_message_skip_8byte_padding(unsigned int *old_length)
{
   unsigned int        padding;

   padding = *old_length % 8;
   if (padding != 0)
      *old_length += 8 - padding;
   return;
}
*/

/* appending/reading functions */

static void
_ecore_dbus_message_append_nbytes(unsigned char *buf, unsigned char *data,
				  unsigned int data_length)
{
   memcpy(buf, data, data_length);
}

static void
_ecore_dbus_message_append_byte(unsigned char *buf, unsigned char c)
{
   *buf = c;
}

static void
_ecore_dbus_message_append_uint32(unsigned char *buf, unsigned long int i)
{
   unsigned char      *c;

   c = (unsigned char *)&i;
   *(buf + 0) = *(c + 0);
   *(buf + 1) = *(c + 1);
   *(buf + 2) = *(c + 2);
   *(buf + 3) = *(c + 3);
}

static unsigned char
_ecore_dbus_message_read_byte(unsigned char *buf)
{
   return *(unsigned char *)buf;
}

static unsigned long
_ecore_dbus_message_read_uint32(unsigned char *buf)
{
   return *(unsigned long *)buf;
}

/* message functions */

static Ecore_DBus_Message *
_ecore_dbus_message_new(Ecore_DBus_Server *svr)
{
   Ecore_DBus_Message *msg;
  
   msg = calloc(1, sizeof(Ecore_DBus_Message));
   if (!msg) return NULL;

   msg->ref_server = svr;
   msg->body_fields = NULL;
   msg->header_fields = NULL;
   msg->hpos = 0;
   msg->bpos = 0;
   msg->header = NULL;
   msg->body = NULL;
   msg->hlength = 0;
   msg->blength = 0;
   return msg;
}

static Ecore_DBus_Message_Field *
_ecore_dbus_message_new_field(Ecore_DBus_Data_Type type, unsigned int offset)
{
   Ecore_DBus_Message_Field *f;

   f = malloc(sizeof(Ecore_DBus_Message_Field));
   if (!f) return NULL;
   f->offset = offset;
   f->type = type;
   f->count = 0;
   return f;
}

static void
_ecore_dbus_message_free(void *data, void *ev)
{
   Ecore_DBus_Message *msg;
   Ecore_List2        *l;
   Ecore_List2        *next;

   msg = ev;

   l = (Ecore_List2 *)msg->body_fields;
   while (l)
     {
	next = l->next;
	free(l);
	l = next;
     }
   l = (Ecore_List2 *) msg->header_fields;
   while (l)
     {
	next = l->next;
	free(l);
	l = next;
     }
   free(msg->header);
   free(msg->body);
   free(msg);
}

/* marshal functions */

static Ecore_DBus_Message_Field *
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

static Ecore_DBus_Message_Field *
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
   _ecore_dbus_message_4byte_padding(buf, old_length);
   _ecore_dbus_message_increase_length(buf, *old_length + 4);
   _ecore_dbus_message_append_uint32(*buf + *old_length, i);
   f = _ecore_dbus_message_new_field(ECORE_DBUS_DATA_TYPE_UINT32, *old_length);
   *old_length += 4;
   return f;
}

static Ecore_DBus_Message_Field *
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
   _ecore_dbus_message_4byte_padding(buf, old_length);
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

static Ecore_DBus_Message_Field *
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
   _ecore_dbus_message_4byte_padding(buf, old_length);
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

#if 0
static Ecore_DBus_Message_Field *
_ecore_dbus_message_marshal_array(unsigned char **msg, unsigned int *old_length)
{
   /* TODO: complete this */
   Ecore_DBus_Message_Field *f;

   _ecore_dbus_message_8byte_padding(msg, old_length);
   /* for the array length value */
   _ecore_dbus_message_increase_length(msg, *old_length + 4);
   f = _ecore_dbus_message_new_field(ECORE_DBUS_DATA_TYPE_ARRAY, *old_length);
   *old_length += 4;
   return f;
}
#endif

static Ecore_DBus_Message_Field *
_ecore_dbus_message_marshal_data(unsigned char **buf, unsigned int *old_length,
				 Ecore_DBus_Data_Type type, void *data)
{
   Ecore_DBus_Message_Field *f = NULL;

   switch (type)
     {
     case ECORE_DBUS_DATA_TYPE_UINT32:
	f = _ecore_dbus_message_marshal_uint32(buf, old_length,
					       *(unsigned long *)data);
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
     case ECORE_DBUS_DATA_TYPE_DICT_ENTRY:
#if 0
     default:
#endif
	printf("[ecore_dbus] unknown/unhandled data type %c\n", type);
	break;
     }
   return f;
}

static Ecore_DBus_Message_Field *
_ecore_dbus_message_marshal_custom_header(unsigned char **buf,
					  unsigned int *old_length,
					  unsigned int code,
					  Ecore_DBus_Data_Type type, void *data)
{
   Ecore_DBus_Message_Field *f;

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

/* unmarshal functions */

static Ecore_DBus_Message_Field *
_ecore_dbus_message_unmarshal_int32(unsigned char *buf,
				    unsigned int *old_length)
{
   Ecore_DBus_Message_Field *f;

   _ecore_dbus_message_skip_4byte_padding(old_length);
   f = _ecore_dbus_message_new_field(ECORE_DBUS_DATA_TYPE_INT32, *old_length);
   *old_length += 4;
   return f;
}

static Ecore_DBus_Message_Field *
_ecore_dbus_message_unmarshal_uint32(unsigned char *buf,
				     unsigned int *old_length)
{
   Ecore_DBus_Message_Field *f;

   _ecore_dbus_message_skip_4byte_padding(old_length);
   f = _ecore_dbus_message_new_field(ECORE_DBUS_DATA_TYPE_UINT32, *old_length);
   *old_length += 4;
   return f;
}

static Ecore_DBus_Message_Field *
_ecore_dbus_message_unmarshal_string(unsigned char *buf,
				     unsigned int *old_length)
{
   Ecore_DBus_Message_Field *f;
   unsigned long             str_len;

   _ecore_dbus_message_skip_4byte_padding(old_length);
   f = _ecore_dbus_message_new_field(ECORE_DBUS_DATA_TYPE_STRING, *old_length);
   str_len = _ecore_dbus_message_read_uint32(buf + *old_length);
   *old_length += 4;
   *old_length += str_len + 1;
   return f;
}

static Ecore_DBus_Message_Field *
_ecore_dbus_message_unmarshal_object_path(unsigned char *buf, unsigned int *old_length)
{
   Ecore_DBus_Message_Field *f;
   unsigned long             str_len;

   _ecore_dbus_message_skip_4byte_padding(old_length);
   f = _ecore_dbus_message_new_field(ECORE_DBUS_DATA_TYPE_OBJECT_PATH,
				     *old_length);
   str_len = _ecore_dbus_message_read_uint32(buf + *old_length);
   *old_length += 4;
   *old_length += str_len + 1;
   return f;
}

static Ecore_DBus_Message_Field *
_ecore_dbus_message_unmarshal_data(unsigned char *buf, unsigned int *old_length)
{
   Ecore_DBus_Message_Field *f = NULL;
   unsigned char             type;
  
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
     case ECORE_DBUS_DATA_TYPE_BYTE:
     case ECORE_DBUS_DATA_TYPE_BOOLEAN:
     case ECORE_DBUS_DATA_TYPE_INT64:
     case ECORE_DBUS_DATA_TYPE_UINT64:
     case ECORE_DBUS_DATA_TYPE_DOUBLE:
     case ECORE_DBUS_DATA_TYPE_ARRAY:
     case ECORE_DBUS_DATA_TYPE_DICT_ENTRY:
     case ECORE_DBUS_DATA_TYPE_INVALID:
#if 0
     default:
#endif
	printf("[ecore_dbus] unknown/unhandled data type %c\n", type);
	break;
     }
   return f;
}

static Ecore_DBus_Message_Field *
_ecore_dbus_message_unmarshal_custom_header(unsigned char *buf,
					    unsigned int *old_length)
{
   Ecore_DBus_Message_Field *f;
   unsigned int              code;

   /* get header field name at cur_length */
   code = _ecore_dbus_message_read_byte(buf + *old_length);
   *old_length += 1;
   /* unmarshal header field data */
   f = _ecore_dbus_message_unmarshal_data(buf, old_length);
   f->hfield = code;
   return f;
}

static Ecore_DBus_Message *
_ecore_dbus_message_unmarshal(Ecore_DBus_Server *svr, unsigned char *message)
{
   Ecore_DBus_Message_Field *f;

   /* init */
   Ecore_DBus_Message *msg = _ecore_dbus_message_new(svr);

   printf("[ecore_dbus] unmarshaling\n");

   /* message header */

   /* common fields  */
   msg->byte_order = *(message + 0);
   msg->type = *(message + 1);
   msg->flags = *(message + 2);
   msg->protocol = *(message + 3);
   msg->hlength = *(unsigned long *)(message + 4);
   msg->blength = *(unsigned long *)(message + 8);
   msg->serial = *(unsigned long *)(message + 12);
   if (msg->type == ECORE_DBUS_MESSAGE_TYPE_INVALID)
     {
	printf("[ecore_dbus] message type invalid\n");
	return NULL;
     }
   /* memcpy the header part */
   _ecore_dbus_message_increase_length(&msg->header, msg->hlength);
   _ecore_dbus_message_append_nbytes(msg->header, message, msg->hlength);
   msg->hpos += 16;
   /* custom fields */
   while ((msg->hpos + 8 - (msg->hpos % 8)) < msg->hlength)
     {
	f = _ecore_dbus_message_unmarshal_custom_header(message, &msg->hpos);
	msg->header_fields = _ecore_list2_append(msg->header_fields, f);
     }
   msg->hpos = msg->hlength;

   /* message body */
   message += msg->hlength;
   _ecore_dbus_message_increase_length(&msg->body, msg->blength);
   _ecore_dbus_message_append_nbytes(msg->body, message, msg->blength);
   while (msg->bpos < msg->blength)
     {
	f = _ecore_dbus_message_unmarshal_data(message, &msg->bpos);
	msg->body_fields = _ecore_list2_append(msg->body_fields, f);

     }
   return msg;
}

/* header functions */

static void
_ecore_dbus_message_common_header(Ecore_DBus_Message *msg, int type, int flags)
{
   /* the body,header length arent filled only alloc */
   _ecore_dbus_message_append_length(&msg->header, msg->hpos, 16);
   /* endiannes (1) */
   msg->header[0] = msg->byte_order = 'l';
   /* type (1) */
   msg->header[1] = msg->type = (char)type;
   /* flags (1) 0x1 = no reply expected, 0x2 auto activiation */
   msg->header[2] = msg->flags = 0x0;
   /* protocol (1) */
   msg->header[3] = msg->protocol = 0x0;
   /* autoincrement the client_serial (0 is invalid) */
   msg->ref_server->cnt_msg++;

   *(msg->header + 12) = msg->serial = msg->ref_server->cnt_msg;
   msg->hpos = 16;
}

static void *
_ecore_dbus_get_field(unsigned char *buf, Ecore_DBus_Message_Field *f)
{
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
	return buf + f->offset;
	break;
     case ECORE_DBUS_DATA_TYPE_STRING:
     case ECORE_DBUS_DATA_TYPE_OBJECT_PATH:
	return buf + f->offset + 4;
	break;
     case ECORE_DBUS_DATA_TYPE_INVALID:
     case ECORE_DBUS_DATA_TYPE_SIGNATURE:
     case ECORE_DBUS_DATA_TYPE_ARRAY:
     case ECORE_DBUS_DATA_TYPE_VARIANT:
     case ECORE_DBUS_DATA_TYPE_STRUCT:
     case ECORE_DBUS_DATA_TYPE_DICT_ENTRY:
#if 0
     default:
#endif
	printf("[ecore_dbus] unknown/unhandled data type %c\n", f->type);
	break;

     }
   return NULL;
}

/* helper functions */

static char *
_ecore_dbus_getuid(void)
{
   /* this calculation is from comp.lang.c faq */
#define MAX_LONG_LEN ((sizeof (long) * 8 + 2) / 3 + 1)	/* +1 for '-' */
   int                 len;
   char               *uid;
   char               *tmp;

   tmp = (char *)malloc(MAX_LONG_LEN);
   len = snprintf(tmp, MAX_LONG_LEN, "%ld", (long) getuid());
   uid = (char *)malloc(len + 1);
   uid = memcpy(uid, tmp, len);
   uid[len] = '\0';

   free(tmp);
   return uid;
}

/* encodes a string into a string of hex values	*/
/* each byte is two hex digits			*/
static char *
_ecore_dbus_hex_encode(char *src_str)
{
   const char          hexdigits[16] = {
      '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
      'a', 'b', 'c', 'd', 'e', 'f'
   };
   char               *enc_str;
   char               *p;
   char               *end;
   int                 len;
   int                 i = 0;

   len = strlen(src_str);
   p = src_str;
   end = p + len;

   enc_str = malloc(2 * len + 1);
   while (p != end)
     {
	enc_str[i] = hexdigits[(*p >> 4)];
	i++;
	enc_str[i] = hexdigits[(*p & 0x0f)];
	i++;
	p++;
     }
   enc_str[i] = '\0';
   return enc_str;
}

/* auth functions */

unsigned char *
_ecore_dbus_auth_external(void *data)
{
   char          *uid, *enc_uid, *msg;

   uid = _ecore_dbus_getuid();
   enc_uid = _ecore_dbus_hex_encode(uid);
   free(uid);
   msg = malloc(strlen(enc_uid) + 17);
   sprintf(msg, "AUTH EXTERNAL %s\r\n", enc_uid);
   free(enc_uid);
   return (unsigned char *)msg;
}

/* con functions */

static int
_ecore_dbus_event_server_add(void *data, int ev_type, void *ev)
{
   Ecore_DBus_Event_Server_Add *e;
   Ecore_DBus_Server           *svr;

   e = ev;
   if (!_ecore_list2_find(servers, ecore_con_server_data_get(e->server))) return 1;

   svr = ecore_con_server_data_get(e->server);
   ecore_dbus_server_send(svr, "\0", 1);
   ecore_dbus_server_send(svr, "AUTH\r\n", 6);
   printf("[ecore_dbus] begining auth process\n");
   return 0;
}

static int
_ecore_dbus_event_server_del(void *udata, int ev_type, void *ev)
{
   Ecore_Con_Event_Server_Del  *e;
   Ecore_DBus_Server           *svr;
   Ecore_DBus_Event_Server_Del *e2;

   e = ev;
   if (!_ecore_list2_find(servers, ecore_con_server_data_get(e->server))) return 1;

   svr = ecore_con_server_data_get(e->server);
   e2 = calloc(1, sizeof(Ecore_DBus_Event_Server_Del));
   if (e2)
     {
	e2->server = svr;
	ecore_event_add(ECORE_DBUS_EVENT_SERVER_DEL, e2, NULL, NULL);
     }
   return 0;
}

static int
_ecore_dbus_event_server_data(void *udata, int ev_type, void *ev)
{
   Ecore_Con_Event_Server_Data *e;
   Ecore_DBus_Server           *svr;
   Ecore_DBus_Event_Server_Add *svr_add;

   e = ev;
   if (!_ecore_list2_find(servers, ecore_con_server_data_get(e->server))) return 1;

   svr = ecore_con_server_data_get(e->server);

   if (!svr->authenticated)
     {
	/* authentication protocol */
	const Ecore_DBus_Auth *auth;
	Ecore_DBus_Auth_Transaction trans;

	if (!strncmp(e->data, "OK", 2))
	  {
	     printf("[ecore_dbus] auth type %s successful\n", auths[svr->auth_type].name);
	     ecore_dbus_server_send(svr, "BEGIN\r\n", 7);
	     svr->authenticated = 1;
	     svr_add = malloc(sizeof(Ecore_DBus_Event_Server_Add));
	     svr_add->server = svr;
	     ecore_event_add(ECORE_DBUS_EVENT_SERVER_ADD, svr_add, NULL, NULL);
	  }
	else if (!strncmp(e->data, "DATA", 4))
	  {
	     printf("[ecore_dbus] requiring data (unavailable)\n");
	  }
	else if (!strncmp(e->data, "ERROR", 5))
	  {
	     printf("[ecore_dbus] auth process error\n");
	  }
	else if (!strncmp(e->data, "REJECTED", 8))
	  {
	     unsigned char      *msg;

	     if (svr->auth_type >= 0)
	       printf("[ecore_dbus] auth type %s rejected\n", auths[svr->auth_type].name);
	     svr->auth_type++;
	     auth = &auths[svr->auth_type];
	     trans = auth->transactions[0];
	     printf("[ecore_dbus] auth type %s started\n", auth->name);
	     msg = trans(NULL);
	     ecore_dbus_server_send(svr, (char *)msg, strlen((char *)msg));
	     free(msg);
	  }
     }
   else
     {
	/* message protocol */
	Ecore_DBus_Message *msg;
	unsigned int        offset = 0;

	printf("[ecore_dbus] received server data, %d bytes\n", e->size);
	while (e->size)
	  {
	     msg = _ecore_dbus_message_unmarshal(svr, (unsigned char *)(e->data) + offset);
	     if (msg == NULL) break;
	     offset += msg->hlength + msg->blength;
	     e->size -= msg->hlength + msg->blength;
	     printf("[ecore_dbus] dbus message length %lu bytes, still %d\n",
		    msg->hlength + msg->blength, e->size);
	     /*ecore_dbus_message_print(msg); */
	     ecore_event_add(ECORE_DBUS_EVENT_SERVER_DATA, msg, _ecore_dbus_message_free, NULL);
	  }
     }
   return 0;
}

/* printing functions */

static Ecore_DBus_Message_Field *
_ecore_dbus_message_print_field(Ecore_DBus_Message_Field *f, unsigned char *buf)
{
   int          i;

   switch (f->type)
     {
      case ECORE_DBUS_DATA_TYPE_BYTE:
	 printf
	    ("[ecore_dbus] field BYTE		: value offset = %d value = %d\n",
	     f->offset, (char)*(buf + f->offset));
	 break;
      case ECORE_DBUS_DATA_TYPE_INT32:
	 printf
	    ("[ecore_dbus] field INT32	: value offset = %d value = %ld\n",
	     f->offset, (long int)*(buf + f->offset));
	 break;
      case ECORE_DBUS_DATA_TYPE_UINT32:
	 printf
	    ("[ecore_dbus] field UINT32	: value offset = %d value = %lu\n",
	     f->offset, (unsigned long int)*(buf + f->offset));
	 break;
      case ECORE_DBUS_DATA_TYPE_STRING:
	 printf
	    ("[ecore_dbus] field STRING	: value offset = %d length = %d value = %s\n",
	     f->offset, *(buf + f->offset), (buf + f->offset + 4));
	 break;
      case ECORE_DBUS_DATA_TYPE_OBJECT_PATH:
	 printf
	    ("[ecore_dbus] field PATH		: value offset = %d length = %d value = %s\n",
	     f->offset, *(buf + f->offset), (buf + f->offset + 4));
	 break;
      case ECORE_DBUS_DATA_TYPE_ARRAY:
	   {
	      printf
		 ("[ecore_dbus] field ARRAY	: value offset = %d length = %lu elements = %u\n",
		  f->offset, *(unsigned long int *)(buf + f->offset), f->count);
	      printf("[ecore_dbus] * ARRAY elements begin *\n");
	      f = (Ecore_DBus_Message_Field *)((Ecore_List2 *)f)->next;
	      for (i = 0; i < f->count; i++)
		f = _ecore_dbus_message_print_field(f, buf);
	      printf("[ecore_dbus] * ARRAY elements end *\n");
	      return f;
	   }
	 break;
      case ECORE_DBUS_DATA_TYPE_INVALID:
      case ECORE_DBUS_DATA_TYPE_BOOLEAN:
      case ECORE_DBUS_DATA_TYPE_INT16:
      case ECORE_DBUS_DATA_TYPE_UINT16:
      case ECORE_DBUS_DATA_TYPE_INT64:
      case ECORE_DBUS_DATA_TYPE_UINT64:
      case ECORE_DBUS_DATA_TYPE_DOUBLE:
      case ECORE_DBUS_DATA_TYPE_SIGNATURE:
      case ECORE_DBUS_DATA_TYPE_VARIANT:
      case ECORE_DBUS_DATA_TYPE_STRUCT:
      case ECORE_DBUS_DATA_TYPE_DICT_ENTRY:
	 /* TODO */
	 printf("[ecore_dbus] field not implemented: %c\n", f->type);
	 break;
#if 0
      default:
	 printf("[ecore_dbus] field !UNKNOWN!: %c\n", f->type);
	 break;
#endif
     }
   return (Ecore_DBus_Message_Field *)((Ecore_List2 *)f)->next;
}

static Ecore_DBus_Message_Field *
_ecore_dbus_message_print_header_field(Ecore_DBus_Message_Field *f, unsigned char *buf)
{
   static const char  *header_fields[] = {
	"INVALID", "PATH", "INTERFACE", "MEMBER", "ERROR_NAME",
	"REPLY_SERIAL", "DESTINATION", "SERIAL", "SIGNATURE"
   };

   printf("[ecore_dbus] header field %s ", header_fields[f->hfield]);
   return _ecore_dbus_message_print_field(f, buf);
}

/*
static void
_ecore_dbus_message_print_fields(Ecore_DBus_Message_Field *f)
{
   int               i;
   Ecore_List2      *l;

   i = 0;
   l = (Ecore_List2 *)f;
   while (l)
     {
	printf("%d\n", i);
	l = l->next;
	i++;
     }
}

static void
_ecore_dbus_message_print_raw(unsigned char *msg, unsigned int msg_len)
{
   int                 i;

   printf("[ecore_dbus] raw message:\n");
   for (i = 0; i < msg_len; i++)
     {
	if (msg[i] == 0)
	   printf(",");
	else if (msg[i] < 21)
	   printf("*");
	else
	   printf("%c", msg[i]);
     }
   printf("\n");
   printf("[ecore_dbus] end raw message\n");
}
*/
