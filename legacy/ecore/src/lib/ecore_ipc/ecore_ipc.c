#include "Ecore.h"
#include "Ecore_Con.h"
#include "ecore_private.h"
#include "ecore_ipc_private.h"
#include "Ecore_Ipc.h"

#include <netinet/in.h>

typedef struct _Ecore_Ipc_Msg_Head Ecore_Ipc_Msg_Head;

struct _Ecore_Ipc_Msg_Head
{
   int major;
   int minor;
   int ref;
   int ref_to;
   int response;
   int size;
} __attribute__ ((packed));

static int _ecore_ipc_event_client_add(void *data, int ev_type, void *ev);
static int _ecore_ipc_event_client_del(void *data, int ev_type, void *ev);
static int _ecore_ipc_event_server_add(void *data, int ev_type, void *ev);
static int _ecore_ipc_event_server_del(void *data, int ev_type, void *ev);
static int _ecore_ipc_event_client_data(void *data, int ev_type, void *ev);
static int _ecore_ipc_event_server_data(void *data, int ev_type, void *ev);
static void _ecore_ipc_event_client_add_free(void *data, void *ev);
static void _ecore_ipc_event_client_del_free(void *data, void *ev);
static void _ecore_ipc_event_server_add_free(void *data, void *ev);
static void _ecore_ipc_event_server_del_free(void *data, void *ev);
static void _ecore_ipc_event_client_data_free(void *data, void *ev);
static void _ecore_ipc_event_server_data_free(void *data, void *ev);
    
int ECORE_IPC_EVENT_CLIENT_ADD = 0;
int ECORE_IPC_EVENT_CLIENT_DEL = 0;
int ECORE_IPC_EVENT_SERVER_ADD = 0;
int ECORE_IPC_EVENT_SERVER_DEL = 0;
int ECORE_IPC_EVENT_CLIENT_DATA = 0;
int ECORE_IPC_EVENT_SERVER_DATA = 0;

static int init_count = 0;
static Ecore_Ipc_Server *servers = NULL;
/**
 * To be documented.
 *
 * FIXME: To be fixed.
 * <hr><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br>
 */
