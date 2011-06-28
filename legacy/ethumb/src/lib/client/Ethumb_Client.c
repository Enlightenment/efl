/**
 * @file
 *
 * This is the client-server thumbnail library, see @ref
 * tutorial_ethumb_client.
 *
 * Copyright (C) 2009 by ProFUSION embedded systems
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library;
 * if not, see <http://www.gnu.org/licenses/>.
 *
 * @author Rafael Antognolli <antognolli@profusion.mobi>
 * @author Gustavo Sverzut Barbieri <barbieri@profusion.mobi>
 */

/**
 * @page tutorial_ethumb_client Client-Server Thumbnailing Tutorial
 *
 * @section tutorial_ethumb_client_intro Introduction
 *
 * Ethumb provides both in process and client-server generation
 * methods. The advantage of the client-server method is that current
 * process will not do the heavy operations that may block, stopping
 * animations and other user interactions. Instead the client library
 * will configure a local #Ethumb instance and mirrors/controls a
 * remote process using DBus. The simple operations like most setters
 * and getters as well as checking for thumbnail existence
 * (ethumb_client_thumb_exists()) is done locally, while expensive
 * (ethumb_client_generate()) are done on server and then reported
 * back to application when it is finished (both success or failure).
 *
 * @section tutorial_ethumb_client_connect Connecting to Server
 *
 * TODO
 *
 * @section tutorial_ethumb_client_generate Requesting Thumbnail Generation
 *
 * TODO
 *
 * @section tutorial_ethumb_client_setup Setup Extra Thumbnail Parameters
 *
 * TODO
 *
 * @section tutorial_ethumb_client_server_died Handle Server Disconnection
 *
 * TODO
 */

