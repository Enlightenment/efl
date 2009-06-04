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

#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>
#include <errno.h>

#include <Ecore.h>
#include <Ethumb.h>
#include <Eina.h>

#include "ethumbd_private.h"

#define DBG(...) EINA_ERROR_PDBG(__VA_ARGS__)
#define INF(...) EINA_ERROR_PINFO(__VA_ARGS__)
#define WRN(...) EINA_ERROR_PWARN(__VA_ARGS__)
#define ERR(...) EINA_ERROR_PERR(__VA_ARGS__)

#define NETHUMBS 100

struct _Ethumbd_Child
{
   Ecore_Fd_Handler *fd_handler;
   Ethumb *ethumbt[NETHUMBS];
   int pipein, pipeout;
};


int
_ec_read_safe(int fd, void *buf, ssize_t size)
{
   ssize_t todo;
   char *p;

   todo = size;
   p = buf;

   while (todo > 0)
     {
	ssize_t r;

	r = read(fd, p, todo);
	if (r > 0)
	  {
	     todo -= r;
	     p += r;
	  }
	else if (r == 0)
	  return 0;
	else
	  {
	     if (errno == EINTR || errno == EAGAIN)
	       continue;
	     else
	       {
		  ERR("could not read from fd %d: %s",
		      fd, strerror(errno));
		  return 0;
	       }
	  }
     }

   return 1;
}

int
_ec_write_safe(int fd, const void *buf, ssize_t size)
{
   ssize_t todo;
   const char *p;

   todo = size;
   p = buf;

   while (todo > 0)
     {
	ssize_t r;

	r = write(fd, p, todo);
	if (r > 0)
	  {
	     todo -= r;
	     p += r;
	  }
	else if (r == 0)
	  return 0;
	else
	  {
	     if (errno == EINTR || errno == EAGAIN)
	       continue;
	     else
	       {
		  ERR("could not write to fd %d: %s", fd, strerror(errno));
		  return 0;
	       }
	  }
     }

   return 1;
}

static int
_ec_pipe_str_read(struct _Ethumbd_Child *ec, char **str)
{
   int size;
   int r;
   char buf[PATH_MAX];

   r = _ec_read_safe(ec->pipein, &size, sizeof(size));
   if (!r)
     {
	*str = NULL;
	return 0;
     }

   if (!size)
     {
	*str = NULL;
	return 1;
     }

   r = _ec_read_safe(ec->pipein, buf, size);
   if (!r)
     {
	*str = NULL;
	return 0;
     }

   *str = strdup(buf);
   return 1;
}

static void
_ec_pipe_str_write(struct _Ethumbd_Child *ec, const char *str)
{
   int size;

   if (!str)
     size = 0;
   else
     size = strlen(str) + 1;

   _ec_write_safe(ec->pipeout, &size, sizeof(size));
   _ec_write_safe(ec->pipeout, str, size);
}

static struct _Ethumbd_Child *
_ec_new(int pipein, int pipeout)
{
   struct _Ethumbd_Child *ec = calloc(1, sizeof(*ec));

   ec->pipein = pipein;
   ec->pipeout = pipeout;

   return ec;
}

static void
_ec_free(struct _Ethumbd_Child *ec)
{
   int i;

   if (ec->fd_handler)
     ecore_main_fd_handler_del(ec->fd_handler);

   for (i = 0; i < NETHUMBS; i++)
     {
	if (ec->ethumbt[i])
	  ethumb_free(ec->ethumbt[i]);
     }

   free(ec);
}

static int
_ec_op_new(struct _Ethumbd_Child *ec)
{
   int r;
   int index;

   r = _ec_read_safe(ec->pipein, &index, sizeof(index));
   if (!r)
     return 0;

   DBG("ethumbd new(). index = %d\n", index);

   ec->ethumbt[index] = ethumb_new();
   return 1;
}

static int
_ec_op_del(struct _Ethumbd_Child *ec)
{
   int r;
   int index;

   r = _ec_read_safe(ec->pipein, &index, sizeof(index));
   if (!r)
     return 0;

   DBG("ethumbd del(). index = %d\n", index);

   ethumb_free(ec->ethumbt[index]);
   ec->ethumbt[index] = NULL;
   return 1;
}

static void
_ec_op_generated_cb(Ethumb *e, Eina_Bool success, void *data)
{
   struct _Ethumbd_Child *ec = data;
   const char *thumb_path, *thumb_key;

   DBG("thumb generated!\n");
   ethumb_thumb_path_get(e, &thumb_path, &thumb_key);
   _ec_write_safe(ec->pipeout, &success, sizeof(success));

   _ec_pipe_str_write(ec, thumb_path);
   _ec_pipe_str_write(ec, thumb_key);
}

