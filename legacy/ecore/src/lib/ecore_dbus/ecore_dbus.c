/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
#include <stdarg.h>

#include "Ecore.h"
#include "ecore_private.h"
#include "Ecore_Con.h"
#include "Ecore_DBus.h"
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

/* message functions */
static void                      _ecore_dbus_message_free(void *data, void *ev);
/* header functions */
static void                      _ecore_dbus_message_common_header(Ecore_DBus_Message *msg, int type, int flags);
static void                     *_ecore_dbus_field_get(unsigned char *buf, Ecore_DBus_Message_Field *f);
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
   unsigned int       serial;
   va_list            ap;
   unsigned int       size = 0;

   Ecore_DBus_Message_Field *f;

   if ((!service) && (!path) && (!interface) && (!method)) return 0;

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
	size++;
     }
   if (service)
     {
	f = _ecore_dbus_message_marshal_custom_header(&msg->header, &msg->hpos,
						      6,
						      ECORE_DBUS_DATA_TYPE_STRING,
						      service);
	msg->header_fields = _ecore_list2_append(msg->header_fields, f);
	size++;
     }
   if (interface)
     {
	f = _ecore_dbus_message_marshal_custom_header(&msg->header, &msg->hpos,
						      2,
						      ECORE_DBUS_DATA_TYPE_STRING,
						      interface);
	msg->header_fields = _ecore_list2_append(msg->header_fields, f);
	size++;
     }
   if (method)
     {
	f = _ecore_dbus_message_marshal_custom_header(&msg->header, &msg->hpos,
						      3,
						      ECORE_DBUS_DATA_TYPE_STRING,
						      method);
	msg->header_fields = _ecore_list2_append(msg->header_fields, f);
	size++;
     }
   if (fmt)
     {
	f = _ecore_dbus_message_marshal_custom_header(&msg->header, &msg->hpos,
						      8,
						      ECORE_DBUS_DATA_TYPE_SIGNATURE,
						      fmt);
	msg->header_fields = _ecore_list2_append(msg->header_fields, f);
	size++;
     }
   /* Remove 16 bytes for the intro */
   *(unsigned int *)(msg->header + 12) = msg->hpos - 16;
   _ecore_dbus_message_padding(&msg->header, &msg->hpos, 8);
   msg->hlength = msg->hpos;
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
							  va_arg(ap, unsigned int));
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
   *(unsigned int *)(msg->header + 4) = msg->blength = msg->bpos;
   /* show message */
   ecore_dbus_message_print(msg);
   /*
      _ecore_dbus_message_print_raw(msg->header, msg->hlength);
      _ecore_dbus_message_print_raw(msg->body, msg->blength);
      */
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
   printf("[ecore_dbus] header endianess	: %c\n", msg->header[0]);
   printf("[ecore_dbus] header type     	: %s\n", msg_type[msg->header[1]]);
   printf("[ecore_dbus] header flags    	: %c\n", msg->header[2]);
   printf("[ecore_dbus] header protocol 	: %c\n", msg->header[3]);
   printf("[ecore_dbus] header hlength  	: %u\n", msg->hlength);
   printf("[ecore_dbus] header blength  	: %u\n",
	 *(unsigned int *)(msg->header + 4));
   printf("[ecore_dbus] header serial   	: %u\n",
	 *(unsigned int *)(msg->header + 8));

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
ecore_dbus_header_field_get(Ecore_DBus_Message *m,
			    Ecore_DBus_Message_Field *mf,
			    Ecore_DBus_Message_Header_Field hft)
{
   Ecore_List2      *l;

   for (l = (Ecore_List2 *)mf; l; l = l->next)
     if (((Ecore_DBus_Message_Field *)l)->hfield == hft)
       return _ecore_dbus_field_get(m->header, (Ecore_DBus_Message_Field *)l);
   return NULL;
}

