/**
 * @file
 *
 * Copyright (C) 2009 by ProFUSION embedded systems
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful,  but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the  GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
 * USA.
 *
 * @author Rafael Antognolli <antognolli@profusion.mobi>
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <Eina.h>
#include <eina_safety_checks.h>
#include <Ethumb.h>
#include "Ethumb_Client.h"
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <stdbool.h>

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

#include <E_DBus.h>

#define MAX_ID 2000000

#define DBG(...) EINA_ERROR_PDBG(__VA_ARGS__)
#define INF(...) EINA_ERROR_PINFO(__VA_ARGS__)
#define WRN(...) EINA_ERROR_PWARN(__VA_ARGS__)
#define ERR(...) EINA_ERROR_PERR(__VA_ARGS__)

struct _Ethumb_Client
{
   Ethumb *ethumb;
   long id_count;

   E_DBus_Connection *conn;
   E_DBus_Signal_Handler *name_owner_changed_handler;
   E_DBus_Signal_Handler *generated_signal;
   DBusPendingCall *pending_get_name_owner;
   DBusPendingCall *pending_start_service_by_name;
   const char *unique_name;
   DBusPendingCall *pending_new;
   ec_connect_callback_t connect_cb;
   void *connect_cb_data;
   Eina_List *pending_add;
   Eina_List *pending_remove;
   Eina_List *pending_gen;
   DBusPendingCall *pending_clear;
   DBusPendingCall *pending_setup;
   void (*on_server_die_cb)(Ethumb_Client *client, void *data);
   void *on_server_die_cb_data;
   const char *object_path;

   Eina_Bool ethumb_dirty : 1;
   Eina_Bool connected : 1;
   Eina_Bool server_started : 1;
};

struct _ethumb_pending_add
{
   long id;
   const char *file;
   const char *key;
   const char *thumb;
   const char *thumb_key;
   generated_callback_t generated_cb;
   void *data;
   DBusPendingCall *pending_call;
   Ethumb_Client *client;
};

struct _ethumb_pending_remove
{
   long id;
   void (*remove_cb)(Eina_Bool result, void *data);
   void *data;
   DBusPendingCall *pending_call;
   Ethumb_Client *client;
};

struct _ethumb_pending_gen
{
   long id;
   const char *file;
   const char *key;
   const char *thumb;
   const char *thumb_key;
   generated_callback_t generated_cb;
   void *data;
};

static const char _ethumb_dbus_bus_name[] = "org.enlightenment.Ethumb";
static const char _ethumb_dbus_interface[] = "org.enlightenment.Ethumb";
static const char _ethumb_dbus_objects_interface[] = "org.enlightenment.Ethumb.objects";
static const char _ethumb_dbus_path[] = "/org/enlightenment/Ethumb";
static const char fdo_interface[] = "org.freedesktop.DBus";
static const char fdo_bus_name[] = "org.freedesktop.DBus";
static const char fdo_path[] = "/org/freedesktop/DBus";

static int _initcount = 0;

static void _ethumb_client_generated_cb(void *data, DBusMessage *msg);
static void _ethumb_client_get_name_owner(void *data, DBusMessage *msg, DBusError *err);

static inline bool
__dbus_callback_check_and_init(const char *file, int line, const char *function, DBusMessage *msg, DBusMessageIter *itr, DBusError *err)
{
   if (!msg)
     {
	ERR("%s:%d:%s() callback without message arguments!\n",
		file, line, function);

	if (err)
	  ERR("%s:%d:%s() an error was reported by server: "
		  "name=\"%s\", message=\"%s\"\n",
		  file, line, function, err->name, err->message);

	return 0;
     }

   if (!dbus_message_iter_init(msg, itr))
     {
	ERR("%s:%d:%s() could not init iterator.\n",
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

   ERR("expected type %s (%c) but got %c instead!\n",
	   expected_name, expected, type);

   return 0;
}
#define _dbus_iter_type_check(t, e) __dbus_iter_type_check(t, e, #e)

#define CHECK_NULL_RETURN(ptr, ...)					\
  do									\
    {									\
       if ((ptr) == NULL)						\
	 {								\
	    ERR("%s == NULL!\n", #ptr);		\
	    return __VA_ARGS__;						\
	 }								\
    }									\
  while (0)

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
	ERR("could not get NameOwnerChanged arguments: %s: %s\n",
	    err.name, err.message);
	dbus_error_free(&err);
	return;
     }

   if (strcmp(name, _ethumb_dbus_bus_name) != 0)
     return;

   DBG("NameOwnerChanged from=[%s] to=[%s]\n", from, to);

   if (from[0] != '\0' && to[0] == '\0')
     {
	DBG("exit ethumbd at %s\n", from);
	if (strcmp(client->unique_name, from) != 0)
	  WRN("%s was not the known name %s, ignored.\n",
	       from, client->unique_name);
	else
	  {
	     ERR("server exit!!!\n");
	     if (client->on_server_die_cb)
	       client->on_server_die_cb(client,
					client->on_server_die_cb_data);
	  }
     }
   else
     DBG("unknown change from %s to %s\n", from, to);
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

   client->connect_cb(client, 1, client->connect_cb_data);
   return;

end_error:
   client->connect_cb(client, 0, client->connect_cb_data);
   return;
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
   unsigned int ret;
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
	ERR("Error starting Ethumbd DBus service by its name: retcode %u\n",
	    ret);
	goto error;
     }

   client->server_started = 1;
   DBG("Ethumbd DBus service started successfully (%d), now request its name\n",
       ret);

   if (client->pending_get_name_owner)
     {
	DBG("already requesting name owner, cancel and try again\n");
	dbus_pending_call_cancel(client->pending_get_name_owner);
     }

   client->pending_get_name_owner = e_dbus_get_name_owner
     (client->conn, _ethumb_dbus_bus_name, _ethumb_client_get_name_owner,
      client);
   if (!client->pending_get_name_owner)
     {
	ERR("could not create a get_name_owner request.\n");
	goto error;
     }

   return;

 error:
   ERR("failed to start Ethumbd DBus service by its name.\n");
   client->connect_cb(client, 0, client->connect_cb_data);
}

static void
_ethumb_client_start_server(Ethumb_Client *client)
{
   if (client->pending_start_service_by_name)
     {
	DBG("already pending start service by name.\n");
	return;
     }

   client->server_started = 0;
   client->pending_start_service_by_name = e_dbus_start_service_by_name
     (client->conn, _ethumb_dbus_bus_name, _ethumb_client_start_server_cb,
      client);
   if (!client->pending_start_service_by_name)
     {
	ERR("could not start service by name!\n");
	client->connect_cb(client, 0, client->connect_cb_data);
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
	DBG("could not find server (%s), try to start it...\n", err->message);
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
	ERR("no name owner!\n");
	goto error;
     }

   DBG("unique name = %s\n", uid);
   client->unique_name = eina_stringshare_add(uid);

   _ethumb_client_call_new(client);
   client->connected = 1;
   return;

error:
   client->connect_cb(client, 0, client->connect_cb_data);
}

EAPI int
ethumb_client_init(void)
{
   if (_initcount)
     return ++_initcount;

   ethumb_init();
   e_dbus_init();

   return ++_initcount;
}

EAPI int
ethumb_client_shutdown(void)
{
   _initcount--;
   if (_initcount > 0)
     return _initcount;

   e_dbus_shutdown();
   ethumb_shutdown();
   return _initcount;
}

EAPI Ethumb_Client *
ethumb_client_connect(ec_connect_callback_t connect_cb, void *data)
{
   Ethumb_Client *eclient;

   EINA_SAFETY_ON_NULL_RETURN_VAL(connect_cb, NULL);

   eclient = calloc(1, sizeof(*eclient));
   if (!eclient)
     {
	ERR("could not allocate Ethumb_Client structure.\n");
	goto err;
     }

   eclient->connect_cb = connect_cb;
   eclient->connect_cb_data = data;

   eclient->ethumb = ethumb_new();
   if (!eclient->ethumb)
     {
	ERR("could not create ethumb handler.\n");
	goto ethumb_new_err;
     }

   eclient->conn = e_dbus_bus_get(DBUS_BUS_SESSION);
   if (!eclient->conn)
     {
	ERR("could not connect to session bus.\n");
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
	ERR("could not create a get_name_owner request.\n");
	goto connection_err;
     }

   return eclient;

connection_err:
   ethumb_free(eclient->ethumb);
ethumb_new_err:
   free(eclient);
err:
   return NULL;
}

EAPI void
ethumb_client_disconnect(Ethumb_Client *client)
{
   Eina_List *l;

   EINA_SAFETY_ON_NULL_RETURN(client);

   if (!client->connected)
     goto end_connection;

   ethumb_client_queue_clear(client);
   l = client->pending_remove;
   while (l)
     {
	struct _ethumb_pending_remove *pending = l->data;
	l = eina_list_remove_list(l, l);
	dbus_pending_call_cancel(pending->pending_call);
	dbus_pending_call_unref(pending->pending_call);
	free(pending);
     }

   l = client->pending_gen;
   while (l)
     {
	struct _ethumb_pending_gen *pending = l->data;
	l = eina_list_remove_list(l, l);
	free(pending);
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

   free(client);
}

EAPI void
ethumb_client_on_server_die_callback_set(Ethumb_Client *client, void (*on_server_die_cb)(Ethumb_Client *client, void *data), void *data)
{
   client->on_server_die_cb = on_server_die_cb;
   client->on_server_die_cb_data = data;
}

static void
_ethumb_client_ethumb_setup_cb(void *data, DBusMessage *msg, DBusError *error)
{
   DBusMessageIter iter;
   int t, result = 0;
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
	ERR("not an byte array element.\n");
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

EAPI void
ethumb_client_ethumb_setup(Ethumb_Client *client)
{
   DBusMessage *msg;
   DBusMessageIter iter, aiter, diter, viter, vaiter;
   Ethumb *e = client->ethumb;
   const char *entry;
   int tw, th, format, aspect;
   float cx, cy;
   const char *theme_file, *group, *swallow;
   const char *directory, *category;
   float video_time;
   int document_page;

   EINA_SAFETY_ON_NULL_RETURN(client);
   client->ethumb_dirty = 0;

   msg = dbus_message_new_method_call(_ethumb_dbus_bus_name,
				      client->object_path,
				      _ethumb_dbus_objects_interface,
				      "ethumb_setup");
   dbus_message_iter_init_append(msg, &iter);
   dbus_message_iter_open_container(&iter, DBUS_TYPE_ARRAY, "{sv}", &aiter);

#define _open_variant_iter(str_entry, str_type, end_iter)		   \
   entry = str_entry;							   \
   dbus_message_iter_open_container(&aiter, DBUS_TYPE_DICT_ENTRY, NULL, &diter); \
   dbus_message_iter_append_basic(&diter, DBUS_TYPE_STRING, &entry);   \
   dbus_message_iter_open_container(&diter, DBUS_TYPE_VARIANT, str_type,   \
				    &end_iter);

#define _close_variant_iter(end_iter)					   \
   dbus_message_iter_close_container(&diter, &end_iter);		   \
   dbus_message_iter_close_container(&aiter, &diter);

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

   _open_variant_iter("crop", "(dd)", viter);
   dbus_message_iter_open_container(&viter, DBUS_TYPE_STRUCT, NULL, &vaiter);
   ethumb_thumb_crop_align_get(e, &cx, &cy);
   dbus_message_iter_append_basic(&vaiter, DBUS_TYPE_DOUBLE, &cx);
   dbus_message_iter_append_basic(&vaiter, DBUS_TYPE_DOUBLE, &cy);
   dbus_message_iter_close_container(&viter, &vaiter);
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

   _open_variant_iter("document_page", "i", viter);
   document_page = ethumb_document_page_get(e);
   dbus_message_iter_append_basic(&viter, DBUS_TYPE_INT32, &document_page);
   _close_variant_iter(viter);

#undef _open_variant_iter
#undef _close_variant_iter

   dbus_message_iter_close_container(&iter, &aiter);

   client->pending_setup = e_dbus_message_send(client->conn, msg,
					       _ethumb_client_ethumb_setup_cb,
					       -1, client);
   dbus_message_unref(msg);
}

static void
_ethumb_client_generated_cb(void *data, DBusMessage *msg)
{
   DBusMessageIter iter;
   long id = -1;
   const char *thumb;
   const char *thumb_key;
   Ethumb_Client *client = data;
   int t;
   int success;
   Eina_List *l;
   int found;
   struct _ethumb_pending_gen *pending;

   dbus_message_iter_init(msg, &iter);

   t = dbus_message_iter_get_arg_type(&iter);
   if (!_dbus_iter_type_check(t, DBUS_TYPE_INT64))
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
	pending->generated_cb(id, pending->file, pending->key, success,
			pending->data);
	eina_stringshare_del(pending->file);
	eina_stringshare_del(pending->key);
	eina_stringshare_del(pending->thumb);
	eina_stringshare_del(pending->thumb_key);
	free(pending);
     }

end:
   eina_stringshare_del(thumb);
   eina_stringshare_del(thumb_key);
}

static void
_ethumb_client_queue_add_cb(void *data, DBusMessage *msg, DBusError *error)
{
   DBusMessageIter iter;
   int t;
   long id = -1;
   struct _ethumb_pending_add *pending = data;
   struct _ethumb_pending_gen *generating;
   Ethumb_Client *client = pending->client;

   client->pending_add = eina_list_remove(client->pending_add, pending);

   if (!_dbus_callback_check_and_init(msg, &iter, error))
     goto end;

   t = dbus_message_iter_get_arg_type(&iter);
   if (!_dbus_iter_type_check(t, DBUS_TYPE_INT64))
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
   client->pending_gen = eina_list_append(client->pending_gen, generating);

end:
   free(pending);
}

static long
_ethumb_client_queue_add(Ethumb_Client *client, const char *file, const char *key, const char *thumb, const char *thumb_key, generated_callback_t generated_cb, void *data)
{
   DBusMessage *msg;
   DBusMessageIter iter;
   struct _ethumb_pending_add *pending;


   if (!client->id_count) 
       client->id_count++;

   pending = calloc(1, sizeof(*pending));
   pending->id = client->id_count;
   pending->file = eina_stringshare_add(file);
   pending->key = eina_stringshare_add(key);
   pending->thumb = eina_stringshare_add(thumb);
   pending->thumb_key = eina_stringshare_add(thumb_key);
   pending->generated_cb = generated_cb;
   pending->data = data;
   pending->client = client;

   client->id_count = (client->id_count + 1) % MAX_ID;

   msg = dbus_message_new_method_call(_ethumb_dbus_bus_name,
				      client->object_path,
				      _ethumb_dbus_objects_interface,
				      "queue_add");

   dbus_message_iter_init_append(msg, &iter);
   dbus_message_iter_append_basic(&iter, DBUS_TYPE_INT64, &pending->id);
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
   int success = 0;
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
   if (pending->remove_cb)
     pending->remove_cb(success, pending->data);
   free(pending);
}

EAPI void
ethumb_client_queue_remove(Ethumb_Client *client, long id, void (*queue_remove_cb)(Eina_Bool success, void *data), void *data)
{
   DBusMessage *msg;
   struct _ethumb_pending_remove *pending;
   Eina_List *l;
   int found;
   EINA_SAFETY_ON_NULL_RETURN(client);

   pending = calloc(1, sizeof(*pending));
   pending->id = id;
   pending->remove_cb = queue_remove_cb;
   pending->data = data;
   pending->client = client;

   msg = dbus_message_new_method_call(_ethumb_dbus_bus_name,
				      client->object_path,
				      _ethumb_dbus_objects_interface,
				      "queue_remove");

   dbus_message_append_args(msg, DBUS_TYPE_INT64, &id, DBUS_TYPE_INVALID);
   pending->pending_call = e_dbus_message_send(client->conn, msg,
					       _ethumb_client_queue_remove_cb,
					       -1, pending);
   client->pending_remove = eina_list_append(client->pending_remove, pending);

   found = 0;
   l = client->pending_add;
   while (l)
     {
	struct _ethumb_pending_add *pending = l->data;
	if (pending->id != id)
	  {
	     l = l->next;
	     continue;
	  }
	l = eina_list_remove_list(l, l);
	eina_stringshare_del(pending->file);
	eina_stringshare_del(pending->key);
	eina_stringshare_del(pending->thumb);
	eina_stringshare_del(pending->thumb_key);
	dbus_pending_call_cancel(pending->pending_call);
	dbus_pending_call_unref(pending->pending_call);
	free(pending);
	found = 1;
	break;
     }

   if (found)
     goto end;

   l = client->pending_gen;
   while (l)
     {
	struct _ethumb_pending_gen *pending = l->data;
	if (pending->id != id)
	  {
	     l = l->next;
	     continue;
	  }
	l = eina_list_remove_list(l, l);
	eina_stringshare_del(pending->file);
	eina_stringshare_del(pending->key);
	eina_stringshare_del(pending->thumb);
	eina_stringshare_del(pending->thumb_key);
	free(pending);
	found = 1;
	break;
     }

end:
   dbus_message_unref(msg);
}

static void
_ethumb_client_queue_clear_cb(void *data, DBusMessage *msg, DBusError *error)
{
   Ethumb_Client *client = data;

   client->pending_clear = NULL;
}

EAPI void
ethumb_client_queue_clear(Ethumb_Client *client)
{
   DBusMessage *msg;
   EINA_SAFETY_ON_NULL_RETURN(client);

   if (client->pending_clear)
     return;

   msg = dbus_message_new_method_call(_ethumb_dbus_bus_name,
				      client->object_path,
				      _ethumb_dbus_objects_interface,
				      "queue_clear");

   client->pending_clear = e_dbus_message_send(client->conn, msg,
					       _ethumb_client_queue_clear_cb,
					       -1, client);
   dbus_message_unref(msg);
}

EAPI void
ethumb_client_fdo_set(Ethumb_Client *client, Ethumb_Thumb_FDO_Size s)
{
   EINA_SAFETY_ON_NULL_RETURN(client);

   client->ethumb_dirty = 1;
   ethumb_thumb_fdo_set(client->ethumb, s);
}

EAPI void
ethumb_client_size_set(Ethumb_Client *client, int tw, int th)
{
   EINA_SAFETY_ON_NULL_RETURN(client);

   client->ethumb_dirty = 1;
   ethumb_thumb_size_set(client->ethumb, tw, th);
}

EAPI void
ethumb_client_size_get(const Ethumb_Client *client, int *tw, int *th)
{
   EINA_SAFETY_ON_NULL_RETURN(client);

   ethumb_thumb_size_get(client->ethumb, tw, th);
}

EAPI void
ethumb_client_format_set(Ethumb_Client *client, Ethumb_Thumb_Format f)
{
   EINA_SAFETY_ON_NULL_RETURN(client);

   client->ethumb_dirty = 1;
   ethumb_thumb_format_set(client->ethumb, f);
}

EAPI Ethumb_Thumb_Format
ethumb_client_format_get(const Ethumb_Client *client)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(client, 0);

   return ethumb_thumb_format_get(client->ethumb);
}

EAPI void
ethumb_client_aspect_set(Ethumb_Client *client, Ethumb_Thumb_Aspect a)
{
   EINA_SAFETY_ON_NULL_RETURN(client);

   client->ethumb_dirty = 1;
   ethumb_thumb_aspect_set(client->ethumb, a);
}

EAPI Ethumb_Thumb_Aspect
ethumb_client_aspect_get(const Ethumb_Client *client)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(client, 0);

   return ethumb_thumb_aspect_get(client->ethumb);
}

EAPI void
ethumb_client_crop_align_set(Ethumb_Client *client, float x, float y)
{
   EINA_SAFETY_ON_NULL_RETURN(client);

   client->ethumb_dirty = 1;
   ethumb_thumb_crop_align_set(client->ethumb, x, y);
}

EAPI void
ethumb_client_crop_align_get(const Ethumb_Client *client, float *x, float *y)
{
   EINA_SAFETY_ON_NULL_RETURN(client);

   ethumb_thumb_crop_align_get(client->ethumb, x, y);
}

EAPI Eina_Bool
ethumb_client_frame_set(Ethumb_Client *client, const char *file, const char *group, const char *swallow)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(client, 0);

   client->ethumb_dirty = 1;
   return ethumb_frame_set(client->ethumb, file, group, swallow);
}

EAPI void
ethumb_client_dir_path_set(Ethumb_Client *client, const char *path)
{
   EINA_SAFETY_ON_NULL_RETURN(client);

   client->ethumb_dirty = 1;
   ethumb_thumb_dir_path_set(client->ethumb, path);
}

EAPI const char *
ethumb_client_dir_path_get(const Ethumb_Client *client)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(client, NULL);

   return ethumb_thumb_dir_path_get(client->ethumb);
}

EAPI void
ethumb_client_category_set(Ethumb_Client *client, const char *category)
{
   EINA_SAFETY_ON_NULL_RETURN(client);

   client->ethumb_dirty = 1;
   ethumb_thumb_category_set(client->ethumb, category);
}

EAPI const char *
ethumb_client_category_get(const Ethumb_Client *client)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(client, NULL);

   return ethumb_thumb_category_get(client->ethumb);
}

EAPI void
ethumb_client_video_time_set(Ethumb_Client *client, float time)
{
   EINA_SAFETY_ON_NULL_RETURN(client);

   client->ethumb_dirty = 1;
   ethumb_video_time_set(client->ethumb, time);
}

EAPI void
ethumb_client_document_page_set(Ethumb_Client *client, int page)
{
   EINA_SAFETY_ON_NULL_RETURN(client);

   client->ethumb_dirty = 1;
   ethumb_document_page_set(client->ethumb, page);
}

EAPI Eina_Bool
ethumb_client_file_set(Ethumb_Client *client, const char *path, const char *key)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(client, 0);

   return ethumb_file_set(client->ethumb, path, key);
}

EAPI void
ethumb_client_file_get(Ethumb_Client *client, const char **path, const char **key)
{
   EINA_SAFETY_ON_NULL_RETURN(client);

   ethumb_file_get(client->ethumb, path, key);
}

EAPI void
ethumb_client_file_free(Ethumb_Client *client)
{
   EINA_SAFETY_ON_NULL_RETURN(client);

   ethumb_file_free(client->ethumb);
}

EAPI void
ethumb_client_thumb_path_set(Ethumb_Client *client, const char *path, const char *key)
{
   EINA_SAFETY_ON_NULL_RETURN(client);

   ethumb_thumb_path_set(client->ethumb, path, key);
}

EAPI void
ethumb_client_thumb_path_get(Ethumb_Client *client, const char **path, const char **key)
{
   EINA_SAFETY_ON_NULL_RETURN(client);

   ethumb_thumb_path_get(client->ethumb, path, key);
}

EAPI Eina_Bool
ethumb_client_thumb_exists(Ethumb_Client *client)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(client, 0);

   return ethumb_exists(client->ethumb);
}

EAPI long
ethumb_client_generate(Ethumb_Client *client, generated_callback_t generated_cb, void *data)
{
   const char *file, *key, *thumb, *thumb_key;
   long id;
   EINA_SAFETY_ON_NULL_RETURN_VAL(client, -1);

   ethumb_file_get(client->ethumb, &file, &key);
   if (!file)
     {
	ERR("no file set.\n");
	return 0;
     }

   ethumb_thumb_path_get(client->ethumb, &thumb, &thumb_key);

   if (client->ethumb_dirty)
     ethumb_client_ethumb_setup(client);
   id = _ethumb_client_queue_add(client, file, key, thumb, thumb_key,
				 generated_cb, data);

   return id;
}
