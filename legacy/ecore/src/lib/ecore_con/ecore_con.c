#include "Ecore.h"
#include "ecore_private.h"
#include "ecore_con_private.h"
#include "Ecore_Con.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <netdb.h>

static void _ecore_con_server_free(Ecore_Con_Server *svr);
static void _ecore_con_client_free(Ecore_Con_Client *cl);
static int _ecore_con_svr_handler(void *data, Ecore_Fd_Handler *fd_handler);
static int _ecore_con_cl_handler(void *data, Ecore_Fd_Handler *fd_handler);
static int _ecore_con_svr_cl_handler(void *data, Ecore_Fd_Handler *fd_handler);
static void _ecore_con_server_flush(Ecore_Con_Server *svr);
static void _ecore_con_client_flush(Ecore_Con_Client *cl);
static void _ecore_con_event_client_add_free(void *data, void *ev);
static void _ecore_con_event_client_del_free(void *data, void *ev);
static void _ecore_con_event_server_add_free(void *data, void *ev);
static void _ecore_con_event_server_del_free(void *data, void *ev);
static void _ecore_con_event_client_data_free(void *data, void *ev);
static void _ecore_con_event_server_data_free(void *data, void *ev);

int ECORE_CON_EVENT_CLIENT_ADD = 0;
int ECORE_CON_EVENT_CLIENT_DEL = 0;
int ECORE_CON_EVENT_SERVER_ADD = 0;
int ECORE_CON_EVENT_SERVER_DEL = 0;
int ECORE_CON_EVENT_CLIENT_DATA = 0;
int ECORE_CON_EVENT_SERVER_DATA = 0;

static Ecore_Con_Server *servers = NULL;
static int init_count = 0;

#define LENGTH_OF_SOCKADDR_UN(s) (strlen((s)->sun_path) + (size_t)(((struct sockaddr_un *)NULL)->sun_path))

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 * <hr><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br>
 */