/**
 * @cond LOCAL
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <stdbool.h>

#include <Eina.h>
#include <eina_safety_checks.h>
#include <E_DBus.h>
#include <Ethumb.h>
#include <Ecore.h>

#include "Ethumb_Client.h"

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

#define MAX_ID 2000000

static int _log_dom = -1;
#define DBG(...) EINA_LOG_DOM_DBG(_log_dom, __VA_ARGS__)
#define INF(...) EINA_LOG_DOM_INFO(_log_dom, __VA_ARGS__)
#define WRN(...) EINA_LOG_DOM_WARN(_log_dom, __VA_ARGS__)
#define ERR(...) EINA_LOG_DOM_ERR(_log_dom, __VA_ARGS__)
#define CRITICAL(...) EINA_LOG_DOM_CRIT(_log_dom, __VA_ARGS__)

struct _Ethumb_Client
{
   Ethumb *ethumb;
   int id_count;

   E_DBus_Connection *conn;
   E_DBus_Signal_Handler *name_owner_changed_handler;
   E_DBus_Signal_Handler *generated_signal;
   DBusPendingCall *pending_get_name_owner;
   DBusPendingCall *pending_start_service_by_name;
   const char *unique_name;
   DBusPendingCall *pending_new;
   struct {
      Ethumb_Client_Connect_Cb cb;
      void *data;
      Eina_Free_Cb free_data;
   } connect;
   Eina_List *pending_add;
   Eina_List *pending_remove;
   Eina_List *pending_gen;
   DBusPendingCall *pending_clear;
   DBusPendingCall *pending_setup;
   struct {
      Ethumb_Client_Die_Cb cb;
      void *data;
      Eina_Free_Cb free_data;
   } die;
   const char *object_path;

   EINA_REFCOUNT;

   Eina_Bool ethumb_dirty : 1;
   Eina_Bool connected : 1;
   Eina_Bool server_started : 1;
};

struct _ethumb_pending_add
{
   dbus_int32_t id;
   const char *file;
   const char *key;
   const char *thumb;
   const char *thumb_key;
   Ethumb_Client_Generate_Cb generated_cb;
   void *data;
   Eina_Free_Cb free_data;
   DBusPendingCall *pending_call;
   Ethumb_Client *client;
};

struct _ethumb_pending_remove
{
   dbus_int32_t id;
   Ethumb_Client_Generate_Cancel_Cb cancel_cb;
   void *data;
   Eina_Free_Cb free_data;
   DBusPendingCall *pending_call;
   Ethumb_Client *client;
};

struct _ethumb_pending_gen
{
   dbus_int32_t id;
   const char *file;
   const char *key;
   const char *thumb;
   const char *thumb_key;
   Ethumb_Client_Generate_Cb generated_cb;
   void *data;
   Eina_Free_Cb free_data;
};

typedef struct _Ethumb_Async_Exists Ethumb_Async_Exists;
typedef struct _Ethumb_Async_Exists_Cb Ethumb_Async_Exists_Cb;

struct _Ethumb_Async_Exists
{
   Ethumb *dup;
   Ethumb_Client *source;

   Eina_List *callbacks;

   Ecore_Thread *thread;
   EINA_REFCOUNT;

   Eina_Bool exists : 1;
   Eina_Bool cancel : 1;
};

struct _Ethumb_Async_Exists_Cb
{
   Ethumb_Client_Thumb_Exists_Cb exists_cb;
   const void *data;
};

static const char _ethumb_dbus_bus_name[] = "org.enlightenment.Ethumb";
static const char _ethumb_dbus_interface[] = "org.enlightenment.Ethumb";
static const char _ethumb_dbus_objects_interface[] = "org.enlightenment.Ethumb.objects";
static const char _ethumb_dbus_path[] = "/org/enlightenment/Ethumb";
static const char fdo_interface[] = "org.freedesktop.DBus";
static const char fdo_bus_name[] = "org.freedesktop.DBus";
static const char fdo_path[] = "/org/freedesktop/DBus";

static int _initcount = 0;
static Eina_Hash *_exists_request = NULL;

static void _ethumb_client_generated_cb(void *data, DBusMessage *msg);
static void _ethumb_client_get_name_owner(void *data, DBusMessage *msg, DBusError *err);

static inline bool
__dbus_callback_check_and_init(const char *file, int line, const char *function, DBusMessage *msg, DBusMessageIter *itr, DBusError *err)
{
   if (!msg)
     {
	ERR("%s:%d:%s() callback without message arguments!",
		file, line, function);

	if (err)
	  ERR("%s:%d:%s() an error was reported by server: "
		  "name=\"%s\", message=\"%s\"",
		  file, line, function, err->name, err->message);

	return 0;
     }

   if (!dbus_message_iter_init(msg, itr))
     {
	ERR("%s:%d:%s() could not init iterator.",
		file, line, function);
	return 0;
     }

   return 1;
}

#define _dbus_callback_check_and_init(msg, itr, err)			\
  __dbus_callback_check_and_init(__FILE__, __LINE__, __FUNCTION__,	\
				 msg, itr, err)

static inline bool
__dbus_iter_type_check(int type, int expected, const char *expected_name)
{
   if (type == expected)
     return 1;

   ERR("expected type %s (%c) but got %c instead!",
	   expected_name, expected, type);

   return 0;
}
#define _dbus_iter_type_check(t, e) __dbus_iter_type_check(t, e, #e)

#define CHECK_NULL_RETURN(ptr, ...)					\
  do									\
    {									\
       if ((ptr) == NULL)						\
	 {								\
	    CRITICAL("%s == NULL!", #ptr);				\
	    return __VA_ARGS__;						\
	 }								\
    }									\
  while (0)

static void
_ethumb_client_free(Ethumb_Client *client)
{
   void *data;

   if (!client->connected)
     goto end_connection;

   EINA_LIST_FREE(client->pending_add, data)
     {
	struct _ethumb_pending_add *pending = data;
	eina_stringshare_del(pending->file);
	eina_stringshare_del(pending->key);
	eina_stringshare_del(pending->thumb);
	eina_stringshare_del(pending->thumb_key);
	dbus_pending_call_cancel(pending->pending_call);
	dbus_pending_call_unref(pending->pending_call);
	if (pending->free_data)
	  pending->free_data(pending->data);
	free(pending);
     }

   EINA_LIST_FREE(client->pending_gen, data)
     {
	struct _ethumb_pending_gen *pending = data;
	eina_stringshare_del(pending->file);
	eina_stringshare_del(pending->key);
	eina_stringshare_del(pending->thumb);
	eina_stringshare_del(pending->thumb_key);
	if (pending->free_data)
	  pending->free_data(pending->data);
	free(pending);
     }

   EINA_LIST_FREE(client->pending_remove, data)
     {
	struct _ethumb_pending_remove *pending = data;
	dbus_pending_call_cancel(pending->pending_call);
	dbus_pending_call_unref(pending->pending_call);
	if (pending->free_data)
	  pending->free_data(pending->data);
	free(pending);
     }

   if (client->pending_clear)
     {
	dbus_pending_call_cancel(client->pending_clear);
	dbus_pending_call_unref(client->pending_clear);
     }

end_connection:
   if (client->object_path)
     eina_stringshare_del(client->object_path);

   if (client->pending_new)
     dbus_pending_call_cancel(client->pending_new);

   if (client->unique_name)
     eina_stringshare_del(client->unique_name);

   if (client->pending_get_name_owner)
     dbus_pending_call_cancel(client->pending_get_name_owner);

   if (client->pending_start_service_by_name)
     dbus_pending_call_cancel(client->pending_start_service_by_name);

   ethumb_free(client->ethumb);

   e_dbus_signal_handler_del(client->conn, client->name_owner_changed_handler);
   if (client->connected)
     e_dbus_signal_handler_del(client->conn, client->generated_signal);
   e_dbus_connection_close(client->conn);

   if (client->connect.free_data)
     client->connect.free_data(client->connect.data);
   if (client->die.free_data)
     client->die.free_data(client->die.data);

   free(client);
}

static void
_ethumb_async_delete(void *data)
{
   Ethumb_Async_Exists *async = data;

   ethumb_free(async->dup);

   EINA_REFCOUNT_UNREF(async->source)
     _ethumb_client_free(async->source);

   free(async);
}

static void
_ethumb_async_cancel(Ethumb_Async_Exists *async)
{
   async->cancel = EINA_TRUE;
   ecore_thread_cancel(async->thread);
}

static void
_ethumb_client_name_owner_changed(void *data, DBusMessage *msg)
{
   DBusError err;
   const char *name, *from, *to;
   Ethumb_Client *client = data;

   dbus_error_init(&err);
   if (!dbus_message_get_args(msg, &err,
       DBUS_TYPE_STRING, &name,
       DBUS_TYPE_STRING, &from,
       DBUS_TYPE_STRING, &to,
       DBUS_TYPE_INVALID))
     {
	ERR("could not get NameOwnerChanged arguments: %s: %s",
	    err.name, err.message);
	dbus_error_free(&err);
	return;
     }

   if(!from || !name)
       return ;
   if (strcmp(name, _ethumb_dbus_bus_name) != 0)
     return;

   DBG("NameOwnerChanged from=[%s] to=[%s]", from, to);

   if (from[0] != '\0' && to[0] == '\0')
     {
	DBG("exit ethumbd at %s", from);
	if (client->unique_name && strcmp(client->unique_name, from) != 0)
	  WRN("%s was not the known name %s, ignored.",
	       from, client->unique_name);
	else if(client->unique_name)
	  {
	     ERR("server exit!!!");
	     if (client->die.cb)
	       {
		  client->die.cb(client->die.data, client);
		  client->die.cb = NULL;
	       }
	     if (client->die.free_data)
	       {
		  client->die.free_data(client->die.data);
		  client->die.free_data = NULL;
		  client->die.data = NULL;
	       }
	  }
     }
   else
     DBG("unknown change from %s to %s", from, to);
}

static void
_ethumb_client_report_connect(Ethumb_Client *client, Eina_Bool success)
{
   if (!client->connect.cb)
     {
	ERR("already called?!");
	return;
     }

   client->connect.cb(client->connect.data, client, success);
   if (client->connect.free_data)
     {
	client->connect.free_data(client->connect.data);
	client->connect.free_data = NULL;
     }
   client->connect.cb = NULL;
   client->connect.data = NULL;
}

static void
_ethumb_client_new_cb(void *data, DBusMessage *msg, DBusError *error)
{
   DBusMessageIter iter;
   const char *opath;
   int t;
   Ethumb_Client *client = data;

   client->pending_new = NULL;

   if (!_dbus_callback_check_and_init(msg, &iter, error))
     goto end_error;
   t = dbus_message_iter_get_arg_type(&iter);
   if (!_dbus_iter_type_check(t, DBUS_TYPE_OBJECT_PATH))
     goto end_error;

   dbus_message_iter_get_basic(&iter, &opath);
   if (opath[0] == '\0')
     goto end_error;

   client->object_path = eina_stringshare_add(opath);

   client->generated_signal = e_dbus_signal_handler_add(
      client->conn, _ethumb_dbus_bus_name, opath,
      _ethumb_dbus_objects_interface, "generated",
      _ethumb_client_generated_cb, client);

   _ethumb_client_report_connect(client, 1);
   return;

end_error:
   _ethumb_client_report_connect(client, 0);
}

static void
_ethumb_client_call_new(Ethumb_Client *client)
{
   DBusMessage *msg;

   msg = dbus_message_new_method_call(_ethumb_dbus_bus_name, _ethumb_dbus_path,
				      _ethumb_dbus_interface, "new");
   client->pending_new = e_dbus_message_send(client->conn, msg,
					     _ethumb_client_new_cb, -1,
					     client);
   dbus_message_unref(msg);
}

static void
_ethumb_client_start_server_cb(void *data, DBusMessage *msg, DBusError *err)
{
   Ethumb_Client *client = data;
   DBusMessageIter iter;
   dbus_uint32_t ret;
   int t;

   client->pending_start_service_by_name = NULL;

   if (!_dbus_callback_check_and_init(msg, &iter, err))
     goto error;

   t = dbus_message_iter_get_arg_type(&iter);
   if (!_dbus_iter_type_check(t, DBUS_TYPE_UINT32))
     goto error;

   dbus_message_iter_get_basic(&iter, &ret);
   if ((ret != 1) && (ret != 2))
     {
	ERR("Error starting Ethumbd DBus service by its name: retcode %u",
	    ret);
	goto error;
     }

   client->server_started = 1;
   DBG("Ethumbd DBus service started successfully (%d), now request its name",
       ret);

   if (client->pending_get_name_owner)
     {
	DBG("already requesting name owner, cancel and try again");
	dbus_pending_call_cancel(client->pending_get_name_owner);
     }

   client->pending_get_name_owner = e_dbus_get_name_owner
     (client->conn, _ethumb_dbus_bus_name, _ethumb_client_get_name_owner,
      client);
   if (!client->pending_get_name_owner)
     {
	ERR("could not create a get_name_owner request.");
	goto error;
     }

   return;

 error:
   ERR("failed to start Ethumbd DBus service by its name.");
   _ethumb_client_report_connect(client, 0);
}

static void
_ethumb_client_start_server(Ethumb_Client *client)
{
   if (client->pending_start_service_by_name)
     {
	DBG("already pending start service by name.");
	return;
     }

   client->server_started = 0;
   client->pending_start_service_by_name = e_dbus_start_service_by_name
     (client->conn, _ethumb_dbus_bus_name, 0, _ethumb_client_start_server_cb,
      client);
   if (!client->pending_start_service_by_name)
     {
	ERR("could not start service by name!");
	_ethumb_client_report_connect(client, 0);
     }
}

static void
_ethumb_client_get_name_owner(void *data, DBusMessage *msg, DBusError *err)
{
   DBusMessageIter iter;
   const char *uid;
   Ethumb_Client *client = data;
   int t;

   client->pending_get_name_owner = NULL;

   if (dbus_error_is_set(err) && (!client->server_started))
     {
	DBG("could not find server (%s), try to start it...", err->message);
	_ethumb_client_start_server(client);
	return;
     }

   if (!_dbus_callback_check_and_init(msg, &iter, err))
     goto error;

   t = dbus_message_iter_get_arg_type(&iter);
   if (!_dbus_iter_type_check(t, DBUS_TYPE_STRING))
     goto error;

   dbus_message_iter_get_basic(&iter, &uid);
   if (!uid)
     {
	ERR("no name owner!");
	goto error;
     }

   DBG("unique name = %s", uid);
   client->unique_name = eina_stringshare_add(uid);

   _ethumb_client_call_new(client);
   client->connected = 1;
   return;

error:
   _ethumb_client_report_connect(client, 0);
}

static void
_ethumb_client_exists_heavy(void *data, Ecore_Thread *thread)
{
   Ethumb_Async_Exists *async = data;

   async->exists = ethumb_exists(async->dup);
}

static void
_ethumb_client_exists_end(void *data, Ecore_Thread *thread)
{
   Ethumb_Async_Exists *async = data;
   Ethumb_Async_Exists_Cb *cb;
   Ethumb *tmp = async->source->ethumb;

   async->source->ethumb = async->dup;
   async->source->ethumb_dirty = ethumb_cmp(tmp, async->dup);

   EINA_LIST_FREE(async->callbacks, cb)
     cb->exists_cb(async->source, (Ethumb_Exists*) async, async->exists, (void*) cb->data);

   async->source->ethumb = tmp;
   async->thread = NULL;

   eina_hash_del(_exists_request, async->dup, async);
}

/**
 * @endcond
 */

/**
 * @brief Initialize the Ethumb_Client library.
 *
 * @return 1 or greater on success, 0 on error.
 *
 * This function sets up all the Ethumb_Client module dependencies. It
 * returns 0 on failure (that is, when one of the dependency fails to
 * initialize), otherwise it returns the number of times it has
 * already been called.
 *
 * When Ethumb_Client is not used anymore, call
 * ethumb_client_shutdown() to shut down the Ethumb_Client library.
 *
 * @see ethumb_client_shutdown()
 * @see ethumb_client_connect()
 * @see @ref tutorial_ethumb_client
 */
EAPI int
ethumb_client_init(void)
{
   if (_initcount)
     return ++_initcount;

   if (!eina_init())
     {
	fprintf(stderr, "ERROR: Could not initialize log module.\n");
	return 0;
     }
   _log_dom = eina_log_domain_register("ethumb_client", EINA_COLOR_YELLOW);
   if (_log_dom < 0)
     {
	EINA_LOG_ERR("Could not register log domain: ethumb_client");
	eina_shutdown();
	return 0;
     }

   ethumb_init();
   e_dbus_init();

   _exists_request = eina_hash_new(ethumb_length,
                                   ethumb_key_cmp,
                                   ethumb_hash,
                                   _ethumb_async_delete,
                                   3);

   return ++_initcount;
}