static int
_ec_op_generate(struct _Ethumbd_Child *ec)
{
   int index;
   char *path, *key, *thumb_path, *thumb_key;
   int r;

   r = _ec_read_safe(ec->pipein, &index, sizeof(index));
   if (!r)
     return 0;

   r = _ec_pipe_str_read(ec, &path);
   if (!r)
     return 0;
   r = _ec_pipe_str_read(ec, &key);
   if (!r)
     return 0;
   r = _ec_pipe_str_read(ec, &thumb_path);
   if (!r)
     return 0;
   r = _ec_pipe_str_read(ec, &thumb_key);
   if (!r)
     return 0;

   ethumb_file_set(ec->ethumbt[index], path, key);
   ethumb_thumb_path_set(ec->ethumbt[index], thumb_path, thumb_key);
   ethumb_generate(ec->ethumbt[index], _ec_op_generated_cb,
		   ec);

   free(path);
   free(key);
   free(thumb_path);
   free(thumb_key);

   return 1;
}

static int
_ec_fdo_set(struct _Ethumbd_Child *ec, Ethumb *e)
{
   int r;
   int value;

   r = _ec_read_safe(ec->pipein, &value, sizeof(value));
   if (!r)
     return 0;
   ethumb_thumb_fdo_set(e, value);
   DBG("fdo = %d\n", value);

   return 1;
}

static int
_ec_size_set(struct _Ethumbd_Child *ec, Ethumb *e)
{
   int r;
   int w, h;
   int type;

   r = _ec_read_safe(ec->pipein, &w, sizeof(w));
   if (!r)
     return 0;
   r = _ec_read_safe(ec->pipein, &type, sizeof(type));
   if (!r)
     return 0;
   r = _ec_read_safe(ec->pipein, &h, sizeof(h));
   if (!r)
     return 0;
   ethumb_thumb_size_set(e, w, h);
   DBG("size = %dx%d\n", w, h);

   return 1;
}

static int
_ec_format_set(struct _Ethumbd_Child *ec, Ethumb *e)
{
   int r;
   int value;

   r = _ec_read_safe(ec->pipein, &value, sizeof(value));
   if (!r)
     return 0;
   ethumb_thumb_format_set(e, value);
   DBG("format = %d\n", value);

   return 1;
}

static int
_ec_aspect_set(struct _Ethumbd_Child *ec, Ethumb *e)
{
   int r;
   int value;

   r = _ec_read_safe(ec->pipein, &value, sizeof(value));
   if (!r)
     return 0;
   ethumb_thumb_aspect_set(e, value);
   DBG("aspect = %d\n", value);

   return 1;
}

static int
_ec_crop_set(struct _Ethumbd_Child *ec, Ethumb *e)
{
   int r;
   float x, y;
   int type;

   r = _ec_read_safe(ec->pipein, &x, sizeof(x));
   if (!r)
     return 0;
   r = _ec_read_safe(ec->pipein, &type, sizeof(type));
   if (!r)
     return 0;
   r = _ec_read_safe(ec->pipein, &y, sizeof(y));
   if (!r)
     return 0;
   ethumb_thumb_crop_align_set(e, x, y);
   DBG("crop = %fx%f\n", x, y);

   return 1;
}

static int
_ec_frame_set(struct _Ethumbd_Child *ec, Ethumb *e)
{
   int r;
   int type;
   char *theme_file, *group, *swallow;

   r = _ec_pipe_str_read(ec, &theme_file);
   if (!r)
     return 0;
   r = _ec_read_safe(ec->pipein, &type, sizeof(type));
   if (!r)
     return 0;
   r = _ec_pipe_str_read(ec, &group);
   if (!r)
     return 0;
   r = _ec_read_safe(ec->pipein, &type, sizeof(type));
   if (!r)
     return 0;
   r = _ec_pipe_str_read(ec, &swallow);
   if (!r)
     return 0;
   DBG("frame = %s:%s:%s\n", theme_file, group, swallow);
   ethumb_frame_set(e, theme_file, group, swallow);
   free(theme_file);
   free(group);
   free(swallow);

   return 1;
}

static int
_ec_directory_set(struct _Ethumbd_Child *ec, Ethumb *e)
{
   int r;
   char *directory;

   r = _ec_pipe_str_read(ec, &directory);
   if (!r)
     return 0;
   ethumb_thumb_dir_path_set(e, directory);
   DBG("directory = %s\n", directory);
   free(directory);

   return 1;
}

static int
_ec_category_set(struct _Ethumbd_Child *ec, Ethumb *e)
{
   int r;
   char *category;

   r = _ec_pipe_str_read(ec, &category);
   if (!r)
     return 0;
   ethumb_thumb_category_set(e, category);
   DBG("category = %s\n", category);
   free(category);

   return 1;
}