EAPI void *
ecore_dbus_body_field_get(Ecore_DBus_Message *m, Ecore_DBus_Message_Field *mf,
			  unsigned int pos)
{
   Ecore_List2      *l;
   unsigned int      i = 0;

   for (l = (Ecore_List2 *) mf; l; l = l->next)
     {
	if (i == pos)
	  return _ecore_dbus_field_get(m->body, (Ecore_DBus_Message_Field *)l);
	i++;
     }
   return NULL;
}

/* private functions */

/* message functions */

Ecore_DBus_Message *
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

Ecore_DBus_Message_Field *
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
   l = (Ecore_List2 *)msg->header_fields;
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

/* header functions */

static void
_ecore_dbus_message_common_header(Ecore_DBus_Message *msg, int type, int flags)
{
   /* the body, header length arent filled only alloc */
   _ecore_dbus_message_append_length(&msg->header, msg->hpos, 16);
   /* endiannes (1) */
   msg->header[0] = msg->byte_order = 'l';
   /* type (1) */
   msg->header[1] = msg->type = (char)type;
   /* flags (1) 0x1 = no reply expected, 0x2 auto activiation */
   msg->header[2] = msg->flags = 0x0;
   /* protocol (1) */
   msg->header[3] = msg->protocol = ECORE_DBUS_MAJOR_PROTOCOL_VERSION;
   /* autoincrement the client_serial (0 is invalid) */
   msg->ref_server->cnt_msg++;

   *(unsigned int *)(msg->header + 8) = msg->serial = msg->ref_server->cnt_msg;
   *(msg->header + 12) = ECORE_DBUS_DATA_TYPE_INT16;
   msg->hpos = 16;
}

static void *
_ecore_dbus_field_get(unsigned char *buf, Ecore_DBus_Message_Field *f)
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
     case ECORE_DBUS_DATA_TYPE_SIGNATURE:
	return buf + f->offset + 1;
	break;
     case ECORE_DBUS_DATA_TYPE_INVALID:
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
	     printf("[ecore_dbus] dbus message length %u bytes, still %d\n",
		    msg->hlength + msg->blength, e->size);
	     ecore_dbus_message_print(msg);
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
	    ("[ecore_dbus] field BYTE: value offset = %d value = %d\n",
	     f->offset, (char)*(buf + f->offset));
	 break;
      case ECORE_DBUS_DATA_TYPE_INT32:
	 printf
	    ("[ecore_dbus] field INT32: value offset = %d value = %d\n",
	     f->offset, (int)*(buf + f->offset));
	 break;
      case ECORE_DBUS_DATA_TYPE_UINT32:
	 printf
	    ("[ecore_dbus] field UINT32: value offset = %d value = %u\n",
	     f->offset, (unsigned int)*(buf + f->offset));
	 break;
      case ECORE_DBUS_DATA_TYPE_STRING:
	 printf
	    ("[ecore_dbus] field STRING: value offset = %d length = %d value = %s\n",
	     f->offset, *(buf + f->offset), (buf + f->offset + 4));
	 break;
      case ECORE_DBUS_DATA_TYPE_OBJECT_PATH:
	 printf
	    ("[ecore_dbus] field PATH: value offset = %d length = %d value = %s\n",
	     f->offset, *(buf + f->offset), (buf + f->offset + 4));
	 break;
      case ECORE_DBUS_DATA_TYPE_SIGNATURE:
	 printf
	    ("[ecore_dbus] field SIGNATURE	: value offset = %d length = %d value = %s\n",
	     f->offset, *(buf + f->offset), (buf + f->offset + 1));
	 break;
      case ECORE_DBUS_DATA_TYPE_ARRAY:
	   {
	      printf
		 ("[ecore_dbus] field ARRAY	: value offset = %d length = %u elements = %u\n",
		  f->offset, *(unsigned int *)(buf + f->offset), f->count);
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
      case ECORE_DBUS_DATA_TYPE_VARIANT:
      case ECORE_DBUS_DATA_TYPE_STRUCT:
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