/**
 * @brief Shut down the Ethumb_Client library.
 *
 * @return 0 when everything is shut down, 1 or greater if there are
 *         other users of the Ethumb_Client library pending shutdown.
 *
 * This function shuts down the Ethumb_Client library. It returns 0
 * when it has been called the same number of times than
 * ethumb_client_init(). In that case it shut down all the
 * Ethumb_Client modules dependencies.
 *
 * Once this function succeeds (that is, @c 0 is returned), you must
 * not call any of the Eina function anymore. You must call
 * ethumb_client_init() again to use the Ethumb_Client functions
 * again.
 */
EAPI int
ethumb_client_shutdown(void)
{
   _initcount--;
   if (_initcount > 0)
     return _initcount;

   /* should find a non racy solution to closing all pending exists request */

   e_dbus_shutdown();
   ethumb_shutdown();
   eina_log_domain_unregister(_log_dom);
   _log_dom = -1;
   eina_shutdown();
   return _initcount;
}

/**
 * Connects to Ethumb server and return the client instance.
 *
 * This is the "constructor" of Ethumb_Client, where everything
 * starts.
 *
 * If server was down, it is tried to start it using DBus activation,
 * then the connection is retried.
 *
 * This call is asynchronous and will not block, instead it will be in
 * "not connected" state until @a connect_cb is called with either
 * success or failure. On failure, then no methods should be
 * called. On success you're now able to setup and then ask generation
 * of thumbnails.
 *
 * Usually you should listen for server death/disconenction with
 * ethumb_client_on_server_die_callback_set().
 *
 * @param connect_cb function to call to report connection success or
 *        failure. Do not call any other ethumb_client method until
 *        this function returns. The first received parameter is the
 *        given argument @a data. Must @b not be @c NULL. This
 *        function will not be called if user explicitly calls
 *        ethumb_client_disconnect().
 * @param data context to give back to @a connect_cb. May be @c NULL.
 * @param free_data function used to release @a data resources, if
 *        any. May be @c NULL. If this function exists, it will be
 *        called immediately after @a connect_cb is called or if user
 *        explicitly calls ethumb_client_disconnect() before such
 *        (that is, don't rely on @a data after @a connect_cb was
 *        called!)
 *
 * @return client instance or NULL if failed. If @a connect_cb is
 *         missing it returns @c NULL. If it fail for other
 *         conditions, @c NULL is also returned and @a connect_cb is
 *         called with @c success=EINA_FALSE. The client instance is
 *         not ready to be used until @a connect_cb is called.
 */
EAPI Ethumb_Client *
ethumb_client_connect(Ethumb_Client_Connect_Cb connect_cb, const void *data, Eina_Free_Cb free_data)
{
   Ethumb_Client *eclient;

   EINA_SAFETY_ON_NULL_RETURN_VAL(connect_cb, NULL);

   eclient = calloc(1, sizeof(*eclient));
   if (!eclient)
     {
	ERR("could not allocate Ethumb_Client structure.");
	goto err;
     }

   eclient->connect.cb = connect_cb;
   eclient->connect.data = (void *)data;
   eclient->connect.free_data = free_data;

   eclient->ethumb = ethumb_new();
   if (!eclient->ethumb)
     {
	ERR("could not create ethumb handler.");
	goto ethumb_new_err;
     }

   eclient->conn = e_dbus_bus_get(DBUS_BUS_SESSION);
   if (!eclient->conn)
     {
	ERR("could not connect to session bus.");
	goto connection_err;
     }

   eclient->name_owner_changed_handler = e_dbus_signal_handler_add(
	 eclient->conn, fdo_bus_name, fdo_path, fdo_interface,
	 "NameOwnerChanged", _ethumb_client_name_owner_changed, eclient);

   eclient->pending_get_name_owner = e_dbus_get_name_owner(
	 eclient->conn, _ethumb_dbus_bus_name, _ethumb_client_get_name_owner,
	 eclient);
   if (!eclient->pending_get_name_owner)
     {
	ERR("could not create a get_name_owner request.");
	goto connection_err;
     }

   EINA_REFCOUNT_INIT(eclient);

   return eclient;

connection_err:
   ethumb_free(eclient->ethumb);
ethumb_new_err:
   free(eclient);
err:
   connect_cb((void *)data, NULL, EINA_FALSE);
   if (free_data)
     free_data((void *)data);
   return NULL;
}

/**
 * Disconnect the client, releasing all client resources.
 *
 * This is the destructor of Ethumb_Client, after it's disconnected
 * the client handle is now gone and should not be used.
 *
 * @param client client instance to be destroyed. Must @b not be @c
 *        NULL.
 */
EAPI void
ethumb_client_disconnect(Ethumb_Client *client)
{
   EINA_SAFETY_ON_NULL_RETURN(client);

   EINA_REFCOUNT_UNREF(client)
     _ethumb_client_free(client);
}

/**
 * Sets the callback to report server died.
 *
 * When server dies there is nothing you can do, just release
 * resources with ethumb_client_disconnect() and probably try to
 * connect again.
 *
 * Usually you should set this callback and handle this case, it does
 * happen!
 *
 * @param client the client instance to monitor. Must @b not be @c
 *        NULL.
 * @param server_die_cb function to call back when server dies. The
 *        first parameter will be the argument @a data. May be @c
 *        NULL.
 * @param data context to give back to @a server_die_cb. May be @c
 *        NULL.
 * @param free_data used to release @a data resources after @a
 *        server_die_cb is called or user calls
 *        ethumb_client_disconnect().
 */
EAPI void
ethumb_client_on_server_die_callback_set(Ethumb_Client *client, Ethumb_Client_Die_Cb server_die_cb, const void *data, Eina_Free_Cb free_data)
{
   EINA_SAFETY_ON_NULL_RETURN(client);

   if (client->die.free_data)
     client->die.free_data(client->die.data);

   client->die.cb = server_die_cb;
   client->die.data = (void *)data;
   client->die.free_data = free_data;
}

/**
 * @cond LOCAL
 */

static void
_ethumb_client_ethumb_setup_cb(void *data, DBusMessage *msg, DBusError *error)
{
   DBusMessageIter iter;
   int t;
   dbus_bool_t result = 0;
   Ethumb_Client *client = data;

   client->pending_setup = NULL;

   if (!_dbus_callback_check_and_init(msg, &iter, error))
     return;

   t = dbus_message_iter_get_arg_type(&iter);
   if (!_dbus_iter_type_check(t, DBUS_TYPE_BOOLEAN))
     return;

   dbus_message_iter_get_basic(&iter, &result);
}

static const char *
_ethumb_client_dbus_get_bytearray(DBusMessageIter *iter)
{
   int el_type;
   int length;
   DBusMessageIter riter;
   const char *result;

   el_type = dbus_message_iter_get_element_type(iter);
   if (el_type != DBUS_TYPE_BYTE)
     {
	ERR("not an byte array element.");
	return NULL;
     }

   dbus_message_iter_recurse(iter, &riter);
   dbus_message_iter_get_fixed_array(&riter, &result, &length);

   if (result[0] == '\0')
     return NULL;
   else
     return eina_stringshare_add(result);
}

static void
_ethumb_client_dbus_append_bytearray(DBusMessageIter *iter, const char *string)
{
   DBusMessageIter viter;

   if (!string)
     string = "";

   dbus_message_iter_open_container(iter, DBUS_TYPE_ARRAY, "y", &viter);
   dbus_message_iter_append_fixed_array(&viter, DBUS_TYPE_BYTE, &string,
					strlen(string) + 1);
   dbus_message_iter_close_container(iter, &viter);
}

/**
 * @endcond
 */

/**
 * Send setup to server.
 *
 * This method is called automatically by ethumb_client_generate() if
 * any property was changed. No need to call it manually.
 *
 * @param client client instance. Must @b not be @c NULL and client
 *        must be connected (after connected_cb is called).
 */
