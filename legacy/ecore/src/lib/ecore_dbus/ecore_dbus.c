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
EAPI int                 ECORE_DBUS_EVENT_SERVER_DATA = 0;

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
static void         _ecore_dbus_event_server_data_free(void *data, void *ev);

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
   svr->methods = ecore_hash_new(ecore_direct_hash, ecore_direct_compare);
   ecore_hash_set_free_value(svr->methods, free);
   servers = _ecore_list2_append(servers, svr);

   return svr;
}

EAPI void
ecore_dbus_server_del(Ecore_DBus_Server *svr)
{
   if (svr->server) ecore_con_server_del(svr->server);
   servers = _ecore_list2_remove(servers, svr);
   if (svr->unique_name) free(svr->unique_name);
   ecore_hash_destroy(svr->methods);
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
	     Ecore_DBus_Event_Server_Data *ev;
	     unsigned int                 *serial;
	     char                         *method;

	     msg = _ecore_dbus_message_unmarshal(svr, (unsigned char *)(e->data) + offset, e->size);
	     if (msg == NULL) break;
	     offset += msg->length;
	     e->size -= msg->length;
	     printf("[ecore_dbus] dbus message length %u bytes, still %d\n",
		    msg->length, e->size);
	     //ecore_dbus_message_print(msg);
	     /* Trap known messages */
	     serial = ecore_dbus_message_header_field_get(msg, ECORE_DBUS_HEADER_FIELD_REPLY_SERIAL);
	     if ((serial) && (msg->type == ECORE_DBUS_MESSAGE_TYPE_METHOD_RETURN))
	       {

		  method = ecore_hash_remove(svr->methods, (void *)(*serial));
		  if (method)
		    {
		       printf("Return from method: %s\n", method);
		       if (!strcmp(method, "org.freedesktop.DBus.Hello"))
			 {
			    char *name;

			    name = ecore_dbus_message_body_field_get(msg, 0);
			    printf("Got unique name: %s\n", name);
			    svr->unique_name = strdup(name);
			 }
		    }
	       }
	     else
	       {
		  /* Get interface and member from the message */
		  char buf[1024];

		  char *interface, *member;
		  interface = ecore_dbus_message_header_field_get(msg, ECORE_DBUS_HEADER_FIELD_INTERFACE);
		  member = ecore_dbus_message_header_field_get(msg, ECORE_DBUS_HEADER_FIELD_MEMBER);
		  if ((member) && (interface))
		    snprintf(buf, sizeof(buf), "%s.%s", interface, member);
		  else if (member)
		    strcpy(buf, member);
		  method = strdup(buf);
	       }
	     ev = malloc(sizeof(Ecore_DBus_Event_Server_Data));
	     ev->type = msg->type;
	     ev->server = svr;
	     ev->message = msg;
	     ev->member = method;
	     ecore_event_add(ECORE_DBUS_EVENT_SERVER_DATA, ev,
			     _ecore_dbus_event_server_data_free, NULL);
	  }
     }
   return 0;
}

static void
_ecore_dbus_event_server_data_free(void *data, void *ev)
{
   Ecore_DBus_Event_Server_Data *event;

   event = ev;
   _ecore_dbus_message_free(event->message);
   if (event->member) free(event->member);
   free(ev);
}
