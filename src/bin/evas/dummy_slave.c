#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

#include "evas_cserve2.h"

static Eina_Bool
command_read(int fd, Slave_Command *cmd, void **params)
{
   ssize_t ret;
   int ints[2], size, got = 0;
   char *buf;

   ret = read(fd, ints, sizeof(int) * 2);
   if (ret < (int)sizeof(int) * 2)
     return EINA_FALSE;

   if(!((ints[0] > 0) && (ints[0] <= 0xFFFF) &&
        (ints[1] >= 0) && (ints[1] < SLAVE_COMMAND_LAST)))
     return EINA_FALSE;

   size = ints[0];
   buf = malloc(size);
   if (!buf) return EINA_FALSE;

   do {
        ret = read(fd, buf + got, size - got);
        if (ret < 0)
          {
             /* EINTR means we were interrupted by a signal before anything
              * was sent, and if we are back here it means that signal was
              * not meant for us to die. Any other error here is fatal and
              * should result in the slave terminating.
              */
             if (errno == EINTR)
               continue;
             free(buf);
             return EINA_FALSE;
          }
        got += ret;
   } while (got < size);

   *cmd = ints[1];
   *params = buf;

   return EINA_TRUE;
}

static Eina_Bool
response_send(int fd, Slave_Command cmd, void *resp, int size)
{
   int sent = 0, ints[2];
   const char *data = resp;
   ssize_t ret;

   ints[0] = size;
   ints[1] = cmd;
   ret = write(fd, ints, sizeof(int) * 2);
   if (ret < 0)
     return EINA_FALSE;
   if (!size)
     return EINA_TRUE;
   do {
        ret = write(fd, data + sent, size - sent);
        if (ret < 0)
          {
             /* EINTR means we were interrupted by a signal before anything
              * was sent, and if we are back here it means that signal was
              * not meant for us to die. Any other error here is fatal and
              * should result in the slave terminating.
              */
             if (errno == EINTR)
               continue;
             return EINA_FALSE;
          }
        sent += ret;
   } while (sent < size);

   return EINA_TRUE;
}

static Eina_Bool
error_send(int fd, Error_Type err)
{
   return response_send(fd, ERROR, &err, sizeof(Error_Type));
}

static void *
_cserve2_shm_map(const char *name, size_t length, off_t offset)
{
   void *map;
   int fd;

   fd = shm_open(name, O_RDWR, 0);
   if (fd == -1)
     return MAP_FAILED;

   map = mmap(NULL, length, PROT_WRITE, MAP_SHARED, fd, offset);

   close(fd);

   return map;
}

static void
_cserve2_shm_unmap(void *map, size_t length)
{
   munmap(map, length);
}

static Error_Type
image_open(const char *file EINA_UNUSED, const char *key EINA_UNUSED, Slave_Msg_Image_Opened *result)
{
   memset(result, 0, sizeof(*result));
   result->w = 32;
   result->h = 32;
   result->frame_count = 1;
   result->loop_count = 0;
   result->loop_hint = 0;
   result->alpha = EINA_TRUE;
   return CSERVE2_NONE;
}

static Error_Type
image_load(const char *shmfile, Slave_Msg_Image_Load *params)
{
   char *map = _cserve2_shm_map(shmfile, params->shm.mmap_size,
                                params->shm.mmap_offset);
   if (map == MAP_FAILED)
     return CSERVE2_RESOURCE_ALLOCATION_FAILED;

   memset(map + params->shm.image_offset, 'A', params->shm.image_size);
   _cserve2_shm_unmap(map, params->shm.mmap_size);

   return CSERVE2_NONE;
}

int main(int c, char **v)
{
   int wfd, rfd;
   Slave_Command cmd;
   void *params = NULL;
   Eina_Bool quit = EINA_FALSE;

   if (c < 3)
     return 1;

   wfd = atoi(v[1]);
   rfd = atoi(v[2]);

   while (!quit)
     {
        if (!command_read(rfd, &cmd, &params))
          {
             error_send(wfd, CSERVE2_INVALID_COMMAND);
             continue;
          }

        switch (cmd)
          {
           case IMAGE_OPEN:
                {
                   Slave_Msg_Image_Opened result;
                   Slave_Msg_Image_Open *p;
                   Error_Type err;
                   const char *file, *key;
                   p = params;
                   file = (const char *)(p + 1);
                   key = file + strlen(file) + 1;
                   if ((err = image_open(file, key, &result)) != CSERVE2_NONE)
                     error_send(wfd, err);
                   else
                     response_send(wfd, IMAGE_OPEN, &result,
                                   sizeof(Slave_Msg_Image_Opened));
                   break;
                }
           case IMAGE_LOAD:
                {
                   Slave_Msg_Image_Load *load_args = params;
                   Error_Type err;
                   const char *shmfile = ((const char *)params) +
                      sizeof(Slave_Msg_Image_Load);
                   if ((err = image_load(shmfile, load_args)) != CSERVE2_NONE)
                     error_send(wfd, err);
                   else
                     response_send(wfd, IMAGE_LOAD, NULL, 0);
                   break;
                }
           case SLAVE_QUIT:
                {
                   quit = EINA_TRUE;
                   break;
                }

           default:
              error_send(wfd, CSERVE2_INVALID_COMMAND);
          }
        free(params);
     }

   return 0;
}