EAPI void
ethumb_client_ethumb_setup(Ethumb_Client *client)
{
   DBusMessage *msg;
   DBusMessageIter iter, aiter, diter, viter, vaiter;
   Ethumb *e = client->ethumb;
   const char *entry;
   dbus_int32_t tw, th, format, aspect, orientation, quality, compress;
   float cx, cy;
   double t;
   const char *theme_file, *group, *swallow;
   const char *directory, *category;
   double video_time, video_start, video_interval;
   dbus_int32_t video_ntimes, video_fps, document_page;

   EINA_SAFETY_ON_NULL_RETURN(client);
   EINA_SAFETY_ON_FALSE_RETURN(client->connected);
   client->ethumb_dirty = 0;

   msg = dbus_message_new_method_call(_ethumb_dbus_bus_name,
				      client->object_path,
				      _ethumb_dbus_objects_interface,
				      "ethumb_setup");
   dbus_message_iter_init_append(msg, &iter);
   dbus_message_iter_open_container(&iter, DBUS_TYPE_ARRAY, "{sv}", &aiter);

/**
 * @cond LOCAL
 */
#define _open_variant_iter(str_entry, str_type, end_iter)		   \
   entry = str_entry;							   \
   dbus_message_iter_open_container(&aiter, DBUS_TYPE_DICT_ENTRY, NULL, &diter); \
   dbus_message_iter_append_basic(&diter, DBUS_TYPE_STRING, &entry);   \
   dbus_message_iter_open_container(&diter, DBUS_TYPE_VARIANT, str_type,   \
				    &end_iter);

#define _close_variant_iter(end_iter)					   \
   dbus_message_iter_close_container(&diter, &end_iter);		   \
   dbus_message_iter_close_container(&aiter, &diter);
/**
 * @endcond
 */

   /* starting array elements */

   _open_variant_iter("size", "(ii)", viter);
   dbus_message_iter_open_container(&viter, DBUS_TYPE_STRUCT, NULL, &vaiter);
   ethumb_thumb_size_get(e, &tw, &th);
   dbus_message_iter_append_basic(&vaiter, DBUS_TYPE_INT32, &tw);
   dbus_message_iter_append_basic(&vaiter, DBUS_TYPE_INT32, &th);
   dbus_message_iter_close_container(&viter, &vaiter);
   _close_variant_iter(viter);

   _open_variant_iter("format", "i", viter);
   format = ethumb_thumb_format_get(e);
   dbus_message_iter_append_basic(&viter, DBUS_TYPE_INT32, &format);
   _close_variant_iter(viter);

   _open_variant_iter("aspect", "i", viter);
   aspect = ethumb_thumb_aspect_get(e);
   dbus_message_iter_append_basic(&viter, DBUS_TYPE_INT32, &aspect);
   _close_variant_iter(viter);

   _open_variant_iter("orientation", "i", viter);
   orientation = ethumb_thumb_orientation_get(e);
   dbus_message_iter_append_basic(&viter, DBUS_TYPE_INT32, &orientation);
   _close_variant_iter(viter);

   _open_variant_iter("crop", "(dd)", viter);
   dbus_message_iter_open_container(&viter, DBUS_TYPE_STRUCT, NULL, &vaiter);
   ethumb_thumb_crop_align_get(e, &cx, &cy);
   t = cx;
   dbus_message_iter_append_basic(&vaiter, DBUS_TYPE_DOUBLE, &t);
   t = cy;
   dbus_message_iter_append_basic(&vaiter, DBUS_TYPE_DOUBLE, &t);
   dbus_message_iter_close_container(&viter, &vaiter);
   _close_variant_iter(viter);

   _open_variant_iter("quality", "i", viter);
   quality = ethumb_thumb_quality_get(e);
   dbus_message_iter_append_basic(&viter, DBUS_TYPE_INT32, &quality);
   _close_variant_iter(viter);

   _open_variant_iter("compress", "i", viter);
   compress = ethumb_thumb_compress_get(e);
   dbus_message_iter_append_basic(&viter, DBUS_TYPE_INT32, &compress);
   _close_variant_iter(viter);

   _open_variant_iter("frame", "(ayayay)", viter);
   dbus_message_iter_open_container(&viter, DBUS_TYPE_STRUCT, NULL, &vaiter);
   ethumb_frame_get(e, &theme_file, &group, &swallow);
   _ethumb_client_dbus_append_bytearray(&vaiter, theme_file);
   _ethumb_client_dbus_append_bytearray(&vaiter, group);
   _ethumb_client_dbus_append_bytearray(&vaiter, swallow);
   dbus_message_iter_close_container(&viter, &vaiter);
   _close_variant_iter(viter);

   _open_variant_iter("directory", "ay", viter);
   directory = ethumb_thumb_dir_path_get(e);
   _ethumb_client_dbus_append_bytearray(&viter, directory);
   _close_variant_iter(viter);

   _open_variant_iter("category", "ay", viter);
   category = ethumb_thumb_category_get(e);
   _ethumb_client_dbus_append_bytearray(&viter, category);
   _close_variant_iter(viter);

   _open_variant_iter("video_time", "d", viter);
   video_time = ethumb_video_time_get(e);
   dbus_message_iter_append_basic(&viter, DBUS_TYPE_DOUBLE, &video_time);
   _close_variant_iter(viter);

   _open_variant_iter("video_start", "d", viter);
   video_start = ethumb_video_start_get(e);
   dbus_message_iter_append_basic(&viter, DBUS_TYPE_DOUBLE, &video_start);
   _close_variant_iter(viter);

   _open_variant_iter("video_interval", "d", viter);
   video_interval = ethumb_video_interval_get(e);
   dbus_message_iter_append_basic(&viter, DBUS_TYPE_DOUBLE, &video_interval);
   _close_variant_iter(viter);

   _open_variant_iter("video_ntimes", "u", viter);
   video_ntimes = ethumb_video_ntimes_get(e);
   dbus_message_iter_append_basic(&viter, DBUS_TYPE_UINT32, &video_ntimes);
   _close_variant_iter(viter);

   _open_variant_iter("video_fps", "u", viter);
   video_fps = ethumb_video_fps_get(e);
   dbus_message_iter_append_basic(&viter, DBUS_TYPE_UINT32, &video_fps);
   _close_variant_iter(viter);

   _open_variant_iter("document_page", "u", viter);
   document_page = ethumb_document_page_get(e);
   dbus_message_iter_append_basic(&viter, DBUS_TYPE_UINT32, &document_page);
   _close_variant_iter(viter);

#undef _open_variant_iter
#undef _close_variant_iter

   dbus_message_iter_close_container(&iter, &aiter);

   client->pending_setup = e_dbus_message_send(client->conn, msg,
					       _ethumb_client_ethumb_setup_cb,
					       -1, client);
   dbus_message_unref(msg);
}

/**
 * @cond LOCAL
 */

static void
_ethumb_client_generated_cb(void *data, DBusMessage *msg)
{
   DBusMessageIter iter;
   dbus_int32_t id = -1;
   const char *thumb = NULL;
   const char *thumb_key = NULL;
   Ethumb_Client *client = data;
   int t;
   dbus_bool_t success;
   Eina_List *l;
   int found;
   struct _ethumb_pending_gen *pending;

   dbus_message_iter_init(msg, &iter);

   t = dbus_message_iter_get_arg_type(&iter);
   if (!_dbus_iter_type_check(t, DBUS_TYPE_INT32))
     goto end;
   dbus_message_iter_get_basic(&iter, &id);
   dbus_message_iter_next(&iter);

   t = dbus_message_iter_get_arg_type(&iter);
   if (!_dbus_iter_type_check(t, DBUS_TYPE_ARRAY))
     goto end;
   thumb = _ethumb_client_dbus_get_bytearray(&iter);
   dbus_message_iter_next(&iter);

   t = dbus_message_iter_get_arg_type(&iter);
   if (!_dbus_iter_type_check(t, DBUS_TYPE_ARRAY))
     goto end;
   thumb_key = _ethumb_client_dbus_get_bytearray(&iter);
   dbus_message_iter_next(&iter);

   t = dbus_message_iter_get_arg_type(&iter);
   if (!_dbus_iter_type_check(t, DBUS_TYPE_BOOLEAN))
     goto end;
   dbus_message_iter_get_basic(&iter, &success);

   found = 0;
   l = client->pending_gen;
   while (l)
     {
	pending = l->data;
	if (pending->id == id)
	  {
	     found = 1;
	     break;
	  }
	l = l->next;
     }

   if (found)
     {
	client->pending_gen = eina_list_remove_list(client->pending_gen, l);
        if (pending->generated_cb)
          pending->generated_cb(pending->data, client, id,
                                pending->file, pending->key,
                                pending->thumb, pending->thumb_key,
                                success);
        if (pending->free_data)
	  pending->free_data(pending->data);
	eina_stringshare_del(pending->file);
	eina_stringshare_del(pending->key);
	eina_stringshare_del(pending->thumb);
	eina_stringshare_del(pending->thumb_key);
	free(pending);
     }

end:
   if (thumb) eina_stringshare_del(thumb);
   if (thumb_key) eina_stringshare_del(thumb_key);
}

static void
_ethumb_client_queue_add_cb(void *data, DBusMessage *msg, DBusError *error)
{
   DBusMessageIter iter;
   int t;
   dbus_int32_t id = -1;
   struct _ethumb_pending_add *pending = data;
   struct _ethumb_pending_gen *generating;
   Ethumb_Client *client = pending->client;

   client->pending_add = eina_list_remove(client->pending_add, pending);

   if (!_dbus_callback_check_and_init(msg, &iter, error))
     goto end;

   t = dbus_message_iter_get_arg_type(&iter);
   if (!_dbus_iter_type_check(t, DBUS_TYPE_INT32))
     goto end;

   dbus_message_iter_get_basic(&iter, &id);

   generating = calloc(1, sizeof(*generating));
   generating->id = id;
   generating->file = pending->file;
   generating->key = pending->key;
   generating->thumb = pending->thumb;
   generating->thumb_key = pending->thumb_key;
   generating->generated_cb = pending->generated_cb;
   generating->data = pending->data;
   generating->free_data = pending->free_data;
   client->pending_gen = eina_list_append(client->pending_gen, generating);

end:
   free(pending);
}