int
ecore_ipc_init(void)
{
   if (!init_count) ecore_con_init();
   init_count++;
   if (!ECORE_IPC_EVENT_CLIENT_ADD)
     {
	ECORE_IPC_EVENT_CLIENT_ADD = ecore_event_type_new();
	ECORE_IPC_EVENT_CLIENT_DEL = ecore_event_type_new();
	ECORE_IPC_EVENT_SERVER_ADD = ecore_event_type_new();
	ECORE_IPC_EVENT_SERVER_DEL = ecore_event_type_new();
	ECORE_IPC_EVENT_CLIENT_DATA = ecore_event_type_new();
	ECORE_IPC_EVENT_SERVER_DATA = ecore_event_type_new();
	
	ecore_event_handler_add(ECORE_CON_EVENT_CLIENT_ADD, _ecore_ipc_event_client_add, NULL);
	ecore_event_handler_add(ECORE_CON_EVENT_CLIENT_DEL, _ecore_ipc_event_client_del, NULL);
	ecore_event_handler_add(ECORE_CON_EVENT_SERVER_ADD, _ecore_ipc_event_server_add, NULL);
	ecore_event_handler_add(ECORE_CON_EVENT_SERVER_DEL, _ecore_ipc_event_server_del, NULL);
	ecore_event_handler_add(ECORE_CON_EVENT_CLIENT_DATA, _ecore_ipc_event_client_data, NULL);
	ecore_event_handler_add(ECORE_CON_EVENT_SERVER_DATA, _ecore_ipc_event_server_data, NULL);
     }
   return init_count;   
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 * <hr><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br>
 */
int
ecore_ipc_shutdown(void)
{
   if (init_count > 0)
     {
	init_count--;
	if (init_count > 0) return init_count;
	while (servers) ecore_ipc_server_del(servers);
	ecore_con_shutdown();
     }
   return 0;
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 * <hr><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br>
 */
Ecore_Ipc_Server *
ecore_ipc_server_add(Ecore_Ipc_Type type, char *name, int port, const void *data)
{
   Ecore_Ipc_Server *svr;
   
   svr = calloc(1, sizeof(Ecore_Ipc_Server));
   if (!svr) return NULL;
   switch (type)
     {
      case ECORE_IPC_LOCAL_USER:
	svr->server = ecore_con_server_add(ECORE_CON_LOCAL_USER, name, port, svr);
	break;
      case ECORE_IPC_LOCAL_SYSTEM:
	svr->server = ecore_con_server_add(ECORE_CON_LOCAL_SYSTEM, name, port, svr);
	break;
      case ECORE_IPC_REMOTE_SYSTEM:
	svr->server = ecore_con_server_add(ECORE_CON_REMOTE_SYSTEM, name, port, svr);
	break;
      default:
	free(svr);
	return NULL;
     }
   if (!svr->server)
     {
	free(svr);
	return NULL;
     }
   svr->data = (void *)data;
   servers = _ecore_list_append(servers, svr);
   ECORE_MAGIC_SET(svr, ECORE_MAGIC_IPC_SERVER);
   return svr;
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 * <hr><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br>
 */
Ecore_Ipc_Server *
ecore_ipc_server_connect(Ecore_Ipc_Type type, char *name, int port, const void *data)
{
   Ecore_Ipc_Server *svr;
   
   svr = calloc(1, sizeof(Ecore_Ipc_Server));
   if (!svr) return NULL;
   switch (type)
     {
      case ECORE_IPC_LOCAL_USER:
	svr->server = ecore_con_server_connect(ECORE_CON_LOCAL_USER, name, port, svr);
	break;
      case ECORE_IPC_LOCAL_SYSTEM:
	svr->server = ecore_con_server_connect(ECORE_CON_LOCAL_SYSTEM, name, port, svr);
	break;
      case ECORE_IPC_REMOTE_SYSTEM:
	svr->server = ecore_con_server_connect(ECORE_CON_REMOTE_SYSTEM, name, port, svr);
	break;
      default:
	free(svr);
	return NULL;
     }
   if (!svr->server)
     {
	free(svr);
	return NULL;
     }
   svr->data = (void *)data;
   servers = _ecore_list_append(servers, svr);
   ECORE_MAGIC_SET(svr, ECORE_MAGIC_IPC_SERVER);
   return svr;
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 * <hr><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br>
 */
void *
ecore_ipc_server_del(Ecore_Ipc_Server *svr)
{
   void *data;

   if (!ECORE_MAGIC_CHECK(svr, ECORE_MAGIC_IPC_SERVER))
     {
	ECORE_MAGIC_FAIL(svr, ECORE_MAGIC_IPC_SERVER,
			 "ecore_ipc_server_del");
	return NULL;
     }
   ECORE_MAGIC_SET(svr, ECORE_MAGIC_NONE);
   data = svr->data;
   while (svr->clients) ecore_ipc_client_del((Ecore_Ipc_Client *)svr->clients);
   ecore_con_server_del(svr->server);
   servers = _ecore_list_remove(servers, svr);
   if (svr->buf) free(svr->buf);
   free(svr);
   return data;
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 * <hr><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br>
 */
void *
ecore_ipc_server_data_get(Ecore_Ipc_Server *svr)
{
   if (!ECORE_MAGIC_CHECK(svr, ECORE_MAGIC_IPC_SERVER))
     {
	ECORE_MAGIC_FAIL(svr, ECORE_MAGIC_IPC_SERVER,
			 "ecore_ipc_server_data_get");
	return NULL;
     }
   return svr->data;
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 * <hr><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br>
 */
int
ecore_ipc_server_connected_get(Ecore_Ipc_Server *svr)
{
   if (!ECORE_MAGIC_CHECK(svr, ECORE_MAGIC_IPC_SERVER))
     {
	ECORE_MAGIC_FAIL(svr, ECORE_MAGIC_IPC_SERVER,
			 "ecore_ipc_server_connected_get");
	return 0;
     }
   return ecore_con_server_connected_get(svr->server);
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 * <hr><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br>
 */
int
ecore_ipc_server_send(Ecore_Ipc_Server *svr, int major, int minor, int ref, int ref_to, int response, void *data, int size)
{
   Ecore_Ipc_Msg_Head msg;
   int ret;
   
   if (!ECORE_MAGIC_CHECK(svr, ECORE_MAGIC_IPC_SERVER))
     {
	ECORE_MAGIC_FAIL(svr, ECORE_MAGIC_IPC_SERVER,
			 "ecore_ipc_server_send");
	return 0;
     }
   if (size < 0) size = 0;
   msg.major    = htonl(major);
   msg.minor    = htonl(minor);
   msg.ref      = htonl(ref);
   msg.ref_to   = htonl(ref_to);
   msg.response = htonl(response);
   msg.size     = htonl(size);
   ret = ecore_con_server_send(svr->server, &msg, sizeof(Ecore_Ipc_Msg_Head));
   if (size > 0) ret |= ecore_con_server_send(svr->server, data, size);
   return ret;
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 * <hr><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br>
 */
int
ecore_ipc_client_send(Ecore_Ipc_Client *cl, int major, int minor, int ref, int ref_to, int response, void *data, int size)
{
   Ecore_Ipc_Msg_Head msg;
   int ret;
   
   if (!ECORE_MAGIC_CHECK(cl, ECORE_MAGIC_IPC_CLIENT))
     {
	ECORE_MAGIC_FAIL(cl, ECORE_MAGIC_IPC_CLIENT,
			 "ecore_ipc_client_send");
	return 0;
     }
   if (size < 0) size = 0;
   msg.major    = htonl(major);
   msg.minor    = htonl(minor);
   msg.ref      = htonl(ref);
   msg.ref_to   = htonl(ref_to);
   msg.response = htonl(response);
   msg.size     = htonl(size);
   ret = ecore_con_client_send(cl->client, &msg, sizeof(Ecore_Ipc_Msg_Head));
   if (size > 0) ret |= ecore_con_client_send(cl->client, data, size);
   return ret;
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 * <hr><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br>
 */
Ecore_Ipc_Server *
ecore_ipc_client_server_get(Ecore_Ipc_Client *cl)
{
   if (!ECORE_MAGIC_CHECK(cl, ECORE_MAGIC_IPC_CLIENT))
     {
	ECORE_MAGIC_FAIL(cl, ECORE_MAGIC_IPC_CLIENT,
			 "ecore_ipc_client_server_get");
	return NULL;
     }
   return (ecore_con_server_data_get(ecore_con_client_server_get(cl->client)));
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 * <hr><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br>
 */
void *
ecore_ipc_client_del(Ecore_Ipc_Client *cl)
{
   void *data;
   Ecore_Ipc_Server *svr;
   
   if (!ECORE_MAGIC_CHECK(cl, ECORE_MAGIC_IPC_CLIENT))
     {
	ECORE_MAGIC_FAIL(cl, ECORE_MAGIC_IPC_CLIENT,
			 "ecore_ipc_client_del");
	return NULL;
     }
   ECORE_MAGIC_SET(cl, ECORE_MAGIC_NONE);
   data = cl->data;
   svr = ecore_con_server_data_get(ecore_con_client_server_get(cl->client));
   ecore_con_client_del(cl->client);
   svr->clients = _ecore_list_remove(svr->clients, cl);
   if (cl->buf) free(cl->buf);
   free(cl);
   return data;
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 * <hr><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br>
 */
void
ecore_ipc_client_data_set(Ecore_Ipc_Client *cl, const void *data)
{  
   if (!ECORE_MAGIC_CHECK(cl, ECORE_MAGIC_IPC_CLIENT))
     {
	ECORE_MAGIC_FAIL(cl, ECORE_MAGIC_IPC_CLIENT,
			 "ecore_ipc_client_data_set");
	return;
     }
   cl->data = (void *)data;
}  

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 * <hr><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br>
 */
void *
ecore_ipc_client_data_get(Ecore_Ipc_Client *cl)
{  
   if (!ECORE_MAGIC_CHECK(cl, ECORE_MAGIC_IPC_CLIENT))
     {
	ECORE_MAGIC_FAIL(cl, ECORE_MAGIC_IPC_CLIENT,
			 "ecore_ipc_client_data_get");
	return NULL;
     }
   return cl->data;
}



static int
_ecore_ipc_event_client_add(void *data, int ev_type, void *ev)
{
   Ecore_Con_Event_Client_Add *e;
   
   e = ev;
   if (!_ecore_list_find(servers, ecore_con_server_data_get(ecore_con_client_server_get(e->client)))) return 1;
   /* handling code here */
     {
	Ecore_Ipc_Client *cl;
	Ecore_Ipc_Server *svr;
	
	cl = calloc(1, sizeof(Ecore_Ipc_Client));
	if (!cl) return 0;
	svr = ecore_con_server_data_get(ecore_con_client_server_get(e->client));
	ECORE_MAGIC_SET(cl, ECORE_MAGIC_IPC_CLIENT);
	cl->client = e->client;
	ecore_con_client_data_set(cl->client, (void *)cl);
	svr->clients = _ecore_list_append(svr->clients, cl);
	  {
	     Ecore_Ipc_Event_Client_Add *e2;
	     
	     e2 = calloc(1, sizeof(Ecore_Ipc_Event_Client_Add));
	     if (e2)
	       {
		  e2->client = cl;
		  ecore_event_add(ECORE_IPC_EVENT_CLIENT_ADD, e2,
				  _ecore_ipc_event_client_add_free, NULL);
	       }
	  }
     }
   return 0;
}

static int
_ecore_ipc_event_client_del(void *data, int ev_type, void *ev)
{
   Ecore_Con_Event_Client_Del *e;
   
   e = ev;
   if (!_ecore_list_find(servers, ecore_con_server_data_get(ecore_con_client_server_get(e->client)))) return 1;
   /* handling code here */
     {
	Ecore_Ipc_Client *cl;
	
	cl = ecore_con_client_data_get(e->client);
	  {
	     Ecore_Ipc_Event_Client_Del *e2;
	     
	     e2 = calloc(1, sizeof(Ecore_Ipc_Event_Client_Del));
	     if (e2)
	       {
		  e2->client = cl;
		  ecore_event_add(ECORE_IPC_EVENT_CLIENT_DEL, e2,
				  _ecore_ipc_event_client_del_free, NULL);
	       }
	  }
     }
   return 0;
}

static int
_ecore_ipc_event_server_add(void *data, int ev_type, void *ev)
{
   Ecore_Con_Event_Server_Add *e;
   
   e = ev;
   if (!_ecore_list_find(servers, ecore_con_server_data_get(e->server))) return 1;
   /* handling code here */
     {
	Ecore_Ipc_Server *svr;
	
	svr = ecore_con_server_data_get(e->server);
	  {
	     Ecore_Ipc_Event_Server_Add *e2;
	     
	     e2 = calloc(1, sizeof(Ecore_Ipc_Event_Server_Add));
	     if (e2)
	       {
		  e2->server = svr;
		  ecore_event_add(ECORE_IPC_EVENT_SERVER_ADD, e2,
				  _ecore_ipc_event_server_add_free, NULL);
	       }
	  }
     }
   return 0;
}

static int
_ecore_ipc_event_server_del(void *data, int ev_type, void *ev)
{
   Ecore_Con_Event_Server_Del *e;
   
   e = ev;
   if (!_ecore_list_find(servers, ecore_con_server_data_get(e->server))) return 1;
   /* handling code here */
     {
	Ecore_Ipc_Server *svr;
	
	svr = ecore_con_server_data_get(e->server);
	  {
	     Ecore_Ipc_Event_Server_Del *e2;
	     
	     e2 = calloc(1, sizeof(Ecore_Ipc_Event_Server_Del));
	     if (e2)
	       {
		  e2->server = svr;
		  ecore_event_add(ECORE_IPC_EVENT_SERVER_DEL, e2,
				  _ecore_ipc_event_server_del_free, NULL);
	       }
	  }
     }
   return 0;
}

static int
_ecore_ipc_event_client_data(void *data, int ev_type, void *ev)
{
   Ecore_Con_Event_Client_Data *e;

   e = ev;
   if (!_ecore_list_find(servers, ecore_con_server_data_get(ecore_con_client_server_get(e->client)))) return 1;
   /* handling code here */
     {
	Ecore_Ipc_Client *cl;
	Ecore_Ipc_Msg_Head *msg;
	
	cl = ecore_con_client_data_get(e->client);
	
	if (!cl->buf)
	  {
	     cl->buf_size = e->size;
	     cl->buf = e->data;
	     e->data = NULL; /* take it out of the old event */
	  }
	else
	  {
	     unsigned char *buf;
	     
	     buf = realloc(cl->buf, cl->buf_size + e->size);
	     if (!buf)
	       {
		  free(cl->buf);
		  cl->buf = 0;
		  cl->buf_size  = 0;
		  return 0;
	       }
	     cl->buf = buf;
	     memcpy(cl->buf + cl->buf_size, e->data, e->size);
	     cl->buf_size += e->size;
	  }
	/* examine header */
	if (cl->buf_size >= sizeof(Ecore_Ipc_Msg_Head))
	  {
	     int major, minor, ref, ref_to, response, size;
	     int offset = 0;

	     msg = (Ecore_Ipc_Msg_Head *)(cl->buf + offset);
	     major    = ntohl(msg->major);
	     minor    = ntohl(msg->minor);
	     ref      = ntohl(msg->ref);
	     ref_to   = ntohl(msg->ref_to);
	     response = ntohl(msg->response);
	     size     = ntohl(msg->size);
	     if (size < 0) size = 0;
	     /* there is enough data in the buffer for a full message */
	     if (cl->buf_size >= (sizeof(Ecore_Ipc_Msg_Head) + size))
	       {
		  unsigned char *buf;
		  Ecore_Ipc_Event_Client_Data *e2;
		  
		  redo:
		  buf = NULL;
		  if (size > 0)
		    {
		       buf = malloc(size);
		       if (!buf) return 0;
		       memcpy(buf, cl->buf + offset + sizeof(Ecore_Ipc_Msg_Head), size);
		    }
		  e2 = calloc(1, sizeof(Ecore_Ipc_Event_Client_Data));
		  if (e2)
		    {
		       e2->client   = cl;
		       e2->major    = major;
		       e2->minor    = minor;
		       e2->ref      = ref;
		       e2->ref_to   = ref_to;
		       e2->response = response;
		       e2->size     = size;
		       e2->data     = buf;
		       ecore_event_add(ECORE_IPC_EVENT_CLIENT_DATA, e2,
				       _ecore_ipc_event_client_data_free, NULL);
		    }
		  offset += (sizeof(Ecore_Ipc_Msg_Head) + size);
		  if (cl->buf_size - offset >= sizeof(Ecore_Ipc_Msg_Head))
		    {
		       msg = (Ecore_Ipc_Msg_Head *)(cl->buf + offset);
		       major    = ntohl(msg->major);
		       minor    = ntohl(msg->minor);
		       ref      = ntohl(msg->ref);
		       ref_to   = ntohl(msg->ref_to);
		       response = ntohl(msg->response);
		       size     = ntohl(msg->size);
		       if (size < 0) size = 0;
		       if (cl->buf_size - offset >= (sizeof(Ecore_Ipc_Msg_Head) + size))
			 goto redo;
		    }
		  /* remove data from our buffer and "scoll" the rest down */
		  size = cl->buf_size - offset;
		  /* if amount left after scroll is > 0 */
		  if (size > 0)
		    {
		       buf = malloc(size);
		       if (!buf)
			 {
			    free(cl->buf);
			    cl->buf = NULL;
			    cl->buf_size = 0;
			    return 0;
			 }
		       memcpy(buf, cl->buf + offset + sizeof(Ecore_Ipc_Msg_Head), size);
		       free(cl->buf);
		       cl->buf = buf;
		       cl->buf_size = size;
		    }
		  else
		    {
		       free(cl->buf);
		       cl->buf = NULL;
		       cl->buf_size = 0;
		    }
	       }
	  }
     }
   return 0;
}

static int
_ecore_ipc_event_server_data(void *data, int ev_type, void *ev)
{
   Ecore_Con_Event_Server_Data *e;
   
   e = ev;
   if (!_ecore_list_find(servers, ecore_con_server_data_get(e->server))) return 1;
   /* handling code here */
     {
        Ecore_Ipc_Server *svr;
	Ecore_Ipc_Msg_Head *msg;
	
	svr = ecore_con_server_data_get(e->server);
	
	if (!svr->buf)
	  {
	     svr->buf_size = e->size;
	     svr->buf = e->data;
	     e->data = NULL; /* take it out of the old event */
	  }
	else
	  {
	     unsigned char *buf;
	     
	     buf = realloc(svr->buf, svr->buf_size + e->size);
	     if (!buf)
	       {
		  free(svr->buf);
		  svr->buf = 0;
		  svr->buf_size  = 0;
		  return 0;
	       }
	     svr->buf = buf;
	     memcpy(svr->buf + svr->buf_size, e->data, e->size);
	     svr->buf_size += e->size;
	  }
	/* examine header */
	if (svr->buf_size >= sizeof(Ecore_Ipc_Msg_Head))
	  {
	     int major, minor, ref, ref_to, response, size;
	     int offset = 0;

	     msg = (Ecore_Ipc_Msg_Head *)(svr->buf + offset);
	     major    = ntohl(msg->major);
	     minor    = ntohl(msg->minor);
	     ref      = ntohl(msg->ref);
	     ref_to   = ntohl(msg->ref_to);
	     response = ntohl(msg->response);
	     size     = ntohl(msg->size);
	     if (size < 0) size = 0;
	     /* there is enough data in the buffer for a full message */
	     if (svr->buf_size >= (sizeof(Ecore_Ipc_Msg_Head) + size))
	       {
		  unsigned char *buf;
		  Ecore_Ipc_Event_Server_Data *e2;
		  
		  redo:
		  buf = NULL;
		  if (size > 0)
		    {
		       buf = malloc(size);
		       if (!buf) return 0;
		       memcpy(buf, svr->buf + offset + sizeof(Ecore_Ipc_Msg_Head), size);
		    }
		  e2 = calloc(1, sizeof(Ecore_Ipc_Event_Server_Data));
		  if (e2)
		    {
		       e2->server   = svr;
		       e2->major    = major;
		       e2->minor    = minor;
		       e2->ref      = ref;
		       e2->ref_to   = ref_to;
		       e2->response = response;
		       e2->size     = size;
		       e2->data     = buf;
		       ecore_event_add(ECORE_IPC_EVENT_SERVER_DATA, e2,
				       _ecore_ipc_event_server_data_free, NULL);
		    }
		  offset += (sizeof(Ecore_Ipc_Msg_Head) + size);
		  if (svr->buf_size - offset >= sizeof(Ecore_Ipc_Msg_Head))
		    {
		       msg = (Ecore_Ipc_Msg_Head *)(svr->buf + offset);
		       major    = ntohl(msg->major);
		       minor    = ntohl(msg->minor);
		       ref      = ntohl(msg->ref);
		       ref_to   = ntohl(msg->ref_to);
		       response = ntohl(msg->response);
		       size     = ntohl(msg->size);
		       if (size < 0) size = 0;
		       if (svr->buf_size - offset >= (sizeof(Ecore_Ipc_Msg_Head) + size))
			 goto redo;
		    }
		  /* remove data from our buffer and "scoll" the rest down */
		  size = svr->buf_size - offset;
		  /* if amount left after scroll is > 0 */
		  if (size > 0)
		    {
		       buf = malloc(size);
		       if (!buf)
			 {
			    free(svr->buf);
			    svr->buf = NULL;
			    svr->buf_size = 0;
			    return 0;
			 }
		       memcpy(buf, svr->buf + offset + sizeof(Ecore_Ipc_Msg_Head), size);
		       free(svr->buf);
		       svr->buf = buf;
		       svr->buf_size = size;
		    }
		  else
		    {
		       free(svr->buf);
		       svr->buf = NULL;
		       svr->buf_size = 0;
		    }
	       }
	  }
     }
   return 0;
}

static void
_ecore_ipc_event_client_add_free(void *data, void *ev)
{
   Ecore_Ipc_Event_Client_Add *e;
   
   e = ev;
   free(e);
}

static void
_ecore_ipc_event_client_del_free(void *data, void *ev)
{
   Ecore_Ipc_Event_Client_Del *e;
   
   e = ev;
   free(e);
}

static void
_ecore_ipc_event_server_add_free(void *data, void *ev)
{
   Ecore_Ipc_Event_Server_Add *e;
   
   e = ev;
   free(e);
}

static void
_ecore_ipc_event_server_del_free(void *data, void *ev)
{
   Ecore_Ipc_Event_Server_Del *e;
   
   e = ev;
   free(e);
}

static void
_ecore_ipc_event_client_data_free(void *data, void *ev)
{
   Ecore_Ipc_Event_Client_Data *e;
   
   e = ev;
   if (e->data) free(e->data);
   free(e);
}

static void
_ecore_ipc_event_server_data_free(void *data, void *ev)
{
   Ecore_Ipc_Event_Server_Data *e;
   
   e = ev;
   if (e->data) free(e->data);
   free(e);
}
