/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

/* by Azundris, with thanks to Corey Donohoe <atmos@atmos.org> */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <ctype.h>

#include <Ecore.h>
#include <Ecore_Ipc.h>

#include "ecore_private.h"
#include "ecore_config_ipc.h"
#include "ecore_config_util.h"
#include "ecore_config_private.h"

#include "Ecore_Config.h"


/*****************************************************************************/

static int
_ecore_config_ipc_ecore_string_get(char **m, char **r)
{
   char               *q;
   int                 l = 0;

   if (!m || !*m)
      return ECORE_CONFIG_ERR_NODATA;
   if (!r)
      return ECORE_CONFIG_ERR_FAIL;
   q = *m;
   if (*q != 's')
      return ECORE_CONFIG_ERR_TYPEMISMATCH;
   q++;
   l = (*(q++)) << 8;
   l += *(q++);
   *r = q;
   q += l;
   *m = q;
   E(1, "IPC/eCore: got string-%d \"%s\"\n", l, *r);
   return ECORE_CONFIG_ERR_SUCC;
}

static char               *
_ecore_config_ipc_global_prop_list(Ecore_Config_Server * srv __UNUSED__, long serial __UNUSED__)
{
   Ecore_Config_DB_File  *db;
   char                 **keys;
   int                    key_count, x;
   estring               *s;
   int                    f;
   char                   buf[PATH_MAX], *p; 	
   // char		*data;   		UNUSED
   Ecore_Config_Type      type;

   db = NULL;
   s = estring_new(8192);
   f = 0;
   if ((p = getenv("HOME")))
     {
	snprintf(buf, sizeof(buf), "%s/.e/config.eet", p);
	if (!(db = _ecore_config_db_open_read(buf)))
	  {
	     strcpy(buf, PACKAGE_DATA_DIR"/system.eet");
	     if (!(db = _ecore_config_db_open_read(buf)))
	       return NULL;
	  }
     }
   if (!db) return NULL;
   key_count = 0;
   keys = _ecore_config_db_keys_get(db, &key_count);
   if (keys)
     {
	for (x = 0; x < key_count; x++)
	  {
	     type = _ecore_config_db_key_type_get(db, keys[x]);
	     switch (type)
	       {
		  case ECORE_CONFIG_INT:
		    estring_appendf(s, "%s%s: integer", f ? "\n" : "", keys[x]);
		    break;
		  case ECORE_CONFIG_BLN:
		    estring_appendf(s, "%s%s: boolean", f ? "\n" : "", keys[x]);
		    break;
		  case ECORE_CONFIG_FLT:
		    estring_appendf(s, "%s%s: float", f ? "\n" : "", keys[x]);
		    break;
		  case ECORE_CONFIG_STR:
		    estring_appendf(s, "%s%s: string", f ? "\n" : "", keys[x]);
		    break;
		  case ECORE_CONFIG_RGB:
		    estring_appendf(s, "%s%s: colour", f ? "\n" : "", keys[x]);
		    break;
		  case ECORE_CONFIG_THM:
		    estring_appendf(s, "%s%s: theme", f ? "\n" : "", keys[x]);
		    break;
		  case ECORE_CONFIG_SCT:
		    estring_appendf(s, "%s%s: structure", f ? "\n" : "", keys[x]);
		    break;
		  default:
		    estring_appendf(s, "%s%s: unknown", f ? "\n" : "", keys[x]);
		    continue;
	       }
	     f = 1;
	  }
     }
   _ecore_config_db_close(db);
   if (keys)
     {
	for (x = 0; x < key_count; x++)
	  {
	     free(keys[x]);
	  }
	free(keys);
     }
   
   return estring_disown(s);
}

/*****************************************************************************/

static int
_ecore_config_ipc_ecore_send(Ecore_Ipc_Event_Client_Data * e, int code,
			     char *reply)
{
   static int          our_ref = 0;
   int                 len = reply ? strlen(reply) + 1 : 0;

   our_ref++;
   E(1, "IPC/eCore: replying [0,0] %d IRT %d => %d {\"%s\":%d}\n", our_ref,
     e->ref, code, reply ? reply : "", len);
   return ecore_ipc_client_send(e->client, 0, 0, our_ref, e->ref, code, reply,
				len);
}