static int
_ethumb_client_queue_add(Ethumb_Client *client, const char *file, const char *key, const char *thumb, const char *thumb_key, Ethumb_Client_Generate_Cb generated_cb, const void *data, Eina_Free_Cb free_data)
{
   DBusMessage *msg;
   DBusMessageIter iter;
   struct _ethumb_pending_add *pending;

   pending = calloc(1, sizeof(*pending));
   pending->id = client->id_count;
   pending->file = eina_stringshare_add(file);
   pending->key = eina_stringshare_add(key);
   pending->thumb = eina_stringshare_add(thumb);
   pending->thumb_key = eina_stringshare_add(thumb_key);
   pending->generated_cb = generated_cb;
   pending->data = (void *)data;
   pending->free_data = free_data;
   pending->client = client;

   client->id_count = (client->id_count + 1) % MAX_ID;

   msg = dbus_message_new_method_call(_ethumb_dbus_bus_name,
				      client->object_path,
				      _ethumb_dbus_objects_interface,
				      "queue_add");

   dbus_message_iter_init_append(msg, &iter);
   dbus_message_iter_append_basic(&iter, DBUS_TYPE_INT32, &pending->id);
   _ethumb_client_dbus_append_bytearray(&iter, file);
   _ethumb_client_dbus_append_bytearray(&iter, key);
   _ethumb_client_dbus_append_bytearray(&iter, thumb);
   _ethumb_client_dbus_append_bytearray(&iter, thumb_key);

   pending->pending_call = e_dbus_message_send(client->conn, msg,
					       _ethumb_client_queue_add_cb,
					       -1, pending);
   client->pending_add = eina_list_append(client->pending_add, pending);
   dbus_message_unref(msg);

   return pending->id;
}

static void
_ethumb_client_queue_remove_cb(void *data, DBusMessage *msg, DBusError *error)
{
   DBusMessageIter iter;
   int t;
   dbus_bool_t success = 0;
   struct _ethumb_pending_remove *pending = data;
   Ethumb_Client *client = pending->client;

   client->pending_remove = eina_list_remove(client->pending_remove, pending);

   if (!_dbus_callback_check_and_init(msg, &iter, error))
     goto end;

   t = dbus_message_iter_get_arg_type(&iter);
   if (!_dbus_iter_type_check(t, DBUS_TYPE_BOOLEAN))
     goto end;

   dbus_message_iter_get_basic(&iter, &success);

end:
   if (pending->cancel_cb)
     pending->cancel_cb(pending->data, success);
   if (pending->free_data)
     pending->free_data(pending->data);
   free(pending);
}
/**
 * @endcond
 */

/**
 * Ask server to cancel generation of thumbnail.
 *
 * @param client client instance. Must @b not be @c NULL and client
 *        must be connected (after connected_cb is called).
 * @param id valid id returned by ethumb_client_generate()
 * @param cancel_cb function to report cancellation results.
 * @param data context argument to give back to @a cancel_cb. May be
 *        @c NULL.
 * @param data context to give back to @a cancel_cb. May be @c
 *        NULL.
 * @param free_data used to release @a data resources after @a
 *        cancel_cb is called or user calls
 *        ethumb_client_disconnect().
 */
EAPI void
ethumb_client_generate_cancel(Ethumb_Client *client, int id, Ethumb_Client_Generate_Cancel_Cb cancel_cb, const void *data, Eina_Free_Cb free_data)
{
   DBusMessage *msg;
   struct _ethumb_pending_remove *pending;
   Eina_List *l;
   int found;
   dbus_int32_t id32 = id;
   EINA_SAFETY_ON_NULL_RETURN(client);
   EINA_SAFETY_ON_FALSE_RETURN(id >= 0);

   pending = calloc(1, sizeof(*pending));
   pending->id = id;
   pending->cancel_cb = cancel_cb;
   pending->data = (void *)data;
   pending->free_data = free_data;
   pending->client = client;

   msg = dbus_message_new_method_call(_ethumb_dbus_bus_name,
				      client->object_path,
				      _ethumb_dbus_objects_interface,
				      "queue_remove");

   dbus_message_append_args(msg, DBUS_TYPE_INT32, &id32, DBUS_TYPE_INVALID);
   pending->pending_call = e_dbus_message_send(client->conn, msg,
					       _ethumb_client_queue_remove_cb,
					       -1, pending);
   client->pending_remove = eina_list_append(client->pending_remove, pending);

   found = 0;
   l = client->pending_add;
   while (l)
     {
	struct _ethumb_pending_add *pending_add = l->data;
	if (pending_add->id != id32)
	  {
	     l = l->next;
	     continue;
	  }
	client->pending_add = eina_list_remove_list(client->pending_add, l);
	eina_stringshare_del(pending_add->file);
	eina_stringshare_del(pending_add->key);
	eina_stringshare_del(pending_add->thumb);
	eina_stringshare_del(pending_add->thumb_key);
	dbus_pending_call_cancel(pending_add->pending_call);
	dbus_pending_call_unref(pending_add->pending_call);
	if (pending_add->free_data)
	  pending_add->free_data(pending_add->data);
	free(pending_add);
	found = 1;
	break;
     }

   if (found)
     goto end;

   l = client->pending_gen;
   while (l)
     {
	struct _ethumb_pending_gen *pending_gen = l->data;
	if (pending_gen->id != id32)
	  {
	     l = l->next;
	     continue;
	  }
	client->pending_gen = eina_list_remove_list(client->pending_gen, l);
	eina_stringshare_del(pending_gen->file);
	eina_stringshare_del(pending_gen->key);
	eina_stringshare_del(pending_gen->thumb);
	eina_stringshare_del(pending_gen->thumb_key);
	if (pending_gen->free_data)
	  pending_gen->free_data(pending_gen->data);
	free(pending_gen);
	break;
     }

end:
   dbus_message_unref(msg);
}

/**
 * @cond LOCAL
 */
static void
_ethumb_client_queue_clear_cb(void *data, DBusMessage *msg __UNUSED__, DBusError *error __UNUSED__)
{
   Ethumb_Client *client = data;

   client->pending_clear = NULL;
}
/**
 * @endcond
 */

/**
 * Ask server to cancel generation of all thumbnails.
 *
 * @param client client instance. Must @b not be @c NULL and client
 *        must be connected (after connected_cb is called).
 *
 * @see ethumb_client_generate_cancel()
 */
EAPI void
ethumb_client_generate_cancel_all(Ethumb_Client *client)
{
   DBusMessage *msg;
   void *data;
   EINA_SAFETY_ON_NULL_RETURN(client);

   if (client->pending_clear)
     return;

   EINA_LIST_FREE(client->pending_add, data)
     {
	struct _ethumb_pending_add *pending = data;
	eina_stringshare_del(pending->file);
	eina_stringshare_del(pending->key);
	eina_stringshare_del(pending->thumb);
	eina_stringshare_del(pending->thumb_key);
	dbus_pending_call_cancel(pending->pending_call);
	dbus_pending_call_unref(pending->pending_call);
	if (pending->free_data)
	  pending->free_data(pending->data);
	free(pending);
     }

   EINA_LIST_FREE(client->pending_gen, data)
     {
	struct _ethumb_pending_gen *pending = data;
	eina_stringshare_del(pending->file);
	eina_stringshare_del(pending->key);
	eina_stringshare_del(pending->thumb);
	eina_stringshare_del(pending->thumb_key);
	if (pending->free_data)
	  pending->free_data(pending->data);
	free(pending);
     }

   msg = dbus_message_new_method_call(_ethumb_dbus_bus_name,
				      client->object_path,
				      _ethumb_dbus_objects_interface,
				      "queue_clear");

   client->pending_clear = e_dbus_message_send(client->conn, msg,
					       _ethumb_client_queue_clear_cb,
					       -1, client);

   dbus_message_unref(msg);
}

/**
 * Configure future requests to use FreeDesktop.Org preset.
 *
 * This is a preset to provide freedesktop.org (fdo) standard
 * compliant thumbnails. That is, files are stored as JPEG under
 * ~/.thumbnails/SIZE, with size being either normal (128x128) or
 * large (256x256).
 *
 * @param client the client instance to use. Must @b not be @c
 *        NULL. May be pending connected (can be called before @c
 *        connected_cb)
 * @param s size identifier, either #ETHUMB_THUMB_NORMAL (0) or
 *        #ETHUMB_THUMB_LARGE (1).
 *
 * @see ethumb_client_size_set()
 * @see ethumb_client_aspect_set()
 * @see ethumb_client_crop_align_set()
 * @see ethumb_client_category_set()
 * @see ethumb_client_dir_path_set()
 */
EAPI void
ethumb_client_fdo_set(Ethumb_Client *client, Ethumb_Thumb_FDO_Size s)
{
   EINA_SAFETY_ON_NULL_RETURN(client);

   client->ethumb_dirty = 1;
   ethumb_thumb_fdo_set(client->ethumb, s);
}

/**
 * Configure future request to use custom size.
 *
 * @param client the client instance to use. Must @b not be @c
 *        NULL. May be pending connected (can be called before @c
 *        connected_cb)
 * @param tw width, default is 128.
 * @param th height, default is 128.
 */
EAPI void
ethumb_client_size_set(Ethumb_Client *client, int tw, int th)
{
   EINA_SAFETY_ON_NULL_RETURN(client);

   client->ethumb_dirty = 1;
   ethumb_thumb_size_set(client->ethumb, tw, th);
}