int
ecore_con_init(void)
{
   init_count++;
   if (!ECORE_CON_EVENT_CLIENT_ADD)
     {
	ECORE_CON_EVENT_CLIENT_ADD = ecore_event_type_new();
	ECORE_CON_EVENT_CLIENT_DEL = ecore_event_type_new();
	ECORE_CON_EVENT_SERVER_ADD = ecore_event_type_new();
	ECORE_CON_EVENT_SERVER_DEL = ecore_event_type_new();
	ECORE_CON_EVENT_CLIENT_DATA = ecore_event_type_new();
	ECORE_CON_EVENT_SERVER_DATA = ecore_event_type_new();
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
ecore_con_shutdown(void)
{
   if (init_count > 0)
     {
	init_count--;
	if (init_count > 0) return init_count;
	while (servers) _ecore_con_server_free(servers);
     }
   return 0;
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 * <hr><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br>
 */
Ecore_Con_Server *
ecore_con_server_add(Ecore_Con_Type type,
		     char *name,
		     int port,
		     const void *data)
{
   Ecore_Con_Server   *svr;
   struct sockaddr_in  socket_addr;
   struct sockaddr_un  socket_unix;
   struct linger       lin;
   char                buf[4096];
   
   if (port < 0) return NULL;
   /* local  user   socket: FILE:   ~/.ecore/[name]/[port] */
   /* local  system socket: FILE:   /tmp/.ecore_service|[name]|[port] */
   /* remote system socket: TCP/IP: [name]:[port] */
   svr = calloc(1, sizeof(Ecore_Con_Server));
   if (!svr) return NULL;

   if ((type == ECORE_CON_LOCAL_USER) ||
       (type == ECORE_CON_LOCAL_SYSTEM))
     {
	char *homedir;
	struct stat st;
	mode_t pmode, mask;
	
	if (!name) goto error;
	mask =
	  S_IRGRP | S_IWGRP | S_IXGRP |
	  S_IROTH | S_IWOTH | S_IXOTH;
	if (type == ECORE_CON_LOCAL_USER)
	  {
	     homedir = getenv("HOME");
	     if (!homedir) homedir = getenv("TMP");
	     if (!homedir) homedir = "/tmp";
	     mask = S_IRUSR | S_IWUSR | S_IXUSR;
	     snprintf(buf, sizeof(buf), "%s/.ecore", homedir);
	     if (stat(buf, &st) < 0) mkdir(buf, mask);
	     snprintf(buf, sizeof(buf), "%s/.ecore/%s", homedir, name);
	     if (stat(buf, &st) < 0) mkdir(buf, mask);
	     snprintf(buf, sizeof(buf), "%s/.ecore/%s/%i", homedir, name, port);
	     mask =
	       S_IRGRP | S_IWGRP | S_IXGRP |
	       S_IROTH | S_IWOTH | S_IXOTH;
	  }
	else if (type == ECORE_CON_LOCAL_SYSTEM)
	  {
	     mask = 0;
	     snprintf(buf, sizeof(buf), "/tmp/.ecore_service|%s|%i", name, port);
	  }
	pmode = umask(mask);
	svr->fd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (svr->fd < 0)
	  {
	     umask(pmode);
	     goto error;
	  }
	if (fcntl(svr->fd, F_SETFL, O_NONBLOCK) < 0)
	  {
	     umask(pmode);	     
	     goto error;
	  }
	if (fcntl(svr->fd, F_SETFD, FD_CLOEXEC) < 0)
	  {
	     umask(pmode);	     
	     goto error;
	  }
	lin.l_onoff = 1;
	lin.l_linger = 100;
	if (setsockopt(svr->fd, SOL_SOCKET, SO_LINGER, &lin, sizeof(struct linger)) < 0)
	  {
	     umask(pmode);	     
	     goto error;
	  }
	socket_unix.sun_family = AF_UNIX;
	strncpy(socket_unix.sun_path, buf, sizeof(socket_unix.sun_path));
	if (bind(svr->fd, (struct sockaddr *)&socket_unix, LENGTH_OF_SOCKADDR_UN(&socket_unix)) < 0)
	  {
	     umask(pmode);	     
	     goto error;
	  }
	if (listen(svr->fd, 4096) < 0)
	  {
	     umask(pmode);	     
	     goto error;
	  }
	svr->path = strdup(buf);
	if (!svr->path)
	  {
	     umask(pmode);	     
	     goto error;
	  }
	svr->fd_handler = ecore_main_fd_handler_add(svr->fd,
						    ECORE_FD_READ,
						    _ecore_con_svr_handler, svr,
						    NULL, NULL);
	umask(pmode);
	if (!svr->fd_handler) goto error;
     }
   else if (type == ECORE_CON_REMOTE_SYSTEM)
     {
	svr->fd = socket(AF_INET, SOCK_STREAM, 0);
	if (svr->fd < 0) goto error;
	if (fcntl(svr->fd, F_SETFL, O_NONBLOCK) < 0) goto error;
	if (fcntl(svr->fd, F_SETFD, FD_CLOEXEC) < 0) goto error;
	lin.l_onoff = 1;
	lin.l_linger = 100;
	if (setsockopt(svr->fd, SOL_SOCKET, SO_LINGER, &lin, sizeof(struct linger)) < 0) goto error;
	socket_addr.sin_family = AF_INET;
	socket_addr.sin_port = htons(port);
	socket_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	if (bind(svr->fd, (struct sockaddr *)&socket_addr, sizeof(struct sockaddr_in)) < 0) goto error;
	if (listen(svr->fd, 4096) < 0) goto error;
	svr->fd_handler = ecore_main_fd_handler_add(svr->fd,
						    ECORE_FD_READ,
						    _ecore_con_svr_handler, svr,
						    NULL, NULL);
	if (!svr->fd_handler) goto error;
     }
   
   svr->name = strdup(name);
   if (!svr->name) goto error;
   svr->type = type;
   svr->port = port;
   svr->data = (void *)data;
   svr->created = 1;
   servers = _ecore_list_append(servers, svr);
   ECORE_MAGIC_SET(svr, ECORE_MAGIC_CON_SERVER);   
   return svr;
   
   error:
   if (svr->name) free(svr->name);
   if (svr->path) free(svr->path);
   if (svr->fd >= 0) close(svr->fd);
   if (svr->fd_handler) ecore_main_fd_handler_del(svr->fd_handler);
   if (svr->buf) free(svr->buf);
   free(svr);
   return NULL;
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 * <hr><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br>
 */
Ecore_Con_Server *
ecore_con_server_connect(Ecore_Con_Type type,
			 char *name,
			 int port,
			 const void *data)
{
   Ecore_Con_Server   *svr;
   struct sockaddr_un  socket_unix;
   struct sockaddr_in  socket_addr;
   int                 curstate;
   char                buf[4096];

   if (!name) return NULL;
   if (port < 0) return NULL;
   /* local  user   socket: FILE:   ~/.ecore/[name]/[port] */
   /* local  system socket: FILE:   /tmp/.ecore_service|[name]|[port] */
   /* remote system socket: TCP/IP: [name]:[port] */
   svr = calloc(1, sizeof(Ecore_Con_Server));
   if (!svr) return NULL;
   
   if ((type == ECORE_CON_LOCAL_USER) ||
       (type == ECORE_CON_LOCAL_SYSTEM))
     {
	char *homedir;
	
	if (type == ECORE_CON_LOCAL_USER)
	  {
	     homedir = getenv("HOME");
	     if (!homedir) homedir = getenv("TMP");
	     if (!homedir) homedir = "/tmp";
	     snprintf(buf, sizeof(buf), "%s/.ecore/%s/%i", homedir, name, port);
	  }
	else if (type == ECORE_CON_LOCAL_SYSTEM)
	  snprintf(buf, sizeof(buf), "/tmp/.ecore_service|%s|%i", name, port);
	svr->fd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (svr->fd < 0) goto error;
	if (fcntl(svr->fd, F_SETFL, O_NONBLOCK) < 0) goto error;
	if (fcntl(svr->fd, F_SETFD, FD_CLOEXEC) < 0) goto error;
	if (setsockopt(svr->fd, SOL_SOCKET, SO_REUSEADDR, &curstate, sizeof(curstate)) < 0) goto error;
	socket_unix.sun_family = AF_UNIX;
	strncpy(socket_unix.sun_path, buf, sizeof(socket_unix.sun_path));
	if (connect(svr->fd, (struct sockaddr *)&socket_unix, LENGTH_OF_SOCKADDR_UN(&socket_unix)) < 0) goto error;
	svr->path = strdup(buf);
	if (!svr->path) goto error;
	svr->fd_handler = ecore_main_fd_handler_add(svr->fd,
						    ECORE_FD_READ,
						    _ecore_con_cl_handler, svr,
						    NULL, NULL);
	if (!svr->fd_handler) goto error;
	  {
	     /* we got our server! */
	     Ecore_Con_Event_Server_Add *e;
	     
	     e = calloc(1, sizeof(Ecore_Con_Event_Server_Add));
	     if (e)
	       {
		  e->server = svr;
		  ecore_event_add(ECORE_CON_EVENT_SERVER_ADD, e,
				  _ecore_con_event_server_add_free, NULL);
	       }
	  }
     }
   else if (type == ECORE_CON_REMOTE_SYSTEM)
     {
	struct hostent *he;
	
	/* FIXME: gethostbyname is blocking... */
	if (!(he = gethostbyname(name))) goto error;
	svr->fd = socket(AF_INET, SOCK_STREAM, 0);
	if (svr->fd < 0) goto error;
	if (fcntl(svr->fd, F_SETFL, O_NONBLOCK) < 0) goto error;
	if (fcntl(svr->fd, F_SETFD, FD_CLOEXEC) < 0) goto error;
	if (setsockopt(svr->fd, SOL_SOCKET, SO_REUSEADDR, &curstate, sizeof(curstate)) < 0) goto error;
	socket_addr.sin_family = AF_INET;
	socket_addr.sin_port = htons(port);
	memcpy((struct in_addr *)&socket_addr.sin_addr, 
	       he->h_addr, sizeof(struct in_addr));
	if (connect(svr->fd, (struct sockaddr *)&socket_addr, sizeof(struct sockaddr_in)) < 0) 
	  {
	     if (errno != EINPROGRESS)
	       goto error;
	     svr->connecting = 1;
	     svr->fd_handler = ecore_main_fd_handler_add(svr->fd,
							 ECORE_FD_READ | ECORE_FD_WRITE,
							 _ecore_con_cl_handler, svr,
							 NULL, NULL);
	  }
	else
	  svr->fd_handler = ecore_main_fd_handler_add(svr->fd,
						      ECORE_FD_READ,
						      _ecore_con_cl_handler, svr,
						      NULL, NULL);
	if (!svr->fd_handler) goto error;
     }
   
   svr->name = strdup(name);
   if (!svr->name) goto error;
   svr->type = type;
   svr->port = port;
   svr->data = (void *)data;
   svr->created = 0;
   servers = _ecore_list_append(servers, svr);
   ECORE_MAGIC_SET(svr, ECORE_MAGIC_CON_SERVER);   
   return svr;
   
   error:
   if (svr->name) free(svr->name);
   if (svr->path) free(svr->path);
   if (svr->fd >= 0) close(svr->fd);
   if (svr->fd_handler) ecore_main_fd_handler_del(svr->fd_handler);
   free(svr);
   return NULL;
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 * <hr><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br>
 */
void *
ecore_con_server_del(Ecore_Con_Server *svr)
{
   void *data;

   if (!ECORE_MAGIC_CHECK(svr, ECORE_MAGIC_CON_SERVER))
     {
	ECORE_MAGIC_FAIL(svr, ECORE_MAGIC_CON_SERVER,
			 "ecore_con_server_del");
	return NULL;
     }   
   data = svr->data;
   _ecore_con_server_free(svr);
   return data;
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 * <hr><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br>
 */
void *
ecore_con_server_data_get(Ecore_Con_Server *svr)
{
   if (!ECORE_MAGIC_CHECK(svr, ECORE_MAGIC_CON_SERVER))
     {
	ECORE_MAGIC_FAIL(svr, ECORE_MAGIC_CON_SERVER,
			 "ecore_con_server_data_get");
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
ecore_con_server_connected_get(Ecore_Con_Server *svr)
{
   if (!ECORE_MAGIC_CHECK(svr, ECORE_MAGIC_CON_SERVER))
     {
	ECORE_MAGIC_FAIL(svr, ECORE_MAGIC_CON_SERVER,
			 "ecore_con_server_connected_get");
	return 0;
     }   
   if (svr->connecting) return 0;
   return 1;
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 * <hr><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br>
 */
int
ecore_con_server_send(Ecore_Con_Server *svr, void *data, int size)
{
   if (!ECORE_MAGIC_CHECK(svr, ECORE_MAGIC_CON_SERVER))
     {
	ECORE_MAGIC_FAIL(svr, ECORE_MAGIC_CON_SERVER,
			 "ecore_con_server_send");
	return 0;
     }   
   if (svr->dead) return 1;
   if (!data) return 0;
   if (size < 1) return 0;
   ecore_main_fd_handler_active_set(svr->fd_handler, ECORE_FD_READ | ECORE_FD_WRITE);
   if (svr->buf)
     {
	unsigned char *newbuf;
	
	newbuf = realloc(svr->buf, svr->buf_size + size);
	if (newbuf) svr->buf = newbuf;
	else return 0;
	memcpy(svr->buf + svr->buf_size, data, size);
	svr->buf_size += size;
     }
   else
     {
	svr->buf = malloc(size);
	if (!svr->buf) return 0;
	svr->buf_size = size;
	memcpy(svr->buf, data, size);
     }
   _ecore_con_server_flush(svr);
   return 1;
}
  
/**
 * To be documented.
 *
 * FIXME: To be fixed.
 * <hr><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br>
 */
int
ecore_con_client_send(Ecore_Con_Client *cl, void *data, int size)
{
   if (!ECORE_MAGIC_CHECK(cl, ECORE_MAGIC_CON_CLIENT))
     {
	ECORE_MAGIC_FAIL(cl, ECORE_MAGIC_CON_CLIENT,
			 "ecore_con_client_send");
	return 0;
     }   
   if (cl->dead) return 0;
   if (!data) return 0;
   if (size < 1) return 0;
   ecore_main_fd_handler_active_set(cl->fd_handler, ECORE_FD_READ | ECORE_FD_WRITE);
   if (cl->buf)
     {
	unsigned char *newbuf;
	
	newbuf = realloc(cl->buf, cl->buf_size + size);
	if (newbuf) cl->buf = newbuf;
	else return 0;
	memcpy(cl->buf + cl->buf_size, data, size);
	cl->buf_size += size;
     }
   else
     {
	cl->buf = malloc(size);
	if (!cl->buf) return 0;
	cl->buf_size = size;
	memcpy(cl->buf, data, size);
     }
   _ecore_con_client_flush(cl);
   return 1;
}
  
/**
 * To be documented.
 *
 * FIXME: To be fixed.
 * <hr><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br>
 */
Ecore_Con_Server *
ecore_con_client_server_get(Ecore_Con_Client *cl)
{
   if (!ECORE_MAGIC_CHECK(cl, ECORE_MAGIC_CON_CLIENT))
     {
	ECORE_MAGIC_FAIL(cl, ECORE_MAGIC_CON_CLIENT,
			 "ecore_con_client_server_get");
	return NULL;
     }   
   return cl->server;
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 * <hr><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br>
 */
void *
ecore_con_client_del(Ecore_Con_Client *cl)
{
   void *data;
   
   if (!ECORE_MAGIC_CHECK(cl, ECORE_MAGIC_CON_CLIENT))
     {
	ECORE_MAGIC_FAIL(cl, ECORE_MAGIC_CON_CLIENT,
			 "ecore_con_client_del");
	return NULL;
     }   
   data = cl->data;
   _ecore_con_client_free(cl);
   return cl->data;
}

/**
 * To be documented.
 *
 * FIXME: To be fixed.
 * <hr><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br><br>
 */
void
ecore_con_client_data_set(Ecore_Con_Client *cl, const void *data)
{
   if (!ECORE_MAGIC_CHECK(cl, ECORE_MAGIC_CON_CLIENT))
     {
	ECORE_MAGIC_FAIL(cl, ECORE_MAGIC_CON_CLIENT,
			 "ecore_con_client_data_set");
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
ecore_con_client_data_get(Ecore_Con_Client *cl)
{
   if (!ECORE_MAGIC_CHECK(cl, ECORE_MAGIC_CON_CLIENT))
     {
	ECORE_MAGIC_FAIL(cl, ECORE_MAGIC_CON_CLIENT,
			 "ecore_con_client_data_get");
	return NULL;
     }
   return cl->data;
}

static void
_ecore_con_server_free(Ecore_Con_Server *svr)
{
   ECORE_MAGIC_SET(svr, ECORE_MAGIC_NONE);   
   while ((svr->buf) && (!svr->dead)) _ecore_con_server_flush(svr);
   if (svr->buf) free(svr->buf);
   servers = _ecore_list_remove(servers, svr);
   while (svr->clients)
     _ecore_con_client_free((Ecore_Con_Client *)svr->clients);
   if ((svr->created) && (svr->path)) unlink(svr->path);
   if (svr->fd >= 0) close(svr->fd);
   if (svr->name) free(svr->name);
   if (svr->path) free(svr->path);
   if (svr->fd_handler) ecore_main_fd_handler_del(svr->fd_handler);
   free(svr);
}

static void
_ecore_con_client_free(Ecore_Con_Client *cl)
{
   ECORE_MAGIC_SET(cl, ECORE_MAGIC_NONE);   
   while ((cl->buf) && (!cl->dead)) _ecore_con_client_flush(cl);
   if (cl->buf) free(cl->buf);
   cl->server->clients = _ecore_list_remove(cl->server->clients, cl);
   if (cl->fd >= 0) close(cl->fd);
   if (cl->fd_handler) ecore_main_fd_handler_del(cl->fd_handler);
   free(cl);
}

static int
_ecore_con_svr_handler(void *data, Ecore_Fd_Handler *fd_handler)
{
   Ecore_Con_Server   *svr;
   int                 new_fd;
   struct sockaddr_in  incoming;
   size_t              size_in;
   
   svr = data;
   if (svr->dead) return 1;
   /* a new client */
   size_in = sizeof(struct sockaddr_in);
   new_fd = accept(svr->fd, (struct sockaddr *)&incoming, &size_in);
   if (new_fd >= 0)
     {
	Ecore_Con_Client *cl;
	
	cl = calloc(1, sizeof(Ecore_Con_Client));
	if (!cl)
	  {
	     close(new_fd);
	     return 1;
	  }
	fcntl(new_fd, F_SETFL, O_NONBLOCK);
	fcntl(new_fd, F_SETFD, FD_CLOEXEC);
	cl->fd = new_fd;
	cl->server = svr;
	cl->fd_handler = ecore_main_fd_handler_add(cl->fd,
						   ECORE_FD_READ,
						   _ecore_con_svr_cl_handler, cl,
						   NULL, NULL);
	ECORE_MAGIC_SET(cl, ECORE_MAGIC_CON_CLIENT);
	svr->clients = _ecore_list_append(svr->clients, cl);
	  {
	     Ecore_Con_Event_Client_Add *e;
	     
	     e = calloc(1, sizeof(Ecore_Con_Event_Client_Add));
	     if (e)
	       {
		  e->client = cl;
		  ecore_event_add(ECORE_CON_EVENT_CLIENT_ADD, e,
				  _ecore_con_event_client_add_free, NULL);
	       }
	  }
     }
   return 1;
}

static int
_ecore_con_cl_handler(void *data, Ecore_Fd_Handler *fd_handler)
{
   Ecore_Con_Server   *svr;
   
   svr = data;
   if (svr->dead) return 1;
   if (ecore_main_fd_handler_active_get(fd_handler, ECORE_FD_READ))
     {
	unsigned char *inbuf = NULL;
	int            inbuf_num = 0;
	
	for (;;)
	  {
	     char buf[65536];
	     int num;
	     
	     num = read(svr->fd, buf, 65536);
	     if (num < 1)
	       {
		  if (inbuf) 
		    {
		       Ecore_Con_Event_Server_Data *e;
		       
		       e = calloc(1, sizeof(Ecore_Con_Event_Server_Data));
		       if (e)
			 {
			    e->server = svr;
			    e->data = inbuf;
			    e->size = inbuf_num;
			    ecore_event_add(ECORE_CON_EVENT_SERVER_DATA, e,
					    _ecore_con_event_server_data_free, NULL);
			 }
		    }
		  if ((errno == EIO) ||  (errno == EBADF) || 
		      (errno == EPIPE) || (errno == EINVAL) || 
		      (errno == ENOSPC) || (num == 0)/* is num == 0 right? */)
		    {
		       /* we lost our server! */
		       Ecore_Con_Event_Server_Del *e;
		       
		       e = calloc(1, sizeof(Ecore_Con_Event_Server_Del));
		       if (e)
			 {
			    e->server = svr;
			    ecore_event_add(ECORE_CON_EVENT_SERVER_DEL, e,
					    _ecore_con_event_server_del_free, NULL);
			 }
		       svr->dead = 1;
		       ecore_main_fd_handler_del(svr->fd_handler);
		       svr->fd_handler = NULL;
		    }
		  break;
	       }
	     else
	       {
		  inbuf = realloc(inbuf, inbuf_num + num);
		  memcpy(inbuf + inbuf_num, buf, num);
		  inbuf_num += num;
	       }
	  }
     }
   else if (ecore_main_fd_handler_active_get(fd_handler, ECORE_FD_WRITE))
     {
	if (svr->connecting)
	  {
	     int so_err;
	     socklen_t size;
	     
	     svr->connecting = 0;
	     so_err = 0;
	     size = sizeof(int);
	     if (getsockopt(svr->fd, SOL_SOCKET, SO_ERROR, &so_err, &size) < 0) so_err = -1;
	     if (so_err != 0)
	       {
		  /* we lost our server! */
		  Ecore_Con_Event_Server_Del *e;
		  
		  e = calloc(1, sizeof(Ecore_Con_Event_Server_Del));
		  if (e)
		    {
		       e->server = svr;
		       ecore_event_add(ECORE_CON_EVENT_SERVER_DEL, e,
				       _ecore_con_event_server_del_free, NULL);
		    }
		  svr->dead = 1;
		  ecore_main_fd_handler_del(svr->fd_handler);
		  svr->fd_handler = NULL;
		  ecore_main_fd_handler_active_set(svr->fd_handler, ECORE_FD_READ);
		  return 1;
	       }
	     else
	       {
		    {
		       /* we got our server! */
		       Ecore_Con_Event_Server_Add *e;
		       
		       e = calloc(1, sizeof(Ecore_Con_Event_Server_Add));
		       if (e)
			 {
			    e->server = svr;
			    ecore_event_add(ECORE_CON_EVENT_SERVER_ADD, e,
					    _ecore_con_event_server_add_free, NULL);
			 }
		    }
		  if (!svr->buf)
		    ecore_main_fd_handler_active_set(svr->fd_handler, ECORE_FD_READ);
	       }
	  }
	_ecore_con_server_flush(svr);
     }
   return 1;
}

static int
_ecore_con_svr_cl_handler(void *data, Ecore_Fd_Handler *fd_handler)
{
   Ecore_Con_Client   *cl;
   
   cl = data;
   if (cl->dead) return 1;
   if (ecore_main_fd_handler_active_get(fd_handler, ECORE_FD_READ))
     {
	unsigned char *inbuf = NULL;
	int            inbuf_num = 0;
	
	for (;;)
	  {
	     char buf[65536];
	     int num;
	     
	     errno = 0;
	     num = read(cl->fd, buf, 65536);
	     if (num < 1)
	       {
		  if (inbuf) 
		    {
		       Ecore_Con_Event_Client_Data *e;
		       
		       e = calloc(1, sizeof(Ecore_Con_Event_Client_Data));
		       if (e)
			 {
			    e->client = cl;
			    e->data = inbuf;
			    e->size = inbuf_num;
			    ecore_event_add(ECORE_CON_EVENT_CLIENT_DATA, e,
					    _ecore_con_event_client_data_free, NULL);
			 }
		    }
		  if ((errno == EIO) ||  (errno == EBADF) || 
		      (errno == EPIPE) || (errno == EINVAL) || 
		      (errno == ENOSPC) || (num == 0)/* is num == 0 right? */)
		    {
		       /* we lost our client! */
		       Ecore_Con_Event_Client_Del *e;
		       
		       e = calloc(1, sizeof(Ecore_Con_Event_Client_Del));
		       if (e)
			 {
			    e->client = cl;
			    ecore_event_add(ECORE_CON_EVENT_CLIENT_DEL, e,
					    _ecore_con_event_client_del_free, NULL);
			 }
		       cl->dead = 1;
		       ecore_main_fd_handler_del(cl->fd_handler);
		       cl->fd_handler = NULL;
		    }
		  break;
	       }
	     else
	       {
		  inbuf = realloc(inbuf, inbuf_num + num);
		  memcpy(inbuf + inbuf_num, buf, num);
		  inbuf_num += num;
	       }
	  }
     }
   else if (ecore_main_fd_handler_active_get(fd_handler, ECORE_FD_WRITE))
     _ecore_con_client_flush(cl);
   return 1;
}

static void
_ecore_con_server_flush(Ecore_Con_Server *svr)
{
   int count, num;

   if (!svr->buf) return;
   num = svr->buf_size - svr->buf_offset;
   count = write(svr->fd, svr->buf + svr->buf_offset, num);
   if (count < 1)
     {
	if ((errno == EIO) || (errno == EBADF) || (errno == EPIPE) ||
	    (errno == EINVAL) || (errno == ENOSPC))
	  {
	     /* we lost our server! */
	     Ecore_Con_Event_Server_Del *e;
	     
	     e = calloc(1, sizeof(Ecore_Con_Event_Server_Del));
	     if (e)
	       {
		  e->server = svr;
		  ecore_event_add(ECORE_CON_EVENT_SERVER_DEL, e,
				  _ecore_con_event_server_del_free, NULL);
	       }
	     svr->dead = 1;
	     ecore_main_fd_handler_del(svr->fd_handler);
	     svr->fd_handler = NULL;
	  }
	return;
     }
   svr->buf_offset += count;
   if (svr->buf_offset >= svr->buf_size)
     {
	svr->buf_size = 0;
	svr->buf_offset = 0;
	free(svr->buf);
	svr->buf = NULL;
	ecore_main_fd_handler_active_set(svr->fd_handler, ECORE_FD_READ);
     }
}

static void
_ecore_con_client_flush(Ecore_Con_Client *cl)
{
   int count, num;

   if (!cl->buf) return;
   num = cl->buf_size - cl->buf_offset;
   count = write(cl->fd, cl->buf + cl->buf_offset, num);
   if (count < 1)
     {
	if ((errno == EIO) || (errno == EBADF) || (errno == EPIPE) ||
	    (errno == EINVAL) || (errno == ENOSPC))
	  {
	     /* we lost our client! */
	     Ecore_Con_Event_Client_Del *e;
	     
	     e = calloc(1, sizeof(Ecore_Con_Event_Client_Del));
	     if (e)
	       {
		  e->client = cl;
		  ecore_event_add(ECORE_CON_EVENT_CLIENT_DEL, e,
				  _ecore_con_event_client_del_free, NULL);
	       }
	     cl->dead = 1;
	     ecore_main_fd_handler_del(cl->fd_handler);
	     cl->fd_handler = NULL;
	  }
	return;
     }
   cl->buf_offset += count;
   if (cl->buf_offset >= cl->buf_size)
     {
	cl->buf_size = 0;
	cl->buf_offset = 0;
	free(cl->buf);
	cl->buf = NULL;
	ecore_main_fd_handler_active_set(cl->fd_handler, ECORE_FD_READ);
     }
}

static void
_ecore_con_event_client_add_free(void *data, void *ev)
{
   Ecore_Con_Event_Client_Add *e;
   
   e = ev;
   free(e);
}

static void
_ecore_con_event_client_del_free(void *data, void *ev)
{
   Ecore_Con_Event_Client_Del *e;

   e = ev;
   free(e);
}

static void
_ecore_con_event_server_add_free(void *data, void *ev)
{
   Ecore_Con_Event_Server_Add *e;

   e = ev;
   free(e);
}

static void
_ecore_con_event_server_del_free(void *data, void *ev)
{
   Ecore_Con_Event_Server_Del *e;

   e = ev;
   free(e);
}

static void
_ecore_con_event_client_data_free(void *data, void *ev)
{
   Ecore_Con_Event_Client_Data *e;

   e = ev;
   if (e->data) free(e->data);
   free(e);
}

static void
_ecore_con_event_server_data_free(void *data, void *ev)
{
   Ecore_Con_Event_Server_Data *e;

   e = ev;
   if (e->data) free(e->data);
   free(e);
}