/*****************************************************************************/

static int
_ecore_config_ipc_ecore_handle_request(Ecore_Ipc_Server * server,
				       Ecore_Ipc_Event_Client_Data * e)
{
   Ecore_Config_Server *srv;
   long                serial;
   int                 ret;
   char               *r, *k, *v, *m;

   srv = _ecore_config_server_convert(server);
   serial = e->minor;
   ret = ECORE_CONFIG_ERR_FAIL;
   r = NULL;
   m = (char *)e->data;
   E(1, "IPC/eCore: client sent: [%d,%d] #%d (%d) @ %p\n", e->major, e->minor,
     e->ref, e->size, server);

   switch (e->major)
     {
     case IPC_PROP_LIST:
	if (srv == __ecore_config_server_global)
	   r = _ecore_config_ipc_global_prop_list(srv, serial);
	else
	   r = _ecore_config_ipc_prop_list(srv, serial);
	break;
     case IPC_PROP_DESC:
	if (_ecore_config_ipc_ecore_string_get(&m, &k) == ECORE_CONFIG_ERR_SUCC)
	   r = _ecore_config_ipc_prop_desc(srv, serial, k);
	break;
     case IPC_PROP_GET:
	if (_ecore_config_ipc_ecore_string_get(&m, &k) == ECORE_CONFIG_ERR_SUCC)
	   r = _ecore_config_ipc_prop_get(srv, serial, k);
	break;
     case IPC_PROP_SET:
	if (_ecore_config_ipc_ecore_string_get(&m, &k) == ECORE_CONFIG_ERR_SUCC)
	  {
	     if (_ecore_config_ipc_ecore_string_get(&m, &v) ==
		 ECORE_CONFIG_ERR_SUCC)
		return _ecore_config_ipc_ecore_send(e,
						    _ecore_config_ipc_prop_set
						    (srv, serial, k, v), NULL);
	  }
	break;

     case IPC_BUNDLE_LIST:
	r = _ecore_config_ipc_bundle_list(srv);
	break;
     case IPC_BUNDLE_NEW:
	if (_ecore_config_ipc_ecore_string_get(&m, &k) == ECORE_CONFIG_ERR_SUCC)
	   return _ecore_config_ipc_ecore_send(e,
					       k ?
					       _ecore_config_ipc_bundle_new(srv,
									    k) :
					       ECORE_CONFIG_ERR_FAIL, NULL);
	break;
     case IPC_BUNDLE_LABEL_SET:
	if (_ecore_config_ipc_ecore_string_get(&m, &k) == ECORE_CONFIG_ERR_SUCC)
	   return _ecore_config_ipc_ecore_send(e,
					       k ?
					       _ecore_config_ipc_bundle_label_set
					       (srv, serial,
						k) : ECORE_CONFIG_ERR_FAIL,
					       NULL);
	break;
     case IPC_BUNDLE_LABEL_FIND:
	if (_ecore_config_ipc_ecore_string_get(&m, &k) == ECORE_CONFIG_ERR_SUCC)
	   return _ecore_config_ipc_ecore_send(e,
					       _ecore_config_ipc_bundle_label_find
					       (srv, k), NULL);
	break;
     case IPC_BUNDLE_LABEL_GET:
	r = _ecore_config_ipc_bundle_label_get(srv, serial);
	break;
     }

   ret =
      _ecore_config_ipc_ecore_send(e,
				   r ? ECORE_CONFIG_ERR_SUCC :
				   ECORE_CONFIG_ERR_FAIL, r);
   if (r)
     {
	free(r);
	return ret;
     }
   return ECORE_CONFIG_ERR_NOTFOUND;
}

/*****************************************************************************/

static int
_ecore_config_ipc_client_add(void *data, int type __UNUSED__, void *event)
{
   Ecore_Ipc_Server  **server;
   Ecore_Ipc_Event_Client_Data *e;

   server = (Ecore_Ipc_Server **) data;
   e = (Ecore_Ipc_Event_Client_Data *) event;

   if (*server != ecore_ipc_client_server_get(e->client))
      return 1;

   E(1, "IPC/eCore: Client connected. @ %p\n", server);
   return 1;
}

