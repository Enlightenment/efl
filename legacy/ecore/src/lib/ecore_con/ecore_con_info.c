/*
 * vim:ts=8:sw=3:sts=8:noexpandtab:cino=>5n-3f0^-2{2
 */
/*
 * getaddrinfo with callback
 *
 * man getaddrinfo
 *
 */
#include "ecore_private.h"
#include "Ecore.h"
#include "ecore_con_private.h"

#include <ctype.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <arpa/nameser.h>
#include <netdb.h>

typedef struct _CB_Data CB_Data;

struct _CB_Data
{
   Ecore_List2 __list_data;
   Ecore_Con_Info_Cb cb_done;
   void *data;
   Ecore_Fd_Handler *fdh;
   pid_t pid;
   Ecore_Event_Handler *handler;
   int fd2;
};


static void _ecore_con_info_readdata(CB_Data *cbdata);
static void _ecore_con_info_slave_free(CB_Data *cbdata);
static int _ecore_con_info_data_handler(void *data, Ecore_Fd_Handler *fd_handler);
static int _ecore_con_info_exit_handler(void *data, int type __UNUSED__, void *event);

static int info_init = 0;
static Ecore_List2 *info_slaves = NULL;

EAPI int
ecore_con_info_init(void)
{
   info_init++;
   return info_init;
}

EAPI int
ecore_con_info_shutdown(void)
{
   info_init--;
   if (info_init == 0)
     {
	while (info_slaves) _ecore_con_info_slave_free((CB_Data *)info_slaves);
     }
   return info_init;
}

EAPI int
ecore_con_info_tcp_connect(Ecore_Con_Server *svr,
			   Ecore_Con_Info_Cb done_cb,
			   void *data)
{
   struct addrinfo hints;

   memset(&hints, 0, sizeof(struct addrinfo));
   hints.ai_family = AF_UNSPEC;
   hints.ai_socktype = SOCK_STREAM;
   hints.ai_flags = AI_CANONNAME;
   hints.ai_protocol = IPPROTO_TCP;
   hints.ai_canonname = NULL;
   hints.ai_next = NULL;
   hints.ai_addr = NULL;

   return ecore_con_info_get(svr, done_cb, data, &hints);
}

EAPI int
ecore_con_info_tcp_listen(Ecore_Con_Server *svr,
			  Ecore_Con_Info_Cb done_cb,
			  void *data)
{
   struct addrinfo hints;

   memset(&hints, 0, sizeof(struct addrinfo));
   hints.ai_family = AF_UNSPEC;
   hints.ai_socktype = SOCK_STREAM;
   hints.ai_flags = AI_PASSIVE;
   hints.ai_protocol = IPPROTO_TCP;
   hints.ai_canonname = NULL;
   hints.ai_next = NULL;
   hints.ai_addr = NULL;

   return ecore_con_info_get(svr, done_cb, data, &hints);
}

EAPI int
ecore_con_info_udp_connect(Ecore_Con_Server *svr,
			   Ecore_Con_Info_Cb done_cb,
			   void *data)
{
   struct addrinfo hints;

   memset(&hints, 0, sizeof(struct addrinfo));
   hints.ai_family = AF_UNSPEC;
   hints.ai_socktype = SOCK_DGRAM;
   hints.ai_flags = AI_CANONNAME;
   hints.ai_protocol = IPPROTO_UDP;
   hints.ai_canonname = NULL;
   hints.ai_next = NULL;
   hints.ai_addr = NULL;

   return ecore_con_info_get(svr, done_cb, data, &hints);
}

EAPI int
ecore_con_info_udp_listen(Ecore_Con_Server *svr,
			  Ecore_Con_Info_Cb done_cb,
			  void *data)
{
   struct addrinfo hints;

   memset(&hints, 0, sizeof(struct addrinfo));
   hints.ai_family = AF_UNSPEC;
   hints.ai_socktype = SOCK_DGRAM;
   hints.ai_flags = AI_PASSIVE;
   hints.ai_protocol = IPPROTO_UDP;
   hints.ai_canonname = NULL;
   hints.ai_next = NULL;
   hints.ai_addr = NULL;

   return ecore_con_info_get(svr, done_cb, data, &hints);
}

EAPI int
ecore_con_info_mcast_listen(Ecore_Con_Server *svr,
			   Ecore_Con_Info_Cb done_cb,
			   void *data)
{
   struct addrinfo hints;

   memset(&hints, 0, sizeof(struct addrinfo));
   hints.ai_family = AF_UNSPEC;
   hints.ai_socktype = SOCK_DGRAM;
   hints.ai_flags = 0;
   hints.ai_protocol = IPPROTO_UDP;
   hints.ai_canonname = NULL;
   hints.ai_next = NULL;
   hints.ai_addr = NULL;

   return ecore_con_info_get(svr, done_cb, data, &hints);
}

