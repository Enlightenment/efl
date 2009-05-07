#include "evas_cs.h"

#ifdef EVAS_CSERVE

EAPI Server *
evas_cserve_server_add(void)
{
   Server *s;
   char buf[PATH_MAX];
   struct sockaddr_un socket_unix;
   struct linger lin;
   mode_t pmode;
   int socket_unix_len;
   
   s = calloc(1, sizeof(Server));
   if (!s) return NULL;
   s->fd = -1;
   snprintf(buf, sizeof(buf), "/tmp/.evas-cserve-%x", getuid());
   s->socket_path = strdup(buf);
   if (!s->socket_path)
     {
        free(s);
        return NULL;
     }
   pmode = umask(~(S_IRUSR | S_IWUSR));
   start:
   s->fd = socket(AF_UNIX, SOCK_STREAM, 0);
   if (s->fd < 0) goto error;
   if (fcntl(s->fd, F_SETFL, O_NONBLOCK) < 0) goto error;
   if (fcntl(s->fd, F_SETFD, FD_CLOEXEC) < 0) goto error;
   lin.l_onoff = 1;
   lin.l_linger = 0;
   if (setsockopt(s->fd, SOL_SOCKET, SO_LINGER, &lin, sizeof(struct linger)) < 0)
     goto error;
   socket_unix.sun_family = AF_UNIX;
   strncpy(socket_unix.sun_path, buf, sizeof(socket_unix.sun_path));
   socket_unix_len = LENGTH_OF_SOCKADDR_UN(&socket_unix);
   if (bind(s->fd, (struct sockaddr *)&socket_unix, socket_unix_len) < 0)
     {
        if ((connect(s->fd, (struct sockaddr *)&socket_unix, socket_unix_len) < 0) &&
            (unlink(s->socket_path) >= 0))
          {
             close(s->fd);
             goto start;
          }
        else
          goto error;
     }
   if (listen(s->fd, 4096) < 0) goto error;
   umask(pmode);
   return s;
   error:
   umask(pmode);
   if (s->fd >= 0) close(s->fd);
   free(s->socket_path);
   free(s);
   return NULL;
}

EAPI void
evas_cserve_server_del(Server *s)
{
   Client *c;
   
   EINA_LIST_FREE(s->clients, c)
     {
        close(c->fd);
        if (c->buf) free(c->buf);
        if (c->inbuf) free(c->inbuf);
        free(c);
     }
   close(s->fd);
   unlink(s->socket_path);
   free(s->socket_path);
   free(s);
}

static void
server_accept(Server *s)
{
   Client *c;
   int new_fd;
   struct sockaddr_in incoming;
   size_t size_in;
   
   size_in = sizeof(struct sockaddr_in);
   new_fd = accept(s->fd, (struct sockaddr *)&incoming, (socklen_t *)&size_in);
   if (new_fd < 0) return;
   fcntl(new_fd, F_SETFL, O_NONBLOCK);
   fcntl(new_fd, F_SETFD, FD_CLOEXEC);
   c = calloc(1, sizeof(Client));
   if (!c)
     {
        close(new_fd);
        return;
     }
   c->server = s;
   c->fd = new_fd;
   s->clients = eina_list_append(s->clients, c);
}

static void
client_flush(Client *c)
{
   int num;
   
   num = write(c->fd, c->buf, c->bufsize);
   if (num < 0)
     {
        c->dead = 1;
        return;
     }
   if (num < c->bufsize)
     {
        unsigned char *buf;
        
        buf = malloc(c->bufsize - num);
        if (buf)
          {
             memcpy(buf, c->buf + num, c->bufsize - num);
             free(c->buf);
             c->bufsize = c->bufsize - num;
             c->bufalloc = c->bufsize;
             c->buf = buf;
          }
     }
   else
     {
        free(c->buf);
        c->buf = NULL;
        c->bufsize = 0;
        c->bufalloc = 0;
     }
}

static void
client_buf_add(Client *c, unsigned char *data, int size)
{
   int newsize;
   unsigned char *buf;

   newsize = c->bufsize + size;
   if (newsize > c->bufalloc)
     {
        c->bufalloc = newsize + 16384;
        buf = realloc(c->buf, c->bufalloc);
        if (buf) c->buf = buf;
        else return;
     }
   memcpy(c->buf + c->bufsize, data, size);
   c->bufsize += size;
}

static void
client_write(Client *c, unsigned char *data, int size)
{
   int num;
   
   if (!c->buf)
     {
        num = write(c->fd, data, size);
        if (num != size)
          client_buf_add(c, data + num, size - num);
     }
   else
     {
        client_buf_add(c, data, size);
     }
}

EAPI void
evas_cserve_client_send(Client *c, int opcode, int size, unsigned char *data)
{
   unsigned char *data2;
   int *ints;
   
   data2 = malloc(size + (sizeof(int) * 2));
   if (!data2) return;
   ints = (int *)data2;
   ints[0] = size;
   ints[1] = opcode;
   memcpy(data2 + (sizeof(int) * 2), data, size);
   client_write(c, data2, size + (sizeof(int) * 2));
   free(data2);
}