/**
 * Retrieve future request to use custom size.
 *
 * @param client the client instance to use. Must @b not be @c
 *        NULL. May be pending connected (can be called before @c
 *        connected_cb)
 * @param tw where to return width. May be @c NULL.
 * @param th where to return height. May be @c NULL.
 */
EAPI void
ethumb_client_size_get(const Ethumb_Client *client, int *tw, int *th)
{
   if (tw) *tw = 0;
   if (th) *th = 0;
   EINA_SAFETY_ON_NULL_RETURN(client);

   ethumb_thumb_size_get(client->ethumb, tw, th);
}

/**
 * Configure format to use for future requests.
 *
 * @param client the client instance to use. Must @b not be @c
 *        NULL. May be pending connected (can be called before @c
 *        connected_cb)
 * @param f format identifier to use, either #ETHUMB_THUMB_FDO (0),
 *        #ETHUMB_THUMB_JPEG (1) or #ETHUMB_THUMB_EET (2). Default is FDO.
 */
EAPI void
ethumb_client_format_set(Ethumb_Client *client, Ethumb_Thumb_Format f)
{
   EINA_SAFETY_ON_NULL_RETURN(client);

   client->ethumb_dirty = 1;
   ethumb_thumb_format_set(client->ethumb, f);
}

/**
 * Retrieve format to use for future requests.
 *
 * @param client the client instance to use. Must @b not be @c
 *        NULL. May be pending connected (can be called before @c
 *        connected_cb)
 *
 * @return format identifier to use, either #ETHUMB_THUMB_FDO (0),
 *         #ETHUMB_THUMB_JPEG (1) or #ETHUMB_THUMB_EET (2).
 */
EAPI Ethumb_Thumb_Format
ethumb_client_format_get(const Ethumb_Client *client)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(client, 0);

   return ethumb_thumb_format_get(client->ethumb);
}

/**
 * Configure aspect mode to use.
 *
 * If aspect is kept (#ETHUMB_THUMB_KEEP_ASPECT), then image will be
 * rescaled so the largest dimension is not bigger than it's specified
 * size (see ethumb_client_size_get()) and the other dimension is
 * resized in the same proportion. Example: size is 256x256, image is
 * 1000x500, resulting thumbnail is 256x128.
 *
 * If aspect is ignored (#ETHUMB_THUMB_IGNORE_ASPECT), then image will
 * be distorted to match required thumbnail size. Example: size is
 * 256x256, image is 1000x500, resulting thumbnail is 256x256.
 *
 * If crop is required (#ETHUMB_THUMB_CROP), then image will be
 * cropped so the smallest dimension is not bigger than its specified
 * size (see ethumb_client_size_get()) and the other dimension will
 * overflow, not being visible in the final image. How it will
 * overflow is speficied by ethumb_client_crop_align_set()
 * alignment. Example: size is 256x256, image is 1000x500, crop
 * alignment is 0.5, 0.5, resulting thumbnail is 256x256 with 250
 * pixels from left and 250 pixels from right being lost, that is just
 * the 500x500 central pixels of image will be considered for scaling.
 *
 * @param client the client instance to use. Must @b not be @c
 *        NULL. May be pending connected (can be called before @c
 *        connected_cb)
 * @param a aspect mode identifier, either #ETHUMB_THUMB_KEEP_ASPECT (0),
 *        #ETHUMB_THUMB_IGNORE_ASPECT (1) or #ETHUMB_THUMB_CROP (2).
 */
EAPI void
ethumb_client_aspect_set(Ethumb_Client *client, Ethumb_Thumb_Aspect a)
{
   EINA_SAFETY_ON_NULL_RETURN(client);

   client->ethumb_dirty = 1;
   ethumb_thumb_aspect_set(client->ethumb, a);
}

/**
 * Get current aspect in use for requests.
 *
 * @param client the client instance to use. Must @b not be @c
 *        NULL. May be pending connected (can be called before @c
 *        connected_cb)
 *
 * @return aspect in use for future requests.
 */
EAPI Ethumb_Thumb_Aspect
ethumb_client_aspect_get(const Ethumb_Client *client)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(client, 0);

   return ethumb_thumb_aspect_get(client->ethumb);
}

/**
 * Configure orientation to use for future requests.
 *
 * Default value is #ETHUMB_THUMB_ORIENT_ORIGINAL: metadata from the file
 * will be used to orient pixel data.
 *
 * @param client the client instance to use. Must @b not be @c
 *        NULL. May be pending connected (can be called before @c
 *        connected_cb)
 * @param f format identifier to use, either #ETHUMB_THUMB_ORIENT_NONE (0),
 *        #ETHUMB_THUMB_ROTATE_90_CW (1), #ETHUMB_THUMB_ROTATE_180 (2),
 *        #ETHUMB_THUMB_ROTATE_90_CCW (3), #ETHUMB_THUMB_FLIP_HORIZONTAL (4),
 *        #ETHUMB_THUMB_FLIP_VERTICAL (5), #ETHUMB_THUMB_FLIP_TRANSPOSE (6),
 *        #ETHUMB_THUMB_FLIP_TRANSVERSE (7) or #ETHUMB_THUMB_ORIENT_ORIGINAL
 *        (8). Default is ORIGINAL.
 */
EAPI void
ethumb_client_orientation_set(Ethumb_Client *client, Ethumb_Thumb_Orientation o)
{
   EINA_SAFETY_ON_NULL_RETURN(client);

   client->ethumb_dirty = 1;
   ethumb_thumb_orientation_set(client->ethumb, o);
}

/**
 * Get current orientation in use for requests.
 *
 * @param client the client instance to use. Must @b not be @c
 *        NULL. May be pending connected (can be called before @c
 *        connected_cb)
 *
 * @return orientation in use for future requests.
 */
EAPI Ethumb_Thumb_Orientation
ethumb_client_orientation_get(const Ethumb_Client *client)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(client, 0);

   return ethumb_thumb_orientation_get(client->ethumb);
}

/**
 * Configure crop alignment in use for future requests.
 *
 * @param client the client instance to use. Must @b not be @c
 *        NULL. May be pending connected (can be called before @c
 *        connected_cb)
 * @param x horizontal alignment. 0.0 means left side will be visible
 *        or right side is being lost. 1.0 means right side will be
 *        visible or left side is being lost. 0.5 means just center is
 *        visible, both sides will be lost.  Default is 0.5.
 * @param y vertical alignment. 0.0 is top visible, 1.0 is bottom
 *        visible, 0.5 is center visible. Default is 0.5
 */
EAPI void
ethumb_client_crop_align_set(Ethumb_Client *client, float x, float y)
{
   EINA_SAFETY_ON_NULL_RETURN(client);

   client->ethumb_dirty = 1;
   ethumb_thumb_crop_align_set(client->ethumb, x, y);
}

/**
 * Get current crop alignment in use for requests.
 *
 * @param client the client instance to use. Must @b not be @c
 *        NULL. May be pending connected (can be called before @c
 *        connected_cb)
 * @param x where to return horizontal alignment. May be @c NULL.
 * @param y where to return vertical alignment. May be @c NULL.
 */
EAPI void
ethumb_client_crop_align_get(const Ethumb_Client *client, float *x, float *y)
{
   if (x) *x = 0.0;
   if (y) *y = 0.0;
   EINA_SAFETY_ON_NULL_RETURN(client);

   ethumb_thumb_crop_align_get(client->ethumb, x, y);
}

/**
 * Configure quality to be used in thumbnails.
 *
 * @param client the client instance to use. Must @b not be @c
 *        NULL. May be pending connected (can be called before @c
 *        connected_cb)
 * @param quality value from 0 to 100, default is 80. The effect
 *        depends on the format being used, PNG will not use it.
 */
EAPI void
ethumb_client_quality_set(Ethumb_Client *client, int quality)
{
   EINA_SAFETY_ON_NULL_RETURN(client);

   ethumb_thumb_quality_set(client->ethumb, quality);
}

/**
 * Get quality to be used in thumbnails.
 *
 * @param client the client instance to use. Must @b not be @c
 *        NULL. May be pending connected (can be called before @c
 *        connected_cb)
 *
 * @return quality value from 0 to 100, default is 80. The effect
 *         depends on the format being used, PNG will not use it.
 */
EAPI int
ethumb_client_quality_get(const Ethumb_Client *client)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(client, 0);

   return ethumb_thumb_quality_get(client->ethumb);
}

/**
 * Configure compression level used in requests.
 *
 * @param client the client instance to use. Must @b not be @c
 *        NULL. May be pending connected (can be called before @c
 *        connected_cb)
 * @param compress value from 0 to 9, default is 9. The effect
 *        depends on the format being used, JPEG will not use it.
 */
EAPI void
ethumb_client_compress_set(Ethumb_Client *client, int compress)
{
   EINA_SAFETY_ON_NULL_RETURN(client);

   ethumb_thumb_compress_set(client->ethumb, compress);
}

/**
 * Get compression level used in requests.
 *
 * @param client the client instance to use. Must @b not be @c
 *        NULL. May be pending connected (can be called before @c
 *        connected_cb)
 *
 * @return compress value from 0 to 9, default is 9. The effect
 *         depends on the format being used, JPEG will not use it.
 */
EAPI int
ethumb_client_compress_get(const Ethumb_Client *client)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(client, 0);

   return ethumb_thumb_compress_get(client->ethumb);
}

