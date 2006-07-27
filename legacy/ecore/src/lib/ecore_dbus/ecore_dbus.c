/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
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
EAPI int                 ECORE_DBUS_EVENT_SERVER_SIGNAL = 0;

/* private function declaration */

/* helper functions */
static char        *_ecore_dbus_getuid(void);
static char        *_ecore_dbus_hex_encode(char *src_str);
/* auth functions */
unsigned char      *_ecore_dbus_auth_external(void *data);
/* con functions */
static int          _ecore_dbus_event_server_add(void *data, int ev_type, void *ev);
static int          _ecore_dbus_event_server_del(void *data, int ev_type, void *ev);
static int          _ecore_dbus_event_server_data(void *data, int ev_type, void *ev);
static void         _ecore_dbus_event_server_del_free(void *data, void *ev);
static void         _ecore_dbus_event_server_data_free(void *data, void *ev);

static Ecore_DBus_Event_Server_Data *_ecore_dbus_event_create(Ecore_DBus_Server *svr, Ecore_DBus_Message *msg);

static void         _ecore_dbus_method_hello_cb(void *data, Ecore_DBus_Method_Return *reply);
static void         _ecore_dbus_method_error_cb(void *data, const char *error);

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
   ECORE_DBUS_EVENT_SERVER_SIGNAL = ecore_event_type_new();

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

   /* FIXME: Delete servers */

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
	/* Get address from DBUS_SESSION_BUS_ADDRESS env */
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
   svr->messages = ecore_hash_new(ecore_direct_hash, ecore_direct_compare);
   ecore_hash_set_free_value(svr->messages, ECORE_FREE_CB(_ecore_dbus_message_free));
   servers = _ecore_list2_append(servers, svr);

   return svr;
}

EAPI void
ecore_dbus_server_del(Ecore_DBus_Server *svr)
{
   if (svr->server) ecore_con_server_del(svr->server);
   servers = _ecore_list2_remove(servers, svr);
   if (svr->unique_name) free(svr->unique_name);
   ecore_hash_destroy(svr->messages);
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
   svr = ecore_con_server_data_get(e->server);
   if (!svr) return 1;
   if (!_ecore_list2_find(servers, svr)) return 1;

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
   svr = ecore_con_server_data_get(e->server);
   if (!svr) return 1;
   if (!_ecore_list2_find(servers, svr)) return 1;

   e2 = calloc(1, sizeof(Ecore_DBus_Event_Server_Del));
   if (e2)
     {
	svr->server = NULL;
	e2->server = svr;
	ecore_event_add(ECORE_DBUS_EVENT_SERVER_DEL, e2, _ecore_dbus_event_server_del_free, NULL);
     }
   return 0;
}

static int
_ecore_dbus_event_server_data(void *udata, int ev_type, void *ev)
{
   Ecore_Con_Event_Server_Data *e;
   Ecore_DBus_Server           *svr;

   e = ev;
   svr = ecore_con_server_data_get(e->server);
   if (!svr) return 1;
   if (!_ecore_list2_find(servers, svr)) return 1;

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
	     /* Register on the bus */
	     ecore_dbus_method_hello(svr, _ecore_dbus_method_hello_cb, _ecore_dbus_method_error_cb, svr);
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
	Ecore_DBus_Message           *msg;
	unsigned int                  offset = 0;

	printf("[ecore_dbus] received server data, %d bytes\n", e->size);
	while (e->size)
	  {
	     Ecore_DBus_Event_Server_Data *ev2;

	     msg = _ecore_dbus_message_unmarshal(svr, (unsigned char *)(e->data) + offset, e->size);
	     if (!msg) break;
	     offset += msg->length;
	     e->size -= msg->length;
	     printf("[ecore_dbus] dbus message length %u bytes, still %d\n",
		    msg->length, e->size);
	     //ecore_dbus_message_print(msg);
	     /* Trap known messages */
	     ev2 = _ecore_dbus_event_create(svr, msg);
	     if (!ev2) break;
	     if (msg->type == ECORE_DBUS_MESSAGE_TYPE_METHOD_RETURN)
	       {
		  Ecore_DBus_Message *sent;
		  sent = ecore_hash_remove(svr->messages, (void *)(ev2->header.reply_serial));
		  if ((sent) && (sent->cb.method_return))
		    {
		       sent->cb.method_return(sent->cb.data, ev2);
		    }
		  else
		    {
		       printf("Ecore_DBus: Reply without reply serial!\n");
		    }
		  if (sent) _ecore_dbus_message_free(sent);
		  _ecore_dbus_event_server_data_free(NULL, ev2);
	       }
	     else if (msg->type == ECORE_DBUS_MESSAGE_TYPE_ERROR)
	       {
		  Ecore_DBus_Message *sent;
		  sent = ecore_hash_remove(svr->messages, (void *)(ev2->header.reply_serial));
		  if ((sent) && (sent->cb.error))
		    {
		       char *error = NULL;
		       if ((ev2->args) && (ev2->args[0].type == ECORE_DBUS_DATA_TYPE_STRING))
			 error = ev2->args[0].value;
		       sent->cb.error(sent->cb.data, error);
		    }
		  else
		    {
		       printf("Ecore_DBus: Error without reply serial!\n");
		    }
		  if (sent) _ecore_dbus_message_free(sent);
		  _ecore_dbus_event_server_data_free(NULL, ev2);
	       }
	     else if (msg->type == ECORE_DBUS_MESSAGE_TYPE_SIGNAL)
	       {
		  Ecore_DBus_Event_Server_Data *ev2;
		  ev2 = _ecore_dbus_event_create(svr, msg);
		  ecore_event_add(ECORE_DBUS_EVENT_SERVER_SIGNAL, ev2,
				  _ecore_dbus_event_server_data_free, NULL);
	       }
	     else
	       {
		  printf("Ecore_DBus: Unknown return type %d\n", msg->type);
		  _ecore_dbus_event_server_data_free(NULL, ev2);
	       }
	  }
     }
   return 0;
}