EAPI int
ecore_con_info_get(Ecore_Con_Server *svr,
		   Ecore_Con_Info_Cb done_cb,
		   void *data,
		   struct addrinfo *hints)
{
   CB_Data *cbdata;
   int fd[2];

   if (pipe(fd) < 0) return 0;
   cbdata = calloc(1, sizeof(CB_Data));
   if (!cbdata)
     {
	close(fd[0]);
	close(fd[1]);
	return 0;
     }
   cbdata->cb_done = done_cb;
   cbdata->data = data;
   cbdata->fd2 = fd[1];
   if (!(cbdata->fdh = ecore_main_fd_handler_add(fd[0], ECORE_FD_READ,
						 _ecore_con_info_data_handler,
						 cbdata,
						 NULL, NULL)))
     {
	free(cbdata);
	close(fd[0]);
	close(fd[1]);
	return 0;
     }

   if ((cbdata->pid = fork()) == 0)
     {
        Ecore_Con_Info container;
	struct addrinfo *result;
	char service[NI_MAXSERV];
	char hbuf[NI_MAXHOST];
	char sbuf[NI_MAXSERV];

	/* FIXME with EINA */
	snprintf(service, NI_MAXSERV, "%i", svr->port);
	/* CHILD */
	if (!getaddrinfo(svr->name, service, &hints, &result) && result)
	  {
	    memcpy(&container.info, result, sizeof(struct addrinfo));
	    container.info.ai_canonname = NULL;
	    container.info.ai_next = NULL;
	    memcpy(&container.addr, result->ai_addr, sizeof(struct sockaddr));
	    memset(container.ip, 0, sizeof(container.ip));
	    memset(container.service, 0, sizeof(container.service));
	    if (!getnameinfo(&container.addr, container.info.ai_addrlen,
			     hbuf, sizeof(hbuf), sbuf, sizeof(sbuf),
			     NI_NUMERICHOST | NI_NUMERICSERV))
	      {
		memcpy(container.ip, hbuf, sizeof(container.ip));
		memcpy(container.service, sbuf, sizeof(container.service));
	      }
	    write(fd[1], &container, sizeof(Ecore_Con_Info));
	  }
	else
	  write(fd[1], "", 1);

	close(fd[1]);
# ifdef __USE_ISOC99
	_Exit(0);
# else
	_exit(0);
# endif
     }
   /* PARENT */
   cbdata->handler = ecore_event_handler_add(ECORE_EXE_EVENT_DEL, _ecore_con_info_exit_handler, cbdata);
   close(fd[1]);
   if (!cbdata->handler)
     {
	ecore_main_fd_handler_del(cbdata->fdh);
	free(cbdata);
	close(fd[0]);
	return 0;
     }
   info_slaves = _ecore_list2_append(info_slaves, cbdata);
   return 1;
}

static void
_ecore_con_info_readdata(CB_Data *cbdata)
{
   Ecore_Con_Info container;
   ssize_t size;

   size = read(ecore_main_fd_handler_fd_get(cbdata->fdh), &container,
	       sizeof(Ecore_Con_Info));
   if (size == sizeof(Ecore_Con_Info))
     {
        container.info.ai_addr = &container.addr;
	cbdata->cb_done(cbdata->data, &container);
     }
   else
     cbdata->cb_done(cbdata->data, NULL);
   cbdata->cb_done = NULL;
}

static void
_ecore_con_info_slave_free(CB_Data *cbdata)
{
   info_slaves = _ecore_list2_remove(info_slaves, cbdata);
   close(ecore_main_fd_handler_fd_get(cbdata->fdh));
   ecore_main_fd_handler_del(cbdata->fdh);
   ecore_event_handler_del(cbdata->handler);
   free(cbdata);
}

static int
_ecore_con_info_data_handler(void *data, Ecore_Fd_Handler *fd_handler)
{
   CB_Data *cbdata;

   cbdata = data;
   if (cbdata->cb_done)
     {
	if (ecore_main_fd_handler_active_get(fd_handler, ECORE_FD_READ))
	  _ecore_con_info_readdata(cbdata);
	else
	  {
	     cbdata->cb_done(cbdata->data, NULL);
	     cbdata->cb_done = NULL;
	  }
     }
   _ecore_con_info_slave_free(cbdata);
   return 0;
}

static int
_ecore_con_info_exit_handler(void *data, int type __UNUSED__, void *event)
{
   CB_Data *cbdata;
   Ecore_Exe_Event_Del *ev;

   ev = event;
   cbdata = data;
   if (cbdata->pid != ev->pid) return 1;
   return 0;
   _ecore_con_info_slave_free(cbdata);
   return 0;
}