/**
 * Set frame to apply to future thumbnails.
 *
 * This will create an edje object that will have image swallowed
 * in. This can be used to simulate Polaroid or wood frames in the
 * generated image. Remeber it is bad to modify the original contents
 * of thumbnails, but sometimes it's useful to have it composited and
 * avoid runtime overhead.
 *
 * @param client the client instance to use. Must @b not be @c
 *        NULL. May be pending connected (can be called before @c
 *        connected_cb)
 * @param file file path to edje.
 * @param group group inside edje to use.
 * @param swallow name of swallow part.
 */
EAPI Eina_Bool
ethumb_client_frame_set(Ethumb_Client *client, const char *file, const char *group, const char *swallow)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(client, 0);

   client->ethumb_dirty = 1;
   return ethumb_frame_set(client->ethumb, file, group, swallow);
}

/**
 * Configure where to store thumbnails in future requests.
 *
 * This value will be used to generate thumbnail paths, that is, it
 * will be used when ethumb_client_thumb_path_set() was not called
 * after last ethumb_client_file_set().
 *
 * Note that this is the base, a category is added to this path as a
 * sub directory. This is not the final directory where files are
 * stored, the thumbnail system will account @b category as well, see
 * ethumb_client_category_set().
 *
 * As other options, this value will only be applied to future
 * requests.
 *
 * @param client the client instance to use. Must @b not be @c
 *        NULL. May be pending connected (can be called before @c
 *        connected_cb)
 * @param path base directory where to store thumbnails. Default is
 *        ~/.thumbnails
 *
 * @see ethumb_client_category_set()
 */
EAPI void
ethumb_client_dir_path_set(Ethumb_Client *client, const char *path)
{
   EINA_SAFETY_ON_NULL_RETURN(client);

   client->ethumb_dirty = 1;
   ethumb_thumb_dir_path_set(client->ethumb, path);
}

/**
 * Get base directory path where to store thumbnails.
 *
 * @param client the client instance to use. Must @b not be @c
 *        NULL. May be pending connected (can be called before @c
 *        connected_cb)
 *
 * @return pointer to internal string with current path. This string
 *         should not be modified or freed.
 *
 * @see ethumb_client_dir_path_set()
 */
EAPI const char *
ethumb_client_dir_path_get(const Ethumb_Client *client)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(client, NULL);

   return ethumb_thumb_dir_path_get(client->ethumb);
}

/**
 * Category directory to store thumbnails.
 *
 * This value will be used to generate thumbnail paths, that is, it
 * will be used when ethumb_client_thumb_path_set() was not called
 * after last ethumb_client_file_set().
 *
 * This is a sub-directory inside base directory
 * (ethumb_client_dir_path_set()) that creates a namespace to avoid
 * different options resulting in the same file.
 *
 * As other options, this value will only be applied to future
 * requests.
 *
 * @param client the client instance to use. Must @b not be @c
 *        NULL. May be pending connected (can be called before @c
 *        connected_cb)
 * @param category category sub directory to store thumbnail. Default
 *        is either "normal" or "large" for FDO compliant thumbnails
 *        or WIDTHxHEIGHT-ASPECT[-FRAMED]-FORMAT. It can be a string
 *        or @c NULL to use auto generated names.
 *
 * @see ethumb_client_dir_path_set()
 */
EAPI void
ethumb_client_category_set(Ethumb_Client *client, const char *category)
{
   EINA_SAFETY_ON_NULL_RETURN(client);

   client->ethumb_dirty = 1;
   ethumb_thumb_category_set(client->ethumb, category);
}

/**
 * Get category sub-directory  where to store thumbnails.
 *
 * @param client the client instance to use. Must @b not be @c
 *        NULL. May be pending connected (can be called before @c
 *        connected_cb)
 *
 * @return pointer to internal string with current path. This string
 *         should not be modified or freed.
 *
 * @see ethumb_client_category_set()
 */
EAPI const char *
ethumb_client_category_get(const Ethumb_Client *client)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(client, NULL);

   return ethumb_thumb_category_get(client->ethumb);
}

/**
 * Set the video time (duration) in seconds.
 *
 * @param client the client instance to use. Must @b not be @c
 *        NULL. May be pending connected (can be called before @c
 *        connected_cb)
 * @param t duration (in seconds). Defaults to 3 seconds.
 */
EAPI void
ethumb_client_video_time_set(Ethumb_Client *client, float t)
{
   EINA_SAFETY_ON_NULL_RETURN(client);

   client->ethumb_dirty = 1;
   ethumb_video_time_set(client->ethumb, t);
}

/**
 * Set initial video position to start thumbnailing, in percentage.
 *
 * This is useful to avoid thumbnailing the company/producer logo or
 * movie opening.
 *
 * @param client the client instance to use. Must @b not be @c
 *        NULL. May be pending connected (can be called before @c
 *        connected_cb)
 * @param start initial video positon to thumbnail, in percentage (0.0
 *        to 1.0, inclusive). Defaults to 10% (0.1).
 */
EAPI void
ethumb_client_video_start_set(Ethumb_Client *client, float start)
{
   EINA_SAFETY_ON_NULL_RETURN(client);
   EINA_SAFETY_ON_FALSE_RETURN(start >= 0.0);
   EINA_SAFETY_ON_FALSE_RETURN(start <= 1.0);

   client->ethumb_dirty = 1;
   ethumb_video_start_set(client->ethumb, start);
}

/**
 * Set the video frame interval, in seconds.
 *
 * This is useful for animated thumbnail and will define skip time
 * before going to the next frame. Note that video backends might not
 * be able to precisely skip that amount as it will depend on various
 * factors, including video encoding.
 *
 * Although this seems similar to ethumb_client_video_fps_set(), this
 * one is the time that will be used to seek. The math is simple, for
 * each new frame the video position will be set to:
 * ((video_length * start_time) + (interval * current_frame_number)).
 *
 * @param client the client instance to use. Must @b not be @c
 *        NULL. May be pending connected (can be called before @c
 *        connected_cb)
 * @param interval time between frames, in seconds. Defaults to 0.05
 *        seconds.
 */
EAPI void
ethumb_client_video_interval_set(Ethumb_Client *client, float interval)
{
   EINA_SAFETY_ON_NULL_RETURN(client);

   client->ethumb_dirty = 1;
   ethumb_video_interval_set(client->ethumb, interval);
}

/**
 * Set the number of frames to thumbnail.
 *
 * This is useful for animated thumbnail and will define how many
 * frames the generated file will have.
 *
 * @param client the client instance to use. Must @b not be @c
 *        NULL. May be pending connected (can be called before @c
 *        connected_cb)
 * @param ntimes number of times, must be greater than zero.
 *        Defaults to 3.
 */
EAPI void
ethumb_client_video_ntimes_set(Ethumb_Client *client, unsigned int ntimes)
{
   EINA_SAFETY_ON_NULL_RETURN(client);
   EINA_SAFETY_ON_FALSE_RETURN(ntimes > 0);

   client->ethumb_dirty = 1;
   ethumb_video_ntimes_set(client->ethumb, ntimes);
}

/**
 * Set the number of frames per second to thumbnail the video.
 *
 * This configures the number of times per seconds the thumbnail will
 * use to create thumbnails.
 *
 * Although this is similar to ethumb_client_video_interval_set(), it
 * is the delay used between calling functions thata generates frames,
 * while the other is the time used to skip inside the video.
 *
 * @param client the client instance to use. Must @b not be @c
 *        NULL. May be pending connected (can be called before @c
 *        connected_cb)
 * @param fps number of frames per second to thumbnail. Must be greater
 *        than zero. Defaults to 10.
 */
EAPI void
ethumb_client_video_fps_set(Ethumb_Client *client, unsigned int fps)
{
   EINA_SAFETY_ON_NULL_RETURN(client);
   EINA_SAFETY_ON_FALSE_RETURN(fps > 0);

   client->ethumb_dirty = 1;
   ethumb_video_fps_set(client->ethumb, fps);
}

/**
 * Set the page number to thumbnail in paged documents.
 *
 * @param client the client instance to use. Must @b not be @c
 *        NULL. May be pending connected (can be called before @c
 *        connected_cb)
 * @param page page number, defaults to 0 (first).
 */
EAPI void
ethumb_client_document_page_set(Ethumb_Client *client, unsigned int page)
{
   EINA_SAFETY_ON_NULL_RETURN(client);

   client->ethumb_dirty = 1;
   ethumb_document_page_set(client->ethumb, page);
}

/**
 * Set source file to be thumbnailed.
 *
 * Calling this function has the side effect of resetting values set
 * with ethumb_client_thumb_path_set() or auto-generated with
 * ethumb_client_thumb_exists().
 *
 * @param client the client instance to use. Must @b not be @c
 *        NULL. May be pending connected (can be called before @c
 *        connected_cb)
 * @param path the filesystem path to use. May be @c NULL.
 * @param key the extra argument/key inside @a path to read image
 *        from. This is only used for formats that allow multiple
 *        resources in one file, like EET or Edje (group name).
 *
 * @return @c EINA_TRUE on success, @c EINA_FALSE on failure.
 */
EAPI Eina_Bool
ethumb_client_file_set(Ethumb_Client *client, const char *path, const char *key)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(client, 0);

   return ethumb_file_set(client->ethumb, path, key);
}