static int
_ec_video_time_set(struct _Ethumbd_Child *ec, Ethumb *e)
{
   int r;
   float value;

   r = _ec_read_safe(ec->pipein, &value, sizeof(value));
   if (!r)
     return 0;
   ethumb_video_time_set(e, value);
   DBG("video_time = %f\n", value);

   return 1;
}

static int
_ec_document_page_set(struct _Ethumbd_Child *ec, Ethumb *e)
{
   int r;
   int value;

   r = _ec_read_safe(ec->pipein, &value, sizeof(value));
   if (!r)
     return 0;
   ethumb_document_page_set(e, value);
   DBG("document_page = %d\n", value);

   return 1;
}

static void
_ec_setup_process(struct _Ethumbd_Child *ec, int index, int type)
{
   Ethumb *e;

   e = ec->ethumbt[index];

   switch (type)
     {
      case ETHUMBD_FDO:
	 _ec_fdo_set(ec, e);
	 break;
      case ETHUMBD_SIZE_W:
	 _ec_size_set(ec, e);
	 break;
      case ETHUMBD_FORMAT:
	 _ec_format_set(ec, e);
	 break;
      case ETHUMBD_ASPECT:
	 _ec_aspect_set(ec, e);
	 break;
      case ETHUMBD_CROP_X:
	 _ec_crop_set(ec, e);
	 break;
      case ETHUMBD_FRAME_FILE:
	 _ec_frame_set(ec, e);
	 break;
      case ETHUMBD_DIRECTORY:
	 _ec_directory_set(ec, e);
	 break;
      case ETHUMBD_CATEGORY:
	 _ec_category_set(ec, e);
	 break;
      case ETHUMBD_VIDEO_TIME:
	 _ec_video_time_set(ec, e);
	 break;
      case ETHUMBD_DOCUMENT_PAGE:
	 _ec_document_page_set(ec, e);
	 break;
      default:
	 ERR("wrong type!\n");
     }
}

static int
_ec_op_setup(struct _Ethumbd_Child *ec)
{
   int r;
   int index;
   int type;

   r = _ec_read_safe(ec->pipein, &index, sizeof(index));
   if (!r)
     return 0;

   r = _ec_read_safe(ec->pipein, &type, sizeof(type));
   if (!r)
     return 0;
   while (type != ETHUMBD_SETUP_FINISHED)
     {
	_ec_setup_process(ec, index, type);
	r = _ec_read_safe(ec->pipein, &type, sizeof(type));
	if (!r)
	  return 0;
     }

   return 1;
}

static int
_ec_fd_handler(void *data, Ecore_Fd_Handler *fd_handler)
{
   struct _Ethumbd_Child *ec = data;
   int op_id;
   int r;

   if (ecore_main_fd_handler_active_get(fd_handler, ECORE_FD_ERROR))
     {
	ERR("error on pipein! child exiting...\n");
	ec->fd_handler = NULL;
	ecore_main_loop_quit();
	return 0;
     }

   r = _ec_read_safe(ec->pipein, &op_id, sizeof(op_id));
   if (!r)
     {
	DBG("ethumbd exited! child exiting...\n");
	ec->fd_handler = NULL;
	ecore_main_loop_quit();
	return 0;
     }

   DBG("received op: %d\n", op_id);

   r = 1;
   switch (op_id)
     {
      case ETHUMBD_OP_NEW:
	 r = _ec_op_new(ec);
	 break;
      case ETHUMBD_OP_GENERATE:
	 r = _ec_op_generate(ec);
	 break;
      case ETHUMBD_OP_SETUP:
	 r = _ec_op_setup(ec);
	 break;
      case ETHUMBD_OP_DEL:
	 r = _ec_op_del(ec);
	 break;
      default:
	 ERR("invalid operation: %d\n", op_id);
	 r = 0;
     }

   if (!r)
     {
	ERR("ethumbd exited! child exiting...\n");
	ec->fd_handler = NULL;
	ecore_main_loop_quit();
     }

   return r;
}

static void
_ec_setup(struct _Ethumbd_Child *ec)
{
   ec->fd_handler = ecore_main_fd_handler_add(
      ec->pipein, ECORE_FD_READ | ECORE_FD_ERROR,
      _ec_fd_handler, ec, NULL, NULL);
}

void
ethumbd_child_start(int pipein, int pipeout)
{
   struct _Ethumbd_Child *ec;

   ethumb_init();

   ec = _ec_new(pipein, pipeout);

   _ec_setup(ec);

   DBG("child started!\n");
   ecore_main_loop_begin();
   DBG("child finishing.\n");

   _ec_free(ec);

   ethumb_shutdown();
}
