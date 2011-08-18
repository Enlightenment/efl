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
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>
#include <errno.h>

#include <Ecore.h>
#include <Ethumb.h>
#include <Eina.h>

#include "ethumbd_private.h"

#define DBG(...) EINA_LOG_DOM_DBG(_log_domain, __VA_ARGS__)
#define INF(...) EINA_LOG_DOM_INFO(_log_domain, __VA_ARGS__)
#define WRN(...) EINA_LOG_DOM_WARN(_log_domain, __VA_ARGS__)
#define ERR(...) EINA_LOG_DOM_ERR(_log_domain, __VA_ARGS__)

#define NETHUMBS 100

static int _log_domain = -1;

struct _Ethumbd_Child
{
   Ecore_Fd_Handler *fd_handler;
   Ethumb *ethumbt[NETHUMBS];
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
_ec_pipe_str_read(struct _Ethumbd_Child *ec __UNUSED__, char **str)
{
   int size;
   int r;
   char buf[PATH_MAX];

   r = _ec_read_safe(STDIN_FILENO, &size, sizeof(size));
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

   r = _ec_read_safe(STDIN_FILENO, buf, size);
   if (!r)
     {
	*str = NULL;
	return 0;
     }

   *str = strdup(buf);
   return 1;
}

static struct _Ethumbd_Child *
_ec_new(void)
{
   struct _Ethumbd_Child *ec = calloc(1, sizeof(*ec));

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
   int idx;

   r = _ec_read_safe(STDIN_FILENO, &idx, sizeof(idx));
   if (!r)
     return 0;

   DBG("ethumbd new(). idx = %d", idx);

   ec->ethumbt[idx] = ethumb_new();
   return 1;
}

static int
_ec_op_del(struct _Ethumbd_Child *ec)
{
   int r;
   int idx;

   r = _ec_read_safe(STDIN_FILENO, &idx, sizeof(idx));
   if (!r)
     return 0;

   DBG("ethumbd del(). idx = %d", idx);

   ethumb_free(ec->ethumbt[idx]);
   ec->ethumbt[idx] = NULL;
   return 1;
}

static void
_ec_op_generated_cb(void *data __UNUSED__, Ethumb *e, Eina_Bool success)
{
   const char *thumb_path, *thumb_key;
   int size_path, size_key, size_cmd;

   DBG("thumb generated (%i)!", success);
   ethumb_thumb_path_get(e, &thumb_path, &thumb_key);

   if (!thumb_path)
     size_path = 0;
   else
     size_path = strlen(thumb_path) + 1;

   if (!thumb_key)
     size_key = 0;
   else
     size_key = strlen(thumb_key) + 1;

   size_cmd = sizeof(success) + sizeof(size_path) + size_path +
      sizeof(size_key) + size_key;

   _ec_write_safe(STDOUT_FILENO, &size_cmd, sizeof(size_cmd));
   _ec_write_safe(STDOUT_FILENO, &success, sizeof(success));

   _ec_write_safe(STDOUT_FILENO, &size_path, sizeof(size_path));
   _ec_write_safe(STDOUT_FILENO, thumb_path, size_path);

   _ec_write_safe(STDOUT_FILENO, &size_key, sizeof(size_key));
   _ec_write_safe(STDOUT_FILENO, thumb_key, size_key);
}

static int
_ec_op_generate(struct _Ethumbd_Child *ec)
{
   int idx;
   char *path, *key, *thumb_path, *thumb_key;
   int r;

   r = _ec_read_safe(STDIN_FILENO, &idx, sizeof(idx));
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

   ethumb_file_set(ec->ethumbt[idx], path, key);
   ethumb_thumb_path_set(ec->ethumbt[idx], thumb_path, thumb_key);

   if (ethumb_exists(ec->ethumbt[idx]))
     {
        _ec_op_generated_cb(ec, ec->ethumbt[idx], EINA_TRUE);
     }
   else
     {
        ethumb_generate(ec->ethumbt[idx], _ec_op_generated_cb, ec, NULL);
     }

   free(path);
   free(key);
   free(thumb_path);
   free(thumb_key);

   return 1;
}

static int
_ec_fdo_set(struct _Ethumbd_Child *ec __UNUSED__, Ethumb *e)
{
   int r;
   int value;

   r = _ec_read_safe(STDIN_FILENO, &value, sizeof(value));
   if (!r)
     return 0;
   ethumb_thumb_fdo_set(e, value);
   DBG("fdo = %d", value);

   return 1;
}

static int
_ec_size_set(struct _Ethumbd_Child *ec __UNUSED__, Ethumb *e)
{
   int r;
   int w, h;
   int type;

   r = _ec_read_safe(STDIN_FILENO, &w, sizeof(w));
   if (!r)
     return 0;
   r = _ec_read_safe(STDIN_FILENO, &type, sizeof(type));
   if (!r)
     return 0;
   r = _ec_read_safe(STDIN_FILENO, &h, sizeof(h));
   if (!r)
     return 0;
   ethumb_thumb_size_set(e, w, h);
   DBG("size = %dx%d", w, h);

   return 1;
}

static int
_ec_format_set(struct _Ethumbd_Child *ec __UNUSED__, Ethumb *e)
{
   int r;
   int value;

   r = _ec_read_safe(STDIN_FILENO, &value, sizeof(value));
   if (!r)
     return 0;
   ethumb_thumb_format_set(e, value);
   DBG("format = %d", value);

   return 1;
}

static int
_ec_aspect_set(struct _Ethumbd_Child *ec __UNUSED__, Ethumb *e)
{
   int r;
   int value;

   r = _ec_read_safe(STDIN_FILENO, &value, sizeof(value));
   if (!r)
     return 0;
   ethumb_thumb_aspect_set(e, value);
   DBG("aspect = %d", value);

   return 1;
}

static int
_ec_orientation_set(struct _Ethumbd_Child *ec __UNUSED__, Ethumb *e)
{
   int r;
   int value;

   r = _ec_read_safe(STDIN_FILENO, &value, sizeof(value));
   if (!r)
     return 0;
   ethumb_thumb_orientation_set(e, value);
   DBG("orientation = %d", value);

   return 1;
}

static int
_ec_crop_set(struct _Ethumbd_Child *ec __UNUSED__, Ethumb *e)
{
   int r;
   float x, y;
   int type;

   r = _ec_read_safe(STDIN_FILENO, &x, sizeof(x));
   if (!r)
     return 0;
   r = _ec_read_safe(STDIN_FILENO, &type, sizeof(type));
   if (!r)
     return 0;
   r = _ec_read_safe(STDIN_FILENO, &y, sizeof(y));
   if (!r)
     return 0;
   ethumb_thumb_crop_align_set(e, x, y);
   DBG("crop = %fx%f", x, y);

   return 1;
}

static int
_ec_quality_set(struct _Ethumbd_Child *ec __UNUSED__, Ethumb *e)
{
   int r;
   int value;

   r = _ec_read_safe(STDIN_FILENO, &value, sizeof(value));
   if (!r)
     return 0;
   ethumb_thumb_quality_set(e, value);
   DBG("quality = %d", value);

   return 1;
}

static int
_ec_compress_set(struct _Ethumbd_Child *ec __UNUSED__, Ethumb *e)
{
   int r;
   int value;

   r = _ec_read_safe(STDIN_FILENO, &value, sizeof(value));
   if (!r)
     return 0;
   ethumb_thumb_compress_set(e, value);
   DBG("compress = %d", value);

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
   r = _ec_read_safe(STDIN_FILENO, &type, sizeof(type));
   if (!r)
     return 0;
   r = _ec_pipe_str_read(ec, &group);
   if (!r)
     return 0;
   r = _ec_read_safe(STDIN_FILENO, &type, sizeof(type));
   if (!r)
     return 0;
   r = _ec_pipe_str_read(ec, &swallow);
   if (!r)
     return 0;
   DBG("frame = %s:%s:%s", theme_file, group, swallow);
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
   DBG("directory = %s", directory);
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
   DBG("category = %s", category);
   free(category);

   return 1;
}

static int
_ec_video_time_set(struct _Ethumbd_Child *ec __UNUSED__, Ethumb *e)
{
   int r;
   float value;

   r = _ec_read_safe(STDIN_FILENO, &value, sizeof(value));
   if (!r)
     return 0;
   ethumb_video_time_set(e, value);
   DBG("video_time = %f", value);

   return 1;
}

static int
_ec_video_start_set(struct _Ethumbd_Child *ec __UNUSED__, Ethumb *e)
{
   int r;
   float value;

   r = _ec_read_safe(STDIN_FILENO, &value, sizeof(value));
   if (!r)
     return 0;
   ethumb_video_start_set(e, value);
   DBG("video_start = %f", value);

   return 1;
}

static int
_ec_video_interval_set(struct _Ethumbd_Child *ec __UNUSED__, Ethumb *e)
{
   int r;
   float value;

   r = _ec_read_safe(STDIN_FILENO, &value, sizeof(value));
   if (!r)
     return 0;
   ethumb_video_interval_set(e, value);
   DBG("video_interval = %f", value);

   return 1;
}

static int
_ec_video_ntimes_set(struct _Ethumbd_Child *ec __UNUSED__, Ethumb *e)
{
   int r;
   int value;

   r = _ec_read_safe(STDIN_FILENO, &value, sizeof(value));
   if (!r)
     return 0;
   ethumb_video_ntimes_set(e, value);
   DBG("video_ntimes = %d", value);

   return 1;
}

static int
_ec_video_fps_set(struct _Ethumbd_Child *ec __UNUSED__, Ethumb *e)
{
   int r;
   int value;

   r = _ec_read_safe(STDIN_FILENO, &value, sizeof(value));
   if (!r)
     return 0;
   ethumb_video_fps_set(e, value);
   DBG("video_fps = %d", value);

   return 1;
}

static int
_ec_document_page_set(struct _Ethumbd_Child *ec __UNUSED__, Ethumb *e)
{
   int r;
   int value;

   r = _ec_read_safe(STDIN_FILENO, &value, sizeof(value));
   if (!r)
     return 0;
   ethumb_document_page_set(e, value);
   DBG("document_page = %d", value);

   return 1;
}

static void
_ec_setup_process(struct _Ethumbd_Child *ec, int idx, int type)
{
   Ethumb *e;

   e = ec->ethumbt[idx];

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
      case ETHUMBD_ORIENTATION:
	 _ec_orientation_set(ec, e);
	 break;
      case ETHUMBD_CROP_X:
	 _ec_crop_set(ec, e);
	 break;
      case ETHUMBD_QUALITY:
	 _ec_quality_set(ec, e);
	 break;
      case ETHUMBD_COMPRESS:
	 _ec_compress_set(ec, e);
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
      case ETHUMBD_VIDEO_START:
	 _ec_video_start_set(ec, e);
	 break;
      case ETHUMBD_VIDEO_INTERVAL:
	 _ec_video_interval_set(ec, e);
	 break;
      case ETHUMBD_VIDEO_NTIMES:
	 _ec_video_ntimes_set(ec, e);
	 break;
      case ETHUMBD_VIDEO_FPS:
	 _ec_video_fps_set(ec, e);
	 break;
      case ETHUMBD_DOCUMENT_PAGE:
	 _ec_document_page_set(ec, e);
	 break;
      default:
	 ERR("wrong type!");
     }
}