/**
 * Get values set with ethumb_client_file_get()
 *
 * @param client the client instance to use. Must @b not be @c
 *        NULL. May be pending connected (can be called before @c
 *        connected_cb)
 * @param path where to return configured path. May be @c NULL.  If
 *        not @c NULL, then it will be a pointer to a stringshared
 *        instance, but @b no references are added (do it with
 *        eina_stringshare_ref())!
 * @param key where to return configured key. May be @c NULL.If not @c
 *        NULL, then it will be a pointer to a stringshared instance,
 *        but @b no references are added (do it with
 *        eina_stringshare_ref())!
 */
EAPI void
ethumb_client_file_get(Ethumb_Client *client, const char **path, const char **key)
{
   if (path) *path = NULL;
   if (key) *key = NULL;
   EINA_SAFETY_ON_NULL_RETURN(client);

   ethumb_file_get(client->ethumb, path, key);
}

/**
 * Reset previously set file to @c NULL.
 *
 * @param client the client instance to use. Must @b not be @c
 *        NULL. May be pending connected (can be called before @c
 *        connected_cb)
 */
EAPI void
ethumb_client_file_free(Ethumb_Client *client)
{
   EINA_SAFETY_ON_NULL_RETURN(client);

   ethumb_file_free(client->ethumb);
}

/**
 * Set a defined path and key to store the thumbnail.
 *
 * If not explicitly given, the thumbnail path will be auto-generated
 * by ethumb_client_thumb_exists() or server using configured
 * parameters like size, aspect and category.
 *
 * Set these to @c NULL to forget previously given values. After
 * ethumb_client_file_set() these values will be reset to @c NULL.
 *
 * @param client the client instance to use. Must @b not be @c
 *        NULL. May be pending connected (can be called before @c
 *        connected_cb)
 * @param path force generated thumbnail to the exact given path. If
 *        @c NULL, then reverts back to auto-generation.
 * @param key force generated thumbnail to the exact given key. If
 *        @c NULL, then reverts back to auto-generation.
 */
EAPI void
ethumb_client_thumb_path_set(Ethumb_Client *client, const char *path, const char *key)
{
   EINA_SAFETY_ON_NULL_RETURN(client);

   ethumb_thumb_path_set(client->ethumb, path, key);
}

/**
 * Get the configured thumbnail path.
 *
 * This returns the value set with ethumb_client_thumb_path_set() or
 * auto-generated by ethumb_client_thumb_exists() if it was not set.
 *
 * @param client the client instance to use. Must @b not be @c
 *        NULL. May be pending connected (can be called before @c
 *        connected_cb)
 * @param path where to return configured path. May be @c NULL.  If
 *        there was no path configured with
 *        ethumb_client_thumb_path_set() and
 *        ethumb_client_thumb_exists() was not called, then it will
 *        probably return @c NULL. If not @c NULL, then it will be a
 *        pointer to a stringshared instance, but @b no references are
 *        added (do it with eina_stringshare_ref())!
 * @param key where to return configured key. May be @c NULL.  If
 *        there was no key configured with
 *        ethumb_client_thumb_key_set() and
 *        ethumb_client_thumb_exists() was not called, then it will
 *        probably return @c NULL. If not @c NULL, then it will be a
 *        pointer to a stringshared instance, but @b no references are
 *        added (do it with eina_stringshare_ref())!
 */
EAPI void
ethumb_client_thumb_path_get(Ethumb_Client *client, const char **path, const char **key)
{
   if (path) *path = NULL;
   if (key) *key = NULL;
   EINA_SAFETY_ON_NULL_RETURN(client);

   ethumb_thumb_path_get(client->ethumb, path, key);
}

/**
 * Checks whenever file already exists (locally!)
 *
 * This will check locally (not calling server) if thumbnail already
 * exists or not, also calculating the thumbnail path. See
 * ethumb_client_thumb_path_get(). Path must be configured with
 * ethumb_client_file_set() before using it and the last set file will
 * be used!
 *
 * @param client client instance. Must @b not be @c NULL and client
 *        must be configured with ethumb_client_file_set().
 *
 * @return @c EINA_TRUE if it exists, @c EINA_FALSE otherwise.
 */
EAPI Ethumb_Exists *
ethumb_client_thumb_exists(Ethumb_Client *client, Ethumb_Client_Thumb_Exists_Cb exists_cb, const void *data)
{
   Ethumb_Async_Exists_Cb *cb;
   Ethumb_Async_Exists *async;

   EINA_SAFETY_ON_NULL_RETURN_VAL(client, NULL);

   cb = malloc(sizeof (Ethumb_Async_Exists_Cb));
   if (!cb)
     {
        exists_cb(client, NULL, EINA_FALSE, (void*) data);
        return NULL;
     }

   cb->exists_cb = exists_cb;
   cb->data = data;

   async = eina_hash_find(_exists_request, client->ethumb);
   if (async)
     {
        EINA_REFCOUNT_REF(async);
        async->callbacks = eina_list_append(async->callbacks, cb);
        return (Ethumb_Exists*) async;
     }

   async = malloc(sizeof (Ethumb_Async_Exists));
   if (!async)
     {
        free(cb);
        exists_cb(client, NULL, EINA_FALSE, (void*) data);
        return NULL;
     }

   async->dup = ethumb_dup(client->ethumb);
   async->source = client;
   EINA_REFCOUNT_REF(async->source);
   async->exists = EINA_FALSE;
   async->cancel = EINA_FALSE;

   async->callbacks = eina_list_append(NULL, cb);

   EINA_REFCOUNT_INIT(async);
   async->thread = ecore_thread_run(_ethumb_client_exists_heavy,
				    _ethumb_client_exists_end,
				    _ethumb_client_exists_end,
				    async);

   eina_hash_direct_add(_exists_request, async->dup, async);

   return (Ethumb_Exists*) async;
}

/**
 * Cancel an ongoing exists request.
 *
 * @param exists the request to cancel.
 */
EAPI void
ethumb_client_thumb_exists_cancel(Ethumb_Exists *exists, Ethumb_Client_Thumb_Exists_Cb exists_cb, const void *data)
{
   Ethumb_Async_Exists_Cb *cb;
   Ethumb_Async_Exists *async = (Ethumb_Async_Exists*) exists;
   Eina_List *l;

   EINA_LIST_FOREACH(async->callbacks, l, cb)
     if (cb->exists_cb == exists_cb && cb->data == data)
       {
          async->callbacks = eina_list_remove_list(async->callbacks, l);
          break;
       }

   EINA_REFCOUNT_UNREF(async)
     _ethumb_async_cancel(async);
}

/**
 * Check if an exists request was cancelled.
 *
 * @param exists the request to check.
 * @result return EINA_TRUE if the request was cancelled.
 */
EAPI Eina_Bool
ethumb_client_thumb_exists_check(Ethumb_Exists *exists)
{
   Ethumb_Async_Exists *async = (Ethumb_Async_Exists*) exists;

   if (!async) return EINA_TRUE;

   if (async->callbacks || async->cancel) return EINA_FALSE;

   return ecore_thread_check(async->thread);
}

/**
 * Ask server to generate thumbnail.
 *
 * This process is asynchronous and will report back from main loop
 * using @a generated_cb. One can cancel this request by calling
 * ethumb_client_generate_cancel() or
 * ethumb_client_generate_cancel_all(), but not that request might be
 * processed by server already and no generated files will be removed
 * if that is the case.
 *
 * This will not check if file already exists, this should be done by
 * explicitly calling ethumb_client_thumb_exists(). That is, this
 * function will override any existing thumbnail.
 *
 * @param client client instance. Must @b not be @c NULL and client
 *        must be connected (after connected_cb is called).
 * @param generated_cb function to report generation results.
 * @param data context argument to give back to @a generated_cb. May
 *        be @c NULL.
 * @param data context to give back to @a generate_cb. May be @c
 *        NULL.
 * @param free_data used to release @a data resources after @a
 *        generated_cb is called or user calls
 *        ethumb_client_disconnect().
 *
 * @return identifier or -1 on error. If -1 is returned (error) then
 *         @a free_data is @b not called!
 *
 * @see ethumb_client_connect()
 * @see ethumb_client_file_set()
 * @see ethumb_client_thumb_exists()
 * @see ethumb_client_generate_cancel()
 * @see ethumb_client_generate_cancel_all()
 */
EAPI int
ethumb_client_generate(Ethumb_Client *client, Ethumb_Client_Generate_Cb generated_cb, const void *data, Eina_Free_Cb free_data)
{
   const char *file, *key, *thumb, *thumb_key;
   int id;
   EINA_SAFETY_ON_NULL_RETURN_VAL(client, -1);
   EINA_SAFETY_ON_FALSE_RETURN_VAL(client->connected, -1);

   ethumb_file_get(client->ethumb, &file, &key);
   if (!file)
     {
	ERR("no file set.");
	return -1;
     }

   ethumb_thumb_path_get(client->ethumb, &thumb, &thumb_key);

   if (client->ethumb_dirty)
     ethumb_client_ethumb_setup(client);
   id = _ethumb_client_queue_add(client, file, key, thumb, thumb_key,
				 generated_cb, data, free_data);

   return id;
}
