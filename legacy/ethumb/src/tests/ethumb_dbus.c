/**
 * @file
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
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <dirent.h>
#include <Ethumb.h>
#include <Ethumb_Client.h>
#include <Eina.h>
#include <Ecore_Getopt.h>
#include <Ecore.h>

static void
_on_server_die_cb(void *data __UNUSED__, Ethumb_Client *client __UNUSED__)
{
   ecore_main_loop_quit();
}

static void
_queue_add_cb(void *data __UNUSED__, Ethumb_Client *client __UNUSED__, int id, const char *file, const char *key __UNUSED__, const char *thumb_path, const char *thumb_key __UNUSED__, Eina_Bool success)
{
   fprintf(stderr, ">>> %hhu file ready: %s; thumb ready: %s; id = %d\n", success, file, thumb_path, id);
}

static void
_request_thumbnails(Ethumb_Client *client, void *data)
{
   const char *path = data;
   DIR *dir;
   struct dirent *de;
   char buf[PATH_MAX];

   dir = opendir(path);
   if (!dir)
     {
	fprintf(stderr, "ERROR: could not open directory: %s\n", path);
	return;
     }

   ethumb_client_format_set(client, ETHUMB_THUMB_JPEG);
   ethumb_client_aspect_set(client, ETHUMB_THUMB_CROP);
   ethumb_client_crop_align_set(client, 0.2, 0.2);
   ethumb_client_size_set(client, 192, 192);
   ethumb_client_category_set(client, "custom");

   while ((de = readdir(dir)))
     {
	if (de->d_type != DT_REG)
	  continue;
	snprintf(buf, sizeof(buf), "%s/%s", path, de->d_name);
	ethumb_client_file_set(client, buf, NULL);
	ethumb_client_generate(client, _queue_add_cb, NULL, NULL);
     }

   closedir(dir);
}

static void
_connect_cb(void *data, Ethumb_Client *client, Eina_Bool success)
{
   fprintf(stderr, "connected: %d\n", success);
   if (!success)
     {
	ecore_main_loop_quit();
	return;
     }

   _request_thumbnails(client, data);
}

int
main(int argc, char *argv[])
{
   Ethumb_Client *client;

   if (argc < 2)
     {
	fprintf(stderr, "ERROR: directory not specified.\n");
	fprintf(stderr, "usage:\n\tethumb_dbus <images directory>\n");
	return -2;
     }

   ethumb_client_init();
   client = ethumb_client_connect(_connect_cb, argv[1], NULL);
   if (!client)
     {
	fprintf(stderr, "ERROR: couldn't connect to server.\n");
	ethumb_client_shutdown();
	return -1;
     }
   ethumb_client_on_server_die_callback_set(client, _on_server_die_cb, NULL, NULL);

   fprintf(stderr, "*** debug\n");
   ecore_main_loop_begin();

   ethumb_client_disconnect(client);

   ethumb_client_shutdown();

   return 0;
}