static int
_ec_op_setup(struct _Ethumbd_Child *ec)
{
   int r;
   int idx;
   int type;

   r = _ec_read_safe(STDIN_FILENO, &idx, sizeof(idx));
   if (!r)
     return 0;

   r = _ec_read_safe(STDIN_FILENO, &type, sizeof(type));
   if (!r)
     return 0;
   while (type != ETHUMBD_SETUP_FINISHED)
     {
	_ec_setup_process(ec, idx, type);
	r = _ec_read_safe(STDIN_FILENO, &type, sizeof(type));
	if (!r)
	  return 0;
     }

   return 1;
}

static Eina_Bool
_ec_fd_handler(void *data, Ecore_Fd_Handler *fd_handler)
{
   struct _Ethumbd_Child *ec = data;
   int op_id;
   int r;

   if (ecore_main_fd_handler_active_get(fd_handler, ECORE_FD_ERROR))
     {
	ERR("error on pipein! child exiting...");
	ec->fd_handler = NULL;
	ecore_main_loop_quit();
	return 0;
     }

   r = _ec_read_safe(STDIN_FILENO, &op_id, sizeof(op_id));
   if (!r)
     {
	DBG("ethumbd exited! child exiting...");
	ec->fd_handler = NULL;
	ecore_main_loop_quit();
	return 0;
     }

   DBG("received op: %d", op_id);

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
	 ERR("invalid operation: %d", op_id);
	 r = 0;
     }

   if (!r)
     {
	ERR("ethumbd exited! child exiting...");
	ec->fd_handler = NULL;
	ecore_main_loop_quit();
     }

   return r;
}

static void
_ec_setup(struct _Ethumbd_Child *ec)
{
   ec->fd_handler = ecore_main_fd_handler_add(
      STDIN_FILENO, ECORE_FD_READ | ECORE_FD_ERROR,
      _ec_fd_handler, ec, NULL, NULL);
}

int
main(int argc __UNUSED__, const char *argv[] __UNUSED__)
{
   struct _Ethumbd_Child *ec;

   ethumb_init();

   if (_log_domain < 0)
     {
	_log_domain = eina_log_domain_register("ethumbd_child", NULL);

	if (_log_domain < 0)
	  {
	     EINA_LOG_CRIT("could not register log domain 'ethumbd_child'");
	     ethumb_shutdown();
	     return 1;
	  }
     }

   ec = _ec_new();

   _ec_setup(ec);

   DBG("child started!");
   ecore_main_loop_begin();
   DBG("child finishing.");

   _ec_free(ec);

   if (_log_domain >= 0)
     {
	eina_log_domain_unregister(_log_domain);
	_log_domain = -1;
     }
   ethumb_shutdown();

   return 0;
}