static int
_ecore_config_ipc_client_del(void *data, int type __UNUSED__, void *event)
{
   Ecore_Ipc_Server  **server;
   Ecore_Ipc_Event_Client_Data *e;

   server = (Ecore_Ipc_Server **) data;
   e = (Ecore_Ipc_Event_Client_Data *) event;

   if (*server != ecore_ipc_client_server_get(e->client))
      return 1;

   E(1, "IPC/eCore: Client disconnected. @ %p\n", server);
   return 1;
}

static int
_ecore_config_ipc_client_sent(void *data, int type __UNUSED__, void *event)
{
   Ecore_Ipc_Server  **server;
   Ecore_Ipc_Event_Client_Data *e;

   server = (Ecore_Ipc_Server **) data;
   e = (Ecore_Ipc_Event_Client_Data *) event;

   if (*server != ecore_ipc_client_server_get(e->client))
      return 1;

   _ecore_config_ipc_ecore_handle_request(*server, e);
   return 1;
}

/*****************************************************************************/

int
_ecore_config_ipc_ecore_init(const char *pipe_name, void **data)
{
   Ecore_Ipc_Server  **server;
   struct stat         st;
   char               *p;
   int                 port;
   char                socket[PATH_MAX];

   server = (Ecore_Ipc_Server **) data;
   port = 0;
   if (!server)
      return ECORE_CONFIG_ERR_FAIL;

/*  if(*server)
      return ECORE_CONFIG_ERR_IGNORED; */

   ecore_init();
   if (ecore_ipc_init() < 1)
      return ECORE_CONFIG_ERR_FAIL;

   if ((p = getenv("HOME")))
     {				/* debug-only ### FIXME */
	int                 stale;

	stale = 1;
	while (stale)
	  {
	     snprintf(socket, PATH_MAX, "%s/.ecore/%s/%d", p, pipe_name, port);

	     if (!stat(socket, &st))
	       {
		  E(0, "IPC/eCore: pipe \"%s\" already exists!?\n", socket);
/*      if(unlink(buf))
  	E(0,"IPC/eCore: could not remove pipe \"%s\": %d\n",buf,errno); }}*/
		  port++;
	       }
	     else
	       {
		  stale = 0;
	       }
	  }
     }
   *server = ecore_ipc_server_add(ECORE_IPC_LOCAL_USER, pipe_name, port, NULL);
   ecore_event_handler_add(ECORE_IPC_EVENT_CLIENT_ADD,
			   _ecore_config_ipc_client_add, server);
   ecore_event_handler_add(ECORE_IPC_EVENT_CLIENT_DEL,
			   _ecore_config_ipc_client_del, server);
   ecore_event_handler_add(ECORE_IPC_EVENT_CLIENT_DATA,
			   _ecore_config_ipc_client_sent, server);

   if (server)
     {
	E(1, "IPC/eCore: Server is listening on %s.\n", pipe_name);
     }

   return ECORE_CONFIG_ERR_SUCC;
}

int
_ecore_config_ipc_ecore_exit(void **data)
{
   int                 ret;
   Ecore_Ipc_Server  **server;

   ret = ECORE_CONFIG_ERR_SUCC;
   server = (Ecore_Ipc_Server **) data;

   if (!server)
      return ECORE_CONFIG_ERR_FAIL;

   if (*server)
     {
	ecore_ipc_server_del(*server);
	*server = NULL;
     }

   ecore_ipc_shutdown();

   return ret;
}

/*****************************************************************************/

int
_ecore_config_ipc_ecore_poll(void **data)
{
   Ecore_Ipc_Server  **server;

   server = (Ecore_Ipc_Server **) data;

   if (!server)
      return ECORE_CONFIG_ERR_FAIL;

   ecore_main_loop_iterate();

   return ECORE_CONFIG_ERR_SUCC;
}

/*****************************************************************************/