static void
_ecore_dbus_event_server_del_free(void *data, void *ev)
{
   Ecore_DBus_Event_Server_Del *event;

   event = ev;
   ecore_dbus_server_del(event->server);
   free(ev);
}

static void
_ecore_dbus_event_server_data_free(void *data, void *ev)
{
   Ecore_DBus_Event_Server_Data *event;

   event = ev;
   _ecore_dbus_message_free(event->message);
   if (event->args) free(event->args);
   free(ev);
}

static Ecore_DBus_Event_Server_Data *
_ecore_dbus_event_create(Ecore_DBus_Server *svr, Ecore_DBus_Message *msg)
{
   Ecore_DBus_Event_Server_Data *ev;
   unsigned int                 *serial;

   ev = calloc(1, sizeof(Ecore_DBus_Event_Server_Data));
   if (!ev) return NULL;
   ev->server = svr;
   ev->type = msg->type;
   ev->message = msg;
   ev->header.path = ecore_dbus_message_header_field_get(msg, ECORE_DBUS_HEADER_FIELD_PATH);
   ev->header.interface = ecore_dbus_message_header_field_get(msg, ECORE_DBUS_HEADER_FIELD_INTERFACE);
   ev->header.member = ecore_dbus_message_header_field_get(msg, ECORE_DBUS_HEADER_FIELD_MEMBER);
   ev->header.error_name = ecore_dbus_message_header_field_get(msg, ECORE_DBUS_HEADER_FIELD_ERROR_NAME);
   serial = ecore_dbus_message_header_field_get(msg, ECORE_DBUS_HEADER_FIELD_REPLY_SERIAL);
   if (serial)
     ev->header.reply_serial = *serial;
   ev->header.destination = ecore_dbus_message_header_field_get(msg, ECORE_DBUS_HEADER_FIELD_DESTINATION);
   ev->header.sender = ecore_dbus_message_header_field_get(msg, ECORE_DBUS_HEADER_FIELD_SENDER);
   ev->header.signature = ecore_dbus_message_header_field_get(msg, ECORE_DBUS_HEADER_FIELD_SIGNATURE);
   if (!ecore_list_is_empty(msg->fields))
     {
	Ecore_DBus_Message_Field *f;
	int i = 0;

	ev->args = malloc(ecore_list_nodes(msg->fields) * sizeof(Ecore_DBus_Message_Arg));
	ecore_list_goto_first(msg->fields);
	while ((f = ecore_list_next(msg->fields)))
	  {
	     ev->args[i].type = f->type;
	     ev->args[i].value = _ecore_dbus_message_field_value_get(f);
	     i++;
	  }
     }
   return ev;
}

static void
_ecore_dbus_method_hello_cb(void *data, Ecore_DBus_Method_Return *reply)
{
   Ecore_DBus_Event_Server_Add *svr_add;
   Ecore_DBus_Server           *svr;
   char                        *name;

   svr = data;
   name = reply->args[0].value;
   printf("Got unique name: %s\n", name);
   if (svr->unique_name)
     {
	printf("Ecore_DBus: Already said hello %s - %s\n",
	      svr->unique_name, name);
	free(svr->unique_name);
     }
   svr->unique_name = strdup(name);

   svr_add = malloc(sizeof(Ecore_DBus_Event_Server_Add));
   svr_add->server = svr;
   ecore_event_add(ECORE_DBUS_EVENT_SERVER_ADD, svr_add, NULL, NULL);
}

static void
_ecore_dbus_method_error_cb(void *data, const char *error)
{
   Ecore_DBus_Event_Server_Del *ev;
   Ecore_DBus_Server           *svr;

   svr = data;
   printf("Ecore_DBus: error %s\n", error);

   ev = malloc(sizeof(Ecore_DBus_Event_Server_Del));
   if (!ev) return;
   ev->server = svr;
   ecore_event_add(ECORE_DBUS_EVENT_SERVER_DEL, ev, _ecore_dbus_event_server_del_free, NULL);
}