static void
server_message_handle(Server *s, Client *c, int opcode, int size, unsigned char *data)
{
   if (s->func) s->func(s->data, s, c, opcode, size, data);
}

EAPI void
evas_cserve_server_message_handler_set(Server *s, int (*func) (void *fdata, Server *s, Client *c, int opcode, int size, unsigned char *data), void *data)
{
   s->func = func;
   s->data = data;
}

static int
server_parse(Server *s, Client *c)
{
   int *ints;
   unsigned char *data, *newbuf;
   
   if (c->inbufsize < sizeof(int)) return;
   ints = (int *)((c->inbuf));
   if ((ints[0] < 0) || (ints[0] > (1024 * 1024)))
     return;
   if (c->inbufsize < (ints[0] + (sizeof(int) * 2)))
     {
        return 0;
     }
   data = c->inbuf + (sizeof(int) * 2);
   server_message_handle(s, c, ints[1], ints[0], data);
   c->inbufalloc -= ints[0] + (sizeof(int) * 2);
   if (c->inbufalloc == 0)
     {
        free(c->inbuf);
        c->inbuf = NULL;
        c->inbufsize = 0;
        return 0;
     }
   newbuf = malloc(c->inbufalloc);
   if (!newbuf)
     {
        c->inbufalloc += ints[0] + (sizeof(int) * 2);
        /* fixme - bad situation */
        return 0;
     }
   memcpy(newbuf, c->inbuf + ints[0] + (sizeof(int) * 2), c->inbufalloc);
   c->inbufsize -= ints[0] + (sizeof(int) * 2);
   free(c->inbuf);
   c->inbuf = newbuf;
   return 1;
}

static void
server_data(Server *s, Client *c, unsigned char *data, int size)
{
   if (!c->inbuf)
     {
        c->inbuf = malloc(size);
        if (c->inbuf)
          {
             memcpy(c->inbuf, data, size);
             c->inbufalloc = size;
             c->inbufsize = size;
          }
        else
          {
             /* fixme - bad situation */
             return;
          }
     }
   else
     {
        int size2;
        
        size2 = c->inbufsize + size;
        if (size2 > c->inbufalloc)
          {
             unsigned char *newbuf;
             
             c->inbufalloc = size2;
             newbuf = realloc(c->inbuf, c->inbufalloc);
             if (newbuf) c->inbuf = newbuf;
             else size2 = 0;
          }
        if (size2 > 0)
          {
             memcpy(c->inbuf + c->inbufsize, data, size);
             c->inbufsize = size2;
          }
        else
          {
             /* fixme - bad situation */
             return;
          }
     }
   while (server_parse(s, c));
}

EAPI void
evas_cserve_server_wait(Server *s, int timeout)
{
   fd_set rset, wset, xset;
   int maxfd;
   int ret;
   struct timeval to;
   Eina_List *l, *dead = NULL;
   Client *c;
   
   maxfd = 0;
   FD_ZERO(&rset);
   FD_ZERO(&wset);
   FD_ZERO(&xset);
   FD_SET(s->fd, &rset);
   if (s->fd > maxfd) maxfd = s->fd;
   EINA_LIST_FOREACH(s->clients, l, c)
     {
        FD_SET(c->fd, &rset);
        if (c->buf)
          FD_SET(c->fd, &wset);
        if (c->fd > maxfd) maxfd = c->fd;
     }
   if (timeout > 0)
     {
        to.tv_sec = timeout / 1000000;
        to.tv_usec = timeout % 1000000;
        ret = select(maxfd + 1, &rset, &wset, &xset, &to);
     }
   else
     ret = select(maxfd + 1, &rset, &wset, &xset, NULL);
   if (ret < 1) return;
   
   EINA_LIST_FOREACH(s->clients, l, c)
     {
        if (c->dead) continue;
        if (FD_ISSET(c->fd, &rset))
          {
             char buf[16384];
             int num;
             
             errno = 0;
             num = read(c->fd, buf, sizeof(buf));
             if (num <= 0)
               {
                  c->dead = 1;
                  dead = eina_list_append(dead, c);
               }
             else if (num > 0)
               {
                  server_data(s, c, buf, num);
               }
          }
        else if (FD_ISSET(c->fd, &wset))
          {
             client_flush(c);
             if (c->dead) dead = eina_list_append(dead, c);
          }
     }
   if (FD_ISSET(s->fd, &rset))
     {
        server_accept(s);
     }
   EINA_LIST_FREE(dead, c)
     {
        if (c->func) c->func(c->data, c);
        s->clients = eina_list_remove(s->clients, c);
        close(c->fd);
        if (c->buf) free(c->buf);
        if (c->inbuf) free(c->inbuf);
        free(c);
     }
}

#endif
